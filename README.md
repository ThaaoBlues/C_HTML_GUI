# C_HTML_GUI
## A Flask-inspired C++ header file to create in few line a web server for your website or a beautiful desktop GUI.


### Example:

```C++

#include <iostream>
#include "webgui.h"

using namespace std;

int main(void){


    //list of authorized URL and their templates
    std::map<std::string, std::string> templates = {
        { "/", "test.html" },
        { "/process", "process.html" },
    };
    
    //number of authorized URL
    int n_path = 2;


    /*Path to your templates, 
    leave blank if they are at the same root as your file
    please put a "/" at the end if you specify a path
    */
    std::string templates_path = "";
    
    //choose a port for your web server
    int port=23456;
    
    
    init_server(templates,n_path,templates_path,port);
    return 0;
}


```

### If you are compiling under windows, please add "-lwsock32" to your g++ command to avoid errors.
