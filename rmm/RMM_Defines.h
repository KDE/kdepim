#ifndef RMM_DEFINES_H
#define RMM_DEFINES_H

#if (!defined NDEBUG) && (defined __GNUG__)

#   include <stdio.h>
#   include <iostream.h>
#   include <qcstring.h>
   
#   define rmmDebug(a) \
        fprintf(stderr, "%s, line %d\n", __PRETTY_FUNCTION__, __LINE__); \
        cerr << QCString(a) << endl;

#else
        
#       define rmmDebug(a)

#endif


#endif // Included this file
// vim:ts=4:sw=4:tw=78
