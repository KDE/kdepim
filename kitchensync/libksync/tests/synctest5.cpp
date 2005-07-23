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
#include "bookmarksyncee.h"

#include <kbookmarkmanager.h>

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
  KAboutData aboutData( "synctest5", "libksync test 5", "0.1" );
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


  TestBookmarkManager bmm11;
  TestBookmarkManager bmm21;

  bmm11.root().addBookmark( &bmm11, "Bookmark 1", "http://bookmark1.org" );
  bmm21.root().addBookmark( &bmm21, "Bookmark 2", "http://bookmark2.org" );

  helper.sync( &bmm11, &bmm21, "201", "Bookmarks, sync new, no history." );

// Doesn't work yet. Implement BookmarkSyncee::removeEntry()
#if 0
  TestBookmarkManager bmm12;
  TestBookmarkManager bmm22;

  bmm12.root().addBookmark( &bmm12, "Changed Bookmark 1", "http://bookmark1.org" );
  bmm12.root().addBookmark( &bmm12, "Bookmark 2", "http://bookmark2.org" );
  bmm22.root().addBookmark( &bmm22, "Bookmark 1", "http://bookmark1.org" );
  bmm22.root().addBookmark( &bmm22, "Bookmark 2", "http://bookmark2.org" );

  helper.sync( &bmm12, &bmm22, "202", "Bookmarks, sync changed 1, no history." );


  TestBookmarkManager bmm13;
  TestBookmarkManager bmm23;

  bmm13.root().addBookmark( &bmm13, "Changed Bookmark 1", "http://bookmark1.org" );
  bmm13.root().addBookmark( &bmm13, "Bookmark 2", "http://bookmark2.org" );
  bmm23.root().addBookmark( &bmm23, "Changed Bookmark 1", "http://bookmark1.org" );
  bmm23.root().addBookmark( &bmm23, "Changed Bookmark 2", "http://bookmark2.org" );

  helper.sync( &bmm13, &bmm23, "203", "Bookmarks, sync changed 2, no history." );
#endif

  return 0;
}
