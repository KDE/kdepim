/*
    This file is part of KitchenSync.

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

#include "localkonnector.h"

#include "localkonnectorconfig.h"

#include <calendarsyncee.h>
#include <addressbooksyncee.h>
#include <bookmarksyncee.h>
#include <synchistory.h>

#include <libkdepim/kpimprefs.h>
#include <libkdepim/progressmanager.h>

#include <kabc/resourcefile.h>

#include <konnectorinfo.h>

#include <kconfig.h>
#include <kgenericfactory.h>

using namespace KSync;

extern "C"
{
  void *init_liblocalkonnector()
  {
    KGlobal::locale()->insertCatalogue( "konnector_local" );
    return new KRES::PluginFactory<LocalKonnector,LocalKonnectorConfig>();
  }
}


LocalKonnector::LocalKonnector( const KConfig *config )
    : Konnector( config ), mConfigWidget( 0 ),
      mCalendar( KPimPrefs::timezone() ), mProgressItem( 0 )
{
  if ( config ) {
    mCalendarFile = config->readPathEntry( "CalendarFile" );
    mAddressBookFile = config->readPathEntry( "AddressBookFile" );
    mBookmarkFile = config->readPathEntry( "BookmarkFile" );
  }

  mMd5sumCal = generateMD5Sum( mCalendarFile ) + "_localkonnector_cal.log";
  mMd5sumAbk = generateMD5Sum( mAddressBookFile ) + "_localkonnector_abk.log";
  mMd5sumBkm = generateMD5Sum( mBookmarkFile ) + "_localkonnector_bkm.log";

  mAddressBookSyncee = new AddressBookSyncee( &mAddressBook );
  mAddressBookSyncee->setTitle( i18n( "Local" ) );

  mCalendarSyncee = new CalendarSyncee( &mCalendar );
  mCalendarSyncee->setTitle( i18n( "Local" ) );

  mSyncees.append( mCalendarSyncee );
  mSyncees.append( mAddressBookSyncee );
  mSyncees.append( new BookmarkSyncee( &mBookmarkManager ) );

  mAddressBookResourceFile = new KABC::ResourceFile( mAddressBookFile );
  mAddressBook.addResource( mAddressBookResourceFile );
}

LocalKonnector::~LocalKonnector()
{
}

void LocalKonnector::writeConfig( KConfig *config )
{
  Konnector::writeConfig( config );

  config->writePathEntry( "CalendarFile", mCalendarFile );
  config->writePathEntry( "AddressBookFile", mAddressBookFile );
  config->writePathEntry( "BookmarkFile", mAddressBookFile );
}

bool LocalKonnector::readSyncees()
{
  kdDebug() << "LocalKonnector::readSyncee()" << endl;

  mProgressItem = progressItem( i18n( "Start loading local data..." ) );

  if ( !mCalendarFile.isEmpty() ) {
    kdDebug() << "LocalKonnector::readSyncee(): calendar: " << mCalendarFile
              << endl;
    mCalendar.close();
    mProgressItem->setStatus( i18n( "Load Calendar..." ) );
    if ( mCalendar.load( mCalendarFile ) ) {
      kdDebug() << "Read succeeded." << endl;
      mCalendarSyncee->reset();
      mCalendarSyncee->setIdentifier( mCalendarFile );
      kdDebug() << "IDENTIFIER: " << mCalendarSyncee->identifier() << endl;

      /* apply SyncInformation here this will also create the SyncEntries */
      CalendarSyncHistory cHelper(  mCalendarSyncee, storagePath() + "/"+mMd5sumCal );
      cHelper.load();
      mProgressItem->setStatus( i18n( "Calendar loaded." ) );
    } else {
      mProgressItem->setStatus( i18n( "Loading Calendar failed." ) );
      emit synceeReadError( this );
      kdDebug() << "Read failed." << endl;
      return false;
    }
  }

  mProgressItem->setProgress( 50 );

    if ( !mAddressBookFile.isEmpty() ) {
      kdDebug() << "LocalKonnector::readSyncee(): addressbook: "
                << mAddressBookFile << endl;

      mProgressItem->setStatus( i18n( "Load AddressBook..." ) );
      mAddressBookResourceFile->setFileName( mAddressBookFile );
      if ( !mAddressBook.load() ) {
        mProgressItem->setStatus( i18n( "Loading AddressBook failed." ) );
        emit synceeReadError( this );
        kdDebug() << "Read failed." << endl;
        return false;
      }

      kdDebug() << "Read succeeded." << endl;

      mAddressBookSyncee->reset();
      mAddressBookSyncee->setIdentifier( mAddressBook.identifier() );
      kdDebug() << "IDENTIFIER: " << mAddressBookSyncee->identifier() << endl;

      KABC::AddressBook::Iterator it;
      for ( it = mAddressBook.begin(); it != mAddressBook.end(); ++it ) {
        KSync::AddressBookSyncEntry entry( *it, mAddressBookSyncee );
        mAddressBookSyncee->addEntry( entry.clone() );
      }

      /* let us apply Sync Information */
      AddressBookSyncHistory aHelper( mAddressBookSyncee, storagePath() + "/"+mMd5sumAbk );
      aHelper.load();
      mProgressItem->setStatus( i18n( "AddressBook loaded." ) );
    }

  // TODO: Read Bookmarks
  mProgressItem->setProgress( 100 );
  mProgressItem->setComplete();
  mProgressItem = 0;

  emit synceesRead( this );

  return true;
}

bool LocalKonnector::connectDevice()
{
  return true;
}

bool LocalKonnector::disconnectDevice()
{
  return true;
}

KSync::KonnectorInfo LocalKonnector::info() const
{
  return KonnectorInfo( i18n("Dummy Konnector"),
                        QIconSet(),
                        "agenda", // icon name
                        false );
}

QStringList LocalKonnector::supportedFilterTypes() const
{
  QStringList types;
  types << "addressbook" << "calendar" << "bookmarks";

  return types;
}

bool LocalKonnector::writeSyncees()
{
  if ( !mCalendarFile.isEmpty() ) {
    purgeRemovedEntries( mCalendarSyncee );

    if ( !mCalendar.save( mCalendarFile ) ) return false;
    CalendarSyncHistory cHelper(  mCalendarSyncee, storagePath() + "/"+mMd5sumCal );
    cHelper.save();
  }

  if ( !mAddressBookFile.isEmpty() ) {
    purgeRemovedEntries( mAddressBookSyncee );
    KABC::Ticket *ticket;
    ticket = mAddressBook.requestSaveTicket( mAddressBookResourceFile );
    if ( !ticket ) {
      kdWarning() << "LocalKonnector::writeSyncees(). Couldn't get ticket for "
                  << "addressbook." << endl;
      emit synceeWriteError( this );
      return false;
    }
    if ( !mAddressBook.save( ticket ) ) return false;
    AddressBookSyncHistory aHelper( mAddressBookSyncee, storagePath() + "/"+mMd5sumAbk );
    aHelper.save();
  }

  // TODO: Write Bookmarks

  emit synceesWritten( this );

  return true;
}


#include "localkonnector.moc"
