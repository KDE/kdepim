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

#ifndef MONITORSMODEL_H
#define MONITORSMODEL_H

#include <QAbstractItemModel>
#include <AkonadiCore/NotificationSubscriber>

class MonitorItem;

namespace Akonadi {
class Monitor;
}

class MonitorsModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Column {
        IdentifierColumn = 0,
        SessionColumn,
        IsAllMonitoredColumn,
        MonitoredCollectionsColumn,
        MonitoredItemsColumn,
        MonitoredTagsColumn,
        MonitoredResourcesColumn,
        MonitoredMimeTypesColumn,
        MonitoredTypesColumn,
        IgnoredSessionsColumn,
        IsExclusiveColumn,

        ColumnsCount
    };

    explicit MonitorsModel(QObject *parent = Q_NULLPTR);
    virtual ~MonitorsModel();

    void setEnabled(bool enabled);

    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &child) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

private Q_SLOTS:
    void init();

    void slotSubscriberAdded(const Akonadi::NotificationSubscriber &subscriber);
    void slotSubscriberChanged(const Akonadi::NotificationSubscriber &subscriber);
    void slotSubscriberRemoved(const Akonadi::NotificationSubscriber &subscriber);

private:
    QVector<Akonadi::NotificationSubscriber> mData;
    Akonadi::Monitor *mMonitor;
};

#endif // MONITORSMODEL_H
