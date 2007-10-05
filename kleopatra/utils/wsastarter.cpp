#include "wsastarter.h"

using namespace Kleo;

#ifdef _WIN32
# include <winsock2.h>
#else
 typedef int WSADATA;
 static inline int WSAStartup( int, int * ) { return 0; }
 static inline void WSACleanup() {}
#endif

static int startWSA() {
    WSADATA dummy;
    return WSAStartup( 0x202, &dummy );
}

WSAStarter::WSAStarter()
    : startupError( startWSA() )
{

}

WSAStarter::~WSAStarter() {
    if ( !startupError )
        WSACleanup();
}
