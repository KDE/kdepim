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
#include <libkcal/calendarlocal.h>
#include <libkdepim/kpimprefs.h>
#include <libkdepim/progressmanager.h>

#include <addressbooksyncee.h>
#include <calendarsyncee.h>
#include <unknownsyncee.h>

#include <idhelper.h>

#include "device.h"
#include "desktop.h"
#include "datebook.h"
#include "addressbook.h"

#include "metaaddressbook.h"
#include "metacalendar.h"

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
    QString src;
    QString dest;
    QSocket* socket;
    QTimer* timer;
    QString path;
    QString storagePath;
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

QString QtopiaSocket::storagePath()const
{
  return d->storagePath;
}

void QtopiaSocket::setStoragePath( const QString& str )
{
  d->storagePath = str;
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
	d->device->setDistribution( OpieHelper::Device::Zaurus );
    }else
	d->device->setDistribution( OpieHelper::Device::Opie );

    d->device->setMeta( name );
}

void QtopiaSocket::startUp()
{
  mProgressItem = KPIM::ProgressManager::instance()->createProgressItem(
      KPIM::ProgressManager::getUniqueID(), i18n( "Connecting" ) );

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

void QtopiaSocket::hangUp()
{
    if (d->isSyncing ) {
//        emit error( Error(Error::CouldNotDisconnect, i18n("Can not disconnect now. Try again after syncing was finished") ) );
        return;
    }

    disconnect(d->socket, SIGNAL(error(int) ),
            this, SLOT(slotError(int) ) );
    disconnect(d->socket, SIGNAL(connected() ),
            this, SLOT(slotConnected() ) );
    disconnect(d->socket, SIGNAL(connectionClosed() ),
            this, SLOT(slotClosed() ) );
    disconnect(d->socket, SIGNAL(readyRead() ),
            this, SLOT(process() ) );

    d->socket->close();
    d->isSyncing = false;
    d->connected = false;
    d->startSync = false;
    d->isConnecting = false;
    d->categories.clear();
    d->getMode = d->NotStarted;
    d->mode = d->Start;
//    emit prog( Progress(i18n("Disconnected from the device.") ) );
  mProgressItem->setComplete();
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
//        emit error( Error( i18n("<qt>Can not write the data back.\n There is no connection to the device") ) );
//        emit prog( StdProgress::done() );
        return;
    }


    AddressBookSyncee *abSyncee = list.addressBookSyncee();
    if ( abSyncee ) writeAddressbook( abSyncee );

    CalendarSyncee *calSyncee = list.calendarSyncee();
    if ( calSyncee ) {
      writeDatebook( calSyncee );
      writeTodoList( calSyncee );

      /*
       * Now write the common meta information for
       * todo/event as they're shared
       */
      OpieHelper::MetaCalendar  metaBook(calSyncee, storagePath() + "/" + d->partnerId + "/calendar_todolist.md5.qtopia" );
      metaBook.save();
    }


    /*
     * write new category information
     */
    writeCategory();
    d->helper->save();

    /*
     * Upload custom files
     */
    KSync::UnknownSyncee *unk = list.unknownSyncee();
    if ( unk )
      writeUnknown( unk );

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
//    emit prog(StdProgress::done() );

    /*
     * Delete the Syncee
     */
//    list.deleteAndClear();
}

QString QtopiaSocket::metaId() const
{
    return d->partnerId;
}

void QtopiaSocket::slotError( int )
{
  mProgressItem->setStatus( i18n( "Error during connect" ) );
    d->isSyncing = false;
    d->isConnecting = false;

//    emit error( StdError::connectionLost() );
}

void QtopiaSocket::slotConnected()
{
  mProgressItem->setStatus( i18n( "Connected" ) );
    d->connected = true;
    delete d->timer;
    d->mode = d->Start;
}

void QtopiaSocket::slotClosed()
{
  mProgressItem->setStatus( i18n( "Connecting closed" ) );
    d->connected    = false;
    d->isConnecting = false;
    d->isSyncing    = false;
//    emit error( StdError::connectionLost() );
}

void QtopiaSocket::slotNOOP()
{
    if (!d->socket ) return;
    sendCommand( "NOOP" );
}

