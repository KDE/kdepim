#ifndef QTOPIA_OPIE_SOCKET_H
#define QTOPIA_OPIE_SOCKET_H

#include <qobject.h>

#include <konnector.h>

class KURL;
namespace KSync {

    class AddressBookSyncee;
    class EventSyncee;
    class TodoSyncee;
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
        void slotError(int);
        void slotConnected();
        void slotClosed();
        void slotNOOP();
        void process();
        void slotStartSync();

    private:
        class Private;
        Private *d;

    private:
        enum Type {
            AddressBook,
            TodoList,
            DateBook
        };
        KURL url( Type );
        KURL url( const QString& path );
        void writeCategory();
        void writeAddressbook( AddressBookSyncee*);
        void writeDatebook( EventSyncee* );
        void writeTodoList( TodoSyncee* );
        Syncee* readAddressbook();
        Syncee* readDatebook();
        Syncee* readTodoList();
        void readPartner();

        /* for processing the connection and authentication */
        void start(const QString& );
        void user( const QString& );
        void pass( const QString& );
        void call( const QString& );
        void noop( const QString& );


    };
};


#endif
