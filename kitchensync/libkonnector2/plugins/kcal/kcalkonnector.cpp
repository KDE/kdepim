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

#include <kconfig.h>
#include <kgenericfactory.h>
#include <konnectorinfo.h>
#include <synchistory.h>

#include <libkcal/resourcecalendar.h>
#include <libkdepim/kpimprefs.h>

#include "kcalkonnector.h"
#include "kcalkonnectorconfig.h"

using namespace KSync;

extern "C"
{
  void *init_libkcalkonnector()
  {
    KGlobal::locale()->insertCatalogue( "konnector_kcal" );
    return new KRES::PluginFactory<KCalKonnector,KCalKonnectorConfig>();
  }
}


KCalKonnector::KCalKonnector( const KConfig *config )
    : Konnector( config ), mConfigWidget( 0 ), mResource( 0 )
{
  if ( config ) {
    mResourceIdentifier = config->readEntry( "CurrentResource" );
  }

  mMd5sum = generateMD5Sum( mResourceIdentifier ) + "_kcalkonnector.log";

  mCalendar = new KCal::CalendarResources( KPimPrefs::timezone() );

  mResource = createResource( mResourceIdentifier );

  if ( mResource ) {
    mCalendar->resourceManager()->add( mResource );
    connect( mResource, SIGNAL( resourceLoaded( ResourceCalendar* ) ),
             SLOT( loadingFinished() ) );
    connect( mResource, SIGNAL( resourceSaved( ResourceCalendar* ) ),
             SLOT( savingFinished() ) );

    mCalendarSyncee = new CalendarSyncee( mCalendar );
    mCalendarSyncee->setTitle( i18n( "Calendar" ) );
    mCalendarSyncee->setIdentifier( "calendar" );

    mSyncees.append( mCalendarSyncee );
  }
}

KCalKonnector::~KCalKonnector()
{
  delete mCalendar;
}

void KCalKonnector::writeConfig( KConfig *config )
{
  Konnector::writeConfig( config );

  config->writeEntry( "CurrentResource", mResourceIdentifier );
}

bool KCalKonnector::readSyncees()
{
  if ( mCalendar->resourceManager()->isEmpty() )
    return false;

  mCalendarSyncee->reset();

  mCalendar->close();
  mCalendar->load();

  return true;
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
                        "korganizer",
                        false );
}

bool KCalKonnector::writeSyncees()
{
  if ( mCalendar->resourceManager()->isEmpty() )
    return false;

  /* remove the deleted entries before saving */
  purgeRemovedEntries( mCalendarSyncee );
  KCal::CalendarResources::Ticket *ticket = mCalendar->requestSaveTicket( mResource );
  if ( !ticket ) {
    kdWarning() << "KCalKonnector::writeSyncees(). Couldn't get ticket for resource." << endl;
    return false;
  }

  mCalendar->save( ticket );

  return true;
}

void KCalKonnector::loadingFinished()
{
  CalendarSyncHistory helper( mCalendarSyncee, storagePath()+"/"+mMd5sum );
  helper.load();
  emit synceesRead( this );
}

void KCalKonnector::savingFinished()
{
  CalendarSyncHistory helper( mCalendarSyncee, storagePath()+"/"+mMd5sum );
  helper.save();
  emit synceesWritten( this );
}

KCal::ResourceCalendar* KCalKonnector::createResource( const QString &identifier )
{
  KConfig config( "kresources/calendar/stdrc" );

  config.setGroup( "General" );
  QStringList activeKeys = config.readListEntry( "ResourceKeys" );
  if ( !activeKeys.contains( identifier ) )
    return 0;

  KRES::Factory *factory = KRES::Factory::self( "calendar" );
  config.setGroup( "Resource_" + identifier );

  QString type = config.readEntry( "ResourceType" );
  QString name = config.readEntry( "ResourceName" );
  KCal::ResourceCalendar *resource = dynamic_cast<KCal::ResourceCalendar*>( factory->resource( type, &config ) );
  if ( !resource ) {
    kdError() << "Failed to create resource with id " << identifier << endl;
    return 0;
  }

  return resource;
}

#include "kcalkonnector.moc"
