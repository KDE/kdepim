#include <qhostaddress.h>
#include <qtimer.h>
#include <qdom.h>
#include <qdir.h>
#include <qfile.h>

#include <kapplication.h>
#include <opiedesktopsyncentry.h>
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

#include <kunknownsyncentry.h>
#include <kalendarsyncentry.h>

#include <idhelper.h>

#include "opiesocket.h"
#include "opiecategories.h"
#include "opiehelper.h"

#include "categoryedit.h"
#include "datebook.h"
#include "todo.h"
#include "addressbook.h"

class OpieSocket::OpieSocketPrivate{
public:
    OpieSocketPrivate(){}
    QString pass;
    QString user;
    bool connected:1;
    bool startSync:1;
    bool isSyncing:1;
    bool isConnecting:1;
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
    QPtrList<KSyncEntry> m_sync;
    QValueList<OpieCategories> m_categories;
    QString partnerId;
    QStringList files;
    // helper
    KonnectorUIDHelper *helper;
    OpieHelper::CategoryEdit *edit;
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
	kdDebug(5202) << "isSyncing" << endl;
	return false;
    }
    d->isSyncing = true;
    d->getMode = d->NOTSTARTED;
    if(d->isConnecting ){
	kdDebug(5202) << "is connecting tell it to startSync if in NOOP" << endl;
	d->startSync = true;
	return true;
    }
    if(!isConnected()  ){
	kdDebug(5202) << "Not connected starting connection" << endl;
	startUp();
	d->startSync = true;
	return true;
    }
    kdDebug(5202) << "start sync the normal way"  << endl;
    slotStartSync();
    return true;
}
bool OpieSocket::isConnected()
{
    if( d->mode == d->CALL || d->mode == d->NOOP || d->mode == d->CONNECTED ){
	return true;
    }else{
	return false;
    }
}
QByteArray OpieSocket::retrFile(const QString &path )
{
    if( d->mode == d->NOOP ){
    //stop the timer

    }
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
void OpieSocket::write(QPtrList<KSyncEntry> lis)
{
// ok the list
    kdDebug(5202) << "Write back" << endl;
    lis.setAutoDelete( TRUE );
    KSyncEntry* entry;
    KURL url;
    url.setProtocol("ftp" );
    url.setUser( d->user );
    url.setPass( d->pass );
    url.setHost( d->dest );
    url.setPort( 4242 );
    //url.setPath( path );
    for ( entry = lis.first(); entry != 0; lis.next() ) {
        // convert to XML
        if ( entry->type() == QString::fromLatin1("KAlendarSyncEntry") ) {
            KAlendarSyncEntry* cal = (KAlendarSyncEntry*) entry;
            OpieHelper::DateBook dateb( d->edit,  d->helper,  d->meta );
            OpieHelper::ToDo todo( d->edit,  d->helper, d->meta );
            QByteArray todos = todo.fromKDE( cal );
            QByteArray events = dateb.fromKDE( cal );
            // Events + Todo
            if ( d->meta ) { // save the bytearray
                QFile aFile(QDir::homeDirPath()+ "/.kitchensync/meta/" + d->partnerId + "/todolist.xml" );
                if ( aFile.open(IO_WriteOnly) ) {
                    aFile.writeBlock(todos);
                }
                QFile bFile(QDir::homeDirPath()+ "/.kitchensync/meta/" + d->partnerId + "/datebook.xml");
                if ( bFile.open(IO_WriteOnly) ) {
                    bFile.writeBlock( events );
                }
            }
        }else if ( entry->type() == QString::fromLatin1("AddressbookSyncEntry") ) {
            KAddressbookSyncEntry* ab = (KAddressbookSyncEntry*) entry;
            OpieHelper::AddressBook abook( d->edit,  d->helper,  d->meta );
            QByteArray book = abook.fromKDE( ab );
            KTempFile file(locateLocal("tmp", "opie-konn-address"), "new" );
            QFile *fi  = file.file();
            fi->writeBlock( book );
            file.close();
            url.setPath(d->path + "/Applications/addressbook/adressbook.xml");
            KIO::NetAccess::upload(file.name(), url);
            file.unlink();
            if ( d->meta ) {
                QFile file2(QDir::homeDirPath()+ "/.kitchensync/meta/" + d->partnerId + "/addressbook.xml");
                if ( file2.open(IO_WriteOnly ) ) {
                    file2.writeBlock(book);
                }
            }
        }else if ( entry->type() == QString::fromLatin1("KUnknownSyncEntry") ) {
            KUnknownSyncEntry* un = (KUnknownSyncEntry*) entry;
            KTempFile file(locateLocal("tmp", "opie-konn-unknown"), "new" );
            QFile *fi = file.file();
            fi->writeBlock( un->byteArray() );
            file.close();
            url.setPath(un->fileName()  );
            KIO::NetAccess::upload( file.name(),  url );
            file.unlink();
        }
    }
    // OpieHelper::CategoryEdit write back
    lis.clear();
}
void OpieSocket::write(QValueList<KOperations> )
{
    kdDebug(5202) << "write KOperations not implemented yet" << endl;
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
	kdDebug(5202) << line.stripWhiteSpace() << endl;
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
		    kdDebug(5202) << "start the NOOP" << endl;
		    d->mode = d->NOOP;
		    d->timer = new QTimer(this );
		    connect(d->timer, SIGNAL(timeout() ), this, SLOT(slotNOOP() ) );
		    d->timer->start(10000 );
                    emit stateChanged( true );
		}
		break;
	    }
	    case d->CALL:
		kdDebug(5202) << "CALL :)" << endl;
		manageCall( line );
		break;
	    case d->NOOP:
		//stream << "NOOP\r\n";
		d->isConnecting = false;
		if(!d->startSync ){
		    kdDebug(5202) << "start NOOP" << endl;
		    d->mode = d->NOOP;
		    d->timer = new QTimer(this );
		    connect(d->timer, SIGNAL(timeout() ), this, SLOT(slotNOOP() ) );
		    d->timer->start(10000 );
		} else {
		    kdDebug (5202) << "slotStartSync now" << endl;
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
  kdDebug(5202) << "NOOP" << endl;
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
    if( line.contains("200 Command okay" ) && ( d->getMode == d->HANDSHAKE || d->getMode == d->ABOOK ) ) {
	return;
    }
    if( line.startsWith("CALL QPE/Desktop docLinks(QString)" ) ){
	kdDebug(5202 ) << "CALL docLinks desktop entry" << endl;
        OpieHelperClass helper;
	helper.toOpieDesktopEntry( line, &d->m_sync, d->edit  );
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
	    // start with the files use KIO::NetAcces for it simpleness first
	    kdDebug(5202) << "Starting fetching" << endl;
	    QString tmpFileName;
	    KURL url;
	    url.setProtocol("ftp" );
	    url.setUser( d->user );
	    url.setPass( d->pass );
	    url.setHost( d->dest );
	    url.setPort( 4242 );
	    url.setPath(d->path + "/Settings/Categories.xml " );
	    //tmpFileName = QString::fromLatin1("/home/ich/categories.xml")
            kdDebug(5202) << "Fetching categories" << endl;;
	    KIO::NetAccess::download( url, tmpFileName );
            delete d->edit;
            d->edit = new OpieHelper::CategoryEdit( tmpFileName );
	    KIO::NetAccess::removeTempFile( tmpFileName );

	    url.setPath(d->path + "/Applications/addressbook/addressbook.xml" );
	    tmpFileName = QString::null;
	    //tmpFileName = "/home/ich/addressbook.xml";
            kdDebug(5202) << "Fetching addressbook " << endl;
	    KIO::NetAccess::download( url, tmpFileName );
	    KIO::UDSEntry uds;
	    KIO::NetAccess::stat( url, uds );
	    KFileItem item(  uds, url );
            kdDebug(5202 ) << "------------------TIMESTAMP---------------"<< endl;
            kdDebug(5202 ) << "------------------TIMESTAMP---------------"<< endl;
            kdDebug(5202 ) << "------------------TIMESTAMP---------------"<< endl;
            kdDebug(5202)  << item.timeString() << endl;
            OpieHelper::AddressBook book( d->edit,  d->helper,  d->meta );
            d->m_sync.append( book.toKDE( tmpFileName ) );
	    KIO::NetAccess::removeTempFile( tmpFileName );

            // Calendar
	    QString todo;
            kdDebug(5202) << "Fetching todolist" << endl;
	    url.setPath(d->path + "/Applications/todolist/todolist.xml" );
	    KIO::NetAccess::download(url, todo );
            kdDebug(5202) << "Fetching calendar" << endl;
	    url.setPath(d->path + "/Applications/datebook/datebook.xml" );
	    KIO::NetAccess::download(url, tmpFileName );

            KAlendarSyncEntry *calEntry = new KAlendarSyncEntry();
            KCal::CalendarLocal *calLoc = new KCal::CalendarLocal();
            calEntry->setCalendar( calLoc );
            OpieHelper::ToDo todoDB( d->edit, d->helper,  d->meta );
            OpieHelper::DateBook dateDB( d->edit, d->helper,  d->meta );
            QPtrList<KCal::Todo> todoList = todoDB.toKDE( todo );
            QPtrList<KCal::Event> dateList = dateDB.toKDE( tmpFileName );
            KCal::Todo* todoEvent;
            KCal::Event * dateEvent;
            for ( todoEvent = todoList.first(); todoEvent != 0; todoEvent = todoList.next() ) {
                calLoc->addTodo( todoEvent );
            }
            for (dateEvent = dateList.first(); dateEvent != 0; dateEvent = dateList.next() ) {
                calLoc->addEvent( dateEvent );
            }
            d->m_sync.append( calEntry );
            KIO::NetAccess::removeTempFile( tmpFileName );
	    KIO::NetAccess::removeTempFile( todo );
	    // done with fetching
            // come back to the normal mode
            // emit signal
            emit sync( d->m_sync );
            stream << "call QPE/System stopSync()" << endl;
            d->isSyncing = false; // do it in the write back later on
            d->mode = d->NOOP;
	    break;
	}
	case d->DESKTOPS:{
            // ok we're now in sync mode let's see if we know this guy
            // if meta mode let's see where the corresponding meta dir is
            // $KDEHOME/share/apps/kitchensync/metadata/konnector-id
            if ( d->meta ) {
                QString file;
                KURL url;
                url.setProtocol("ftp" );
                url.setUser( d->user );
                url.setPass( d->pass );
                url.setHost( d->dest );
                url.setPort( 4242 );
                url.setPath(d->path + "/Settings/meinPartner");
                if ( !KIO::NetAccess::download( url,  file ) ) {
                    newPartner();
                    QDir dir;
                    dir.mkdir(QDir::homeDirPath() + "/.kitchensync/meta");
                    dir.mkdir(QDir::homeDirPath() + "/.kitchensync/meta/" + d->partnerId );
                }else{
                    readPartner(file);
                    KIO::NetAccess::removeTempFile( file );
                }
                delete d->helper;
                d->helper = new KonnectorUIDHelper(QDir::homeDirPath() + "/.kitchensync/meta/" + d->partnerId);
            }
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
    d->partnerId = randomString( 10 );
    kdDebug(5202) << "New Sync  " << d->partnerId << endl;
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
}
void OpieSocket::readPartner( const QString &fileName )
{
    QFile file( fileName );
    if ( file.open(IO_ReadOnly ) ) {
        QTextStream stream ( &file );
        QString string;
        stream >> string;
        if ( string.left( 5 ) == QString::fromLatin1("opie-") )
            d->partnerId = string.mid( 6 );
    }
    kdDebug(5202) << "Known Partner " << d->partnerId << endl;
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
KSyncEntry* retrEntry( const QString& )
{
    return 0l;
}
