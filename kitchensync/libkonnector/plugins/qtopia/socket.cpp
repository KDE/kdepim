#include <qsocket.h>
#include <qfile.h>
#include <qdir.h>
#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <ktempfile.h>
#include <kio/netaccess.h>
#include <kfileitem.h>
#include <kstandarddirs.h>
#include <kurl.h>

#include <addressbooksyncee.h>
#include <eventsyncee.h>
#include <todosyncee.h>

#include <idhelper.h>

#include "categoryedit.h"
#include "opiecategories.h"
#include "desktop.h"
#include "datebook.h"
#include "todo.h"
#include "addressbook.h"

#include "metaaddressbook.h"
#include "metadatebook.h"
#include "metatodo.h"

#include "socket.h"

using namespace KSync;

namespace {
    void outputIt( int area, Syncee* );
}

class QtopiaSocket::Private {
public:
    enum CallIt {
        NotStarted = 0,
        Handshake  = 0,
        ABook,
        Todo,
        Calendar,
        Transactions,
        Files,
        Desktops
    };
    enum Status {
        Start = 0,
        User,
        Pass,
        Call,
        Noop,
        Done,
        Connected
    };
    Private(){}

    QString pass;
    QString user;
    bool connected    : 1;
    bool startSync    : 1;
    bool isSyncing    : 1;
    bool isConnecting : 1;
    bool first        : 1;
    bool meta         : 1;
    QString src;
    QString dest;
    QSocket* socket;
    QTimer* timer;
    QString path;
    int mode;
    int getMode;
    Syncee::PtrList m_sync;

    QValueList<OpieCategories> categories;
    QString partnerId;
    QStringList files;
    QString tz;
    OpieHelper::CategoryEdit* edit;
    KonnectorUIDHelper* helper;

};
namespace {
    void parseTZ( const QString& fileName,  QString& tz );
};
/**
 * QtopiaSocket is somehow a state machine
 * during authentication
 * then it takes care of fethcing and converting
 */
QtopiaSocket::QtopiaSocket( QObject* obj, const char* name )
    : QObject( obj, name )
{
    d = new Private;
    d->socket = 0;
    d->timer  = 0;
    d->connected    = false;
    d->startSync    = false;
    d->isSyncing    = false;
    d->isConnecting = false;
    d->meta         = true ;
    d->helper  = 0;
    d->edit    = 0;
    d->first   = false;
}
QtopiaSocket::~QtopiaSocket() {
    delete d;
}
void QtopiaSocket::setUser( const QString& user ) {
    d->user = user;
}
void QtopiaSocket::setPassword( const QString& pass ) {
    d->pass = pass;
}
void QtopiaSocket::setSrcIP( const QString& src) {
    d->src = src;
}
void QtopiaSocket::setDestIP( const QString& dest) {
    d->dest = dest;
}
void QtopiaSocket::startUp() {
    delete d->socket;
    d->socket = new QSocket(this, "Qtopia Socket" );

    /* now connect to some slots */
    connect(d->socket, SIGNAL(error(int) ),
            this, SLOT(slotError(int) ) );
    connect(d->socket, SIGNAL(connected() ),
            this, SLOT(slotConnected() ) );
    connect(d->socket, SIGNAL(connectionClosed() ),
            this, SLOT(slotClosed() ) );
    connect(d->socket, SIGNAL(readyRead() ),
            this, SLOT(process() ) );

    d->connected    = false;
    d->startSync    = false;
    d->isConnecting = true;

    d->categories.clear();
    d->isSyncing = false;
    d->socket->connectToHost(d->dest, 4243 );
}
bool QtopiaSocket::startSync() {
    if ( d->isSyncing )
        return false;
    d->isSyncing = true;
    d->getMode   = d->NotStarted;

    if (d->isConnecting ) {
        d->startSync = true;
        return true;
    }
    if (!isConnected() ) {
        startUp();
        d->startSync = true;
        return true;
    }
    slotStartSync();

    return true;
}
/*
 * check if we're connected
 */
