#ifndef RMM_TOKEN_H
#define RMM_TOKEN_H

#include <qstrlist.h>

namespace RMM {

    Q_UINT32 RTokenise(
        const char * str,
        const char * delim,
        QStrList & l,
        bool skipComments = true,
        bool quotedTokens = true);
}

#endif

// vim:ts=4:sw=4:tw=78
