#include <qhostaddress.h>
#include <qtimer.h>
#include <qdom.h>
#include <qdir.h>
#include <qfile.h>

#include <kapplication.h>
#include <koperations.h>
#include <kgenericfactory.h>
#include <qsocket.h>
#include <kdebug.h>
#include <ktempfile.h>
#include <kio/netaccess.h>
#include <kfileitem.h>
#include <kurl.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <qregexp.h>


#include <idhelper.h>

#include "opiesocket.h"
#include "opiecategories.h"
//#include "opiehelper.h"

#include "desktop.h"
#include "categoryedit.h"
#include "datebook.h"
#include "todo.h"
#include "addressbook.h"
#include "metatodo.h"
#include "metaevent.h"
#include "metaaddress.h"


using namespace KSync;

class OpieSocket::OpieSocketPrivate{
public:
    OpieSocketPrivate(){}
    QString pass;
    QString user;
    bool connected:1;
    bool startSync:1;
    bool isSyncing:1;
    bool isConnecting:1;
    bool first:1;
    QString src;
    QString dest;
    QSocket *socket;
    QTimer *timer;
    QString path;
    bool meta:1;
    int mode;
    int getMode;
    enum Call{NOTSTARTED=0, HANDSHAKE=0, ABOOK, TODO, CALENDAR, TRANSACTIONS, FILES, DESKTOPS};
    enum Status {START=0, USER=1, PASS, CALL, NOOP, DONE , CONNECTED};
    Syncee::PtrList m_sync;
    QValueList<OpieCategories> m_categories;
    QString partnerId;
    QStringList files;
    QString tz;
    // helper
    KonnectorUIDHelper *helper;
    OpieHelper::CategoryEdit *edit;
};

namespace {
    void parseTZ( const QString& fileName,  QString &tz );
};


OpieSocket::OpieSocket(QObject *obj, const char *name )
    : QObject( obj, name )
{
    d = new OpieSocketPrivate;
    d->socket = 0;
    d->timer = 0;
    d->connected = false;
    d->startSync = false;
    d->isSyncing = false;
    d->isConnecting = false;
    d->meta = false;
    d->helper = 0;
    d->edit = 0;
    d->first= false;
}
OpieSocket::~OpieSocket()
{
    kdDebug(5202) << "Delete OpieSocket" << endl;
    delete d->edit;
    delete d->helper;
}
void OpieSocket::setUser(const QString &user )
{
    d->user = user;
}
void OpieSocket::setPassword(const QString &pass )
{
    d->pass = pass;
}
void OpieSocket::setSrcIP(const QString &src )
{
    d->src = src;
}