bool QtopiaSocket::isConnected() {
    if ( d->connected || d->mode == d->Call || d->mode  == d->Noop || d->mode == d->Connected )
        return true;
    else
        return false;
}
QByteArray QtopiaSocket::retrFile( const QString& fileName ) {
    QByteArray array;

    if( isConnected() ) {
        QString file;
        KURL ur = url( fileName );
        if (  KIO::NetAccess::download( ur,  file ) ) {
            QFile file2( file );
            if ( file2.open( IO_ReadOnly ) )
                array = file2.readAll();
        }
        KIO::NetAccess::removeTempFile( file );
    }
    return array;
}
Syncee* QtopiaSocket::retrEntry( const QString&) {
    return 0l;
}
bool QtopiaSocket::insertFile( const QString& file) {
    if (!d->connected )
        return false;

    d->files.append( file );
    return true;
}
void QtopiaSocket::write( const QString& str, const QByteArray& array) {
    KTempFile temp(locateLocal("tmp", "opie-konn-tmp"),  "konn" );
    QFile* file = temp.file();

    if ( file!=0 ) {
        file->writeBlock( array  );
        temp.close();

        KURL ur = url( str );
        KIO::NetAccess::upload(temp.name(), ur );
    }
    temp.unlink();
}
void QtopiaSocket::write( Syncee::PtrList list) {
    Syncee *syncee;

    /*
     * For all Syncees we see if it
     * is one of the Opie built in functionality
     */
    for ( syncee = list.first(); syncee != 0l; syncee = list.next() ) {
        if ( syncee->type() == QString::fromLatin1("AddressBookSyncee") ) {
            AddressBookSyncee* abSyncee = dynamic_cast<AddressBookSyncee*>(syncee );
            writeAddressbook( abSyncee );
        }else if ( syncee->type() == QString::fromLatin1("EventSyncee") ) {
            EventSyncee* evSyncee = dynamic_cast<EventSyncee*>(syncee);
            writeDatebook( evSyncee );
        }else if ( syncee->type() == QString::fromLatin1("TodoSyncee") ) {
            TodoSyncee* toSyncee = dynamic_cast<TodoSyncee*>(syncee);
            writeTodoList( toSyncee );
        }
    }
    /*
     * so that a clear frees the Syncees
     */
    list.setAutoDelete( true );
    list.clear();

    /*
     * write new category information
     */
    writeCategory();
    d->helper->save();

    /*
     * tell Opie/Qtopia that we're ready
     */
    QTextStream stream( d->socket );
    stream << "call QPE/System stopSync()" << endl;
    d->isSyncing = false;

    /*
     * now we need that it's not first sync
     */
    d->first = false;
}
/*
 * write back some Operations later ;)
 */
void QtopiaSocket::write( KOperations::ValueList ) {

}
QString QtopiaSocket::metaId()const {
    return d->partnerId;
};
void QtopiaSocket::slotError(int error) {
    d->isSyncing = false;
    d->isConnecting = false;

    emit stateChanged( false );
    emit errorKonnector( error, i18n("Connection error") );
}
void QtopiaSocket::slotConnected() {
    d->connected = true;
    delete d->timer;
    d->mode = d->Start;
}
void QtopiaSocket::slotClosed() {
    d->connected    = false;
    d->isConnecting = false;
    d->isSyncing    = false;
    emit stateChanged( false );
}
void QtopiaSocket::slotNOOP() {
    QTextStream stream( d->socket );
    stream << "NOOP" << endl;
}
void QtopiaSocket::process() {
    while ( d->socket->canReadLine() ) {
        QTextStream stream( d->socket );
        QString line = d->socket->readLine();
        kdDebug(5225) << line << endl;
        kdDebug(5225) << d->mode << endl;
        switch( d->mode ) {
        case d->Start:
            start(line);
            break;
        case d->User:
            user(line);
            break;
        case d->Pass:
            pass(line);
            break;
        case d->Call:
            call(line);
            break;
        case d->Noop:
            noop(line);
            break;
        }
    }
}
void QtopiaSocket::slotStartSync() {
    d->startSync = false;
    QTextStream stream( d->socket );
    stream << "call QPE/System sendHandshakeInfo()" << endl;
    d->getMode = d->Handshake;
    d->mode = d->Call;
}
KURL QtopiaSocket::url( Type  t) {
    QString uri;
    uri = d->path + "/Applications/";
    switch( t ) {
    case AddressBook:
        uri += "addressbook/addressbook.xml";
        break;
    case TodoList:
        uri += "todolist/todolist.xml";
        break;
    case DateBook:
        uri += "datebook/datebook.xml";
        break;
    }
    return url( uri );
}
KURL QtopiaSocket::url( const QString& path ) {
    KURL url;
    url.setProtocol("ftp" );
    url.setUser( d->user );
    url.setPass( d->pass );
    url.setHost( d->dest );
    url.setPort( 4242 );
    url.setPath( path );

    return url;
}
/*
 * write the categories file
 */
