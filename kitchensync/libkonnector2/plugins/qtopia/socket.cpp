/*
    This file is part of KitchenSync.

    Copyright (c) 2002,2003 Holger Freyther <freyther@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qsocket.h>
#include <qdir.h>
#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>
#include <kio/netaccess.h>

#include <addressbooksyncee.h>
#include <eventsyncee.h>
#include <todosyncee.h>

#include <idhelper.h>

#include "device.h"
#include "desktop.h"
#include "datebook.h"
#include "addressbook.h"

#include "metaaddressbook.h"
#include "metadatebook.h"
#include "metatodo.h"

#include "todo.h"

#include "socket.h"

using namespace KSync;

namespace {
    void outputIt( int area, Syncee* );
}

class QtopiaSocket::Private
{
  public:
    enum CallIt
    {
        NotStarted = 0,
        Handshake  = 0,
        ABook,
        Todo,
        Calendar,
        Transactions,
        Files,
        Desktops,
        Flush
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
    SynceeList m_sync;

    QValueList<OpieCategories> categories;
    QString partnerId;
    QStringList files;
    QString tz;
    OpieHelper::CategoryEdit* edit;
    KonnectorUIDHelper* helper;
    OpieHelper::Device* device;
    OpieHelper::ExtraMap extras;
};

namespace {
    void parseTZ( const QString& fileName,  QString& tz );
}

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
    d->device = new OpieHelper::Device;
    m_flushedApps = 0;
}

QtopiaSocket::~QtopiaSocket()
{
    delete d->device;
    delete d;
}

void QtopiaSocket::setUser( const QString& user )
{
    d->device->setUser( user );
}

void QtopiaSocket::setPassword( const QString& pass )
{
    d->device->setPassword( pass );
}

void QtopiaSocket::setSrcIP( const QString& src)
{
    d->src = src;
}

void QtopiaSocket::setDestIP( const QString& dest)
{
    d->dest = dest;
}

void QtopiaSocket::setModel( const QString& model, const QString& name )
{
    if( model == QString::fromLatin1("Sharp Zaurus ROM") ){
	kdDebug(5225) << "Sharp Zaurus ROM match " << endl;
	d->device->setDistribution( OpieHelper::Device::Zaurus );
    }else
	d->device->setDistribution( OpieHelper::Device::Opie );

    d->device->setMeta( name );
}

void QtopiaSocket::startUp()
{
    kdDebug(5225) << "Start Up " << endl;
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

void QtopiaSocket::hangUP()
{
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

void QtopiaSocket::setResources( const QStringList& list )
{
    d->files = list;
}

bool QtopiaSocket::startSync()
{
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
bool QtopiaSocket::isConnected()
{
    if ( d->connected || d->mode == d->Call || d->mode  == d->Noop || d->mode == d->Connected )
        return true;
    else
        return false;
}

void QtopiaSocket::write( SynceeList list )
{
    if (!isConnected() ) {
        emit error( Error( i18n("<qt>Can not write the data back.\n There is no connection to the device") ) );
        emit prog( StdProgress::done() );
        return;
    }

    kdDebug(5225) << "Writing information back now. Count is " << list.count() << endl;

    AddressBookSyncee *abSyncee = list.addressBookSyncee();
    if ( abSyncee ) writeAddressbook( abSyncee );

    EventSyncee *evSyncee = list.eventSyncee();
    if ( evSyncee ) writeDatebook( evSyncee );

    TodoSyncee *toSyncee = list.todoSyncee();
    if ( toSyncee ) writeTodoList( toSyncee );

    /*
     * write new category information
     */
    writeCategory();
    d->helper->save();


    /* trigger reload for apps on pda */
    sendCommand( "call QPE/Application/datebook reload()" );
    sendCommand( "call QPE/Application/addressbook reload()" );
    sendCommand( "call QPE/Application/todolist reload()" );

    /*
     * tell Opie/Qtopia that we're ready
     */
    sendCommand( "call QPE/System stopSync()" );
    d->isSyncing = false;

    /*
     * now we need that it's not first sync
     */
    d->first = false;
    emit prog(StdProgress::done() );
}

QString QtopiaSocket::metaId() const
{
    return d->partnerId;
}

void QtopiaSocket::slotError( int err )
{
    d->isSyncing = false;
    d->isConnecting = false;
    kdDebug(5225) << "Error " << err << " for ip = " << d->dest << endl;

    emit error( StdError::connectionLost() );
}

void QtopiaSocket::slotConnected()
{
    emit prog( StdProgress::connection() );
    d->connected = true;
    delete d->timer;
    d->mode = d->Start;
}

void QtopiaSocket::slotClosed()
{
    d->connected    = false;
    d->isConnecting = false;
    d->isSyncing    = false;
    emit error( StdError::connectionLost() );
}

