#ifndef VR3_SOCKET_H
#define VR3_SOCKET_H

#include <qobject.h>

#include <syncee.h>
#include <synceelist.h>

namespace KSync {
    class AgendaSocket : public QObject {
        Q_OBJECT
    public:
        AgendaSocket( QObject* obj );
        ~AgendaSocket();

        /** set the ip */
        void setIP( const QString& ip );

        /** set the meta name */
        void setMetaName( const QString& name );
        QString metaName()const;

        void startUP();
        void hangUP();

        /** are we currently connected */
        bool isConnected()const;

        /** start to sync  now */
        void startSync();

        void write( SynceeList );
        /* signals for the bridge */
    signals:
        void sync(SynceeList);
        void error(const Error& );
        void prog( const Progress& );

    private:
        bool m_isConnected :1 ;
        QString m_ip;
        QString m_meta;
    };
}


#endif
