/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef MESSAGELISTPROXY_H
#define MESSAGELISTPROXY_H

#include <QtGui/QSortFilterProxyModel>
#include <akonadi/entitytreemodel.h>

/** Proxy model to provide roles for accessing KMime::Message properties from QML. */
class MessageListProxy : public QSortFilterProxyModel
{
  Q_OBJECT
  Q_PROPERTY( int messageCount READ messageCount )

  public:
    explicit MessageListProxy(QObject* parent = 0);
    enum Role {
      SubjectRole = Akonadi::EntityTreeModel::UserRole,
      FromRole,
      DateRole
    };

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    void setSourceModel(QAbstractItemModel* sourceModel);

    int messageCount() const; /// Returns the number of rows c.q. items
    Q_INVOKABLE qint64 messageId( int row ) const;
};

#endif