void QtopiaSocket::writeCategory() {
    QString fileName = QDir::homeDirPath() + "/.kitchensync/meta/" +d->partnerId + "/categories.xml";
    d->edit->save( fileName );
    KURL uri = url(  d->path + "/Settings/Categories.xml" );
    KIO::NetAccess::upload( fileName,  uri );
}
void QtopiaSocket::writeAddressbook( AddressBookSyncee* syncee) {
    OpieHelper::AddressBook abDB(d->edit, d->helper, d->tz, d->meta );
    KTempFile* file = abDB.fromKDE( syncee );
    KURL uri = url( AddressBook );

    KIO::NetAccess::upload( file->name(), uri );
    file->unlink();
    delete file;

    if ( d->meta ) {
        OpieHelper::MD5Map map(QDir::homeDirPath() + "/.kitchensync/meta/" + d->partnerId + "/contacts.md5.qtopia");
        OpieHelper::MetaAddressbook metaBook;
        metaBook.saveMeta( syncee,  map );
        map.save( );
    }
}
void QtopiaSocket::writeDatebook( EventSyncee* syncee) {
    OpieHelper::DateBook dbDB(d->edit, d->helper, d->tz, d->meta );
    KTempFile* file = dbDB.fromKDE( syncee );
    KURL uri = url( DateBook );

    KIO::NetAccess::upload( file->name(), uri );
    file->unlink();
    delete file;

    if ( d->meta ) {
        OpieHelper::MD5Map map(QDir::homeDirPath() + "/.kitchensync/meta/" + d->partnerId + "/datebook.md5.qtopia");
        OpieHelper::MetaDatebook metaBook;
        metaBook.saveMeta( syncee,  map );
        map.save( );
    }
}
void QtopiaSocket::writeTodoList( TodoSyncee* syncee) {
    OpieHelper::ToDo toDB(d->edit, d->helper, d->tz, d->meta );
    KTempFile* file = toDB.fromKDE( syncee );
    KURL uri = url( TodoList );

    KIO::NetAccess::upload( file->name(), uri );
    file->unlink();
    delete file;

    if ( d->meta ) {
        OpieHelper::MD5Map map(QDir::homeDirPath() + "/.kitchensync/meta/" + d->partnerId + "/todolist.md5.qtopia");
        OpieHelper::MetaTodo  metaBook;
        metaBook.saveMeta( syncee,  map );
        map.save();
    }
}
void QtopiaSocket::readAddressbook() {
    QString tempfile;
    if (!downloadFile( "/Applications/addressbook/addressbook.xml", tempfile ) )
        return;

    OpieHelper::AddressBook abDB( d->edit, d->helper, d->tz, d->meta );
    KSync::AddressBookSyncee* syncee = abDB.toKDE( tempfile );
    if (!syncee ) {
        KIO::NetAccess::removeTempFile( tempfile );
        return;
    }

    syncee->setFirstSync( d->first );

    /*
     * If in meta mode but not the first syncee
     * collect some meta infos
     */
    if ( d->meta && !d->first ) {
        syncee->setSyncMode( KSync::Syncee::MetaMode );
        OpieHelper::MD5Map map( QDir::homeDirPath() + "/.kitchensync/meta/" + d->partnerId + "/contacts.md5.qtopia" );
        OpieHelper::MetaAddressbook metaBook;
        metaBook.doMeta( syncee,  map );
    }
    d->m_sync.append( syncee );

    KIO::NetAccess::removeTempFile( tempfile );
}
void QtopiaSocket::readDatebook() {
    QString tempfile;
    if (!downloadFile( "/Applications/datebook/datebook.xml", tempfile ) )
        return;

    OpieHelper::DateBook dateDB( d->edit, d->helper, d->tz, d->meta );
    KSync::EventSyncee* syncee = dateDB.toKDE( tempfile );
    if (!syncee ) {
        KIO::NetAccess::removeTempFile( tempfile );
        return;
    }

    syncee->setFirstSync( d->first );

    /*
     * for meta mode get meta info
     */
    if ( d->meta && !d->first ) {
        syncee->setSyncMode( KSync::Syncee::MetaMode );
        OpieHelper::MD5Map map( QDir::homeDirPath() + "/.kitchensync/meta/" + d->partnerId + "/datebook.md5.qtopia" );
        OpieHelper::MetaDatebook metaBook;
        metaBook.doMeta( syncee,  map );
        kdDebug(5229) << "Did Meta" << endl;
        outputIt(5229, syncee );
    }
    d->m_sync.append( syncee );

    KIO::NetAccess::removeTempFile( tempfile );
}
void QtopiaSocket::readTodoList() {
    QString tempfile;
    if (!downloadFile( "/Applications/todolist/todolist.xml", tempfile ) )
        return;

    OpieHelper::ToDo toDB( d->edit, d->helper, d->tz, d->meta );
    KSync::TodoSyncee* syncee = toDB.toKDE( tempfile );
    if (!syncee ) {
        KIO::NetAccess::removeTempFile( tempfile );
        return;
    }

    syncee->setFirstSync( d->first );

    if ( d->meta && !d->first ) {
        syncee->setSyncMode( KSync::Syncee::MetaMode );
        OpieHelper::MD5Map map( QDir::homeDirPath() + "/.kitchensync/meta/" + d->partnerId + "/todolist.md5.qtopia" );
        OpieHelper::MetaTodo metaBook;
        metaBook.doMeta( syncee, map );
        kdDebug(5227) << "Did Meta " << endl;
        outputIt(5227, syncee );
    }

    d->m_sync.append( syncee );

    KIO::NetAccess::removeTempFile( tempfile );
}
void QtopiaSocket::readPartner() {

}