void OpieSocket::setDestIP(const QString &dest )
{
    d->dest = dest;
}
void OpieSocket::setMeta( bool meta ) {
    d->meta = meta;
}
void OpieSocket::startUp() // start the connection
// and find out the the Homedir Path ;)
// start connecting
{
    kdDebug(5202) << "OPieQCOPSocket" << endl;
    delete d->socket;
    d->socket = new QSocket(this, "OpieQCOPSocket" );
    connect( d->socket, SIGNAL(error(int) ), this,
	     SLOT(slotError(int) ) );
    connect( d->socket, SIGNAL(connected() ), this,
	     SLOT(slotConnected() ) );
    connect(d->socket, SIGNAL(connectionClosed() ), this,
	    SLOT(slotClosed() ) );
    connect(d->socket, SIGNAL(readyRead() ), this,
	    SLOT(process() ) );
    //connect(d->timer, SIGNAL(), this,
    //    SLOT(slotNOOP() ) );
    d->connected = false;
    d->startSync = false;
    d->isConnecting = true;
    d->m_categories.clear();
    d->isSyncing = false;
    d->socket->connectToHost(d->dest, 4243 );
}
bool OpieSocket::startSync()
{
    kdDebug(5202) << "startSync " << endl;
    if( d->isSyncing ){
//	kdDebug(5202) << "isSyncing" << endl;
	return false;
    }
    d->isSyncing = true;
    d->getMode = d->NOTSTARTED;
    if(d->isConnecting ){
//	kdDebug(5202) << "is connecting tell it to startSync if in NOOP" << endl;
	d->startSync = true;
	return true;
    }
    if(!isConnected()  ){
//	kdDebug(5202) << "Not connected starting connection" << endl;
	startUp();
	d->startSync = true;
	return true;
    }
//    kdDebug(5202) << "start sync the normal way"  << endl;
    slotStartSync();
    return true;
}
bool OpieSocket::isConnected()
{
    if( d->connected || d->mode == d->CALL || d->mode == d->NOOP || d->mode == d->CONNECTED ){
	return true;
    }else{
	return false;
    }
}
Syncee* OpieSocket::retrEntry( const QString& path ) {
    return 0l;
}
QByteArray OpieSocket::retrFile(const QString &path )
{
    QByteArray array;
    if( isConnected() ) {
        QString file;
        KURL url;
        url.setProtocol("ftp" );
        url.setUser( d->user );
        url.setPass( d->pass );
        url.setHost( d->dest );
        url.setPort( 4242 );
        url.setPath( path );
        if (  KIO::NetAccess::download( url,  file ) ) {
            QFile file2( file );
            if ( file2.open( IO_ReadOnly ) ) {
                array = file2.readAll();
            }
        }
        KIO::NetAccess::removeTempFile( file );
    }
    return array;
}
bool OpieSocket::insertFile( const QString &fileName )
{
  if ( !d->connected )
      return false;

  d->files.append( fileName );
  return true;
}
//if starts with / we got a absolute target else
// relatve to QPEApplication::documentDir()
void OpieSocket::write(const QString &path, const QByteArray &array )
{
    KTempFile temp(locateLocal("tmp", "opie-konn-tmp"),  "konn" );
    QFile* file = temp.file();
    if ( file!=0 ) {
        file->writeBlock( array  );
        temp.close();

        KURL url;
        url.setProtocol("ftp" );
        url.setUser( d->user );
        url.setPass( d->pass );
        url.setHost( d->dest );
        url.setPort( 4242 );
        url.setPath( path );
        KIO::NetAccess::upload(temp.name(), url );
    }
    temp.unlink();
}
// write back to my iPAQ
void OpieSocket::write(Syncee::PtrList lis)
{
    kdDebug() << "WriteBack to Opie lis count =" << lis.count() << endl;
    Syncee* syncee;
    for ( syncee = lis.first(); syncee!= 0; syncee = lis.next() ) {
        if ( syncee->type() == QString::fromLatin1("AddressBookSyncee") ) {
            kdDebug() << "AddressBookSyncee " << endl;
            AddressBookSyncee* abSyncee = dynamic_cast<AddressBookSyncee*>( syncee);
            writeAddressbook( abSyncee );
        }else if ( syncee->type() == QString::fromLatin1("EventSyncee") ) {
            kdDebug() << "EventSyncee " << endl;
            EventSyncee* evSyncee = dynamic_cast<EventSyncee*>(syncee );
            writeDatebook( evSyncee );
        }else if ( syncee->type() == QString::fromLatin1("TodoSyncee") ) {
            kdDebug() << "TodoSyncee" << endl;
            TodoSyncee* tdSyncee = dynamic_cast<TodoSyncee*>(syncee);
            writeTodoList( tdSyncee );
        }// else if UnknownSyncEntry.... upload

    };
    lis.setAutoDelete( true );
    lis.clear();

    writeCategory();
    d->helper->save();
    QTextStream stream( d->socket );
    stream << "call QPE/System stopSync()" << endl;
    d->isSyncing = false; // do it in the write back later on
}
void OpieSocket::writeTodoList( TodoSyncee* to) {
    KURL url;
    url.setProtocol("ftp" );
    url.setUser( d->user );
    url.setPass( d->pass );
    url.setHost( d->dest );
    url.setPort( 4242 );

    OpieHelper::ToDo todoDB(d->edit,  d->helper,  d->tz, d->meta );
    QByteArray array = todoDB.fromKDE( to );

    KTempFile todoFile( locateLocal("tmp",  "opie-todolist"),  "new");
    QFile *file = todoFile.file();
    file->writeBlock( array );
    todoFile.close();

    url.setPath(d->path + "/Applications/todolist/todolist.xml");
    KIO::NetAccess::upload( todoFile.name(),  url );
    todoFile.unlink();

    /**
     * store meta data
     */
    if (d->meta ) {
        QFile file ( QDir::homeDirPath()+ "/.kitchensync/meta/" +
                     d->partnerId + "/todolist.xml" );
        if ( file.open(IO_WriteOnly ) ) {
            file.writeBlock( array );
        }
    }
}
void OpieSocket::writeDatebook( EventSyncee* ev ) {
    KURL url;
    url.setProtocol("ftp" );
    url.setUser( d->user );
    url.setPass( d->pass );
    url.setHost( d->dest );
    url.setPort( 4242 );

    OpieHelper::DateBook dateDB(d->edit,  d->helper,  d->tz, d->meta );
    QByteArray array = dateDB.fromKDE( ev );

    KTempFile File( locateLocal("tmp",  "opie-datebook"),  "new");
    QFile *file = File.file();
    file->writeBlock( array );
    File.close();

    url.setPath(d->path + "/Applications/datebook/datebook.xml");
    KIO::NetAccess::upload( File.name(),  url );
    File.unlink();

    /**
     * store meta data
     */
    if (d->meta ) {
        QFile file ( QDir::homeDirPath()+ "/.kitchensync/meta/" +
                     d->partnerId + "/datebook.xml" );
        if ( file.open(IO_WriteOnly ) ) {
            file.writeBlock( array );
        }
    }
}
void OpieSocket::writeAddressbook( AddressBookSyncee* ab)  {
    KURL url;
    url.setProtocol("ftp" );
    url.setUser( d->user );
    url.setPass( d->pass );
    url.setHost( d->dest );
    url.setPort( 4242 );

    OpieHelper::AddressBook abDB(d->edit,  d->helper,  d->tz, d->meta );
    QByteArray array = abDB.fromKDE( ab );

    KTempFile File( locateLocal("tmp",  "opie-contacts"),  "new");
    QFile *file = File.file();
    file->writeBlock( array );
    File.close();

    url.setPath(d->path + "/Applications/addressbook/addressbook.xml");
    KIO::NetAccess::upload( File.name(),  url );
    File.unlink();

    /**
     * store meta data
     */
    if (d->meta ) {
        QFile file ( QDir::homeDirPath()+ "/.kitchensync/meta/" +
                     d->partnerId + "/addressbook.xml" );
        if ( file.open(IO_WriteOnly ) ) {
            file.writeBlock( array );
        }
    }
}
void OpieSocket::write(KOperations::ValueList )
{
//    kdDebug(5202) << "write KOperations not implemented yet" << endl;
}
void OpieSocket::slotError(int error )
{
    kdDebug(5202) << "error" << endl;
    d->isSyncing = false;
    d->isConnecting = false;

    emit stateChanged( false );
    emit errorKonnector(error, "Connection Fehlschlag");
}
void OpieSocket::slotConnected()
{
    d->connected = true;
    delete d->timer;
    d->mode = d->START;
}
void OpieSocket::slotClosed()
{
    d->connected = false;
    d->isConnecting = false;
    d->isSyncing = false;
    emit stateChanged( false );
}
void OpieSocket::process()
{
    while(d->socket->canReadLine() ){
	QTextStream stream( d->socket );
	QString line = d->socket->readLine();
//	kdDebug(5202) << line.stripWhiteSpace() << endl;
	switch( d->mode ){
	    case d->START: {
		if( line.left(3) != QString::fromLatin1("220") ){
		    // error
		    d->socket->close();
		    d->mode = d->DONE;
		    d->connected = false;
		    d->isConnecting = false;
		}else {
		    stream << "USER " << d->user << "\r\n";
		    d->mode = d->USER;
		}
		break;
	    }
	    case d->USER:{
		if(line.left(3) != QString::fromLatin1("331" ) ){
                // wrong user name
		    d->socket->close();
		    d->mode = d->DONE;
		    d->connected = false;
		    d->isConnecting = false;
		}else{
		    stream << "PASS " << d->pass << "\r\n";
		    d->mode = d->PASS;
		}
		break;
	    }
	    case d->PASS:{
		if(line.left(3) != QString::fromLatin1("230" ) ){
                // error
		    d->socket->close();
		    d->mode = d->DONE;
		    d->connected = false;
		    d->isConnecting = false;
		}else{
                // ok start with the NOOP
                // start the Timer
//		    kdDebug(5202) << "start the NOOP" << endl;
		    d->mode = d->NOOP;
		    d->timer = new QTimer(this );
		    connect(d->timer, SIGNAL(timeout() ), this, SLOT(slotNOOP() ) );
		    d->timer->start(10000 );
                    emit stateChanged( true );
		}
		break;
	    }
	    case d->CALL:
//		kdDebug(5202) << "CALL :)" << endl;
		manageCall( line );
		break;
	    case d->NOOP:
		//stream << "NOOP\r\n";
		d->isConnecting = false;
		if(!d->startSync ){
//		    kdDebug(5202) << "start NOOP" << endl;
		    d->mode = d->NOOP;
		    d->timer = new QTimer(this );
		    connect(d->timer, SIGNAL(timeout() ), this, SLOT(slotNOOP() ) );
		    d->timer->start(10000 );
		} else {
//		    kdDebug (5202) << "slotStartSync now" << endl;
		    slotStartSync();
		}
		break;
	};
    }
}
void OpieSocket::slotNOOP()
{
  QTextStream stream( d->socket );
  stream << "NOOP\r\n";
//  kdDebug(5202) << "NOOP" << endl;
  delete d->timer;
  d->timer = 0;
}

