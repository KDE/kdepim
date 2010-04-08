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

#include "etmstatesaver.h"

#include <QtCore/QModelIndex>
#include <QtGui/QItemSelection>
#include <QtGui/QTreeView>

#include <akonadi/entitytreemodel.h>

using namespace Akonadi;

ETMStateSaver::ETMStateSaver(QObject* parent)
  : KViewStateSaver(parent)
{
}

QModelIndex ETMStateSaver::indexFromConfigString(const QAbstractItemModel *model, const QString& key) const
{
  if ( key.startsWith( 'x' ) )
    return QModelIndex();

  Entity::Id id = key.mid( 1 ).toLongLong();
  if ( id < 0 )
    return QModelIndex();

  if ( key.startsWith( QLatin1Char( 'c' ) ) )
    return model->match( QModelIndex(), EntityTreeModel::CollectionIdRole, id, 1, Qt::MatchRecursive ).first();
  else if ( key.startsWith( QLatin1Char( 'i' ) ) )
    return model->match( QModelIndex(), EntityTreeModel::ItemIdRole, id, 1, Qt::MatchRecursive ).first();
  return QModelIndex();
}

QString ETMStateSaver::indexToConfigString(const QModelIndex& index) const
{
  if ( !index.isValid() )
    return QLatin1String( "x-1" );
  const Collection c = index.data( EntityTreeModel::CollectionRole ).value<Collection>();
  if ( c.isValid() )
    return QString::fromLatin1( "c%1" ).arg( c.id() );
  Entity::Id id = index.data( EntityTreeModel::ItemIdRole ).value<Entity::Id>();
  if ( id >= 0 )
    return QString::fromLatin1( "i%1" ).arg( id );
  return QString();
}
