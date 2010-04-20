/*
    Copyright (c) 2010 Bertjan Broeksema <b.broeksema@home.nl>

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

class TaskListProxy : public ListProxy
{
public:
  enum Role {
    Summary,
    Description
  };

public:
  explicit TaskListProxy( int customRoleBaseline, QObject* parent = 0 );

  virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;

  virtual void setSourceModel( QAbstractItemModel* sourceModel );

private:
  int absoluteCustomRole( int role ) const { return role + mCustomRoleBaseline; }
  int relativeCustomRole( int role ) const { return role - mCustomRoleBaseline; }

private:
  int mCustomRoleBaseline;
};

#endif // TASKLISTPROXY_H
