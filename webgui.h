#include <iostream>
#include <string>
#if defined (WIN32)
    #include <winsock2.h>
    typedef int socklen_t;
    #pragma comment(lib, "ws2_32")
#elif defined (linux)
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket(s) close(s)
    typedef int SOCKET;
    typedef struct sockaddr_in SOCKADDR_IN;
    typedef struct sockaddr SOCKADDR;
#endif
 
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <fstream>
#include <streambuf>
#include <sstream>
#include <cstring>
#include <vector>

#define HTTP_OK_RESPONSE "HTTP/1.1 200 OK\nServer: Blue-Webserv v1\nContent-Type: text/html\n\n"
#define HTTP_NOT_FOUND_RESPONSE "HTTP/1.1 404 NOT FOUND\nServer: Blue-Webserv v1\nContent-Type: text/html\n\n"
#define TEST_RESPONSE "<html><body><h1>Hello, World!</h1></body></html>\r\n\r\n"
#define HTML_NOT_FOUND_PAGE "<html><body><h1>There is nothing to see here , Sorry !</h1></body></html>\r\n\r\n"

using namespace std;



std::string render_template(std::string &template_path){

    std::cout<<"reading template file : "<<template_path<<endl;
    std::string line;
    std::string ctt;
    ifstream myfile (template_path);
    if (myfile.is_open())
    {
        while ( getline (myfile,line) )
        {
            ctt = ctt + line;
        }
        myfile.close();
    }
    return ctt;

}




std::string readfile(std::string templates_path,std::string filename){

    std::string line;
    std::string ctt;
    ifstream myfile (templates_path+filename);
    if (myfile.is_open())
    {
        while ( getline (myfile,line) )
        {
            ctt = ctt + line;
        }
        myfile.close();
    }

    return ctt;
}


void pinfo(std::string info){

    std::cout<<"[+] "<<info<<std::endl;
}


void save_uploaded_file(std::string file_path,std::string &file_name, std::string &file_data){
    std::string full_path(file_path+file_name);
    ofstream myfile (full_path,ios::binary);
    if (myfile.is_open())
    {
        myfile << file_data;
        myfile.close();
    }else{
        pinfo("unable to open : "+full_path);
    }
    
}

std::vector<std::string> split_string_by_newline(const std::string& str)
{
    auto result = std::vector<std::string>{};
    auto ss = std::stringstream{str};

    for (std::string line; std::getline(ss, line, '\n');)
        result.push_back(line);

    return result;
}

std::string get_request_method(const std::string& str){

    std::string result = "";
    auto ss = std::stringstream{str};

    std::getline(ss,result,' ');

    return result;
}


std::string get_multipart_file_name(const std::string& str){


    auto vector = std::vector<std::string>{};
    auto ss = std::stringstream{str};


    std::string result;

    for (std::string line; std::getline(ss, line, ' ');){
        vector.push_back(line);

    }

    
    for(int i = 0; i <= vector.size();i++){
        if(vector[i].find("filename") != std::string::npos){
            result = vector[i].erase(0,vector[i].find("=\""));
            result = result.substr(0,result.length()-2);
            result = result.substr(2,result.length());
            break;
        }
    }
     
    return result;

}


std::string get_multipart_file_boundary(const std::string& headers){


    auto vector = std::vector<std::string>{};
    auto ss = std::stringstream{headers};


    for (std::string line; std::getline(ss, line, ' ');){
        vector.push_back(line);

    }

    std::string boundary;

    for(int i = 0; i <= vector.size();i++){

        if(vector[i].find("boundary") != std::string::npos){
            boundary = vector[i].erase(0,vector[i].find("="));
            boundary = boundary.substr(1,boundary.find("\n")-1);
            break;
        }
    }

    return boundary;

}

std::string get_request_headers(const std::string& str)
{   

    std::string result;
    auto ss = std::stringstream{str};
    std::string line = "NONE";

    while(line != "\n"){
        std::getline(ss, line, '\r');
        result = result + line;
    }  
    //remove the method and url from headers string 
    result.erase(0,result.find("\n")+1);

    return result;
}


