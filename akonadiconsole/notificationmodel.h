/*
    Copyright (c) 2009 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#ifndef AKONADICONSOLE_NOTIFICATIONMODEL_H
#define AKONADICONSOLE_NOTIFICATIONMODEL_H

#include <akonadi/private/notificationmessagev3_p.h>

#include <QtCore/QAbstractItemModel>
#include <QtCore/QDateTime>

#include "notificationsourceinterface.h"
#include "notificationmanagerinterface.h"

class NotificationModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit NotificationModel(QObject *parent);
    ~NotificationModel();

    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    bool isEnabled() const
    {
        return m_source != 0;
    }

public slots:
    void clear();
    void setEnabled(bool enable);

private slots:
    void slotNotify(const Akonadi::NotificationMessageV3::List &msgs);

private:
    class Item;
    class NotificationBlock;
    class NotificationNode;
    class NotificationEntity;

    QList<NotificationBlock *> m_data;

    org::freedesktop::Akonadi::NotificationManager *m_manager;
    org::freedesktop::Akonadi::NotificationSource *m_source;
};

#endif
