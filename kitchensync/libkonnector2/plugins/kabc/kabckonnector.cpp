/*
    This file is part of KitchenSync.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

#include <qtimer.h>

#include <addressbooksyncee.h>
#include <synchistory.h>


#include <kabc/resource.h>
#include <kabc/resourcefile.h>
#include <kapabilities.h>
#include <kconfig.h>
#include <kgenericfactory.h>
#include <konnectorinfo.h>
#include <libkdepim/kabcresourcenull.h>

#include "kabckonnector.h"
#include "kabckonnectorconfig.h"

using namespace KSync;

extern "C"
{
  void *init_libkabckonnector()
  {
    return new KRES::PluginFactory<KABCKonnector,KABCKonnectorConfig>();
  }
}


KABCKonnector::KABCKonnector( const KConfig *config )
    : Konnector( config ), mConfigWidget( 0 ), mResource( 0 )
{
  if ( config ) {
    mResourceIdentifier = config->readEntry( "CurrentResource" );
  }
  mMd5sum = generateMD5Sum( mResourceIdentifier ) + "_kabckonnector.log";

  mManager = new KRES::Manager<KABC::Resource>( "contact" );
  mManager->readConfig();

  mAddressBook.addResource( new KABC::ResourceNull() );

  mAddressBookSyncee = new AddressBookSyncee( &mAddressBook );
  mAddressBookSyncee->setTitle( i18n( "Address Book" ) );

  mSyncees.append( mAddressBookSyncee );

  KRES::Manager<KABC::Resource>::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    if ( (*it)->identifier() == mResourceIdentifier ) {
      mResource = *it;
      break;
    }
  }

  if ( mResource ) {
    connect( mResource, SIGNAL( loadingFinished( Resource* ) ),
             SLOT( loadingFinished() ) );

    mResource->setAddressBook( &mAddressBook );
  }
}

KABCKonnector::~KABCKonnector()
{
  delete mManager;
}

void KABCKonnector::writeConfig( KConfig *config )
{
  Konnector::writeConfig( config );

  config->writeEntry( "CurrentResource", mResourceIdentifier );
}

KSync::Kapabilities KABCKonnector::capabilities()
{
  KSync::Kapabilities caps;

  caps.setSupportMetaSyncing( false ); // we can meta sync
  caps.setSupportsPushSync( false ); // we can initialize the sync from here
  caps.setNeedsConnection( false ); // we need to have pppd running
  caps.setSupportsListDir( false ); // we will support that once there is API for it...
  caps.setNeedsIPs( false ); // we need the IP
  caps.setNeedsSrcIP( false ); // we do not bind to any address...
  caps.setNeedsDestIP( false ); // we need to know where to connect
  caps.setAutoHandle( false ); // we currently do not support auto handling
  caps.setNeedAuthentication( false ); // HennevL says we do not need that
  caps.setNeedsModelName( false ); // we need a name for our meta path!

  return caps;
}

void KABCKonnector::setCapabilities( const KSync::Kapabilities& )
{
}

bool KABCKonnector::readSyncees()
{
  if ( !mResource )
    return false;

  if ( !mResource->open() )
    return false;

  mResource->asyncLoad();

  return true;
}

bool KABCKonnector::connectDevice()
{
  return true;
}

bool KABCKonnector::disconnectDevice()
{
  return true;
}

KSync::KonnectorInfo KABCKonnector::info() const
{
  return KonnectorInfo( i18n( "Address Book Konnector" ),
                        QIconSet(),
                        QString::fromLatin1( "KABCKonnector" ),
                        "Address Book Konnector",
                        "kaddressbook",
                        false );
}

void KABCKonnector::download( const QString& )
{
  error( StdError::downloadNotSupported() );
}

bool KABCKonnector::writeSyncees()
{
  if ( !mResource )
    return false;


  purgeRemovedEntries( mAddressBookSyncee );

  KABC::AddressBook::Iterator it;
  for ( it = mAddressBook.begin(); it != mAddressBook.end(); ++it )
    mResource->insertAddressee( *it );

  if ( !mResource->readOnly() ) {
    KABC::Ticket *ticket;
    ticket = mResource->requestSaveTicket();
    if ( !ticket ) {
      kdWarning() << "KABCKonnector::writeSyncees(). Couldn't get ticket for "
                  << "resource." << endl;
      return false;
    }

    if ( !mResource->save( ticket ) ) {
      kdWarning() << "KABCKonnector::writeSyncees(). Couldn't save resource." << endl;
      return false;
    }
  }

  AddressBookSyncHistory syncInfo(mAddressBookSyncee, storagePath()+"/"+mMd5sum );
  syncInfo.save();

  emit synceesWritten( this );

  return true;
}

void KABCKonnector::loadingFinished()
{
  mAddressBookSyncee->reset();

  KABC::Resource::Iterator it;
  for ( it = mResource->begin(); it != mResource->end(); ++it ) {
    KSync::AddressBookSyncEntry entry( *it, mAddressBookSyncee );
    mAddressBookSyncee->addEntry( entry.clone() );
  }

  AddressBookSyncHistory syncInfo(mAddressBookSyncee, storagePath()+"/"+mMd5sum );
  syncInfo.load();

  emit synceesRead( this );
}

#include "kabckonnector.moc"
