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

#include <konnectorinfo.h>
#include <kapabilities.h>

#include <kconfig.h>
#include <kgenericfactory.h>

using namespace KSync;

extern "C"
{
  void *init_liblocalkonnector()
  {
    return new KRES::PluginFactory<LocalKonnector,LocalKonnectorConfig>();
  }
}


LocalKonnector::LocalKonnector( const KConfig *config )
    : Konnector( config ), mConfigWidget( 0 )
{
  if ( config ) {
    mCalendarFile = config->readPathEntry( "CalendarFile" );
    mAddressBookFile = config->readPathEntry( "AddressBookFile" );
  }

  mSyncees.append( new CalendarSyncee( &mCalendar ) );    
}

LocalKonnector::~LocalKonnector()
{
}

void LocalKonnector::writeConfig( KConfig *config )
{
  Konnector::writeConfig( config );

  config->writePathEntry( "CalendarFile", mCalendarFile );
  config->writeEntry( "AddressBookFile", mAddressBookFile );
}

KSync::Kapabilities LocalKonnector::capabilities()
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

void LocalKonnector::setCapabilities( const KSync::Kapabilities& )
{
}

bool LocalKonnector::readSyncees()
{
  if ( !mCalendar.load( mCalendarFile ) ) return false;

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
                        QString::fromLatin1("LocalKonnector"),  // same as the .desktop file
                        "Dummy Konnector",
                        "agenda", // icon name
                        false );
}

void LocalKonnector::download( const QString& )
{
  error( StdError::downloadNotSupported() );
}

bool LocalKonnector::writeSyncees()
{
  mCalendar.save( mCalendarFile );

  emit synceesWritten( this );

  return true;
}


#include "localkonnector.moc"
