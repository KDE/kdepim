#include "groupwiseserver.h"

#include <iostream>

using namespace std;

int main()
{
  // FIXME: Get from commandline
  std::string user;
  std::string pass;
  const char *url = 0;

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

