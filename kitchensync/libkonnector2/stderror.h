#ifndef KSYNC_STDERROR_H
#define KSYNC_STDERROR_H

#include "error.h"

namespace KSync {

struct StdError
{
    static Error connectionLost();
    static Error wrongPassword();
    static Error authenticationError();
    static Error wrongUser( const QString &user = QString::null );
    static Error wrongIP();
    static Error couldNotConnect();
    static Error downloadError( const QString & );
    static Error uploadError( const QString & );
    static Error konnectorDoesNotExist();
    static Error backupNotSupported();
    static Error restoreNotSupported();
    static Error downloadNotSupported();
};

}
#endif
