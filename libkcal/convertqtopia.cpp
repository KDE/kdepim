/*
    This file is part of libkcal.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include "calendarlocal.h"
#include "icalformat.h"
#include "qtopiaformat.h"

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kstandarddirs.h>

#include <iostream>

using namespace KCal;

static const KCmdLineOptions options[] =
{
  {"q", 0, 0 },
  {"qtopia2icalendar", I18N_NOOP("Convert Qtopia calendar file to iCalendar"), 0 },
  {"i", 0, 0 },
  {"icalendar2qtopia", I18N_NOOP("Convert iCalendar to iCalendar"), 0 },
  {"o", 0, 0},
  {"output <file>", I18N_NOOP("Output file"), 0 },
  {"+input", I18N_NOOP("Input file"), 0 },
  KCmdLineLastOption
};

int main(int argc,char **argv)
{
  KAboutData aboutData("convertqtopia",I18N_NOOP("Qtopia calendar file converter"),"0.1");
  aboutData.addAuthor("Cornelius Schumacher", 0, "schumacher@kde.org");

  KCmdLineArgs::init(argc,argv,&aboutData);
  KCmdLineArgs::addCmdLineOptions( options );

  KApplication app;

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  bool sourceQtopia = false;
  bool sourceIcalendar = false;

  if ( args->isSet( "qtopia2icalendar" ) ) {
    sourceQtopia = true;
  }

  if ( args->isSet( "icalendar2qtopia" ) ) {
    sourceIcalendar = true;
  }

  if ( sourceQtopia && sourceIcalendar ) {
    KCmdLineArgs::usage(
        i18n("Please specify only one of the conversion options.") );
  }
  if ( !sourceQtopia && !sourceIcalendar ) {
    KCmdLineArgs::usage(
        i18n("You have to specify one conversion option.") );
  }

  if ( args->count() != 1 ) {
    KCmdLineArgs::usage( i18n("Error: No input file.") );
  }

  QString inputFile = args->arg( 0 );

  QString outputFile;
  if ( args->isSet("output") ) outputFile = args->getOption( "output" );

  kdDebug() << "Input File: '" << inputFile << "'" << endl;
  kdDebug() << "Output File: '" << outputFile << "'" << endl;

  if ( sourceQtopia ) {
    CalendarLocal cal;
    
    QtopiaFormat qtopiaFormat;
    qtopiaFormat.load( &cal, inputFile );

    ICalFormat icalendarFormat;
    if ( outputFile.isEmpty() ) {
      QString out = icalendarFormat.toString( &cal );
      std::cout << out.local8Bit() << std::endl;
    } else {
      bool success = icalendarFormat.save( &cal, outputFile );
      if ( !success ) {
        std::cerr << i18n( "Error saving to '%1'." ).arg( outputFile ).local8Bit()
                  << std::endl;
        return 1;
      }
    }
  }
  
  if ( sourceIcalendar ) {
    std::cerr << "Not implemented yet." << std::endl;
    return 1;
  }
}
