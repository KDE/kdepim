#ifndef KSYNC_ERROR_H
#define KSYNC_ERROR_H

#include <qstring.h>

#include "notify.h"

namespace KSync {
    /**
     * Errors
     */
    class Error : public Notify {
    public:
        enum ErrorCodes{
            ConnectionLost, WrongPassword,  Authentication, WrongUser,
            WrongIP, CouldNotConnect, DownloadError, UploadError, UserDefined
        };
        Error( int number,  const QString& text );
        Error( const QString& text = QString::null);

        bool operator==( const Error& rhs);

    private:
        class Private;
        Private* d;

    };
}

#endif
