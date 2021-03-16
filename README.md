# C_HTML_GUI
## A Flask-inspired C++ header file to create in few line a web server for your website or a beautiful desktop GUI.


### Example:

```C++
#include <iostream>
#include "webgui.h"

using namespace std;

int f1(void){

    cout<<"callback of url /"<<endl;
    return 0;
}
int f2(void){

    cout<<"callback of url /process"<<endl;
    return 0;
}


int main(void){
    
    //list of authorized URL and their templates
    std::map<std::string, std::string> templates = {
        { "/", "test.html" },
        { "/process", "process.html" },
    };
    
    //number of authorized URL
    int n_path = 2;


    /*defining functions arrays for callback 
    (keeps the same order as the map of url)
    */
    int (*p[n_path])();

    p[0] = f1;
    p[1] = f2;


    /*Path to your templates, 
    leave blank if they are at the same root as your file
    please put a "/" at the end if you specify a path
    */
    std::string templates_path = "";
    
    //choose a port for your web server
    int port=80;
    
    
    init_server(templates,n_path,templates_path,port,p);

    return 0;
}

```

### If you are compiling under windows, please add "-lwsock32" to your g++ command to avoid errors.
