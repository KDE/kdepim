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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "opengroupware.h"
#include "webdavhandler.h"

#include <kdebug.h>
#include <kurl.h>
#include <kio/job.h>
#include <kio/davjob.h>
#include <klocale.h>

#include <libkdepim/kabcresourcecached.h>

#include <libkcal/freebusy.h>
#include <libkcal/icalformat.h>
#include <libkcal/scheduler.h>
#include <libkcal/calendarlocal.h>

#include <kabc/vcardconverter.h>

#include <kinstance.h>
#include <kdebug.h>
#include <klocale.h>
#include <ktempfile.h>

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

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
int kdemain( int argc, char **argv );
}

int kdemain( int argc, char **argv )
{
  KInstance instance( "kio_OpenGroupware" );
  
  kdDebug(7000) << "Starting kio_OpenGroupware(pid:  " << getpid() << ")" << endl;
  
  if (argc != 4) {
    fprintf( stderr, "Usage: kio_OpenGroupware protocol domain-socket1 domain-socket2\n");
    exit( -1 );
  }
  
  OpenGroupware slave( argv[1], argv[2], argv[3] );
  slave.dispatchLoop();
  
  return 0;
}

OpenGroupware::OpenGroupware( const QCString &protocol, const QCString &pool,
  const QCString &app )
  : SlaveBase( protocol, pool, app )
{
}

void OpenGroupware::get( const KURL &url )
{
  kdDebug(7000) << "OpenGroupware::get()" << endl;
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

  if ( path.startsWith( "/freebusy/" ) ) {
    getFreeBusy( url );
  } else if ( path.startsWith( "/calendar/" ) ) {
    getCalendar( url );
  } else if ( path.startsWith( "/addressbook/" ) ) {
    getAddressbook( url );
  } else {
    QString error = i18n("Unknown path. Known paths are '/freebusy/', "
      "'/calendar/' and '/addressbook/'.");
    errorMessage( error );
  }
  
  kdDebug(7000) << "OpenGroupwareCgiProtocol::get() done" << endl;
}

void OpenGroupware::getFreeBusy( const KURL &url )
{
  QString file = url.filename();
  if ( file.right( 4 ) != ".ifb" ) {
    QString error = i18n("Illegal filename. File has to have '.ifb' suffix.");
    errorMessage( error );
  } else {
    QString email = file.left( file.length() - 4 );
    debugMessage( "Email: " + email );

    QString user = url.user();
    QString pass = url.pass();

    debugMessage( "URL: "  );
    debugMessage( "User: " + user );
    debugMessage( "Password: " + pass );

    KCal::FreeBusy *fb = new KCal::FreeBusy;

    if ( user.isEmpty() || pass.isEmpty() ) {
      errorMessage( i18n("Need username and password.") );
    } else {
      // FIXME get from server

      // FIXME: Read range from configuration or URL parameters.
      QDate start = QDate::currentDate().addDays( -3 );
      QDate end = QDate::currentDate().addDays( 60 );

      fb->setDtStart( start );
      fb->setDtEnd( end );

      kdDebug() << "Login" << endl;

    }

#if 0
    QDateTime s = QDateTime( QDate( 2004, 9, 27 ), QTime( 10, 0 ) );
    QDateTime e = QDateTime( QDate( 2004, 9, 27 ), QTime( 11, 0 ) );

    fb->addPeriod( s, e );
#endif

    KCal::ICalFormat format;

    QString ical = format.createScheduleMessage( fb, KCal::Scheduler::Publish );

    data( ical.utf8() );

    finished();
  }
}


void OpenGroupware::getCalendar( const KURL &_url )
{

  KURL url( _url ); // we'll be changing it
  QString user = url.user();
  QString pass = url.pass();

  QDomDocument props = WebdavHandler::createAllPropsRequest();

  debugMessage( "URL: "  );
  debugMessage( "User: " + user );
  debugMessage( "Password: " + pass );

  url.setProtocol( "webdav" );
  url.setPath ( "/zidestore/dav/till/" );

  kdDebug(7000) << "getCalendar: " << url.prettyURL() << endl;

  // FIXME do progress handling
  mListEventsJob = KIO::davPropFind( url, props, "0", false );
  connect( mListEventsJob, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotGetCalendarListingResult( KIO::Job * ) ) );
}

void OpenGroupware::getAddressbook( const KURL &url )
{
  
}

void OpenGroupware::errorMessage( const QString &msg )
{
  error( KIO::ERR_SLAVE_DEFINED, msg );
}

void OpenGroupware::debugMessage( const QString &msg )
{
#if 0
  data( ( msg + "\n" ).utf8() );
#else
  Q_UNUSED( msg );
#endif
}


void OpenGroupware::slotGetCalendarListingResult( KIO::Job *job )
{
  
  kdDebug(7000) << k_funcinfo << endl;

  if (  job->error() ) {
    job->showErrorDialog(  0 );
  } else {
    kdDebug() << "ResourceSlox::slotResult() success" << endl;

    QDomDocument doc = mListEventsJob->response();

  }
  KCal::ICalFormat format;
  KCal::CalendarLocal calendar;

  QString ical = format.toString( &calendar );

  data( ical.utf8() );

  finished();
}


void OpenGroupware::slotGetCalendarResult( KIO::Job *job )
{
  Q_UNUSED( job );
}
#include "opengroupware.moc"

