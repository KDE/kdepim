/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

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

#ifndef THREADMODEL_H
#define THREADMODEL_H

#include "mobileui_export.h"

#include <AkonadiCore/EntityTreeModel>

#include <QtCore/QAbstractItemModel>
#include <QItemSelectionModel>
#include <QSortFilterProxyModel>

class ThreadModelPrivate;

class MOBILEUI_EXPORT ThreadModel : public QAbstractListModel
{
  Q_OBJECT

  public:

    enum Roles {
      ThreadRangeStartRole = Akonadi::EntityTreeModel::UserRole + 20,
      ThreadRangeEndRole,
      ThreadSizeRole,
      ThreadUnreadCountRole
    };

    explicit ThreadModel( QAbstractItemModel *emailModel, QObject *parent = 0 );
    virtual ~ThreadModel();

    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;

  private:
    Q_DECLARE_PRIVATE( ThreadModel )
    ThreadModelPrivate* const d_ptr;
    Q_PRIVATE_SLOT( d_func(), void populateThreadModel() )
    Q_PRIVATE_SLOT( d_func(), void slotRowsInserted( QModelIndex, int, int ) )
    Q_PRIVATE_SLOT( d_func(), void slotRowsRemoved( QModelIndex, int, int ) )
    Q_PRIVATE_SLOT( d_func(), void slotResetModel() )
};

#endif
