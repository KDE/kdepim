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

#ifndef THREADGROUPERMODEL_H
#define THREADGROUPERMODEL_H

#include "mobileui_export.h"

#include <Akonadi/EntityTreeModel>

#include <QtGui/QItemSelectionModel>
#include <QtGui/QSortFilterProxyModel>

class ThreadGrouperModelPrivate;

class MOBILEUI_EXPORT ThreadGrouperModel : public QSortFilterProxyModel
{
  Q_OBJECT

  public:
    enum CustomRoles {
      // FIXME Fix custom role handling in proxies.
      ThreadIdRole = Akonadi::EntityTreeModel::UserRole + 30
    };

    enum OrderScheme {
      ThreadsWithNewRepliesOrder,
      ThreadsStartedOrder
    };

    ThreadGrouperModel( QObject* parent = 0 );
    virtual ~ThreadGrouperModel();

    void setThreadOrder( OrderScheme order );
    OrderScheme threadOrder() const;

    virtual void setSourceModel( QAbstractItemModel *sourceModel );

    virtual bool lessThan( const QModelIndex &left, const QModelIndex &right ) const;
    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;

  private:
    Q_DECLARE_PRIVATE( ThreadGrouperModel )
    ThreadGrouperModelPrivate* const d_ptr;
    Q_PRIVATE_SLOT( d_func(), void populateThreadGrouperModel() )
};

#endif
