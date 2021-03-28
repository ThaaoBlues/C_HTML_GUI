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

#define HTTP_OK_RESPONSE "HTTP/1.1 200 OK\nServer: Blue-Webserv v1\nContent-Type: text/html\n\n"
#define HTTP_NOT_FOUND_RESPONSE "HTTP/1.1 404 NOT FOUND\nServer: Blue-Webserv v1\nContent-Type: text/html\n\n"
#define TEST_RESPONSE "<html><body><h1>Hello, World!</h1></body></html>\r\n\r\n"
#define HTML_NOT_FOUND_PAGE "<html><body><h1>There is nothing to see here , Sorry !</h1></body></html>\r\n\r\n"

using namespace std;
 

void pinfo(std::string info){

    std::cout<<"[+] "<<info<<std::endl;
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


int handle_client(SOCKET csock,std::map<std::string, std::string> &templates,int n_path,std::string templates_path,int (*p[10000])(std::string url)){

    char buffer[1024];
    int result;
    std::string it_url;

    recv(csock, buffer, sizeof(buffer), 0);

    std::string asked_uri(buffer,sizeof(buffer));

    std::cout<<"==========================================\n"<<asked_uri<<"\n\n\nPROCESSING :\n";


    if(asked_uri.find("/favicon.ico")!= std::string::npos){
        pinfo("SENDING WEBSITE ICON");

        return 0;
    }


    //enumerating authorized uri


    for(int i=0;i<n_path;i++){
        
        map<std::string, std::string>::iterator it;

        for (it = templates.begin(); it != templates.end(); it++)
        {

            it_url = it->first;


            if(it_url.back() == '*'){


                it_url.erase(it_url.end()-1);


                if(asked_uri.find(' '+it_url) != std::string::npos){
                    pinfo(std::string("matching url : ")+it_url);

                    pinfo("calling callback");


                    if(it->first.back() == '*'){
                        result = (*p[i+1]) (asked_uri);
                    }else{
                        result = (*p[i+1]) (asked_uri);
                    }

                    //Warn of an error in callback
                    if(result != 0){
                        std::cout<<"[!] Error in callback function. \n URL:"<<it_url;
                    }
                    
                    

                    pinfo(std::string("reading template file : ")+it->second);


                    std::string file_ctt = readfile(templates_path,it->second);
                    const char* response = file_ctt.c_str();
                    
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


                    if(it->first.back() == '*'){
                        result = (*p[i]) (asked_uri);
                    }else{
                        result = (*p[i]) (asked_uri);
                    }

                    //Warn of an error in callback
                    if(result != 0){
                        std::cout<<"[!] Error in callback function. \n URL:"<<it_url;
                    }
                    
                    

                    pinfo(std::string("reading template file : ")+it->second);


                    std::string file_ctt = readfile(templates_path,it->second);
                    const char* response = file_ctt.c_str();
                    
                    //send OK response
                    send(csock,HTTP_OK_RESPONSE, sizeof(HTTP_OK_RESPONSE),0);



                    //send response content
                    send(csock, response, strlen(response), 0);
                    pinfo("sent response.");

                    return 0;

                }


            }

            
            
        }    
            
        
    }

    //went here so the response will be 404 not found
    pinfo("sending 404 response");
    send(csock,HTTP_NOT_FOUND_RESPONSE, sizeof(HTTP_NOT_FOUND_RESPONSE),0);
    send(csock,HTML_NOT_FOUND_PAGE,sizeof(HTML_NOT_FOUND_PAGE),0);
    return 0;

}
 
 
int init_server(std::map<std::string, std::string> &templates,int n_path,std::string templates_path,int port,int (*p[10000])(std::string url))
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
                        handle_client(csock,templates,n_path,templates_path,p);

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

