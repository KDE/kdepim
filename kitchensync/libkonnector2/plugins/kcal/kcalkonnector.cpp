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

#include <calendarsyncee.h>

#include <kapabilities.h>
#include <kconfig.h>
#include <kgenericfactory.h>
#include <konnectorinfo.h>
#include <libkcal/resourcecalendar.h>

#include "kcalkonnector.h"
#include "kcalkonnectorconfig.h"

using namespace KSync;

extern "C"
{
  void *init_libkcalkonnector()
  {
    return new KRES::PluginFactory<KCalKonnector,KCalKonnectorConfig>();
  }
}


KCalKonnector::KCalKonnector( const KConfig *config )
    : Konnector( config ), mConfigWidget( 0 ), mResource( 0 )
{
  if ( config ) {
    mResourceIdentifier = config->readEntry( "CurrentResource" );
  }

  mManager = new KRES::Manager<KCal::ResourceCalendar>( "calendar" );
  mManager->readConfig();

  mCalendarSyncee = new CalendarSyncee( &mCalendar );
  mCalendarSyncee->setSource( i18n( "Calendar" ) );
  
  mSyncees.append( mCalendarSyncee );

  KRES::Manager<KCal::ResourceCalendar>::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    if ( (*it)->identifier() == mResourceIdentifier ) {
      mResource = *it;
      break;
    }
  }

  if ( mResource ) {
    connect( mResource, SIGNAL( resourceLoaded( ResourceCalendar* ) ),
             SLOT( loadingFinished() ) );
    connect( mResource, SIGNAL( resourceSaved( ResourceCalendar* ) ),
             SLOT( savingFinished() ) );
  }
}

KCalKonnector::~KCalKonnector()
{
  delete mManager;
}

void KCalKonnector::writeConfig( KConfig *config )
{
  Konnector::writeConfig( config );

  config->writeEntry( "CurrentResource", mResourceIdentifier );
}

KSync::Kapabilities KCalKonnector::capabilities()
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

void KCalKonnector::setCapabilities( const KSync::Kapabilities& )
{
}

bool KCalKonnector::readSyncees()
{
  if ( !mResource )
    return false;

  if ( !mResource->open() )
    return false;

  return mResource->load();
}

bool KCalKonnector::connectDevice()
{
  return true;
}

bool KCalKonnector::disconnectDevice()
{
  return true;
}

KSync::KonnectorInfo KCalKonnector::info() const
{
  return KonnectorInfo( i18n( "Calendar Konnector" ),
                        QIconSet(),
                        QString::fromLatin1( "KCalKonnector" ),
                        "Calendar Konnector",
                        "korganizer",
                        false );
}

void KCalKonnector::download( const QString& )
{
  error( StdError::downloadNotSupported() );
}

bool KCalKonnector::writeSyncees()
{
  if ( !mResource )
    return false;

  bool ok = true;

  KCal::Incidence::List oldList = mResource->rawIncidences();
  KCal::Incidence::List::Iterator oldIt;

  if ( !mResource->readOnly() ) {
    KCal::Incidence::List list = mCalendar.incidences();
    KCal::Incidence::List::Iterator it;
    bool found = false;
    for ( it = list.begin(); it != list.end(); ++it ) {
      for ( oldIt = oldList.begin(); oldIt != oldList.end(); ++oldIt ) {
        if ( (*oldIt)->uid() == (*it)->uid() ) {
          (*(*oldIt)) = (*(*it));
          found = true;
        }
      }

      if ( !found )
        mResource->addIncidence( *it );
    }

    ok = mResource->save();
  }

  return ok;
}

void KCalKonnector::loadingFinished()
{
  mCalendarSyncee->reset();

  KCal::Incidence::List list = mResource->rawIncidences();
  KCal::Incidence::List::Iterator it;
  for ( it = list.begin(); it != list.end(); ++it ) {
    KSync::CalendarSyncEntry entry( *it, mCalendarSyncee );
    mCalendarSyncee->addEntry( &entry );
  }

  emit synceesRead( this );
}

void KCalKonnector::savingFinished()
{
  emit synceesWritten( this );
}

#include "kcalkonnector.moc"
