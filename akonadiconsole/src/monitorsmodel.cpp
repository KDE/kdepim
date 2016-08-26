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

#include "monitorsmodel.h"
#include "akonadiconsole_debug.h"
#include <AkonadiCore/Monitor>
#include <AkonadiCore/NotificationSubscriber>
#include <AkonadiCore/Session>

#include <QTimer>

MonitorsModel::MonitorsModel(QObject *parent):
    QAbstractItemModel(parent),
    mMonitor(Q_NULLPTR)
{
    QTimer::singleShot(0, this, &MonitorsModel::init);
}

MonitorsModel::~MonitorsModel()
{
}

void MonitorsModel::init()
{
    mMonitor = new Akonadi::Monitor(this);
    mMonitor->setTypeMonitored(Akonadi::Monitor::Subscribers, true);
    connect(mMonitor, &Akonadi::Monitor::notificationSubscriberAdded,
            this, &MonitorsModel::slotSubscriberAdded);
    connect(mMonitor, &Akonadi::Monitor::notificationSubscriberChanged,
            this, &MonitorsModel::slotSubscriberChanged);
    connect(mMonitor, &Akonadi::Monitor::notificationSubscriberRemoved,
            this, &MonitorsModel::slotSubscriberRemoved);
}

void MonitorsModel::slotSubscriberAdded(const Akonadi::NotificationSubscriber &subscriber)
{
    beginInsertRows(QModelIndex(), mData.count(), mData.count());
    mData.push_back(subscriber);
    endInsertRows();
}

void MonitorsModel::slotSubscriberRemoved(const Akonadi::NotificationSubscriber &subscriber)
{
    int idx = -1;
    for (auto it = mData.begin(), end = mData.end(); it != end; ++it) {
        ++idx;
        if (it->subscriber() == subscriber.subscriber()) {
            beginRemoveRows(QModelIndex(), idx, idx);
            mData.erase(it);
            endRemoveRows();
            return;
        }
    }
}

void MonitorsModel::slotSubscriberChanged(const Akonadi::NotificationSubscriber &subscriber)
{
    int idx = -1;
    for (auto it = mData.begin(), end = mData.end(); it != end; ++it) {
        ++idx;
        if (it->subscriber() == subscriber.subscriber()) {
            *it = subscriber;
            Q_EMIT dataChanged(index(idx, 0), index(idx, ColumnsCount));
            return;
        }
    }
}

QVariant MonitorsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (static_cast<Column>(section)) {
            case IdentifierColumn: return QStringLiteral("Subscriber");
            case SessionColumn: return QStringLiteral("Session");
            case IsAllMonitoredColumn: return QStringLiteral("All");
            case MonitoredCollectionsColumn: return QStringLiteral("Collections");
            case MonitoredItemsColumn: return QStringLiteral("Items");
            case MonitoredTagsColumn: return QStringLiteral("Tags");
            case MonitoredResourcesColumn: return QStringLiteral("Resources");
            case MonitoredMimeTypesColumn: return QStringLiteral("Mime Types");
            case MonitoredTypesColumn: return QStringLiteral("Types");
            case IgnoredSessionsColumn: return QStringLiteral("Ignored Sessions");
            case IsExclusiveColumn: return QStringLiteral("Exclusive");
            case ColumnsCount: Q_ASSERT(false); return QString();
            }
        }
    }

    return QVariant();
}

namespace
{

template<typename T>
QString toString(const QSet<T> &set)
{
    QStringList rv;
    for (const auto &v : set) {
        rv << QVariant(v).toString();
    }
    return rv.join(QStringLiteral(", "));
}

template<>
QString toString(const QSet<Akonadi::Monitor::Type> &set)
{
    QStringList rv;
    for (auto v : set) {
        switch (v) {
        case Akonadi::Monitor::Items:
            rv << QStringLiteral("Items");
            break;
        case Akonadi::Monitor::Collections:
            rv << QStringLiteral("Collections");
            break;
        case Akonadi::Monitor::Tags:
            rv << QStringLiteral("Tags");
            break;
        case Akonadi::Monitor::Relations:
            rv << QStringLiteral("Relations");
            break;
        case Akonadi::Monitor::Subscribers:
            rv << QStringLiteral("Subscribers");
            break;
        }
    }
    return rv.join(QStringLiteral(", "));
}

}

QVariant MonitorsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    if (index.row() >= mData.count()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
        const auto subscriber = mData[index.row()];
        switch (static_cast<Column>(index.column())) {
        case IdentifierColumn: return subscriber.subscriber();
        case SessionColumn: return subscriber.sessionId();
        case IsAllMonitoredColumn: return subscriber.isAllMonitored();
        case MonitoredCollectionsColumn: return toString(subscriber.monitoredCollections());
        case MonitoredItemsColumn: return toString(subscriber.monitoredItems());
        case MonitoredTagsColumn: return toString(subscriber.monitoredTags());
        case MonitoredResourcesColumn: return toString(subscriber.monitoredResources());
        case MonitoredMimeTypesColumn: return toString(subscriber.monitoredMimeTypes());
        case MonitoredTypesColumn: return toString(subscriber.monitoredTypes());
        case IgnoredSessionsColumn: return toString(subscriber.ignoredSessions());
        case IsExclusiveColumn: return subscriber.isExclusive();
        case ColumnsCount: Q_ASSERT(false); return QString();
        }
    }

    return QVariant();
}

int MonitorsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return ColumnsCount;
}

int MonitorsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return mData.count();
}

QModelIndex MonitorsModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child);

    return QModelIndex();
}

QModelIndex MonitorsModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (row >= mData.count()) {
        return QModelIndex();
    }

    return createIndex(row, column);
}

