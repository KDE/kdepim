#ifndef KSYNC_PROGRESS_H
#define KSYNC_PROGRESS_H

#include <qstring.h>

#include "notify.h"

namespace KSync {
    /* base class for error and progress? -zecke */
    class Progress : public Notify{
    public:
        enum ProgressCodes {
            Connection, Connected, Authenticated,
            Syncing, Downloading, Uploading, Converting,
            Reconverting, Done,  Undefined
        };
        Progress(int code, const QString& text);
        Progress( const QString& text = QString::null );

        bool operator==( const Progress& );

    private:
        class Private;
        Private *d;
    };
}


#endif
