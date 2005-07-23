/*
    This file is part of libksync.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2004 Holger Hans Peter Freyther <freyther@kde.org>

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
#include "addressbooksyncee.h"
#include "bookmarksyncee.h"
#include "synchistory.h"

#include <libkcal/calendarlocal.h>
#include <kabc/addressbook.h>
#include <kabc/vcardconverter.h>

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
using namespace KABC;

SyncTestHelper::SyncTestHelper( const QString &outputDir)
  : mOutputDir( outputDir )
{
}

void SyncTestHelper::writeTestFile( const QString &prefix, const QString &type,
                                    const QString &title )
{
  kdDebug() << "SYNC " << prefix << endl;

  QString testFile = mOutputDir + "/" + prefix + "." + type + ".test";

  QFile f( testFile );
  if ( !f.open( IO_WriteOnly ) ) {
    std::cerr << "Unable to write '" << QFile::encodeName( testFile ) << "'."
              << std::endl;
  }
  QTextStream t( &f );
  t << title;
  f.close();

}

void SyncTestHelper::sync( CalendarLocal *cal1, CalendarLocal *cal2,
                           const QString &prefix, const QString &title )
{
  writeTestFile( prefix, "cal", title );

  Syncer syncer;

  CalendarSyncee syncee1( cal1 );
  CalendarSyncee syncee2( cal2 );

  syncee1.setIdentifier( "cal1" );
  syncee2.setIdentifier( "cal2" );

  CalendarSyncHistory h1( &syncee1, QString::fromLatin1("cal1-fooo.syncee"));
  h1.load();

  CalendarSyncHistory h2( &syncee2, QString::fromLatin1("cal2-fooo.syncee" ));
  h2.load();

  syncer.addSyncee( &syncee1 );
  syncer.addSyncee( &syncee2 );

  cal1->save( mOutputDir + "/" + prefix + ".cal.1.in" );
  cal2->save( mOutputDir + "/" + prefix + ".cal.2.in" );

  cal1->save( mOutputDir + "/" + prefix + ".cal.1.in_a" );
  cal2->save( mOutputDir + "/" + prefix + ".cal.2.in_b" );

  syncer.sync();

  cal1->save( mOutputDir + "/" + prefix + ".cal.1.out" );
  cal2->save( mOutputDir + "/" + prefix + ".cal.2.out" );

  h1.save();
  h2.save();
}

void SyncTestHelper::sync( AddressBook *ab1, AddressBook *ab2,
                           const QString &prefix, const QString &title )
{
  writeTestFile( prefix, "ab", title );

  Syncer syncer;

  AddressBookSyncee syncee1( ab1 );
  AddressBookSyncee syncee2( ab2 );

  syncee1.setIdentifier( "ab1" );
  syncee2.setIdentifier( "ab2" );

  /* Fill the Log */
  AddressBookSyncHistory h1( &syncee1, "ab1-fooo.syncee" ); h1.load();
  AddressBookSyncHistory h2( &syncee2, "ab2-fooo.syncee" ); h2.load();

  syncer.addSyncee( &syncee1 );
  syncer.addSyncee( &syncee2 );

  saveAddressBook( ab1, mOutputDir + "/" + prefix + ".ab.1.in" );
  saveAddressBook( ab2, mOutputDir + "/" + prefix + ".ab.2.in" );

  syncer.sync();

  h1.save();
  h2.save();


  saveAddressBook( ab1, mOutputDir + "/" + prefix + ".ab.1.out" );
  saveAddressBook( ab2, mOutputDir + "/" + prefix + ".ab.2.out" );
}

void SyncTestHelper::saveAddressBook( AddressBook *ab, const QString &filename )
{
  Addressee::List addressees = ab->allAddressees();

  QFile f( filename );
  if ( !f.open( IO_WriteOnly ) ) {
    std::cerr << "Unable to open file '" << QFile::encodeName( filename )
              << "'." << std::endl;
    exit( 1 );
  }
  QTextStream t( &f );
  VCardConverter vcard;
  t << vcard.createVCards( addressees );
}

void SyncTestHelper::sync( KBookmarkManager *bmm1, KBookmarkManager *bmm2,
                           const QString &prefix, const QString &title )
{
  writeTestFile( prefix, "bm", title );

  Syncer syncer;

  BookmarkSyncee syncee1( bmm1 );
  BookmarkSyncee syncee2( bmm2 );

  syncee1.setIdentifier( "bmm1" );
  syncee2.setIdentifier( "bmm2" );

  BookmarkSyncHistory h1( &syncee1, "bmm1-fooo.syncee" ); h1.load();
  BookmarkSyncHistory h2( &syncee2, "bmm2-fooo.syncee" ); h2.load();


  syncer.addSyncee( &syncee1 );
  syncer.addSyncee( &syncee2 );

  bmm1->saveAs( mOutputDir + "/" + prefix + ".bm.1.in" );
  bmm2->saveAs( mOutputDir + "/" + prefix + ".bm.2.in" );

  syncer.sync();

  h1.save();
  h2.save();

  bmm1->saveAs( mOutputDir + "/" + prefix + ".bm.1.out" );
  bmm2->saveAs( mOutputDir + "/" + prefix + ".bm.2.out" );
}

