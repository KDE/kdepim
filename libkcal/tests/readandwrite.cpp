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

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include <qfile.h>
#include <qfileinfo.h>

using namespace KCal;

static const KCmdLineOptions options[] =
{
  { "verbose", "Verbose output", 0 },
  { "+input", "Name of input file", 0 },
  { "+output", "Name of output file", 0 },
  KCmdLineLastOption
};

int main( int argc, char **argv )
{
  KAboutData aboutData( "readandwrite", "Read and Write Calendar", "0.1" );
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options );

  KApplication app( false, false );

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  if ( args->count() != 2 ) {
    args->usage( "Wrong number of arguments." );
  }

  QString input = QFile::decodeName( args->arg( 0 ) );
  QString output = QFile::decodeName( args->arg( 1 ) );

  QFileInfo outputFileInfo( output );
  output = outputFileInfo.absFilePath();

  kdDebug(5800) << "Input file: " << input << endl;
  kdDebug(5800) << "Output file: " << output << endl;


  CalendarLocal cal;

  // Force save() to save in sorted order
//  extern bool KCal_CalendarLocal_saveOrdered;
//  KCal_CalendarLocal_saveOrdered = true;

  if ( !cal.load( input ) ) return 1;
  if ( !cal.save( output ) ) return 1;

  return 0;
}
