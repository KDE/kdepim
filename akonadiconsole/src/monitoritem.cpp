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
#include "akonadiconsole_debug.h"
#include <AkonadiCore/servermanager.h>

#include <QStringList>
#include <QTimer>
#include <QVector>

MonitorItem::MonitorItem(const QString &identifier_, MonitorsModel *model):
    QObject(model),
    identifier(identifier_),
    allMonitored(false)
{
    QTimer::singleShot(0, this, &MonitorItem::init);
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
    //allMonitored = mInterface->isAllMonitored();
    Q_EMIT changed(MonitorsModel::IsAllMonitoredColumn);
}

void MonitorItem::monitoredCollectionsChanged()
{
#if 0
    const QVector<qint64> list = mInterface->monitoredCollections();
    monitoredCollections.clear();
    for (int i = 0; i < list.size(); i++) {
        if (i > 0) {
            monitoredCollections += QLatin1String(", ");
        }
        monitoredCollections += QString::number(list[i]);
    }

    Q_EMIT changed(MonitorsModel::MonitoredCollectionsColumn);
#endif
}

void MonitorItem::monitoredItemsChanged()
{
#if 0
    const QVector<qint64> list = mInterface->monitoredItems();
    monitoredItems.clear();
    for (int i = 0; i < list.size(); i++) {
        if (i > 0) {
            monitoredItems += QLatin1String(", ");
        }
        monitoredItems += QString::number(list[i]);
    }

    Q_EMIT changed(MonitorsModel::MonitoredItemsColumn);
#endif
}

void MonitorItem::monitoredMimeTypesChanged()
{
#if 0
    const QStringList mimeTypes = mInterface->monitoredMimeTypes();
    monitoredMimeTypes = mimeTypes.join(QStringLiteral(", "));
    Q_EMIT changed(MonitorsModel::MonitoredMimeTypesColumn);
#endif
}

void MonitorItem::monitoredResourcesChanged()
{
#if 0
    const QVector<QByteArray> list = mInterface->monitoredResources();
    monitoredResources.clear();
    for (int i = 0; i < list.size(); i++) {
        if (i > 0) {
            monitoredResources += QStringLiteral(", ");
        }
        monitoredResources += QLatin1String(list[i]);
    }

    Q_EMIT changed(MonitorsModel::MonitoredResourcesColumn);
#endif
}

void MonitorItem::ignoredSessionsChanged()
{
#if 0
    const QVector<QByteArray> list = mInterface->ignoredSessions();
    ignoredSessions.clear();
    for (int i = 0; i < list.size(); i++) {
        if (i > 0) {
            ignoredSessions += QLatin1String(", ");
        }
        ignoredSessions += QLatin1String(list[i]);
    }

    Q_EMIT changed(MonitorsModel::IgnoredSessionsColumn);
#endif
}

