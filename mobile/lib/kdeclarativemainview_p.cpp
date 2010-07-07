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
#include "kdeclarativemainview_p.h"

#include <KDE/KConfigGroup>
#include <KDE/KGlobal>
#include <KDE/KSharedConfig>

#include <akonadi_next/etmstatesaver.h>

KDeclarativeMainViewPrivate::KDeclarativeMainViewPrivate()
  : mChangeRecorder( 0 )
  , mCollectionFilter( 0 )
{ }

void KDeclarativeMainViewPrivate::restoreState()
{
  ETMStateSaver *saver = new ETMStateSaver;
  saver->setSelectionModel( mBnf->selectionModel() );
  KConfigGroup cfg( KGlobal::config(), "SelectionState" );
  saver->restoreState( cfg );
}

void KDeclarativeMainViewPrivate::saveState()
{
  ETMStateSaver saver;
  saver.setSelectionModel( mBnf->selectionModel() );

  KConfigGroup cfg( KGlobal::config(), "SelectionState" );
  saver.saveState( cfg );
  cfg.sync();
}

QStringList KDeclarativeMainViewPrivate::getFavoritesList()
{
  QStringList names;
  foreach ( const QString &group, KGlobal::config()->groupList() )
    if ( group.startsWith( sFavoritePrefix ) )
      names.append( QString( group ).remove( 0, sFavoritePrefixLength ) );
  return names;
}

QAbstractItemModel* KDeclarativeMainViewPrivate::getFavoritesListModel()
{
  mFavsListModel = new QStringListModel( getFavoritesList(), this );

  QSortFilterProxyModel *sortModel = new QSortFilterProxyModel( this );
  sortModel->setSourceModel( mFavsListModel );
  sortModel->setDynamicSortFilter( true );
  sortModel->sort(0, Qt::AscendingOrder);

  return sortModel;
}
