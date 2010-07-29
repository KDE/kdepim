/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>

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
#ifndef TASKLISTPROXY_H
#define TASKLISTPROXY_H

#include "listproxy.h"

#include <akonadi/entitytreemodel.h>

class TaskListProxy : public ListProxy
{
public:
  enum Role {
    // TODO: Find a better way to make this configurable. Possibly templates.
    Summary = Akonadi::EntityTreeModel::UserRole,
    Description,
    PercentComplete
  };

public:
  explicit TaskListProxy( QObject* parent = 0 );

  virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
  virtual bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );

  virtual void setSourceModel( QAbstractItemModel* sourceModel );

};

#endif // TASKLISTPROXY_H
