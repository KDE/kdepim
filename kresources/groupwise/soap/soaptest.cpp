#include "groupwiseserver.h"

#include <iostream>

using namespace std;

int main()
{
#if 0
    std::string user = "demo1";
    std::string pass = "demo1";
    const char *url = "192.108.102.234";
#else 
    std::string user = "user1";
    std::string pass = "user1";
    const char *url = "192.108.102.237";
#endif

    GroupwiseServer server( url, user.c_str(), pass.c_str() );

//  char* url = "";

    if ( !server.login() ) {
      cerr << "Unable to login to server " << url << endl;
    }

#if 0
    server.dumpData();
#endif

#if 0
    server.getCategoryList();
#endif

#if 1
    server.dumpFolderList();
#endif

#if 0    
    server.getDelta();
#endif
    
    server.logout();

    return 0;
}

