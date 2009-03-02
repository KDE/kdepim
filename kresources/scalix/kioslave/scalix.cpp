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

#include <QtCore/QEventLoop>
#include <QtCore/QFile>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kio/global.h>
#include <klocale.h>

#include <stdlib.h>

#include "scalix.h"

extern "C" {
  KDE_EXPORT int kdemain( int argc, char **argv );
}

int kdemain( int argc, char **argv )
{
  putenv( strdup( "SESSION_MANAGER=" ) );

  KCmdLineArgs::init(argc, argv, "kio_scalix", 0, KLocalizedString(), 0);
  KCmdLineOptions options;
  options.add("+protocol", ki18n( "Protocol name" ));
  options.add("+pool", ki18n( "Socket name" ));
  options.add("+app", ki18n( "Socket name" ));
  KCmdLineArgs::addCmdLineOptions( options );
  KApplication app( false );

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  Scalix slave( QFile::encodeName( args->arg(0) ),
                QFile::encodeName( args->arg(1) ),
                QFile::encodeName( args->arg(2) ) );
  slave.dispatchLoop();

  return 0;
}

Scalix::Scalix( const QByteArray &protocol, const QByteArray &pool, const QByteArray &app )
  : SlaveBase( protocol, pool, app )
{
}

void Scalix::enterLoop()
{
  QEventLoop eventLoop;
  connect(this, SIGNAL(leaveModality()), &eventLoop, SLOT(quit()));
  eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
}

void Scalix::get( const KUrl &url )
{
  mimeType( "text/plain" );

  QString path = url.path();

  if ( path.contains( "/freebusy/" ) ) {
    retrieveFreeBusy( url );
  } else {
    error( KIO::ERR_SLAVE_DEFINED, i18n( "Unknown path. Known path is '/freebusy/'" ) );
  }
}

void Scalix::put( const KUrl& url, int, bool, bool )
{
  QString path = url.path();

  if ( path.contains( "/freebusy/" ) ) {
    publishFreeBusy( url );
  } else {
    error( KIO::ERR_SLAVE_DEFINED, i18n( "Unknown path. Known path is '/freebusy/'" ) );
  }
}

void Scalix::retrieveFreeBusy( const KUrl &url )
{
  /**
   * The url is of the following form:
   *  scalix://user:password@host/freebusy/user@domain.ifb
   */

  // Extract user@domain (e.g. everything between '/freebusy/' and '.ifb')
  const QString requestUser = url.path().mid( 10, url.path().length() - 14 );

  QByteArray packedArgs;
  QDataStream stream( &packedArgs, QIODevice::WriteOnly );

  const QString argument = QString( "BEGIN:VFREEBUSY\nATTENDEE:MAILTO:%1\nEND:VFREEBUSY" ).arg( requestUser );
  const QString command = QString( "X-GET-ICAL-FREEBUSY {%1}" ).arg( argument.length() );

  stream << (int) 'X' << 'E' << command << argument;

  QString imapUrl = QString( "imap://%1@%3/" ).arg( url.pass().isEmpty() ?
                                                    url.user() : url.user() + ':' + url.pass() ).arg( url.host() );

  mFreeBusyData = QString();

  KIO::SimpleJob *job = KIO::special( imapUrl, packedArgs, false );
  connect( job, SIGNAL( infoMessage( KIO::Job*, const QString& ) ),
           this, SLOT( slotInfoMessage( KIO::Job*, const QString& ) ) );
  connect( job, SIGNAL( result( KIO::Job* ) ),
           this, SLOT( slotRetrieveResult( KIO::Job* ) ) );

  enterLoop();
}

void Scalix::publishFreeBusy( const KUrl &url )
{
  /**
   * The url is of the following form:
   *  scalix://user:password@host/freebusy/path/to/calendar/user@domain
   */
  QString requestUser, calendar;
  QString path = url.path();

  // extract user name
  int lastSlash = path.lastIndexOf( '/' );
  if ( lastSlash != -1 )
    requestUser = path.mid( lastSlash + 1 );

  // extract calendar name
  int secondSlash = path.indexOf( '/', 1 );
  if ( secondSlash != -1 )
    calendar = path.mid( secondSlash + 1, lastSlash - secondSlash - 1 );

  if ( requestUser.isEmpty() || calendar.isEmpty() ) {
    error( KIO::ERR_SLAVE_DEFINED, i18n( "No user or calendar given." ) );
    return;
  };

  // read freebusy information
  QByteArray data;
  while ( true ) {
    dataReq();

    QByteArray buffer;
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

  QByteArray packedArgs;
  QDataStream stream( &packedArgs, QIODevice::WriteOnly );

  const QString argument = QString::fromUtf8( data );
  const QString command = QString( "X-PUT-ICAL-FREEBUSY Calendar {%1}" ).arg( argument.length() );

  stream << (int) 'X' << 'E' << command << argument;

  QString imapUrl = QString( "imap://%1@%3/" ).arg( url.pass().isEmpty() ?
                                                    url.user() : url.user() + ':' + url.pass() )
                                                .arg( url.host() );

  KIO::SimpleJob *job = KIO::special( imapUrl, packedArgs, false );
  connect( job, SIGNAL( result( KIO::Job* ) ),
           this, SLOT( slotPublishResult( KIO::Job* ) ) );

  enterLoop();
}

void Scalix::slotInfoMessage( KIO::Job *job, const QString &data )
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
    data( mFreeBusyData.toUtf8() );
    finished();
  }

  emit leaveModality();
}

void Scalix::slotPublishResult( KIO::Job *job )
{
  if ( job->error() ) {
    error( KIO::ERR_SLAVE_DEFINED, job->errorString() );
  } else {
    finished();
  }

  emit leaveModality();
}

#include "scalix.moc"
