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
#include "addressbooksyncee.h"

#include <kabc/addressbook.h>
#include <libkdepim/kabcresourcenull.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include <qfile.h>

#include <iostream>

using namespace KABC;
using namespace KSync;

static const KCmdLineOptions options[] =
{
  { "verbose", "Verbose output", 0 },
  { "+outputdir", "Name of output directory", 0 },
  KCmdLineLastOption
};

int main( int argc, char **argv )
{
  KAboutData aboutData( "synctest4", "libksync test 4", "0.1" );
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

  KABC::AddressBook ab1;
  ab1.addResource( new KABC::ResourceNull() );
  KABC::AddressBook ab2;
  ab2.addResource( new KABC::ResourceNull() );

  Addressee a1;
  a1.setResource( 0 );
  a1.setUid( "SYNCTEST3_ADDRESSEE_1" );
  a1.setNameFromString( "Hans Wurst" );
  a1.insertEmail( "hw@abc.de" );
  a1.setRevision( dt1 );

  ab1.insertAddressee( a1 );

  Addressee a2;
  a2.setResource( 0 );
  a2.setUid( "SYNCTEST3_ADDRESSEE_2" );
  a2.setNameFromString( "Zwerg Nase" );
  a2.insertEmail( "zn@abc.de" );
  a2.setRevision( dt1 );

  ab2.insertAddressee( a2 );

  helper.sync( &ab1, &ab2, "111", "Addressbook, sync new, with history." );

  a1.setNameFromString( "Haenschen Wuerstchen" );
  a1.setRevision( dt2 );
  ab1.insertAddressee( a1 );

  helper.sync( &ab1, &ab2, "112", "Addressbook, sync changed 1, with history" );

  a2.setNameFromString( "Zwergchen Naeschen" );
  a2.setRevision( dt3 );
  ab2.insertAddressee( a2 );

  helper.sync( &ab1, &ab2, "113", "Addressbook, sync changed 2, with history" );

  return 0;
}