void QtopiaSocket::process()
{
  // it can happen that the socket emitted a signal before we deleted it
  if ( d->socket == 0 )
    return;

  while ( d->socket->canReadLine() ) {
    QTextStream stream( d->socket );
    QString line = d->socket->readLine();
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
//    emit prog( Progress( i18n("Starting to sync now") ) );
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
//    emit prog(Progress(i18n("Writing AddressBook back to the device") ) );
    OpieHelper::AddressBook abDB(d->edit, d->helper, d->tz, d->device );
    KTempFile* file = abDB.fromKDE( syncee, d->extras );
    KURL uri = url( AddressBook );

    KIO::NetAccess::upload( file->name(), uri, 0 );
    file->unlink();
    delete file;


    OpieHelper::MetaAddressbook metaBook( syncee, storagePath() + "/" + d->partnerId + "/contacts.md5.qtopia" );
    metaBook.save();
}

void QtopiaSocket::writeDatebook( CalendarSyncee* syncee )
{
    OpieHelper::DateBook dbDB(d->edit, d->helper, d->tz, d->device );
    KTempFile* file = dbDB.fromKDE( syncee, d->extras );
    KURL uri = url( DateBook );

    KIO::NetAccess::upload( file->name(), uri, 0 );
    file->unlink();
    delete file;

    /*
     * The SyncHistory is saved after both datebook and todo
     * was written
     */
}

void QtopiaSocket::writeTodoList( CalendarSyncee* syncee)
{
    OpieHelper::ToDo toDB(d->edit, d->helper, d->tz, d->device );
    KTempFile* file = toDB.fromKDE( syncee, d->extras );
    KURL uri = url( TodoList );

    KIO::NetAccess::upload( file->name(), uri, 0 );
    file->unlink();
    delete file;

    /*
     * The SyncHistory is saved after both datebook and todo
     * was written
     */
}

void QtopiaSocket::writeUnknown( KSync::UnknownSyncee *syncee )
{
  for ( KSync::UnknownSyncEntry* entry = syncee->firstEntry();
        entry; entry = syncee->nextEntry() ) {
    QString baseName = QFileInfo( entry->fileName() ).fileName();

    kdDebug() << "Writing " << entry->fileName() << " "
              << d->path+"/"+baseName << endl;
    KIO::NetAccess::upload(entry->fileName(),
                           url(d->path+"/"+baseName), 0 );
  }
}

void QtopiaSocket::readAddressbook()
{
    KSync::AddressBookSyncee* syncee = 0;
//    emit prog( StdProgress::downloading(i18n("Addressbook") ) );
    QString tempfile;

    if (!downloadFile( "/Applications/addressbook/addressbook.xml", tempfile ) ) {
//        emit error( StdError::downloadError(i18n("Addressbook") ) );
        syncee = new KSync::AddressBookSyncee;
        tempfile = QString::null;
    }

//    emit prog( StdProgress::converting(i18n("Addressbook") ) );

    if (!syncee) {
        OpieHelper::AddressBook abDB( d->edit, d->helper, d->tz, d->device );
        syncee = abDB.toKDE( tempfile, d->extras );
        syncee->setMerger( d->device ? d->device->merger( OpieHelper::Device::Addressbook ) : 0l );
    }

    if (!syncee ) {
        KIO::NetAccess::removeTempFile( tempfile );
//         emit error( i18n("Cannot read the addressbook file. It is corrupted.") );
        return;
    }


    /*
     * If in meta mode but not the first syncee
     * collect some meta infos
     */
//    emit prog( Progress(i18n("Collecting the changes now") ) );

    OpieHelper::MetaAddressbook metaBook( syncee, storagePath() + "/" + d->partnerId + "/contacts.md5.qtopia" );
    metaBook.load();

    d->m_sync.append( syncee );

    if (!tempfile.isEmpty() )
        KIO::NetAccess::removeTempFile( tempfile );
}

CalendarSyncee *QtopiaSocket::defaultCalendarSyncee()
{
  CalendarSyncee* syncee = d->m_sync.calendarSyncee();
  if ( syncee == 0 ) {
    syncee = new KSync::CalendarSyncee( new KCal::CalendarLocal(KPimPrefs::timezone()) );

    /* if we've a device lets set the merger */
    syncee->setMerger( d->device ? d->device->merger( OpieHelper::Device::Calendar ) : 0);

    /*  Set title */
    syncee->setTitle( i18n("Opie") );
    syncee->setIdentifier( "Opie Todolist and Datebook" );
  }

  return syncee;
}

void QtopiaSocket::readDatebook()
{
    KSync::CalendarSyncee* syncee = defaultCalendarSyncee();
//    emit prog( StdProgress::downloading(i18n("Datebook") ) );
    QString tempfile;

    bool ok = downloadFile( "/Applications/datebook/datebook.xml", tempfile );
    if ( !ok ) {
//      emit error( StdError::downloadError(i18n("Datebook") ) );
      tempfile = QString::null;
    }
//    emit prog( StdProgress::converting(i18n("Datebook") ) );

    /* the datebook.xml might not exist in this case we created an empty Entry
     * and there is no need to parse a non existint file
     */
    if ( ok ) {
      OpieHelper::DateBook dateDB( d->edit, d->helper, d->tz, d->device );
      ok = dateDB.toKDE( tempfile, d->extras, syncee );
    }

    if ( !ok ) {
        KIO::NetAccess::removeTempFile( tempfile );
//        emit error( i18n("Cannot read the datebook file. It is corrupted.") );
        return;
    }

    /*
     * for meta mode get meta info
     */
//    emit prog( StdProgress::converting(i18n("Datebook") ) );

    /*
     * SyncHistory applying is done after both calendar and todo
     * are read and before emitting the records
     * in download() for now
     */

    if ( d->m_sync.find( syncee ) == d->m_sync.end() )
      d->m_sync.append( syncee );

    if (!tempfile.isEmpty() )
        KIO::NetAccess::removeTempFile( tempfile );
}

void QtopiaSocket::readTodoList()
{
    KSync::CalendarSyncee* syncee = defaultCalendarSyncee();
    QString tempfile;
//    emit prog( StdProgress::downloading(i18n("TodoList") ) );

    bool ok = downloadFile( "/Applications/todolist/todolist.xml", tempfile );
    if ( !ok ) {
//      emit error( StdError::downloadError(i18n("TodoList") ) );
      tempfile = QString::null;
    }

    if ( ok ) {
        OpieHelper::ToDo toDB( d->edit, d->helper, d->tz, d->device );
        ok = toDB.toKDE( tempfile, d->extras, syncee );
    }

    if ( !ok ) {
        KIO::NetAccess::removeTempFile( tempfile );
//        emit error( i18n("Cannot read the TodoList file. It is corrupted.") );
        return;
    }

//    emit prog( Progress(i18n("Collection changes for todolist") ) );

    /*
     * SyncHistory applying is done after both calendar and todo
     * are read and before emitting the records
     * in download() for now
     */

    if ( d->m_sync.find( syncee ) == d->m_sync.end() )
      d->m_sync.append( syncee );

    if (!tempfile.isEmpty() )
        KIO::NetAccess::removeTempFile( tempfile );
}

void QtopiaSocket::start( const QString& line )
{
    if ( line.left(3) != QString::fromLatin1("220") ) {
//        emit error( Error(i18n("The device returned bogus data. giving up now.") ) );
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
//    emit prog( StdProgress::connected() );
//    emit prog( StdProgress::authentication() );
    if ( line.left(3) != QString::fromLatin1("331") ) {
//        emit error( StdError::wrongUser( d->device->user() ) );
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
//        emit error( StdError::wrongPassword() );
        // wrong password
        d->socket->close();
        d->mode = d->Done;
        d->connected    = false;
        d->isConnecting = false;
    } else {
//        emit prog( StdProgress::authenticated() );
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
//        emit prog( Progress(i18n("Getting the Document Links of the Document Tab") ) );
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
//        emit prog( Progress( i18n( "Flushed " ) + appName ) );
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
   * As Calendar and Todo are shared in one Syncee we need
   * to do the sync information getting and applying here
   *
   */
  KSync::CalendarSyncee* syncee = defaultCalendarSyncee();
  OpieHelper::MetaCalendar  metaBook(syncee, storagePath() + "/" + d->partnerId + "/calendar_todolist.md5.qtopia" );
  metaBook.load();
  kdDebug(5227) << "Did Meta " << endl;
  outputIt(5227, syncee );


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
//    emit prog( StdProgress::downloading("Categories.xml") );
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
    QString pref = KPimPrefs::timezone();
    d->tz = pref.isEmpty() ?
            QString::fromLatin1("Europe/London") : pref;
    kdDebug() << "TimeZone is " << d->tz << endl;
}

bool QtopiaSocket::downloadFile( const QString& str, QString& dest )
{
    KURL uri = url( d->path + str );
    bool b = KIO::NetAccess::download( uri, dest, 0 );
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


  QTextStream stream( d->socket );
  stream << cmd << endl;
}

namespace {

void outputAll( int area, QPtrList<SyncEntry> list )
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
    outputAll( area, s->added() );

    kdDebug(area) << "Modified " <<endl;
    outputAll( area, s->modified() );

    kdDebug(area) << "Removed " << endl;
    outputAll( area, s->removed() );
}

}

#include "socket.moc"