void QtopiaSocket::slotNOOP()
{
    if (!d->socket ) return;
    sendCommand( "NOOP" );
}

void QtopiaSocket::process()
{
    while ( d->socket->canReadLine() ) {
        QTextStream stream( d->socket );
        QString line = d->socket->readLine();
        kdDebug() << "100O " << line << endl;
        //kdDebug(5225) << line << endl;
        //kdDebug(5225) << d->mode << endl;
        switch( d->mode ) {
        case QtopiaSocket::Private::Start:
            start(line);
            break;
        case QtopiaSocket::Private::User:
            user(line);
            break;
        case QtopiaSocket::Private::Pass:
            pass(line);
            break;
        case QtopiaSocket::Private::Call:
            call(line);
            break;
        case QtopiaSocket::Private::Noop:
            noop(line);
            break;
        default:
            break;
        }
    }
}

void QtopiaSocket::slotStartSync()
{
    emit prog( Progress( i18n("Starting to sync now") ) );
    d->startSync = false;
    sendCommand( "call QPE/System sendHandshakeInfo()" );
    d->getMode = d->Handshake;
    d->mode = d->Call;
}

KURL QtopiaSocket::url( Type t )
{
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

KURL QtopiaSocket::url( const QString& path )
{
    KURL url;
    url.setProtocol("ftp" );
    url.setUser( d->device->user() );
    url.setPass( d->device->password() );
    url.setHost( d->dest );
    url.setPort( 4242 );
    url.setPath( path );

    return url;
}

/*
 * write the categories file
 */
void QtopiaSocket::writeCategory()
{
    QString fileName = QDir::homeDirPath() + "/.kitchensync/meta/" +d->partnerId + "/categories.xml";
    d->edit->save( fileName );
    KURL uri = url(  d->path + "/Settings/Categories.xml" );
    KIO::NetAccess::upload( fileName,  uri, 0 );
}

void QtopiaSocket::writeAddressbook( AddressBookSyncee* syncee )
{
    emit prog(Progress(i18n("Writing AddressBook back to the device") ) );
    OpieHelper::AddressBook abDB(d->edit, d->helper, d->tz, d->meta, d->device );
    KTempFile* file = abDB.fromKDE( syncee, d->extras );
    KURL uri = url( AddressBook );

    KIO::NetAccess::upload( file->name(), uri, 0 );
    file->unlink();
    delete file;

    if ( d->meta ) {
        OpieHelper::MD5Map map(QDir::homeDirPath() + "/.kitchensync/meta/" + d->partnerId + "/contacts.md5.qtopia");
        OpieHelper::MetaAddressbook metaBook;
        metaBook.saveMeta( syncee,  map );
        map.save( );
    }
}

void QtopiaSocket::writeDatebook( EventSyncee* syncee )
{
    OpieHelper::DateBook dbDB(d->edit, d->helper, d->tz, d->meta, d->device );
    KTempFile* file = dbDB.fromKDE( syncee, d->extras );
    KURL uri = url( DateBook );

    KIO::NetAccess::upload( file->name(), uri, 0 );
    file->unlink();
    delete file;

    if ( d->meta ) {
        OpieHelper::MD5Map map(QDir::homeDirPath() + "/.kitchensync/meta/" + d->partnerId + "/datebook.md5.qtopia");
        OpieHelper::MetaDatebook metaBook;
        metaBook.saveMeta( syncee,  map );
        map.save( );
    }
}

void QtopiaSocket::writeTodoList( TodoSyncee* syncee)
{
    OpieHelper::ToDo toDB(d->edit, d->helper, d->tz, d->meta, d->device );
    KTempFile* file = toDB.fromKDE( syncee, d->extras );
    KURL uri = url( TodoList );

    KIO::NetAccess::upload( file->name(), uri, 0 );
    file->unlink();
    delete file;

    if ( d->meta ) {
        OpieHelper::MD5Map map(QDir::homeDirPath() + "/.kitchensync/meta/" + d->partnerId + "/todolist.md5.qtopia");
        OpieHelper::MetaTodo  metaBook;
        metaBook.saveMeta( syncee,  map );
        map.save();
    }
}

void QtopiaSocket::readAddressbook()
{
    KSync::AddressBookSyncee* syncee = 0;
    emit prog( StdProgress::downloading(i18n("Addressbook") ) );
    QString tempfile;

    if (!downloadFile( "/Applications/addressbook/addressbook.xml", tempfile ) ) {
        emit error( StdError::downloadError(i18n("Addressbook") ) );
        syncee = new KSync::AddressBookSyncee;
        tempfile = QString::null;
    }

    emit prog( StdProgress::converting(i18n("Addressbook") ) );

    if (!syncee) {
        OpieHelper::AddressBook abDB( d->edit, d->helper, d->tz, d->meta, d->device );
        syncee = abDB.toKDE( tempfile, d->extras );
    }

    if (!syncee ) {
        KIO::NetAccess::removeTempFile( tempfile );
         emit error( i18n("Cannot read the addressbook file. It is corrupted.") );
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

    if (!tempfile.isEmpty() )
        KIO::NetAccess::removeTempFile( tempfile );
}

void QtopiaSocket::readDatebook()
{
    KSync::EventSyncee* syncee = 0;
    emit prog( StdProgress::downloading(i18n("Datebook") ) );
    QString tempfile;
    if (!downloadFile( "/Applications/datebook/datebook.xml", tempfile ) ) {
        emit error( StdError::downloadError(i18n("Datebook") ) );
        syncee = new KSync::EventSyncee;
        tempfile = QString::null;
    }
    emit prog( StdProgress::converting(i18n("Datebook") ) );

    /* the datebook.xml might not exist in this case we created an empty Entry
     * and there is no need to parse a non existint file
     */
    if (!syncee ) {
        OpieHelper::DateBook dateDB( d->edit, d->helper, d->tz, d->meta, d->device );
        syncee = dateDB.toKDE( tempfile, d->extras );
    }
    if (!syncee ) {
        KIO::NetAccess::removeTempFile( tempfile );
        emit error( i18n("Cannot read the datebook file. It is corrupted.") );
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

    if (!tempfile.isEmpty() )
        KIO::NetAccess::removeTempFile( tempfile );
}

void QtopiaSocket::readTodoList()
{
    KSync::TodoSyncee* syncee = 0;
    QString tempfile;
    emit prog( StdProgress::downloading(i18n("TodoList") ) );
    if (!downloadFile( "/Applications/todolist/todolist.xml", tempfile ) ) {
        emit error( StdError::downloadError(i18n("TodoList") ) );
        syncee = new KSync::TodoSyncee;
        tempfile = QString::null;
    }

    if (!syncee ) {
        OpieHelper::ToDo toDB( d->edit, d->helper, d->tz, d->meta, d->device );
        syncee = toDB.toKDE( tempfile, d->extras );
    }

    if (!syncee ) {
        KIO::NetAccess::removeTempFile( tempfile );
         emit error( i18n("Cannot read the TodoList file. It is corrupted.") );
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

    if (!tempfile.isEmpty() )
        KIO::NetAccess::removeTempFile( tempfile );
}

void QtopiaSocket::start( const QString& line )
{
    if ( line.left(3) != QString::fromLatin1("220") ) {
        emit error( Error(i18n("The device returned bogus data. giving up now.") ) );
        // something went wrong
        d->socket->close();
        d->mode = d->Done;
        d->connected    = false;
        d->isConnecting = false;
    } else {
        /*
         * parse the uuid
         * here if not zaurus
         */
        if( d->device->distribution() == OpieHelper::Device::Zaurus ){
            d->partnerId = d->device->meta();
        } else {
            QStringList list = QStringList::split(";", line );
            QString uid = list[1];
            uid = uid.mid(11, uid.length()-12 );
	          d->partnerId = uid;
        }
        initFiles();
        sendCommand( "USER " + d->device->user() );
        d->mode = d->User;
    }
}

void QtopiaSocket::user( const QString &line )
{
    emit prog( StdProgress::connected() );
//    emit prog( StdProgress::authentication() );
    if ( line.left(3) != QString::fromLatin1("331") ) {
        emit error( StdError::wrongUser( d->device->user() ) );
        // wrong user name
        d->socket->close();
        d->mode = d->Done;
        d->connected    = false;
        d->isConnecting = false;
    } else{
        sendCommand( "PASS " + d->device->password() );
        d->mode = d->Pass;
    }
}

void QtopiaSocket::pass( const QString& line)
{
    if ( line.left(3) != QString::fromLatin1("230") ) {
        emit error( StdError::wrongPassword() );
        // wrong password
        d->socket->close();
        d->mode = d->Done;
        d->connected    = false;
        d->isConnecting = false;
    } else {
        emit prog( StdProgress::authenticated() );
        kdDebug(5225) << "Konnected" << endl;
        d->mode = d->Noop;
        QTimer::singleShot(10000, this, SLOT(slotNOOP() ) );
    }
}

void QtopiaSocket::call( const QString& line)
{
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
    case QtopiaSocket::Private::Handshake:
        handshake( line );
        break;
    case QtopiaSocket::Private::Flush:
        flush( line );
        break;
    case QtopiaSocket::Private::ABook:
        download();
        break;
    case QtopiaSocket::Private::Desktops:
        initSync( line );
        break;
    }
}

void QtopiaSocket::flush( const QString& _line )
{
    if ( _line.startsWith("CALL QPE/Desktop flushDone(QString)") ||
         _line.startsWith("599 ChannelNotRegistered") ) {

        QString line = _line.stripWhiteSpace();
        QString appName;

        if ( line.endsWith( "datebook" ) ) {
            readDatebook();
            appName = i18n( "datebook" );
            m_flushedApps++;
        } else if ( line.endsWith( "todolist" ) ) {
            readTodoList();
            appName = i18n( "todolist" );
            m_flushedApps++;
        } else if ( line.endsWith( "addressbook" ) )  {
            readAddressbook();
            appName = i18n( "addressbook" );
            m_flushedApps++;
        }
        emit prog( Progress( i18n( "Flushed " ) + appName ) );
    }

    /* all apps have been flushed or have not been running */
    if ( m_flushedApps == 3 ) {
        /*
         * now we can progress during sync
         */
        d->getMode  = d->ABook;
        sendCommand( "call QPE/System getAllDocLinks()" );
        m_flushedApps = 0;
    }
}

void QtopiaSocket::noop( const QString & )
{
    d->isConnecting = false;
    if (!d->startSync ) {
        d->mode = d->Noop;
        QTimer::singleShot(10000, this, SLOT(slotNOOP() ) );
    }else
        slotStartSync();
}

void QtopiaSocket::handshake( const QString &line )
{
    QStringList list = QStringList::split( QString::fromLatin1(" "), line );
    d->path = list[3];
    if (!d->path.isEmpty() ) {
        d->getMode = d->Desktops;
        sendCommand( "call QPE/System startSync(QString) KitchenSync" );
    }
}

void QtopiaSocket::download()
{
    /*
     * we're all set now
     * start sync
     * and clear our list
     */
    emit sync( d->m_sync );
    d->mode = d->Noop;
    d->m_sync.clear();
}

void QtopiaSocket::initSync( const QString& )
{
    /* clear the extra map for the next round */
    d->extras.clear();
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

    /* flush the data on pda side to make sure to get the latest version */
    sendCommand( "call QPE/Application/datebook flush()" );
    sendCommand( "call QPE/Application/addressbook flush()" );
    sendCommand( "call QPE/Application/todolist flush()" );
    d->getMode  = d->Flush;
}

void QtopiaSocket::initFiles()
{
    QDir di( QDir::homeDirPath() + "/.kitchensync/meta/" + d->partnerId );
    /*
     * if our meta path exists do not recreate it
     */
    if ( di.exists() ) {
        d->first = false;
        return;
    }

    d->first = true;
    QDir dir;
    dir.mkdir(QDir::homeDirPath() + "/.kitchensync");
    dir.mkdir(QDir::homeDirPath() + "/.kitchensync/meta");
    dir.mkdir(QDir::homeDirPath() + "/.kitchensync/meta/" + d->partnerId );
}

QString QtopiaSocket::partnerIdPath() const
{
    QString str = QDir::homeDirPath();
    str += "/.kitchensync/meta/";
    str += d->partnerId;

    return str;
}

/*
 * As long as Qtopia/Opie is broken
 * in regards to handling timezones and events
 * we set the TimeZone to the one from Korganizer
 * for evolution we need to fix that!!!
 *
 */
void QtopiaSocket::readTimeZones()
{
    KConfig conf("korganizerrc");
    conf.setGroup("Time & Date");
    d->tz = conf.readEntry("TimeZoneId", QString::fromLatin1("UTC") );
    kdDebug(5225) << "TimeZone of Korg is " << d->tz << endl;
}

bool QtopiaSocket::downloadFile( const QString& str, QString& dest )
{
    KURL uri = url( d->path + str );
    bool b = KIO::NetAccess::download( uri, dest, 0 );
    kdDebug(5225) << "Getting " << str << " " << b << endl;
    return b;
}

void QtopiaSocket::download( const QString& res )
{
  Q_UNUSED( res );
}

void QtopiaSocket::sendCommand( const QString& cmd )
{
  if ( !d->socket )
    kdError() << "No socket available" << endl;

  kdDebug() << "100I: " << cmd << endl;

  QTextStream stream( d->socket );
  stream << cmd << endl;
}

namespace {

void forAll( int area, QPtrList<SyncEntry> list )
{
    for (SyncEntry* entry = list.first(); entry != 0; entry = list.next() ) {
        kdDebug(area) << "State " << entry->state() << endl;
        kdDebug(area) << "Summary " << entry->name() << endl;
        kdDebug(area) << "Uid " << entry->id() << endl;
    }
}

void outputIt( int area, Syncee *s )
{
    kdDebug(area) << "Added entries" << endl;
    forAll( area, s->added() );

    kdDebug(area) << "Modified " <<endl;
    forAll( area, s->modified() );

    kdDebug(area) << "Removed " << endl;
    forAll( area, s->removed() );
}

}

#include "socket.moc"
