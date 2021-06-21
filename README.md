
## A Flask-inspired C++ header file to create in few line a web server for your website or a beautiful desktop GUI.

### You can handle multiples url, and even pass variables values into the url to use it into a callback funtion


### Example:

```C++
#include "webgui.h"
#include <string>
#include <iostream>
using namespace std;

std::string f1(std::string headers, std::string method, std::string url, std::vector<std::string> file_data){

    pinfo("callback of : "+url);

    std::string response;

    if(method == "GET"){
        std::string html_doc = "test2.html";

        response = render_template(html_doc);


    }
    
    if(method == "POST"){
        //A file is posted, get data and write it

        pinfo("file name : "+file_data[0]);
        pinfo("file content : \n"+file_data[1]);

        save_uploaded_file("",file_data[0],file_data[1]);


        std::string html_doc = "test2.html";

       response = render_template(html_doc);

       
    }
    

    return response;
}


std::string f2(std::string headers, std::string method, std::string url, std::vector<std::string> file_data){
    
    
    pinfo("callback of : "+url);

    std::string response;

    if((url == "/api/test_api_call" != std::string::npos) && (method == "GET")){
        response = "{ 'test' : '1' }";
    }else{
        response = "{ 'error' : 'wrong api call' }";
    }


    return response;
}




int main(void){ 
    
    
    //number of authorized URL
    const int n_path = 2;
    
    //list of authorized URL
    std::vector<std::string> auth_url = {"/"};
    
   


    /*defining functions arrays for callback 
    (keeps the same order as the map of url)
    */
    std::string (*p[n_path])(std::string headers, std::string method, std::string url, std::vector<std::string> file_data);

    p[0] = f1;


    /*Path to your templates, 
    leave blank if they are at the same root as your file
    please put a "/" at the end if you specify a path
    */
    std::string templates_path = "";
    
    //choose a port for your web server
    int port=80;
    
    
    init_server(auth_url,n_path,templates_path,port,p);


    return 0;
}

```

### If you are compiling under windows, please add "-lwsock32" to your g++ command to avoid errors.