void QtopiaSocket::start(const QString& line ) {
    QTextStream stream( d->socket );
    if ( line.left(3) != QString::fromLatin1("220") ) {
        // something went wrong
        d->socket->close();
        d->mode = d->Done;
        d->connected    = false;
        d->isConnecting = false;
    }else{
        /*
         * parse the uuid
         * here
         */
        QStringList list = QStringList::split(";", line );
        qWarning(list[1] );
        QString uid = list[1];
        uid = uid.mid(11, uid.length()-12 );
        d->partnerId = uid;
        initFiles();
        stream << "USER " << d->user << endl;
        d->mode = d->User;
    }
}

void QtopiaSocket::user( const QString& line) {
    QTextStream stream( d->socket );
    if ( line.left(3) != QString::fromLatin1("331") ) {
        // wrong user name
        d->socket->close();
        d->mode = d->Done;
        d->connected    = false;
        d->isConnecting = false;
    }else{
        stream << "PASS " << d->pass << endl;
        d->mode = d->Pass;
    }
}
void QtopiaSocket::pass( const QString& line) {
    if ( line.left(3) != QString::fromLatin1("230") ) {
        // wrong password
        d->socket->close();
        d->mode = d->Done;
        d->connected    = false;
        d->isConnecting = false;
    }else {
        kdDebug(5225) << "Konnected" << endl;
        d->mode = d->Noop;
        QTimer::singleShot(10000, this, SLOT(slotNOOP() ) );
        emit stateChanged( true );
    }
}
void QtopiaSocket::call( const QString& line) {
    if ( line.contains("220 Command okay" ) &&
         ( d->getMode == d->Handshake || d->getMode == d->ABook ) )
        return;

    if ( line.startsWith("CALL QPE/Desktop docLinks(QString)") ) {
        OpieHelper::Desktop desk( d->edit );
        Syncee* sync = desk.toSyncee( line );
        if ( sync )
            d->m_sync.append( sync );
    }
    switch( d->getMode ) {
    case d->Handshake:
        handshake(line);
        break;
    case d->ABook:
        download();
        break;
    case d->Desktops:
        initSync( line );
        break;
    }
}
void QtopiaSocket::noop( const QString&) {
    d->isConnecting = false;
    if (!d->startSync ) {
        d->mode = d->Noop;
        QTimer::singleShot(10000, this, SLOT(slotNOOP() ) );
    }else
        slotStartSync();
}
void QtopiaSocket::handshake( const QString& line) {
    QTextStream stream( d->socket );
    QStringList list = QStringList::split( QString::fromLatin1(" "), line );
    d->path = list[3];
    kdDebug(5225) << "D->PATH is " << d->path << endl;
    kdDebug(5225) << "D Line Was " << line << endl;
    if (!d->path.isEmpty() ) {
        d->getMode = d->Desktops;
        stream << "call QPE/System startSync(QString) KitchenSync" << endl;
    }
}
void QtopiaSocket::download() {
    readAddressbook();
    readDatebook();
    readTodoList();

    /*
     * we're all set now
     * start sync
     * and clear our list
     */
    emit sync( d->m_sync );
    d->mode = d->Noop;
    d->m_sync.clear();
}
void QtopiaSocket::initSync( const QString& ) {
    QString tmpFileName;
    downloadFile( "/Settings/Categories.xml", tmpFileName );

    /* Category Edit */
    delete d->edit;
    d->edit = new OpieHelper::CategoryEdit( tmpFileName );
    KIO::NetAccess::removeTempFile( tmpFileName );

    /* KonnectorUIDHelper */
    delete d->helper;
    d->helper = new KonnectorUIDHelper( partnerIdPath() );

    /* TimeZones */
    readTimeZones();

    /*
     * now we can progress during sync
     */
    QTextStream stream( d->socket );
    stream << "call QPE/System getAllDocLinks()" << endl;
    d->getMode  = d->ABook;
}
void QtopiaSocket::initFiles() {
    QDir di( QDir::homeDirPath() + "/.kitchensync/meta/" + d->partnerId );
    /*
     * if our meta path exists do not recreate it
     */
    if (di.exists()  ) {
        d->first = false;
        return;
    }

    d->first = true;
    QDir dir;
    dir.mkdir(QDir::homeDirPath() + "/.kitchensync");
    dir.mkdir(QDir::homeDirPath() + "/.kitchensync/meta");
    dir.mkdir(QDir::homeDirPath() + "/.kitchensync/meta/" + d->partnerId );
}
QString QtopiaSocket::partnerIdPath()const {
    QString str = QDir::homeDirPath();
    str += "/.kitchensync/meta/";
    str += d->partnerId;

    return str;
};

