/*
    This file is part of KDE.

    Copyright (C) 2007 Trolltech ASA. All rights reserved.

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

#include <tqapplication.h>
#include <tqeventloop.h>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kio/global.h>
#include <klocale.h>

#include <kdepimmacros.h>

#include <stdlib.h>

#include "scalix.h"

extern "C" {
  KDE_EXPORT int kdemain( int argc, char **argv );
}

static const KCmdLineOptions options[] =
{
  { "+protocol", I18N_NOOP( "Protocol name" ), 0 },
  { "+pool", I18N_NOOP( "Socket name" ), 0 },
  { "+app", I18N_NOOP( "Socket name" ), 0 },
  KCmdLineLastOption
};

int kdemain( int argc, char **argv )
{
  putenv( strdup( "SESSION_MANAGER=" ) );
  KApplication::disableAutoDcopRegistration();

  KCmdLineArgs::init( argc, argv, "kio_scalix", 0, 0, 0, 0 );
  KCmdLineArgs::addCmdLineOptions( options );
  KApplication app( false, false );

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  Scalix slave( args->arg( 0 ), args->arg( 1 ), args->arg( 2 ) );
  slave.dispatchLoop();

  return 0;
}

Scalix::Scalix( const TQCString &protocol, const TQCString &pool, const TQCString &app )
  : SlaveBase( protocol, pool, app )
{
}

void Scalix::get( const KURL &url )
{
  mimeType( "text/plain" );

  TQString path = url.path();

  if ( path.contains( "/freebusy/" ) ) {
    retrieveFreeBusy( url );
  } else {
    error( KIO::ERR_SLAVE_DEFINED, i18n( "Unknown path. Known path is '/freebusy/'" ) );
  }
}

void Scalix::put( const KURL& url, int, bool, bool )
{
  TQString path = url.path();

  if ( path.contains( "/freebusy/" ) ) {
    publishFreeBusy( url );
  } else {
    error( KIO::ERR_SLAVE_DEFINED, i18n( "Unknown path. Known path is '/freebusy/'" ) );
  }
}

void Scalix::retrieveFreeBusy( const KURL &url )
{
  /**
   * The url is of the following form:
   *  scalix://user:password@host/freebusy/user@domain.ifb
   */

  // Extract user@domain (e.g. everything between '/freebusy/' and '.ifb')
  const TQString requestUser = url.path().mid( 10, url.path().length() - 14 );

  TQByteArray packedArgs;
  TQDataStream stream( packedArgs, IO_WriteOnly );

  const TQString argument = TQString( "BEGIN:VFREEBUSY\nATTENDEE:MAILTO:%1\nEND:VFREEBUSY" ).arg( requestUser );
  const TQString command = TQString( "X-GET-ICAL-FREEBUSY {%1}" ).arg( argument.length() );

  stream << (int) 'X' << 'E' << command << argument;

  TQString imapUrl = TQString( "imap://%1@%3/" ).arg( url.pass().isEmpty() ?
                                                   url.user() : url.user() + ":" + url.pass() )
                                                .arg( url.host() );

  mFreeBusyData = TQString();

  KIO::SimpleJob *job = KIO::special( imapUrl, packedArgs, false );
  connect( job, TQT_SIGNAL( infoMessage( KIO::Job*, const TQString& ) ),
           this, TQT_SLOT( slotInfoMessage( KIO::Job*, const TQString& ) ) );
  connect( job, TQT_SIGNAL( result( KIO::Job* ) ),
           this, TQT_SLOT( slotRetrieveResult( KIO::Job* ) ) );

  qApp->eventLoop()->enterLoop();
}

void Scalix::publishFreeBusy( const KURL &url )
{
  /**
   * The url is of the following form:
   *  scalix://user:password@host/freebusy/path/to/calendar/user@domain
   */
  TQString requestUser, calendar;
  TQString path = url.path();

  // extract user name
  int lastSlash = path.findRev( '/' );
  if ( lastSlash != -1 )
    requestUser = path.mid( lastSlash + 1 );

  // extract calendar name
  int secondSlash = path.find( '/', 1 );
  if ( secondSlash != -1 )
    calendar = path.mid( secondSlash + 1, lastSlash - secondSlash - 1 );

  if ( requestUser.isEmpty() || calendar.isEmpty() ) {
    error( KIO::ERR_SLAVE_DEFINED, i18n( "No user or calendar given!" ) );
    return;
  };

  // read freebusy information
  TQByteArray data;
  while ( true ) {
    dataReq();

    TQByteArray buffer;
    const int newSize = readData(buffer);
    if ( newSize < 0 ) {
      // read error: network in unknown state so disconnect
      error( KIO::ERR_COULD_NOT_READ, i18n("KIO data supply error.") );
      return;
    }

    if ( newSize == 0 )
      break;

    unsigned int oldSize = data.size();
    data.resize( oldSize + buffer.size() );
    memcpy( data.data() + oldSize, buffer.data(), buffer.size() );
  }

  TQByteArray packedArgs;
  TQDataStream stream( packedArgs, IO_WriteOnly );

  const TQString argument = TQString::fromUtf8( data );
  const TQString command = TQString( "X-PUT-ICAL-FREEBUSY Calendar {%1}" ).arg( argument.length() );

  stream << (int) 'X' << 'E' << command << argument;

  TQString imapUrl = TQString( "imap://%1@%3/" ).arg( url.pass().isEmpty() ?
                                                   url.user() : url.user() + ":" + url.pass() )
                                                .arg( url.host() );

  KIO::SimpleJob *job = KIO::special( imapUrl, packedArgs, false );
  connect( job, TQT_SIGNAL( result( KIO::Job* ) ),
           this, TQT_SLOT( slotPublishResult( KIO::Job* ) ) );

  qApp->eventLoop()->enterLoop();
}

void Scalix::slotInfoMessage( KIO::Job *job, const TQString &data )
{
  if ( job->error() ) {
    // error is handled in slotResult
    return;
  }

  mFreeBusyData = data;
}


void Scalix::slotRetrieveResult( KIO::Job *job )
{
  if ( job->error() ) {
    error( KIO::ERR_SLAVE_DEFINED, job->errorString() );
  } else {
    data( mFreeBusyData.utf8() );
    finished();
  }

  qApp->eventLoop()->exitLoop();
}

void Scalix::slotPublishResult( KIO::Job *job )
{
  if ( job->error() ) {
    error( KIO::ERR_SLAVE_DEFINED, job->errorString() );
  } else {
    finished();
  }

  qApp->eventLoop()->exitLoop();
}

#include "scalix.moc"
