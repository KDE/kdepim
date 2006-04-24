/*
    This file is part of libkholidays.
    Copyright (c) 2004,2006 Allen Winter <winter@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "kde-holidays.h"
#include "kde-holidays_parser.h"

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kdebug.h>

#include <qfile.h>
#include <qtextstream.h>

#include <iostream>
using namespace std;

static const KCmdLineOptions options[] =
{
  { "+file", "Name of holiday XML file", 0 },
  KCmdLineLastOption
};

//void displaySpec( Datespec *d )
//{
//}

void displayHolidays( const QList<Holiday> holidays )
{
  foreach( Holiday h, holidays ) {
    cout << "NAME: " << h.name().local8Bit().data() << endl;
    cout << "  TYPE: " << h.type().local8Bit().data() << endl;
    if ( !h.description().isEmpty() )
      cout << "  DESC: " << h.description().local8Bit().data() << endl;
    else
      cout << "  DESC: " << "(none)" << endl;
//    displaySpec( h.datespec() );
    cout << endl;
  }
}

int main( int argc, char **argv )
{
  KApplication::disableAutoDcopRegistration();
  KAboutData aboutData( "dumpholidaylist", "Dump XML holiday list to stdout",
                        "0.1" );
  KCmdLineArgs::init( argc, argv, &aboutData, KCmdLineArgs::CmdLineArgNone );
  KCmdLineArgs::addCmdLineOptions( options );

  KApplication app( false );

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  if ( args->count() != 1 ) {
    args->usage( "Wrong number of arguments." );
  }

  QString filename = QFile::decodeName( args->arg( 0 ) );

  HolidayCalendarParser parser;

  bool ok;
  HolidayCalendar holcalendar = parser.parseFile( filename, &ok );

  if ( ok ) {
    displayHolidays( holcalendar.holidayList() );
  } else {
    kError() << "Parse error" << endl;
  }

  QString out = filename + ".out";
  if ( !holcalendar.writeFile( out ) ) {
    kError() << "Write error" << endl;
  }
}
