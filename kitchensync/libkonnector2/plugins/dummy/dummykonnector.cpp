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

#include <kgenericfactory.h>
#include <kdebug.h>

#include <konnectorinfo.h>
#include <kapabilities.h>

#include <calendarsyncee.h>

#include "dummykonnector.h"

using namespace KSync;
using namespace KCal;

class DummyKonnectorFactory : public KRES::PluginFactoryBase
{
  public:
    KRES::Resource *resource( const KConfig *config )
    {
      return new DummyKonnector( config );
    }

    KRES::ConfigWidget *configWidget( QWidget * )
    {
      return 0;
    }
};

extern "C"
{
  void *init_libdummykonnector()
  {
    return new DummyKonnectorFactory();
  }
}


DummyKonnector::DummyKonnector( const KConfig *config )
    : Konnector( config )
{
  Event *event = new Event;
  event->setSummary( "An Event" );
  mCalendar.addEvent( event );

  event = new Event;
  event->setSummary( "Another Event" );
  mCalendar.addEvent( event );
}

DummyKonnector::~DummyKonnector()
{
}

KSync::Kapabilities DummyKonnector::capabilities()
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

void DummyKonnector::setCapabilities( const KSync::Kapabilities & )
{
}

bool DummyKonnector::readSyncees()
{
  kdDebug() << "DummyKonnector::readSyncees()" << endl;

  SynceeList synceeList;

  CalendarSyncee *calendarSyncee = new CalendarSyncee( &mCalendar );
  synceeList.append( calendarSyncee );
  
  emit synceesRead( this, synceeList );

  return true;
}

bool DummyKonnector::startBackup(const QString& path)
{
  error ( StdError::backupNotSupported() );
  return false;
}

bool DummyKonnector::startRestore( const QString& path )
{
  error ( StdError::backupNotSupported() );
  return false;
}

bool DummyKonnector::connectDevice()
{
  return true;
}

bool DummyKonnector::disconnectDevice()
{
  return true;
}

KSync::KonnectorInfo DummyKonnector::info() const
{
  return KonnectorInfo( i18n("Dummy Konnector"),
                        QIconSet(),
                        QString::fromLatin1("dummykonnector"),  // same as the .desktop file
                        "Dummy Konnector",
                        "agenda", // icon name
                        false );
}

void DummyKonnector::download( const QString& )
{
  error( StdError::downloadNotSupported() );
}

KSync::ConfigWidget *DummyKonnector::configWidget( const KSync::Kapabilities& cap, QWidget* parent,
                                                   const char* name )
{
  return 0;
}

KSync::ConfigWidget *DummyKonnector::configWidget( QWidget* parent, const char* name )
{
  return 0;
}

bool DummyKonnector::writeSyncees()
{
  emit synceesWritten( this );
  return true;
}


#include "dummykonnector.moc"
