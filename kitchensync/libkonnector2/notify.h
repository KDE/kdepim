#ifndef KSYNC_NOTIFY_H
#define KSYNC_NOTIFY_H

#include <qstring.h>

namespace KSync {
    /**
     * Notify is the base class for the communication
     * with the users...
     */
    class Notify {
    public:
        Notify( int code,  const QString& text );
        Notify( const QString& );
        virtual ~Notify();

        bool operator==( const Notify& );

        int code()const;
        QString text()const;

    private:
        int m_code;
        QString m_text;
        class Private;
        Private* d;
    };
}

#endif