std::string remove_trash_after_boundary(const std::string& str,std::string boundary){
    std::string result;
    auto ss = std::stringstream{str};
    std::string line = "NONE";

    while(line.find(boundary) == std::string::npos){
        std::getline(ss, line, '\n');
        result = result + line + "\n";
    }
    result = result.substr(0,result.length()-(boundary.length()+4));
    
    return result;
}


std::string remove_one_request_file_headers(const std::string& str, const std::string& boundary){
    std::string result;
    auto ss = std::stringstream{str};
    std::string line = "NONE";

    while(line.find(boundary+"--") == std::string::npos){
        std::getline(ss, line, '\n');
        result = result + line;
    }

    result = result.substr(0,result.length()-line.length());


    //remove the method and url from headers string 

    return result;
}

int handle_client(SOCKET csock,std::vector<std::string> &auth_url,int n_path,std::string templates_path,std::string (*p[10000])(std::string headers, std::string method, std::string url, std::vector<std::string> file_data)){

    char buffer[1024];
    std::string sResponse;
    std::string it_url;

    recv(csock, buffer, sizeof(buffer), 0);

    std::string request(buffer,sizeof(buffer));
    std::string request_to_split = request;

    //DIFFERENT PARTS OF THE REQUEST
    //METHOD+URL
    std::string asked_uri(split_string_by_newline(request_to_split)[0]);
    //METHOD
    std::string method = get_request_method(asked_uri);
    //HEADERS
    std::string headers(get_request_headers(request));
    


    std::cout<<"==========================================\n"<<asked_uri<<"\n"<<headers<<"\n\n\nPROCESSING :\n";


    if(asked_uri.find("/favicon.ico")!= std::string::npos){
        pinfo("SENDING WEBSITE ICON");

        return 0;
    }


    //handle files upload
    std::string tmp_data;
    std::string file_data;
    std::string file_info_line = "None";
    std::string filename;
    if(headers.find("multipart")!= std::string::npos){
        pinfo("MULTIPART DATA");
        recv(csock, buffer, sizeof(buffer), 0);
        request = string(buffer);
        request_to_split = request;
        std::string mutlipart_headers = get_request_headers(request);

        //erase request and keep only bytes of file data
        tmp_data = request.erase(0,request.find(mutlipart_headers)+1);
        tmp_data.erase(0,tmp_data.find("\n")+1);

        //get the file header
        std::stringstream f(tmp_data);
        std::getline(f,file_info_line,'\n');


        filename = get_multipart_file_name(file_info_line);

        pinfo("file name : " + filename);

        
        //erase the remaining header from file data
        tmp_data.erase(0,tmp_data.find("\n")+1);
        tmp_data.erase(0,tmp_data.find("\n")+1);

        file_data = file_data + tmp_data;


        std::string boundary = get_multipart_file_boundary(headers);

        pinfo("multipart boundary : "+ boundary);

        //if the file is not sent in one request
        if(request.find(boundary+"--") == std::string::npos){
            
            //wait the end of file
            while(true){
                recv(csock, buffer, sizeof(buffer), 0);
                request = string(buffer);
                file_data = file_data + request;

                //exit the loop if the boundary is hit
                if(request.find(boundary+"--") != std::string::npos){
                    break;
                }
            }
            //remove the duplicate data and some trash resent after boundary
            file_data = remove_trash_after_boundary(file_data,boundary+"--");
        }else{
            //erase the headers after the data that are sent with the one-request file
            file_data = remove_one_request_file_headers(file_data,boundary);
        }

    }
    


    
    

    //enumerating authorized uri

        for (int i=0; i < auth_url.size(); i++)
        {

            it_url = auth_url[i];

            if(it_url.back() == '*'){


                it_url.erase(it_url.end()-1);


                if(asked_uri.find(' '+it_url) != std::string::npos){
                    pinfo(std::string("matching url : ")+it_url+std::string("*"));

                    pinfo("calling callback");


                    std::vector<std::string> file_data_v = {filename,file_data};
                    sResponse = (*p[i]) (headers, method ,asked_uri, file_data_v);


                    pinfo("end of callback");
                    pinfo("sending response");
                    

                    const char* response = sResponse.c_str();
                    
                    //send OK response
                    send(csock,HTTP_OK_RESPONSE, sizeof(HTTP_OK_RESPONSE),0);



                    //send response content
                    send(csock, response, strlen(response), 0);
                    pinfo("sent response.");

                    return 0;

                }

            }else{
                if(asked_uri.find(" "+it_url+" ") != std::string::npos){


                    pinfo(std::string("matching url : ")+it_url);

                    pinfo("calling callback");


                    std::vector<std::string> file_data_v = {filename,file_data};
                    sResponse = (*p[i]) (headers, method ,asked_uri, file_data_v);



                    const char* response = sResponse.c_str();
                    
                    //send OK response
                    send(csock,HTTP_OK_RESPONSE, sizeof(HTTP_OK_RESPONSE),0);



                    //send response content
                    send(csock, response, strlen(response), 0);
                    pinfo("sent response.");

                    return 0;

                }


            }

            
            
        }    
            

    //went here so the response will be 404 not found
    pinfo("sending 404 response");
    send(csock,HTTP_NOT_FOUND_RESPONSE, sizeof(HTTP_NOT_FOUND_RESPONSE),0);
    send(csock,HTML_NOT_FOUND_PAGE,sizeof(HTML_NOT_FOUND_PAGE),0);
    return 0;

}
 
 
int init_server(std::vector<std::string> &auth_url,int n_path,std::string templates_path,int port,std::string (*p[10000])(std::string headers, std::string method, std::string url, std::vector<std::string> file_data))
{
    #if defined (WIN32)
        WSADATA WSAData;
        int erreur = WSAStartup(MAKEWORD(2,2), &WSAData);
    #else
        int erreur = 0;
    #endif

    

    /* Socket et contexte d'adressage du serveur */
    SOCKADDR_IN sin;
    SOCKET sock;
    socklen_t recsize = sizeof(sin);
    
    /* Socket et contexte d'adressage du client */
    SOCKADDR_IN csin;
    SOCKET csock;
    socklen_t crecsize = sizeof(csin);
    
    int sock_err;
    
    
    if(!erreur)
    {
        /* Création d'une socket */
        sock = socket(AF_INET, SOCK_STREAM, 0);
        
        /* Si la socket est valide */
        if(sock != INVALID_SOCKET)
        {
            cout<<"The socket is now open"<<endl;
            
            /* Configuration */
            sin.sin_addr.s_addr = htonl(INADDR_ANY);  /* Adresse IP automatique */
            sin.sin_family = AF_INET;                 /* Protocole familial (IP) */
            sin.sin_port = htons(port);               /* Listage du port */
            sock_err = bind(sock, (SOCKADDR*)&sin, recsize);
            
            /* Si la socket fonctionne */
            if(sock_err != SOCKET_ERROR)
            {
                /* Démarrage du listage (mode server) */
                sock_err = listen(sock, 5);
                cout<<"Listening on port : "<<port<<endl;
                

                /* accepte à l'infini*/
                while(true){
                    /* Si la socket fonctionne */
                    if(sock_err != SOCKET_ERROR)
                    {
                        /* Attente pendant laquelle le client se connecte */
                        csock = accept(sock, (SOCKADDR*)&csin, &crecsize);

                        //handling client request
                        handle_client(csock,auth_url,n_path,templates_path,p);

                        cout<<"closing client socket\n=========================================="<<endl;
                        closesocket(csock);
                    }
                    else{
                        perror("listen");
                    }
                        

                }
                
            }
            else
                perror("bind");
            
            /* Fermeture de la socket client et de la socket serveur */
            cout<<"Closing client socket"<<endl;
            closesocket(csock);
            cout<<"Closing server socket"<<endl;
            closesocket(sock);

        }
        else
            perror("socket");
        
        #if defined (WIN32)
            WSACleanup();
        #endif
    }
    
    return EXIT_SUCCESS;
}