#ifndef QTOPIA_OPIE_SOCKET_H
#define QTOPIA_OPIE_SOCKET_H

#include <qobject.h>

#include <konnector.h>


namespace KSync {

    class QtopiaSocket : public QObject{
        Q_OBJECT
    public:
        QtopiaSocket( QObject* obj, const char* name );
        ~QtopiaSocket();

        void setUser( const QString& user );
        void setPassword( const QString& pass );
        void setSrcIP( const QString& );
        void setDestIP( const QString& );
        void startUp();

        bool startSync();
        bool isConnected();

        QByteArray retrFile( const QString& file );
        Syncee* retrEntry( const QString& );

        bool insertFile( const QString& fileName );
        void write( const QString&, const QByteArray& );
        void write( Syncee::PtrList );
        void write( KOperations::ValueList );
        QString metaId()const;

    signals:
        void sync( Syncee::PtrList );
        void errorKonnector(int, QString );
        void stateChanged( bool );
	
   private slots:


    };
};


#endif