void OpieSocket::slotStartSync()
{
  kdDebug(5202) << "slotStartSync()" << endl;
  delete d->timer;
  kdDebug(5202) << "after deleting" << endl;
  d->startSync = false;
  QTextStream stream( d->socket );
  stream << "call QPE/System sendHandshakeInfo()\r\n";
  d->getMode = d->HANDSHAKE;
  d->mode = d->CALL;
}
void OpieSocket::manageCall(const QString &line )
{
    QTextStream stream( d->socket );
    // if command okay && not handshake or getAllDocLinks( ABOOK ) return
    if( line.contains("200 Command okay" ) &&
        ( d->getMode == d->HANDSHAKE || d->getMode == d->ABOOK ) ) {
	return;
    }
    if( line.startsWith("CALL QPE/Desktop docLinks(QString)" ) ){
	kdDebug(5202 ) << "CALL docLinks desktop entry" << endl;
        // FIXME      OpieHelperClass helper;
//	helper.toOpieDesktopEntry( line, &d->m_sync, d->edit  );
        OpieHelper::Desktop desk( d->edit );
        Syncee* sync = desk.toSyncee( line );
        if ( sync == 0l ) kdDebug() << "--" << endl << "DocLinks null" << endl;
        if (sync )
            d->m_sync.append( desk.toSyncee( line ) );
    }
    switch( d->getMode ){
    case d->HANDSHAKE: {
        QStringList list = QStringList::split(QString::fromLatin1(" "), line );
        kdDebug(5202) << list[3] << endl;
        d->path = list[3];
        d->getMode = d->DESKTOPS;
        stream << "call QPE/System startSync(QString) KitchenSync\r\n";
        break;
    }
    case d->ABOOK:{
        doAddressbook();
        doCal();
        // done with fetching
        // come back to the normal mode
        // emit signal
        emit sync( d->m_sync );
        d->mode = d->NOOP;
        d->m_sync.clear();
        break;
    }
    case d->DESKTOPS:{
        // ok we're now in sync mode let's see if we know this guy
        // if meta mode let's see where the corresponding meta dir is
        // $KDEHOME/share/apps/kitchensync/metadata/konnector-id
        QString tmpFileName;

        downloadFile( "/Settings/Categories.xml", tmpFileName );
        //tmpFileName = QString::fromLatin1("/home/ich/categories.xml")
        kdDebug(5202) << "Fetching categories" << endl;;
        delete d->edit;
        d->edit = new OpieHelper::CategoryEdit( tmpFileName );
        KIO::NetAccess::removeTempFile( tmpFileName );

        delete d->helper;
        if ( d->meta ) {
            QString file;
            if ( !downloadFile("/Settings/meinPartner",  file ) ) {
                newPartner();
                QDir dir;
                dir.mkdir(QDir::homeDirPath() + "/.kitchensync");
                dir.mkdir(QDir::homeDirPath() + "/.kitchensync/meta");
                dir.mkdir(QDir::homeDirPath() + "/.kitchensync/meta/" + d->partnerId );
            }else{
                readPartner(file);
                KIO::NetAccess::removeTempFile( file );
            }
        }else
            d->first = false;

        d->helper = new KonnectorUIDHelper(QDir::homeDirPath()
                                           + "/.kitchensync/meta/"
                                           + d->partnerId);

        if (downloadFile("/Settings/locale.conf",  tmpFileName ) ) {
            parseTZ( tmpFileName,  d->tz );
            KIO::NetAccess::removeTempFile( tmpFileName );
        }else
            d->tz = "America/New_York";

        kdDebug(5202) << "desktops entries" << endl;
        stream << "call QPE/System getAllDocLinks()\r\n";
        d->getMode = d->ABOOK;
        break;
    }
    }
}
// we never synced with him though
// generate a id and store it on the device
void OpieSocket::newPartner()
{
    d->first = true;
    d->partnerId = randomString( 10 );
//    kdDebug(5202) << "New Sync  " << d->partnerId << endl;
    // fixme use KTempFile
    QString fileN = QString::fromLatin1("/tmp/opiekonnector-") + d->partnerId;
    QFile file( fileN );
    if ( file.open( IO_WriteOnly ) ) {
        QTextStream stream(&file );
        stream << "opie-" << d->partnerId << endl;
        file.close();
        KURL url;
        url.setProtocol("ftp" );
        url.setUser( d->user );
        url.setPass( d->pass );
        url.setHost( d->dest );
        url.setPort( 4242 );
        url.setPath(d->path + "/Settings/meinPartner");
        KIO::NetAccess::upload( fileN,  url );
    }
    QFile::remove( fileN );
}
void OpieSocket::readPartner( const QString &fileName )
{
    d->first= false;
    QFile file( fileName );
    if ( file.open(IO_ReadOnly ) ) {
        QTextStream stream ( &file );
        QString string;
        stream >> string;
        if ( string.left( 5 ) == QString::fromLatin1("opie-") )
            d->partnerId = string.mid( 5 );
    }
}
QString OpieSocket::randomString( int length )
{
    if ( kapp )
        return kapp->randomString( length );

    // from KApplication
    QString str;
    while (--length)
    {
        int r=random() % 62;
        r+=48;
        if (r>57) r+=7;
        if (r>90) r+=6;
        str += char(r);
        // so what if I work backwards?
    }
    return str;
}
QString OpieSocket::metaId() const{
    return d->partnerId;
}
Syncee* retrEntry( const QString& )
{
    return 0l;
}
void OpieSocket::writeCategory()
{
    QString fileName = QDir::homeDirPath() + "/.kitchensync/meta/" +d->partnerId;
    QFile file( fileName + "/categories.xml");
    if ( file.open(IO_WriteOnly ) ) {
        QByteArray array = d->edit->file();
        file.writeBlock( array );
        file.close();
        KURL url;
        url.setProtocol("ftp" );
        url.setUser( d->user );
        url.setPass( d->pass );
        url.setHost( d->dest );
        url.setPort( 4242 );
        url.setPath( d->path + "/Settings/Categories.xml" );
        KIO::NetAccess::upload( fileName + "/categories.xml",  url );
    }
}
void OpieSocket::doCal()
{
/*
 * First to todo and then datebook
 * But this applies to both
 * Fetch the file from the OpiePDA (H3870 )
 * use the DB to load it
 * the see if in meta mode and then load the second
 * old file from the local host
 * the use Meta* and then you're done
 * append it
 */
    doTodo();
    doDatebook();
}
void OpieSocket::doTodo() {
    QString tempfile;
    if (!downloadFile( "/Applications/todolist/todolist.xml", tempfile ) )
        return;
    OpieHelper::ToDo todoDB( d->edit, d->helper, d->tz, d->meta );
    KSync::TodoSyncee* synceeNew = todoDB.toKDE( tempfile );
    if (!synceeNew ) {
        kdDebug() <<"No Todo from Opie" << endl;
        KIO::NetAccess::removeTempFile( tempfile );
        return;
    }

    synceeNew->setFirstSync( d->first );

    /**
     * If in meta mode but not the first sync
     * collect some meta infos
     */
    if (d->meta && !d->first ) {
        synceeNew->setSyncMode( KSync::Syncee::MetaMode );
        KSync::TodoSyncee *synceeOld;
        synceeOld = todoDB.toKDE( QDir::homeDirPath() +
                                  "/.kitchensync/meta/"
                                  + d->partnerId
                                  + "/todolist.xml"  );
        if (synceeOld ) {
            OpieHelper::MetaTodo meta;
            synceeNew = meta.doMeta( synceeNew, synceeOld );
        }
    };
    if (synceeNew == 0l )
        kdDebug() << "syncee new == 0l" << endl;
    d->m_sync.append( synceeNew );
    KIO::NetAccess::removeTempFile( tempfile );
};
void OpieSocket::doDatebook() {
    QString tempfile;
    if (!downloadFile( "/Applications/datebook/datebook.xml", tempfile ) )
        return;
    OpieHelper::DateBook dateDB( d->edit, d->helper, d->tz, d->meta );
    KSync::EventSyncee* synceeNew = dateDB.toKDE( tempfile );
    if (!synceeNew ) {
        KIO::NetAccess::removeTempFile( tempfile );
        return;
    }

    synceeNew->setFirstSync( d->first );

    /**
     * If in meta mode but not the first sync
     * collect some meta infos
     */
    if (d->meta && !d->first ) {
        synceeNew->setSyncMode( KSync::Syncee::MetaMode );
        KSync::EventSyncee *synceeOld;
        synceeOld = dateDB.toKDE( QDir::homeDirPath() +
                                  "/.kitchensync/meta/"
                                  + d->partnerId
                                  + "/datebook.xml"  );
        if (synceeOld ) {
            OpieHelper::MetaEvent meta;
            synceeNew = meta.doMeta( synceeNew, synceeOld );
        }
    };

    if (synceeNew == 0l )
        kdDebug() << "Cal Syncee == 0l" << endl;
    d->m_sync.append( synceeNew );
    KIO::NetAccess::removeTempFile( tempfile );
}
void OpieSocket::doAddressbook()
{
    QString tempfile;
    if (!downloadFile( "/Applications/addressbook/addressbook.xml", tempfile ) )
        return;
    OpieHelper::AddressBook abDB( d->edit, d->helper, d->tz, d->meta );
    KSync::AddressBookSyncee* synceeNew = abDB.toKDE( tempfile );
    if (!synceeNew ) {
        KIO::NetAccess::removeTempFile( tempfile );
        return;
    }

    synceeNew->setFirstSync( d->first );

    /**
     * If in meta mode but not the first sync
     * collect some meta infos
     */
    if (d->meta && !d->first ) {
        synceeNew->setSyncMode( KSync::Syncee::MetaMode );
        KSync::AddressBookSyncee *synceeOld;
        synceeOld = abDB.toKDE( QDir::homeDirPath() +
                                  "/.kitchensync/meta/"
                                  + d->partnerId
                                  + "/addressbook.xml"  );
        if (synceeOld ) {
            OpieHelper::MetaAddress meta;
            synceeNew = meta.doMeta( synceeNew, synceeOld );
        }
    };

    if ( synceeNew == 0l )
        kdDebug() << "Adi == 0l " << endl;
    d->m_sync.append( synceeNew );
    KIO::NetAccess::removeTempFile( tempfile );
}

namespace {
    void parseTZ( const QString &fileName,  QString &tz )
    {
        QFile file( fileName );
        if (file.open(IO_ReadOnly ) ) {
            QTextStream stream( &file );
            QString line;
            while ( !stream.atEnd() ) {
                line = stream.readLine();
                if ( line.startsWith("Timezone = ") ) {
                    tz = line.mid(11 );
                }
            }
            if ( tz.isEmpty() )
                tz = "America/New_York";
        }else
            tz = "America/New_York";

    }

};
bool OpieSocket::downloadFile(const QString& path, QString& tmpFileName ) {
    KURL url;
    url.setProtocol("ftp" );
    url.setUser( d->user );
    url.setPass( d->pass );
    url.setHost( d->dest );
    url.setPort( 4242 );
    url.setPath(d->path + path );
    return KIO::NetAccess::download( url,  tmpFileName );
};
#include "opiesocket.moc"