/*
 * As long as Qtopia/Opie is broken
 * in regards to handling timezones and events
 * we set the TimeZone to the one from Korganizer
 * for evolution we need to fix that!!!
 *
 */
void QtopiaSocket::readTimeZones() {
    KConfig conf("korganizerrc");
    conf.setGroup("Time & Date");
    d->tz = conf.readEntry("TimeZoneId", QString::fromLatin1("UTC") );
    kdDebug(5229) << "TimeZone of Korg is " << d->tz << endl;
}
bool QtopiaSocket::downloadFile( const QString& str, QString& dest ) {
    KURL uri = url( d->path + str );
    bool b = KIO::NetAccess::download( uri, dest );
    kdDebug(5225) << "Getting " << str << " " << b << endl;
    return b;
}

namespace {
    void forAll( int area, QPtrList<SyncEntry> list) {
        for (SyncEntry* entry = list.first(); entry != 0; entry = list.next() ) {
            kdDebug(area) << "State " << entry->state() << endl;
            kdDebug(area) << "Summary " << entry->name() << endl;
            kdDebug(area) << "Uid " << entry->id() << endl;
        }
    }
    void outputIt( int area,  Syncee* s) {
        kdDebug(area) << "Added entries" << endl;
        forAll( area, s->added() );

        kdDebug(area) << "Modified " <<endl;
        forAll( area, s->modified() );

        kdDebug(area) << "Removed " << endl;
        forAll( area, s->removed() );
    }

}

#include "socket.moc"
