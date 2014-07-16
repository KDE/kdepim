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

#include "favoriteslistmodel.h"

#include <KConfig>
#include <KConfigGroup>

static const QString sFavoritePrefix = QLatin1String("Favorite_");
static const QString sFavoriteOrder = QLatin1String("FavoriteOrder");
static const int sFavoritePrefixLength = 9;

FavoritesListModel::FavoritesListModel( const KSharedConfigPtr &config, QObject *parent )
  : QStringListModel( parent ), mConfig( config )
{
  reparseConfiguration();
}

void FavoritesListModel::reparseConfiguration()
{
  QStringList list;
  foreach ( const QString &group, mConfig->groupList() )
    if ( group.startsWith( sFavoritePrefix ) )
      list.append( QString( group ).remove( 0, sFavoritePrefixLength ) );

  const KConfigGroup group = mConfig->group("FavoriteGeneral");
  QStringList favsList = group.readEntry( "Order", QStringList() );

  QStringList::iterator it = favsList.begin();
  while ( it != favsList.end() ) {
    if ( !list.contains( *it ) ) {
      it = favsList.erase( it );
    } else {
      ++it;
    }
  }

  foreach ( const QString &item, list ) {
    if ( !favsList.contains( item ) )
      favsList.append( item );
  }

  setStringList( favsList );
}

void FavoritesListModel::saveConfig()
{
  QStringList favsList = stringList();
  foreach ( const QString &group, mConfig->groupList() ) {
    if ( group.startsWith( sFavoritePrefix ) ) {
      const QString name = QString( group ).remove( 0, sFavoritePrefixLength );
      if ( !favsList.contains( name ) ) {
        mConfig->deleteGroup( group );
      }
    }
  }

  QStringList::iterator it = favsList.begin();
  while ( it != favsList.end() ) {
    if ( !mConfig->groupList().contains( sFavoritePrefix + *it ) ) {
      it = favsList.erase( it );
    } else {
      ++it;
    }
  }

  KConfigGroup group = mConfig->group( "FavoriteGeneral" );
  group.writeEntry( "Order", favsList );
  mConfig->sync();
  mConfig->reparseConfiguration(); // needed to make sure that the groups got really removed from the KConfig object
}

void FavoritesListModel::moveUp( int row )
{
  if ( row <= 0 )
    return;

  QStringList list = stringList();

  if ( row >= list.size() )
    return;

  list.move( row, row - 1 );

  // resets the model.
  setStringList( list );
  saveConfig();
}

void FavoritesListModel::moveDown( int row )
{
  if ( row < 0 )
    return;

  QStringList list = stringList();

  if ( row >= list.size() - 1 )
    return;

  list.move( row, row + 1 );

  // resets the model.
  setStringList( list );
  saveConfig();
}

void FavoritesListModel::removeItem( int row )
{
  QStringListModel::removeRow( row );
  saveConfig();
}
