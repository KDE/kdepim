#ifndef KSYNC_STD_ERROR_H
#define KSYNC_STD_ERROR_H

#include "error.h"

namespace KSync {
    struct StdError{
        static Error connectionLost();
        static Error wrongPassword();
        static Error authenticationError();
        static Error wrongUser(const QString& user = QString::null );
        static Error wrongIP();
        static Error couldNotConnect();
        static Error downloadError(const QString& );
        static Error uploadError( const QString& );
        static Error konnectorDoesNotExist(const QString& udi );
        static Error backupNotSupported();
        static Error restoreNotSupported();
        static Error downloadNotSupported();
    };
}
#endif
