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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "synctesthelper.h"

#include "syncer.h"
#include "calendarsyncee.h"
#include "addressbooksyncee.h"
#include "bookmarksyncee.h"

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

SyncTestHelper::SyncTestHelper( const QString &outputDir, bool loadLog )
  : mOutputDir( outputDir ), mLoadLog( loadLog )
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

  if ( mLoadLog ) {
    syncee1.loadLog();
    syncee2.loadLog();
  }
  
  syncer.addSyncee( &syncee1 );
  syncer.addSyncee( &syncee2 );
  
  cal1->save( mOutputDir + "/" + prefix + ".cal.1.in" );
  cal2->save( mOutputDir + "/" + prefix + ".cal.2.in" );

  syncer.sync();
  
  cal1->save( mOutputDir + "/" + prefix + ".cal.1.out" );
  cal2->save( mOutputDir + "/" + prefix + ".cal.2.out" );
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

  if ( mLoadLog ) {
    syncee1.loadLog();
    syncee2.loadLog();
  }
  
  syncer.addSyncee( &syncee1 );
  syncer.addSyncee( &syncee2 );
  
  saveAddressBook( ab1, mOutputDir + "/" + prefix + ".ab.1.in" );
  saveAddressBook( ab2, mOutputDir + "/" + prefix + ".ab.2.in" );

  syncer.sync();
  
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

  if ( mLoadLog ) {
    syncee1.loadLog();
    syncee2.loadLog();
  }
  
  syncer.addSyncee( &syncee1 );
  syncer.addSyncee( &syncee2 );

  bmm1->saveAs( mOutputDir + "/" + prefix + ".bm.1.in" );
  bmm2->saveAs( mOutputDir + "/" + prefix + ".bm.2.in" );

  syncer.sync();
  
  bmm1->saveAs( mOutputDir + "/" + prefix + ".bm.1.out" );
  bmm2->saveAs( mOutputDir + "/" + prefix + ".bm.2.out" );
}

