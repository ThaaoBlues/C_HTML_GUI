# C_HTML_GUI
## A Flask-inspired C++ header file to create in few line a web server for your website or a beautiful desktop GUI.

### You can handle multiples url, and even pass variables values into the url to use it into a callback funtion


### Example:

```C++
#include <iostream>
#include "webgui.h"

using namespace std;

int f1(std::string request){

    std::cout<<"callback of url /"<<std::endl;
    return 0;
}


int f2(std::string request){

    std::cout<<"callback of url /process"<<std::endl;

    std::cout<<"Request made by the user : \n"<<request<<std::endl;

    return 0;
}


int main(void){
    
    /*
    list of authorized URL and their templates
    You can add /* at the end of an url to enable
    variables-in-url feature
    */
    std::map<std::string, std::string> templates = {
        { "/", "index.html" },
        { "/process/*", "process.html" },
    };
    
    //number of authorized URL
    int n_path = 2;


    /*defining functions arrays for callback 
    (keeps the same order as the map of url)
    */
    int (*p[n_path])(std::string url);

    p[0] = f1;
    p[1] = f2;


    /*Path to your templates, 
    leave blank if they are at the same root as your file
    please put a "/" at the end if you specify a path
    */
    std::string templates_path = "";
    
    //choose a port for your web server
    int port=8000;
    
    
    init_server(templates,n_path,templates_path,port,p);

    return 0;
}

```

### If you are compiling under windows, please add "-lwsock32" to your g++ command to avoid errors.
