
#ifndef opiesocket_h
#define opiesocket_h

#include <qcstring.h>
#include <qhostaddress.h>
#include <qobject.h>


#include <konnector.h>

//#include "categoryedit.h"

namespace KSync{

    class OpieSocket : public QObject
    {
        Q_OBJECT
    public:
        OpieSocket(QObject *obj, const char *name );
        ~OpieSocket();
        void setUser(const QString &user );
        void setPassword(const QString &pass );
        void setSrcIP( const QString & );
        void setDestIP(const QString & );
        void setMeta( bool );
        void startUp();
        bool startSync();
        bool isConnected();
        QByteArray retrFile(const QString &path );
        Syncee* retrEntry( const QString& );
        bool insertFile(const QString &fileName );
        void write(const QString &, const QByteArray & );
        void write(Syncee::PtrList );
        void write(KOperations::ValueList );
        QString metaId()const;

    signals:
        void sync( Syncee::PtrList );
        void errorKonnector(int, QString );
        void stateChanged( bool );

    private:
        void doAddressbook();
        void doCal();
        void writeCategory();
        void newPartner();
        void readPartner( const QString & );
        QString randomString( int len );
        class OpieSocketPrivate;
        OpieSocketPrivate *d;

    private slots:
        void slotError(int );
        void slotConnected( );
        // void slotDis();
        void slotClosed();
        void process(); // ready read
        void slotNOOP();
        void slotStartSync();
        void manageCall(const QString &line );
    };
};

#endif

