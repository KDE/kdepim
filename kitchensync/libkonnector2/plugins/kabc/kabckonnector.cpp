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
    KGlobal::locale()->insertCatalogue( "konnector_kabc" );
    return new KRES::PluginFactory<KABCKonnector,KABCKonnectorConfig>();
  }
}

class AddressBookWrapper : public KABC::AddressBook
{
  public:
    AddressBookWrapper( KABC::AddressBook* );

    KRES::Manager<KABC::Resource>* getResourceManager()
    {
      return resourceManager();
    }
};

KABCKonnector::KABCKonnector( const KConfig *config )
    : Konnector( config ), mConfigWidget( 0 ), mResource( 0 )
{
  if ( config ) {
    mResourceIdentifier = config->readEntry( "CurrentResource" );
  }

  mMd5sum = generateMD5Sum( mResourceIdentifier ) + "_kabckonnector.log";

  mResource = createResource( mResourceIdentifier );
  if ( mResource ) {
    mAddressBook.addResource( mResource );

    mAddressBookSyncee = new AddressBookSyncee( &mAddressBook );
    mAddressBookSyncee->setTitle( i18n( "Address Book" ) );

    mSyncees.append( mAddressBookSyncee );

    connect( mResource, SIGNAL( loadingFinished( Resource* ) ),
             SLOT( loadingFinished() ) );
  }
}

KABCKonnector::~KABCKonnector()
{
}

void KABCKonnector::writeConfig( KConfig *config )
{
  Konnector::writeConfig( config );

  config->writeEntry( "CurrentResource", mResourceIdentifier );
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

bool KABCKonnector::writeSyncees()
{
  if ( !mResource )
    return false;

  purgeRemovedEntries( mAddressBookSyncee );

  KABC::Ticket *ticket = mAddressBook.requestSaveTicket( mResource );
  if ( !ticket ) {
    kdWarning() << "KABCKonnector::writeSyncees(). Couldn't get ticket for resource." << endl;
    return false;
  }

  if ( !mAddressBook.save( ticket ) ) {
    kdWarning() << "KABCKonnector::writeSyncees(). Couldn't save resource." << endl;
    return false;
  }

  AddressBookSyncHistory syncInfo( mAddressBookSyncee, storagePath() + "/" + mMd5sum );
  syncInfo.save();

  emit synceesWritten( this );

  return true;
}

void KABCKonnector::loadingFinished()
{
  mAddressBookSyncee->reset();

  KABC::AddressBook::Iterator it;
  for ( it = mAddressBook.begin(); it != mAddressBook.end(); ++it ) {
    KSync::AddressBookSyncEntry entry( *it, mAddressBookSyncee );
    mAddressBookSyncee->addEntry( entry.clone() );
  }

  AddressBookSyncHistory syncInfo( mAddressBookSyncee, storagePath() + "/" + mMd5sum );
  syncInfo.load();

  emit synceesRead( this );
}

KABC::Resource* KABCKonnector::createResource( const QString &identifier )
{
  KConfig config( "kresources/contact/stdrc" );

  config.setGroup( "General" );
  QStringList activeKeys = config.readListEntry( "ResourceKeys" );
  if ( !activeKeys.contains( identifier ) )
    return 0;

  KRES::Factory *factory = KRES::Factory::self( "contact" );
  config.setGroup( "Resource_" + identifier );

  QString type = config.readEntry( "ResourceType" );
  QString name = config.readEntry( "ResourceName" );
  KABC::Resource *resource = dynamic_cast<KABC::Resource*>( factory->resource( type, &config ) );
  if ( !resource ) {
    kdError() << "Failed to create resource with id " << identifier << endl;
    return 0;
  }

  return resource;
}

#include "kabckonnector.moc"
