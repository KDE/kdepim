/*
    This file is part of libkpimexchange.
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
    02111-1307, USA.
*/
#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <krandomsequence.h>

#include "dateset.h"

static const KCmdLineOptions options[] =
{
  {"verbose", "Verbose output", 0},
  KCmdLineLastOption
};

void check( DateSet& dates, QMap<QDate,int> &map) 
{
  for( QDate date = QDate( 2002,6,1 ); date <= QDate( 2002,9,1 ); date = date.addDays( 1 ) ) {
    if ( map.contains( date ) != dates.contains( date ) ) {
      if ( map.contains( date ) ) 
        kdDebug() << "ERROR: " << date.toString() << " inserted, but not present!" << endl;
      else
        kdDebug() << "ERROR: " << date.toString() << " not inserted, but present!" << endl;
    }
  }
}

int main(int argc,char **argv)
{
//  KAboutData aboutData("testdateset","Test DateSet","0.1");
//  KCmdLineArgs::init(argc,argv,&aboutData);
//  KCmdLineArgs::addCmdLineOptions( options );

//  KApplication app( false, false );
//  KApplication app;

//  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  
  DateSet dates;
  QMap<QDate,int> map;
  KRandomSequence random( 0 );

  // dates.add( QDate( 2002, 7, 15 ) );
  // map.insert( QDate( 2002, 7, 16 ), 1 );

  for( int i=0; i<50; i++ ) {
    int month = 6 + random.getLong(2);
    int day = 1+random.getLong(29);
    QDate start( 2002, month, day );
    QDate end = start.addDays( random.getLong(7) );
    kdDebug() << endl << "Adding range " << start.toString() << "--" << end.toString() << endl;
    dates.add( start, end );
    dates.print();
    for( QDate date = start; date <= end; date = date.addDays( 1 ) ) {
      // kdDebug() << "Inserting to map: " << date.toString() << endl;
      map.insert( date, 1 );
    }
    check( dates, map );
 }

  for( int i=0; i<25; i++ ) {
    int month = 6 + random.getLong(2);
    int day = 1+random.getLong(29);
    QDate date( 2002, month, day );
    kdDebug() << endl << "Adding Date " << date.toString() << endl;
    dates.add( date );
    dates.print();
    map.insert( date, 1 );
    check( dates, map );
  }

  for( int i=0; i<25; i++ ) {
    int month = 6 + random.getLong(2);
    int day = 1+random.getLong(29);
    QDate date( 2002, month, day );
    kdDebug() << endl << "Removing Date " << date.toString() << endl;
    dates.remove( date );
    dates.print();
    map.remove( date );
    check( dates, map );
  }

  for( int i=0; i<65; i++ ) {
    int month = 6 + random.getLong(2);
    int day = 1+random.getLong(29);
    QDate start( 2002, month, day );
    QDate end = start.addDays( random.getLong(7) );
    kdDebug() << endl << "Removing range " << start.toString() << "--" << end.toString() << endl;
    dates.remove( start, end );
    dates.print();
    for( QDate date = start; date <= end; date = date.addDays( 1 ) ) {
      // kdDebug() << "Removing from map: " << date.toString() << endl;
      map.remove( date );
    }
    check( dates, map );
  }


/*
  dates.add( QDate( 2002, 6, 7 ), QDate( 2002, 6,16 ) );
  dates.print();
  kdDebug() << "contains(16 june): " << dates.contains( QDate( 2002, 6, 16 ) ) << endl;
  kdDebug() << endl;

  dates.add( QDate( 2002, 7, 7 ), QDate( 2002, 7, 9 ) );
  dates.print();
  kdDebug() << "contains(16 june): " << dates.contains( QDate( 2002, 6, 16 ) ) << endl;
*/

/*
  QDate date( 2002, 7, 15 );
  dates.add( date );
  dates.print();
  dates.add( QDate( 2002, 7, 18 ), QDate( 2002, 7, 25 ) );
  dates.print();
  dates.add( QDate( 2002, 7, 30 ) );
  dates.print();
  dates.add( QDate( 2002, 8, 1 ) );
  dates.print();
  dates.add( QDate( 2002, 7, 28 ) );
  dates.print();
  dates.add( QDate( 2002, 8, 2 ) );
  dates.print();
  dates.add( QDate( 2002, 7, 1 ) );
  dates.print();
  dates.add( QDate( 2002, 8, 10 ) );
  dates.print();
  dates.add( QDate( 2002, 8, 8 ), QDate( 2002, 8, 9 ) );
  dates.print();
  dates.add( QDate( 2002, 8, 8 ), QDate( 2002, 8, 9 ) );
  dates.print();
  dates.add( QDate( 2002, 8, 6 ), QDate( 2002, 8, 9 ) );
  dates.print();
  dates.add( QDate( 2002, 8, 4 ), QDate( 2002, 8, 10) );
  dates.print();
  dates.add( QDate( 2002, 7, 31), QDate( 2002, 8, 10) );
  dates.print();
  dates.remove( QDate( 2002, 7, 29 ) );
  dates.print();
  dates.remove( QDate( 2002, 6, 29 ) );
  dates.print();
  dates.remove( QDate( 2002, 8, 11 ) );
  dates.print();
  dates.remove( QDate( 2002, 8, 10 ) );
  dates.print();
  dates.remove( QDate( 2002, 7, 30 ) );
  dates.print();
  dates.remove( QDate( 2002, 8, 5 ) );
  dates.print();
*/





}


