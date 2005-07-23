/*
    This file is part of libksync.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "synctesthelper.h"

#include "syncer.h"
#include "calendarsyncee.h"

#include <libkcal/calendarlocal.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include <qfile.h>
#include <qfileinfo.h>

#include <iostream>

using namespace KCal;
using namespace KSync;

static const KCmdLineOptions options[] =
{
  { "verbose", "Verbose output", 0 },
  { "+outputdir", "Name of output directory", 0 },
  KCmdLineLastOption
};

int main( int argc, char **argv )
{
  KAboutData aboutData( "synctest2", "libksync test 2", "0.1" );
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options );

  KApplication app( false, false );

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  if ( args->count() != 1 ) {
    args->usage( "Wrong number of arguments." );
  }

  QString outputDir = QFile::decodeName( args->arg( 0 ) );

  QDateTime dt1 = QDateTime( QDate( 2000, 1, 1 ), QTime( 12, 0 ) );
  QDateTime dt2 = QDateTime( QDate( 2000, 2, 1 ), QTime( 12, 0 ) );
  QDateTime dt3 = QDateTime( QDate( 2000, 3, 1 ), QTime( 12, 0 ) );

  SyncTestHelper helper( outputDir );

  // Force save() to save in sorted order
  extern bool KCal_CalendarLocal_saveOrdered;
  KCal_CalendarLocal_saveOrdered = true;

  CalendarLocal cal1;
  CalendarLocal cal2;

  Event *event1 = new Event;
  event1->setUid( "SYNCTEST1_EVENT_1" );
  event1->setSummary( "Event 1" );
  event1->setDtStart( QDateTime( QDate( 2004, 2, 15 ), QTime( 12, 0 ) ) );
  event1->setDtEnd( QDateTime( QDate( 2004, 2, 15 ), QTime( 13, 0 ) ) );
  event1->setFloats( false );
  event1->setLastModified( dt1 );

  cal1.addEvent( event1 );

  Event *event2 = new Event;
  event2->setUid( "SYNCTEST1_EVENT_2" );
  event2->setSummary( "Event 2" );
  event2->setDtStart( QDateTime( QDate( 2004, 2, 15 ), QTime( 14, 0 ) ) );
  event2->setDtEnd( QDateTime( QDate( 2004, 2, 15 ), QTime( 15, 0 ) ) );
  event2->setFloats( false );
  event2->setLastModified( dt1 );

  cal2.addEvent( event2 );

  helper.sync( &cal1, &cal2, "011", "Calendar, sync new, with history." );

  event1->setSummary( "Modified event 1" );
  event1->setLastModified( dt2 );

  helper.sync( &cal1, &cal2, "012", "Calendar, sync changed 1, with history" );

  event2->setSummary( "Modified event 2" );
  event2->setLastModified( dt3 );

  helper.sync( &cal1, &cal2, "013", "Calendar, sync changed 2, with history" );

  return 0;
}
