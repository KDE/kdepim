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

#include <QtGui/QHeaderView>

#include "delegatemanager.h"

#include "delegateview.h"

DelegateModel::DelegateModel( DelegateManager *manager, QObject *parent )
  : QAbstractTableModel( parent ), mManager( manager )
{
  connect( mManager, SIGNAL( changed() ), SLOT( delegateChanged() ) );
}

int DelegateModel::rowCount( const QModelIndex &parent ) const
{
  if ( !parent.isValid() )
    return mManager->delegates().count();
  else
    return 0;
}

int DelegateModel::columnCount( const QModelIndex &parent ) const
{
  if ( !parent.isValid() )
    return 2;
  else
    return 0;
}

QVariant DelegateModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || index.row() >= mManager->delegates().count() ) {
    return QVariant();
  }

  if ( role == Qt::DisplayRole ) {
    if ( index.column() == 0 )
      return mManager->delegates().at( index.row() ).email();
    else if ( index.column() == 1 )
      return Scalix::Delegate::rightsAsString( mManager->delegates().at( index.row() ).rights() );
  } else if ( role == Qt::UserRole ) {
    QVariant variant;
    variant.setValue( mManager->delegates().at( index.row() ) );
    return variant;
  }

  return QVariant();
}

QVariant DelegateModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Vertical || role != Qt::DisplayRole || section > 1 )
    return QVariant();

  if ( section == 0 )
    return i18n( "Delegate" );
  else if ( section == 1 )
    return i18n( "Rights" );

  return QVariant();
}

void DelegateModel::delegateChanged()
{
  reset();
}


DelegateView::DelegateView( DelegateManager *manager, QWidget *parent )
  : QTableView( parent ), mManager( manager )
{
  horizontalHeader()->setStretchLastSection( true );
  setSelectionBehavior( QAbstractItemView::SelectRows );

  mModel = new DelegateModel( mManager, this );

  setModel( mModel );

  connect( selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
           this, SIGNAL( selectionChanged() ) );
}

Scalix::Delegate DelegateView::selectedDelegate() const
{
  const QModelIndex index = selectionModel()->currentIndex();
  if ( !index.isValid() )
    return Scalix::Delegate();

  return index.data( Qt::UserRole ).value<Scalix::Delegate>();
}

#include "delegateview.moc"
