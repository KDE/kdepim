/*
    This file is part of KDE.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


#include "groupwiseserver.h"

#include <qregexp.h>

#include <libkdepim/kabcresourcecached.h>

#include <libkcal/freebusy.h>
#include <libkcal/icalformat.h>
#include <libkcal/scheduler.h>
#include <libkcal/calendarlocal.h>

#include <kabc/addressee.h>
#include <kabc/vcardconverter.h>

#include <kinstance.h>
#include <kio/global.h>
#include <kdebug.h>
#include <klocale.h>
#include <ktempfile.h>
#include <kdeversion.h>

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#include <kdepimmacros.h>

#include "groupwise.h"

namespace KABC {

class ResourceMemory : public ResourceCached
{
  public:
    ResourceMemory() : ResourceCached( 0 ) {}
    
    Ticket *requestSaveTicket() { return 0; }
    bool load() { return true; }
    bool save( Ticket * ) { return true; }
    void releaseSaveTicket( Ticket * ) {}
};

}

extern "C" {
KDE_EXPORT int kdemain( int argc, char **argv );
}

int kdemain( int argc, char **argv )
{
  KInstance instance( "kio_groupwise" );
  
  kdDebug(7000) << "Starting kio_groupwise(pid:  " << getpid() << ")" << endl;
  
  if (argc != 4) {
    fprintf( stderr, "Usage: kio_groupwise protocol domain-socket1 domain-socket2\n");
    exit( -1 );
  }
  
  Groupwise slave( argv[1], argv[2], argv[3] );
  slave.dispatchLoop();
  
  return 0;
}

Groupwise::Groupwise( const QCString &protocol, const QCString &pool,
  const QCString &app )
  : SlaveBase( protocol, pool, app )
{
}

void Groupwise::get( const KURL &url )
{
  kdDebug(7000) << "Groupwise::get()" << endl;
  kdDebug(7000) << " URL: " << url.url() << endl;
  #if 1
  kdDebug(7000) << " Path: " << url.path() << endl;
  kdDebug(7000) << " Query: " << url.query() << endl;
  kdDebug(7000) << " Protocol: " << url.protocol() << endl;
  kdDebug(7000) << " Filename: " << url.filename() << endl;
  #endif

  mimeType( "text/plain" );

  QString path = url.path();
  debugMessage( "Path: " + path );

  if ( path.contains( "/freebusy" ) ) {
    getFreeBusy( url );
  } else if ( path.contains( "/calendar" ) ) {
    getCalendar( url );
  } else if ( path.contains( "/addressbook" ) ) {
    if ( url.query().contains( "update=true" ) )
      updateAddressbook( url );
    else
      getAddressbook( url );
  } else {
    QString error = i18n("Unknown path. Known paths are '/freebusy/', "
        "'/calendar/' and '/addressbook/'.") + QString(" path was %1" ).arg( url.url() );
    errorMessage( error );
  }
  
  kdDebug(7000) << "Groupwise::get() done" << endl;
}

QString Groupwise::soapUrl( const KURL &url )
{
  // FIXME: Get SSL from parameter
  bool useSsl = url.protocol() == "groupwises";

  QString u;
  if ( useSsl ) u = "https";
  else u = "http";
  
  u += "://" + url.host() + ":";
  if ( url.port() ) 
    u += QString::number( url.port() );
  else {
    u += "7191";
  }

  // check for a soap path in the URL
  // assume that if a path to soap is included in the URL,
  // it will be at the start of the path, eg.
  // groupwise://host:port/soap2/freebusy
  if ( ! ( url.path().startsWith("/freebusy/") ||
           url.path().startsWith("/calendar/") ||
           url.path().startsWith("/addressbook/" ) ) )
  {
    QString soapPath = QString("/") + QStringList::split('/', url.path())[0];
    u += soapPath;
  }
  else
    u += "/soap";

  return u;
}

void Groupwise::getFreeBusy( const KURL &url )
{
  QString file = url.filename();
  if ( file.right( 4 ) != ".ifb" ) {
    QString error = i18n("Illegal filename. File has to have '.ifb' suffix.");
    errorMessage( error );
  } else {
    QString email = file.left( file.length() - 4 );
    debugMessage( "Email: " + email );

    // Sanitise local Nuernberg email addresses
    kdDebug() << "Email before sanitizing: " << email << endl;
    email = email.replace(QRegExp("\\.EMEA5-1\\.EMEA5" ), "" );
    email = email.replace(QRegExp("\\.Suse.INTERNET" ), "" );
    kdDebug() << "Email after sanitizing: " << email << endl;

    QString u = soapUrl( url );

    QString user = url.user();
    QString pass = url.pass();

    debugMessage( "URL: " + u );
    debugMessage( "User: " + user );
    debugMessage( "Password: " + pass );

    KCal::FreeBusy *fb = new KCal::FreeBusy;

    if ( user.isEmpty() || pass.isEmpty() ) {
      errorMessage( i18n("Need username and password to read Free/Busy information.") );
    } else {
      GroupwiseServer server( u, user, pass, 0 );

      // FIXME: Read range from configuration or URL parameters.
      QDate start = QDate::currentDate().addDays( -3 );
      QDate end = QDate::currentDate().addDays( 60 );

      fb->setDtStart( start );
      fb->setDtEnd( end );

      kdDebug() << "Login" << endl;

      if ( !server.login() ) {
        errorMessage( i18n("Unable to login: ") + server.errorText() );
      } else {
        kdDebug() << "Read free/busy" << endl;
        if ( !server.readFreeBusy( email, start, end, fb ) ) {
          errorMessage( i18n("Unable to read free/busy data: ") + server.errorText() );
        }
        kdDebug() << "Read free/busy" << endl;
        server.logout();
      }
    }

#if 0
    QDateTime s = QDateTime( QDate( 2004, 9, 27 ), QTime( 10, 0 ) );
    QDateTime e = QDateTime( QDate( 2004, 9, 27 ), QTime( 11, 0 ) );

    fb->addPeriod( s, e );
#endif

    // FIXME: This does not take into account the time zone!
    KCal::ICalFormat format;

    QString ical = format.createScheduleMessage( fb, KCal::Scheduler::Publish );

    data( ical.utf8() );

    finished();
  }
}

void Groupwise::getCalendar( const KURL &url )
{
  QString u = soapUrl( url );

  QString user = url.user();
  QString pass = url.pass();

  debugMessage( "URL: " + u );
  debugMessage( "User: " + user );
  debugMessage( "Password: " + pass );

  GroupwiseServer server( u, user, pass, 0 );

  KCal::CalendarLocal calendar( QString::fromLatin1("UTC"));

  kdDebug() << "Login" << endl;
  if ( !server.login() ) {
    errorMessage( i18n("Unable to login: ") + server.errorText() );
  } else {
    kdDebug() << "Read calendar" << endl;
    if ( !server.readCalendarSynchronous( &calendar ) ) {
      errorMessage( i18n("Unable to read calendar data: ") + server.errorText() );
    }
    kdDebug() << "Logout" << endl;
    server.logout();
  }

  KCal::ICalFormat format;

  QString ical = format.toString( &calendar );

  data( ical.utf8() );

  finished();
}

void Groupwise::getAddressbook( const KURL &url )
{
  QString u = soapUrl( url );

  QString user = url.user();
  QString pass = url.pass();

  debugMessage( "URL: " + u );
  debugMessage( "User: " + user );
  debugMessage( "Password: " + pass );

  QString query = url.query();
  if ( query.isEmpty() || query == "?" ) {
    errorMessage( i18n("No addressbook IDs given.") );
  } else {
    QStringList ids;

    query = query.mid( 1 );
    QStringList queryItems = QStringList::split( "&", query );
    QStringList::ConstIterator it;
    for( it = queryItems.begin(); it != queryItems.end(); ++it ) {
      QStringList item = QStringList::split( "=", (*it) );
      if ( item.count() == 2 && item[ 0 ] == "addressbookid" ) {
        ids.append( item[ 1 ] );
      }
    }
    
    debugMessage( "IDs: " + ids.join( "," ) );

    GroupwiseServer server( u, user, pass, 0 );

    connect( &server, SIGNAL( readAddressBookTotalSize( int ) ),
      SLOT( slotReadAddressBookTotalSize( int ) ) );
    connect( &server, SIGNAL( readAddressBookProcessedSize( int ) ),
      SLOT( slotReadAddressBookProcessedSize( int ) ) );
    connect( &server, SIGNAL( errorMessage( const QString &, bool ) ),
      SLOT( slotServerErrorMessage( const QString &, bool ) ) );
    connect( &server, SIGNAL( gotAddressees( const KABC::Addressee::List ) ),
      SLOT( slotReadReceiveAddressees( const KABC::Addressee::List ) ) );

    kdDebug() << "Login" << endl;
    if ( !server.login() ) {
      errorMessage( i18n("Unable to login: ") + server.errorText() );
    } else {
      kdDebug() << "Read Addressbook" << endl;
      if ( !server.readAddressBooksSynchronous( ids ) ) {
        errorMessage( i18n("Unable to read addressbook data: ") + server.errorText() );
      }
      kdDebug() << "Logout" << endl;
      server.logout();
      finished();
    }
  }
}

void Groupwise::slotReadReceiveAddressees( const KABC::Addressee::List addressees )
{
    kdDebug() << "Groupwise::slotReadReceiveAddressees() - passing " << addressees.count() << " contacts back to application" << endl;
    KABC::VCardConverter conv;

    QString vcard = conv.createVCards( addressees );

    data( vcard.utf8() );
}

void Groupwise::updateAddressbook( const KURL &url )
{
  kdDebug() << "Groupwise::updateAddressbook() " << url << endl;
  QString u = soapUrl( url );

  QString user = url.user();
  QString pass = url.pass();

  debugMessage( "update AB URL: " + u );
  debugMessage( "update AB User: " + user );
  debugMessage( "update AB Password: " + pass );

  QString query = url.query();

  unsigned long lastSequenceNumber = 0;
  unsigned long lastPORebuildTime = 0;

  if ( query.isEmpty() || query == "?" ) {
    errorMessage( i18n("No addressbook IDs given.") );
    return;
  } else {
    QStringList ids;

    query = query.mid( 1 );
    QStringList queryItems = QStringList::split( "&", query );
    QStringList::ConstIterator it;
    for( it = queryItems.begin(); it != queryItems.end(); ++it ) {
      QStringList item = QStringList::split( "=", (*it) );
      if ( item.count() == 2 && item[ 0 ] == "addressbookid" ) {
        ids.append( item[ 1 ] );
      }
      if ( item.count() == 2 && item[ 0 ] == "lastSeqNo" )
        lastSequenceNumber = item[ 1 ].toULong();
      if ( item.count() == 2 && item[ 0 ] == "PORebuildTime" )
        lastPORebuildTime = item[ 1 ].toULong();
    }
    
    debugMessage( "update IDs: " + ids.join( "," ) );

    GroupwiseServer server( u, user, pass, 0 );
    connect( &server, SIGNAL( errorMessage( const QString &, bool ) ),
      SLOT( slotServerErrorMessage( const QString &, bool ) ) );
    connect( &server, SIGNAL( gotAddressees( const KABC::Addressee::List ) ),
      SLOT( slotReadReceiveAddressees( const KABC::Addressee::List ) ) );

    kdDebug() << "  Login" << endl;
    if ( !server.login() ) {
      errorMessage( i18n("Unable to login: ") + server.errorText() );
    } else {
      kdDebug() << "  Updating Addressbook" << endl;
      if ( !server.updateAddressBooks( ids, lastSequenceNumber + 1, lastPORebuildTime ) )
      {
        error( KIO::ERR_NO_CONTENT, server.errorText() );
        //errorMessage( i18n("Unable to update addressbook data: ") + server.errorText() );
      }
      kdDebug() << "  Logout" << endl;
      server.logout();
      finished();
    }
  }
}

void Groupwise::errorMessage( const QString &msg )
{
  error( KIO::ERR_SLAVE_DEFINED, msg );
}

void Groupwise::debugMessage( const QString &msg )
{
#if 0
  data( ( msg + "\n" ).utf8() );
#else
  Q_UNUSED( msg );
#endif
}

void Groupwise::slotReadAddressBookTotalSize( int size )
{
  totalSize( size );
}

void Groupwise::slotReadAddressBookProcessedSize( int size )
{
  kdDebug() << "Groupwise::processedSize(): " << size << endl;
  processedSize( size );
}

void Groupwise::slotServerErrorMessage( const QString & serverErrorMessage, bool fatal )
{
  kdDebug() << "Groupwise::slotJobErrorMessage()" << serverErrorMessage << ( fatal ? ", FATAL!" : ", proceeding" ) << endl;
  errorMessage( i18n( "An error occurred while communicating with the GroupWise server:\n%1" ).arg( serverErrorMessage ) );
}

#include "groupwise.moc"

