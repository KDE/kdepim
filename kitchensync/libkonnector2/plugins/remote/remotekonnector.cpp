/*
    This file is part of KitchenSync.

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

#include "remotekonnector.h"

#include "remotekonnectorconfig.h"

#include <calendarsyncee.h>
#include <addressbooksyncee.h>
#include <bookmarksyncee.h>
#include <synchistory.h>

#include <kabc/vcardconverter.h>
#include <libkcal/icalformat.h>
#include <libkdepim/kabcresourcenull.h>
#include <libkdepim/kpimprefs.h>

#include <konnectorinfo.h>

#include <kconfig.h>
#include <kgenericfactory.h>

using namespace KSync;
using namespace KABC;
using namespace KCal;

extern "C"
{
  void *init_libremotekonnector()
  {
    return new KRES::PluginFactory<RemoteKonnector,RemoteKonnectorConfig>();
  }
}


RemoteKonnector::RemoteKonnector( const KConfig *config )
    : Konnector( config ), mConfigWidget( 0 ),
    mCalendar( KPimPrefs::timezone() )
{
  mAddressBook.addResource( new KABC::ResourceNull() );
  if ( config ) {
    mCalendarUrl = config->readPathEntry( "CalendarUrl" );
    mAddressBookUrl = config->readPathEntry( "AddressBookUrl" );
    mBookmarkUrl = config->readPathEntry( "BookmarkUrl" );
  }

  mMd5sumCal = generateMD5Sum( mCalendarUrl ) +    "_remotekonnector_cal.log";
  mMd5sumBkm = generateMD5Sum( mBookmarkUrl ) +    "_remotekonnector_bkm.log";
  mMd5sumAbk = generateMD5Sum( mAddressBookUrl ) + "_remotekonnector_abk.log";

  mAddressBookSyncee =	new AddressBookSyncee( &mAddressBook );
  mAddressBookSyncee->setTitle( i18n( "Remote" ) );
  mCalendarSyncee = new CalendarSyncee( &mCalendar );
  mCalendarSyncee->setTitle( i18n( "Remote" ) );

  mSyncees.append( mCalendarSyncee );
  mSyncees.append( mAddressBookSyncee );
  mSyncees.append( new BookmarkSyncee( &mBookmarkManager ) );
}

RemoteKonnector::~RemoteKonnector()
{
}

void RemoteKonnector::writeConfig( KConfig *config )
{
  Konnector::writeConfig( config );

  config->writePathEntry( "CalendarUrl", mCalendarUrl );
  config->writeEntry( "AddressBookUrl", mAddressBookUrl );
  config->writeEntry( "BookmarkFile", mAddressBookUrl );
}

bool RemoteKonnector::readSyncees()
{
  kdDebug() << "RemoteKonnector::readSyncees()" << endl;

  mSynceeReadCount = 0;

  if ( !mCalendarUrl.isEmpty() ) {
    kdDebug() << "RemoteKonnector::readSyncees(): calendar: " << mCalendarUrl
              << endl;

    mCalendarData = "";

    KIO::TransferJob *job = KIO::get( KURL( mCalendarUrl ) );
    connect( job, SIGNAL( result( KIO::Job * ) ),
             SLOT( slotCalendarReadResult( KIO::Job * ) ) );
    connect( job, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
             SLOT( slotCalendarData( KIO::Job *, const QByteArray & ) ) );

    ++mSynceeReadCount;
  }

  if ( !mAddressBookUrl.isEmpty() ) {
    kdDebug() << "RemoteKonnector::readSyncees(): AddressBook: "
              << mAddressBookUrl << endl;

    mAddressBookData = "";

    KIO::TransferJob *job = KIO::get( KURL( mAddressBookUrl ) );
    connect( job, SIGNAL( result( KIO::Job * ) ),
             SLOT( slotAddressBookReadResult( KIO::Job * ) ) );
    connect( job, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
             SLOT( slotAddressBookData( KIO::Job *, const QByteArray & ) ) );

    ++mSynceeReadCount;
  }

  // TODO: Read Bookmarks

  return true;
}

void RemoteKonnector::slotCalendarData( KIO::Job *, const QByteArray &d )
{
  mCalendarData += QString::fromUtf8( d );
}

void RemoteKonnector::slotCalendarReadResult( KIO::Job *job )
{
  --mSynceeReadCount;

  if ( job->error() ) {
    job->showErrorDialog( 0 );
    emit synceeReadError( this );
  } else {
    mCalendar.close();
    ICalFormat ical;
    if ( ical.fromString( &mCalendar, mCalendarData ) ) {
      kdDebug() << "Read succeeded." << endl;
      mCalendarSyncee->reset();
      mCalendarSyncee->setIdentifier( mCalendarUrl );
      kdDebug() << "IDENTIFIER: " << mCalendarSyncee->identifier() << endl;
    } else {
      kdDebug() << "Read failed." << endl;
      emit synceeReadError( this );
    }
  }

  finishRead();
}

void RemoteKonnector::slotAddressBookData( KIO::Job *, const QByteArray &d )
{
  mAddressBookData += QString::fromUtf8( d );
}

void RemoteKonnector::slotAddressBookReadResult( KIO::Job *job )
{
  --mSynceeReadCount;

  if ( job->error() ) {
    job->showErrorDialog( 0 );
    emit synceeReadError( this );
  } else {
    mAddressBook.clear();
    VCardConverter v;
    Addressee::List a = v.parseVCards( mAddressBookData );
    Addressee::List::ConstIterator it;
    for( it = a.begin(); it != a.end(); ++it ) {
      mAddressBook.insertAddressee( *it );
      KSync::AddressBookSyncEntry entry( *it, mAddressBookSyncee );
      mAddressBookSyncee->addEntry( entry.clone() );
    }
  }

  finishRead();
}

void RemoteKonnector::finishRead()
{
  if ( mSynceeReadCount > 0 ) return;


  CalendarSyncHistory cHelper( mCalendarSyncee, storagePath()+"/"+mMd5sumCal );
  cHelper.load();

  AddressBookSyncHistory aHelper( mAddressBookSyncee, storagePath()+"/"+mMd5sumAbk);
  aHelper.load();

  emit synceesRead( this );
}

bool RemoteKonnector::connectDevice()
{
  return true;
}

bool RemoteKonnector::disconnectDevice()
{
  return true;
}

KSync::KonnectorInfo RemoteKonnector::info() const
{
  return KonnectorInfo( i18n("Remote Konnector"),
                        QIconSet(),
                        QString::fromLatin1("RemoteKonnector"),  // same as the .desktop file
                        "Remote Konnector",
                        "agenda", // icon name
                        false );
}

bool RemoteKonnector::writeSyncees()
{
  kdDebug() << "RemoteKonnector::writeSyncees()" << endl;

  mSynceeWriteCount = 0;

  if ( !mCalendarUrl.isEmpty() ) {
    kdDebug() << "RemoteKonnector::writeSyncees(): calendar: " << mCalendarUrl
              << endl;
    purgeRemovedEntries( mCalendarSyncee );

    ICalFormat ical;
    mCalendarData = ical.toString( &mCalendar );
    if ( !mCalendarData.isEmpty() ) {
      KIO::TransferJob *job = KIO::put( KURL( mCalendarUrl ), -1, true, false );
      connect( job, SIGNAL( result( KIO::Job * ) ),
               SLOT( slotCalendarWriteResult( KIO::Job * ) ) );
      connect( job, SIGNAL( dataReq( KIO::Job *, QByteArray & ) ),
               SLOT( slotCalendarDataReq( KIO::Job *, QByteArray & ) ) );

      ++mSynceeWriteCount;
    }
  }

  if ( !mAddressBookUrl.isEmpty() ) {
    kdDebug() << "RemoteKonnector::writeSyncees(): AddressBook: "
              << mAddressBookUrl << endl;
    purgeRemovedEntries( mAddressBookSyncee );

    mAddressBookData = "";

    VCardConverter v;
    AddressBook::ConstIterator it;
    for ( it = mAddressBook.begin(); it != mAddressBook.end(); ++it ) {
      mAddressBookData.append( v.createVCard( *it ) );
    }

    if ( !mAddressBookData.isEmpty() ) {
      KIO::TransferJob *job = KIO::put( KURL( mAddressBookUrl ), -1, true,
                                        false );
      connect( job, SIGNAL( result( KIO::Job * ) ),
               SLOT( slotAddressBookWriteResult( KIO::Job * ) ) );
      connect( job, SIGNAL( dataReq( KIO::Job *, QByteArray & ) ),
               SLOT( slotAddressBookDataReq( KIO::Job *, QByteArray & ) ) );

      ++mSynceeWriteCount;
    }
  }

  // TODO: Write Bookmarks

  return true;
}

void RemoteKonnector::slotCalendarDataReq( KIO::Job *, QByteArray &d )
{
  if ( !mCalendarData.isEmpty() ) {
    d = mCalendarData.utf8();
    mCalendarData = QString::null;
  }
}

void RemoteKonnector::slotCalendarWriteResult( KIO::Job *job )
{
  --mSynceeWriteCount;

  if ( job->error() ) {
    job->showErrorDialog( 0 );
    emit synceeWriteError( this );
  }

  finishWrite();
}

void RemoteKonnector::slotAddressBookDataReq( KIO::Job *, QByteArray &d )
{
  if ( !mAddressBookData.isEmpty() ) {
    d = mAddressBookData.utf8();
    mAddressBookData = QString::null;
  }
}

void RemoteKonnector::slotAddressBookWriteResult( KIO::Job *job )
{
  --mSynceeWriteCount;

  if ( job->error() ) {
    job->showErrorDialog( 0 );
    emit synceeWriteError( this );
  }

  finishWrite();
}

void RemoteKonnector::finishWrite()
{
  if ( mSynceeWriteCount > 0 ) return;


  CalendarSyncHistory cHelper( mCalendarSyncee, storagePath()+"/"+mMd5sumCal );
  cHelper.save();

  AddressBookSyncHistory aHelper( mAddressBookSyncee, storagePath()+"/"+mMd5sumAbk);
  aHelper.save();

  emit synceesWritten( this );
}


#include "remotekonnector.moc"
