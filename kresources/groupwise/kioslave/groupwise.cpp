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

#include "groupwise.h"

#include <libkcal/freebusy.h>
#include <libkcal/icalformat.h>
#include <libkcal/scheduler.h>

#include <kinstance.h>
#include <kdebug.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

extern "C" {
int kdemain( int argc, char **argv );
}

int kdemain( int argc, char **argv )
{
  KInstance instance( "kio_groupwise" );
  
  kdDebug(7000) << "Starting kio_groupwise(pid:  " << getpid() << ")" << endl;
  
  if (argc != 4) {
    fprintf( stderr, "Usage: kio_groupwise protocol domain-socket1 domain-socket2\n");
    exit( -1 );
  }
  
  Groupwise slave( argv[2], argv[3] );
  slave.dispatchLoop();
  
  return 0;
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

  KCal::FreeBusy *fb = new KCal::FreeBusy;
  
  QDateTime start = QDateTime( QDate( 2004, 9, 27 ), QTime( 10, 0 ) );
  QDateTime end = QDateTime( QDate( 2004, 9, 27 ), QTime( 11, 0 ) );
  
  fb->addPeriod( start, end );

  KCal::ICalFormat format;

  QString ical = format.createScheduleMessage( fb, KCal::Scheduler::Publish );
 
  data( ical.utf8() );
  
  finished();
  
  kdDebug(7000) << "GroupwiseCgiProtocol::get() done" << endl;
}

Groupwise::Groupwise( const QCString &pool, const QCString &app )
: SlaveBase( "groupwise", pool, app )
{
}


