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
    kdDebug(5210) << "Start Up " << endl;
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
void QtopiaSocket::hangUP() {
    if (d->isSyncing ) {
        emit error( Error(Error::CouldNotDisconnect, i18n("Can not disconnect now. Try again after syncing was finished") ) );
        return;
    }
    /* now connect to some slots */
    disconnect(d->socket, SIGNAL(error(int) ),
            this, SLOT(slotError(int) ) );
    disconnect(d->socket, SIGNAL(connected() ),
            this, SLOT(slotConnected() ) );
    disconnect(d->socket, SIGNAL(connectionClosed() ),
            this, SLOT(slotClosed() ) );
    disconnect(d->socket, SIGNAL(readyRead() ),
            this, SLOT(process() ) );
    delete d->socket;
    d->socket = 0;
    d->isSyncing = false;
    d->connected = false;
    d->startSync = false;
    d->isConnecting = false;
    d->categories.clear();
    d->getMode = d->NotStarted;
    d->mode = d->Start;
    emit prog( Progress(i18n("Disconnected from the device.") ) );
}
void QtopiaSocket::setResources( const QStringList& list ) {
    d->files = list;
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
void QtopiaSocket::write( Syncee::PtrList list) {
    if (!isConnected() ) {
        emit error( Error( i18n("<qt>Can not write the data back.\n There is no connection to the device") ) );
        list.setAutoDelete( true );
        emit prog( StdProgress::done() );
        return;
    }
    Syncee *syncee;

    kdDebug(5225) << "Writing informations back now. Count is " << list.count() << endl;
    /*
     * For all Syncees we see if it
     * is one of the Opie built in functionality
     */
    for ( syncee = list.first(); syncee != 0l; syncee = list.next() ) {
        kdDebug(5225) << " Trying to write " << syncee->type() << endl;
        if ( syncee->type() == QString::fromLatin1("AddressBookSyncee") ) {
            AddressBookSyncee* abSyncee = static_cast<AddressBookSyncee*>(syncee );
            writeAddressbook( abSyncee );
        }else if ( syncee->type() == QString::fromLatin1("EventSyncee") ) {
            EventSyncee* evSyncee = static_cast<EventSyncee*>(syncee);
            writeDatebook( evSyncee );
        }else if ( syncee->type() == QString::fromLatin1("TodoSyncee") ) {
            TodoSyncee* toSyncee = static_cast<TodoSyncee*>(syncee);
            writeTodoList( toSyncee );
        }
    }
    /*
     * so that a clear frees the Syncees
     */
    list.setAutoDelete( true );
    list.clear();

    /*
     * write new category informations
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
    emit prog(StdProgress::done() );
}
QString QtopiaSocket::metaId()const {
    return d->partnerId;
};
void QtopiaSocket::slotError(int err) {
    d->isSyncing = false;
    d->isConnecting = false;
    kdDebug(5225) << "Error " << err << " for ip = " << d->dest << endl;

    emit error( StdError::connectionLost() );
}
void QtopiaSocket::slotConnected() {
    emit prog( StdProgress::connection() );
    d->connected = true;
    delete d->timer;
    d->mode = d->Start;
}
void QtopiaSocket::slotClosed() {
    d->connected    = false;
    d->isConnecting = false;
    d->isSyncing    = false;
    emit error( StdError::connectionLost() );
}
void QtopiaSocket::slotNOOP() {
    if (!d->socket ) return;
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
        default:
            break;
        }
    }
}
void QtopiaSocket::slotStartSync() {
    emit prog( Progress( i18n("Starting to sync now") ) );
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
    emit prog(Progress(i18n("Writing AddressBook back to the device") ) );
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
    emit prog( StdProgress::downloading(i18n("Addressbook") ) );
    QString tempfile;
    if (!downloadFile( "/Applications/addressbook/addressbook.xml", tempfile ) ) {
        emit error( StdError::downloadError(i18n("Addressbook") ) );
        return;
    }

    emit prog( StdProgress::converting(i18n("Addressbook") ) );
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
        emit prog( Progress(i18n("Not first sync collecting the changes now") ) );
        syncee->setSyncMode( KSync::Syncee::MetaMode );
        OpieHelper::MD5Map map( QDir::homeDirPath() + "/.kitchensync/meta/" + d->partnerId + "/contacts.md5.qtopia" );
        OpieHelper::MetaAddressbook metaBook;
        metaBook.doMeta( syncee,  map );
    }
    d->m_sync.append( syncee );

    KIO::NetAccess::removeTempFile( tempfile );
}
void QtopiaSocket::readDatebook() {
    emit prog( StdProgress::downloading(i18n("Datebook") ) );
    QString tempfile;
    if (!downloadFile( "/Applications/datebook/datebook.xml", tempfile ) ) {
        emit error( StdError::downloadError(i18n("Datebook") ) );
        return;
    }
    emit prog( StdProgress::converting(i18n("Datebook") ) );
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
        emit prog( StdProgress::converting(i18n("Datebook") ) );
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
    emit prog( StdProgress::downloading(i18n("TodoList") ) );
    if (!downloadFile( "/Applications/todolist/todolist.xml", tempfile ) ) {
        emit error( StdError::downloadError(i18n("TodoList") ) );
        return;
    }

    OpieHelper::ToDo toDB( d->edit, d->helper, d->tz, d->meta );
    KSync::TodoSyncee* syncee = toDB.toKDE( tempfile );
    if (!syncee ) {
        KIO::NetAccess::removeTempFile( tempfile );
        return;
    }

    syncee->setFirstSync( d->first );

    if ( d->meta && !d->first ) {
        emit prog( Progress(i18n("Not first sync collecting the changes now") ) );
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
        emit error( Error(i18n("The device returned bogus data. giving up now.") ) );
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
        qWarning(list[1].latin1() );
        QString uid = list[1];
        uid = uid.mid(11, uid.length()-12 );
        d->partnerId = uid;
        initFiles();
        stream << "USER " << d->user << endl;
        d->mode = d->User;
    }
}

void QtopiaSocket::user( const QString& line) {
    emit prog( StdProgress::connected() );
//    emit prog( StdProgress::authentication() );
    QTextStream stream( d->socket );
    if ( line.left(3) != QString::fromLatin1("331") ) {
        emit error( StdError::wrongUser( d->user ) );
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
        emit error( StdError::wrongPassword() );
        // wrong password
        d->socket->close();
        d->mode = d->Done;
        d->connected    = false;
        d->isConnecting = false;
    }else {
        emit prog( StdProgress::authenticated() );
        kdDebug(5225) << "Konnected" << endl;
        d->mode = d->Noop;
        QTimer::singleShot(10000, this, SLOT(slotNOOP() ) );
    }
}
void QtopiaSocket::call( const QString& line) {
    if ( line.contains("220 Command okay" ) &&
         ( d->getMode == d->Handshake || d->getMode == d->ABook ) )
        return;

    if ( line.startsWith("CALL QPE/Desktop docLinks(QString)") ) {
        emit prog( Progress(i18n("Getting the Document Links of the Document Tab") ) );
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
    emit prog( StdProgress::downloading("Categories.xml") );
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
    kdDebug(5225) << "TimeZone of Korg is " << d->tz << endl;
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
