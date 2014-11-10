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

#include <QtCore/QAbstractItemModel>

#include "notificationmanagerinterface.h"

class MonitorItem;

class MonitorsModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Column {
        IdentifierColumn = 0,
        IsAllMonitoredColumn ,
        MonitoredCollectionsColumn,
        MonitoredItemsColumn,
        MonitoredResourcesColumn,
        MonitoredMimeTypesColumn,
        IgnoredSessionsColumn,

        ColumnsCount
    };

    explicit MonitorsModel(QObject *parent = 0);
    virtual ~MonitorsModel();

    void setEnabled(bool enabled);

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    int columnCount(const QModelIndex &parent) const;
    int rowCount(const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;

private Q_SLOTS:
    void init();

    void slotItemChanged(MonitorsModel::Column row);
    void slotSubscriberSubscribed(const QString &identifier);
    void slotSubscriberUnsubscribed(const QString &identifier);

private:
    QMap<QString, MonitorItem * > mData;
    org::freedesktop::Akonadi::NotificationManager *mManager;
};

#endif // MONITORSMODEL_H
