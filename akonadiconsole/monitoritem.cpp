/*
 * Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "monitoritem.h"
#include "monitorsmodel.h"
#include "notificationsourceinterface.h"

#include <akonadi/servermanager.h>

#include <QtCore/QStringList>
#include <QtCore/QTimer>
#include <QtCore/QVector>

#include <KDebug>

MonitorItem::MonitorItem( const QString &identifier_, MonitorsModel* model):
  QObject( model ),
  identifier( identifier_ )
{

  QString service = QLatin1String( "org.freedesktop.Akonadi" );
  if ( Akonadi::ServerManager::hasInstanceIdentifier() ) {
    service += "." + Akonadi::ServerManager::instanceIdentifier();
  }

  mInterface = new org::freedesktop::Akonadi::NotificationSource(
      service, QLatin1String( "/subscriber/" ) + identifier,
      QDBusConnection::sessionBus(), this );
  if ( !mInterface ) {
    kWarning() << "Failed to connect to org.freedesktop.Akonadi.NotificationSource of subscriber" << identifier_;
    return;
  }

  if ( mInterface->lastError().isValid() ) {
    kWarning() << mInterface->lastError().message();
    return;
  }

  connect( mInterface, SIGNAL(monitoredCollectionsChanged()),
           this, SLOT(monitoredCollectionsChanged()) );
  connect( mInterface, SIGNAL(monitoredItemsChanged()),
           this, SLOT(monitoredItemsChanged()) );
  connect( mInterface, SIGNAL(monitoredResourcesChanged()),
           this, SLOT(monitoredResourcesChanged()) );
  connect( mInterface, SIGNAL(monitoredMimeTypesChanged()),
           this, SLOT(monitoredMimeTypesChanged()) );
  connect( mInterface, SIGNAL(isAllMonitoredChanged()),
           this, SLOT(isAllMonitoredChanged()) );
  connect( mInterface, SIGNAL(ignoredSessionsChanged()),
           this, SLOT(ignoredSessionsChanged()) );

  QTimer::singleShot( 0, this, SLOT(init()));
}

MonitorItem::~MonitorItem()
{
}

void MonitorItem::init()
{
  isAllMonitoredChanged();
  monitoredCollectionsChanged();
  monitoredItemsChanged();
  monitoredMimeTypesChanged();
  monitoredResourcesChanged();
  ignoredSessionsChanged();
}

void MonitorItem::isAllMonitoredChanged()
{
  allMonitored = mInterface->isAllMonitored();
  Q_EMIT changed( MonitorsModel::IsAllMonitoredColumn );
}

void MonitorItem::monitoredCollectionsChanged()
{
  const QVector<long long> list = mInterface->monitoredCollections();
  monitoredCollections.clear();
  for ( int i = 0; i < list.size(); i++ ) {
    if ( i > 0 ) monitoredCollections += QLatin1String( ", " );
    monitoredCollections += QString::number( list[i] );
  }

  Q_EMIT changed( MonitorsModel::MonitoredCollectionsColumn );
}

void MonitorItem::monitoredItemsChanged()
{
  const QVector<long long> list = mInterface->monitoredItems();
  monitoredItems.clear();
  for ( int i = 0; i < list.size(); i++ ) {
    if ( i > 0 ) monitoredItems += QLatin1String( ", " );
    monitoredItems += QString::number( list[i] );
  }

  Q_EMIT changed( MonitorsModel::MonitoredItemsColumn );
}

void MonitorItem::monitoredMimeTypesChanged()
{
  const QStringList mimeTypes = mInterface->monitoredMimeTypes();
  monitoredMimeTypes = mimeTypes.join( QLatin1String( ", " ) );
  Q_EMIT changed( MonitorsModel::MonitorsModel::MonitoredMimeTypesColumn );
}

void MonitorItem::monitoredResourcesChanged()
{
  const QVector<QByteArray> list = mInterface->monitoredResources();
  monitoredResources.clear();
  for ( int i = 0; i < list.size(); i++ ) {
    if ( i > 0 ) monitoredResources += QLatin1String( ", " );
    monitoredResources += list[i];
  }

  Q_EMIT changed( MonitorsModel::MonitorsModel::MonitoredResourcesColumn );
}

void MonitorItem::ignoredSessionsChanged()
{
  const QVector<QByteArray> list = mInterface->ignoredSessions();
  ignoredSessions.clear();
  for ( int i = 0; i < list.size(); i++ ) {
    if ( i > 0 ) ignoredSessions += QLatin1String( ", " );
    ignoredSessions += list[i];
  }

  Q_EMIT changed( MonitorsModel::MonitorsModel::MonitorsModel::IgnoredSessionsColumn );
}

