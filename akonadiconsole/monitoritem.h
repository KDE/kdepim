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

#ifndef MONITORITEM_H
#define MONITORITEM_H

#include <QtCore/QObject>

#include "monitorsmodel.h"
#include "notificationsourceinterface.h"


class MonitorItem : public QObject
{
    Q_OBJECT

  public:
    explicit MonitorItem( const QString &identifier_, MonitorsModel *model );
    virtual ~MonitorItem();

    QString identifier;
    bool allMonitored;
    QString monitoredCollections;
    QString monitoredItems;
    QString monitoredResources;
    QString monitoredMimeTypes;
    QString ignoredSessions;

  Q_SIGNALS:
    void changed( MonitorsModel::Column column );

  private Q_SLOTS:
    void init();

    void collectionsMonitoredChanged();
    void itemsMonitoredChanged();
    void resourcesMonitoredChanged();
    void mimeTypesMonitoredChanged();
    void isAllMonitoredChanged();
    void ignoredSessionsChanged();

  private:
    org::freedesktop::Akonadi::NotificationSource *mInterface;
};

#endif // MONITORITEM_H
