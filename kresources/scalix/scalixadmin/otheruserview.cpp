/*
 *   This file is part of ScalixAdmin.
 *
 *   Copyright (C) 2007 Trolltech ASA. All rights reserved.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <klocale.h>

#include "otherusermanager.h"

#include "otheruserview.h"

OtherUserModel::OtherUserModel( OtherUserManager *manager, QObject *parent )
  : QAbstractListModel( parent ), mManager( manager )
{
  connect( mManager, SIGNAL( changed() ), SLOT( userChanged() ) );
}

int OtherUserModel::rowCount( const QModelIndex &parent ) const
{
  if ( !parent.isValid() )
    return mManager->otherUsers().count();
  else
    return 0;
}

QVariant OtherUserModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || index.row() >= mManager->otherUsers().count() ) {
    return QVariant();
  }

  if ( role == Qt::DisplayRole || role == Qt::UserRole ) {
    return mManager->otherUsers()[ index.row() ];
  } else {
    return QVariant();
  }
}

QVariant OtherUserModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( section != 0 || orientation == Qt::Vertical || role != Qt::DisplayRole )
    return QVariant();

  return i18n( "Registered Accounts" );
}

void OtherUserModel::userChanged()
{
  reset();
}


OtherUserView::OtherUserView( OtherUserManager *manager, QWidget *parent )
  : QListView( parent ), mManager( manager )
{
  mModel = new OtherUserModel( mManager, this );

  setModel( mModel );

  connect( selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
           this, SIGNAL( selectionChanged() ) );
}

QString OtherUserView::selectedUser() const
{
  const QModelIndex index = selectionModel()->currentIndex();
  if ( index.isValid() )
    return index.data( Qt::DisplayRole ).toString();

  return QString();
}

#include "otheruserview.moc"
