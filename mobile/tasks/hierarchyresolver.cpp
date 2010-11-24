/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Tobias Koenig <tokoe@kdab.com>

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

#include "hierarchyresolver.h"

void HierarchyResolver::addNode( const QByteArray &identifier )
{
  mTopNodes.insert( identifier );
}

void HierarchyResolver::addRelation( const QByteArray &child, const QByteArray &parent )
{
  mRelations.insert( child, parent );
}

QHash<QByteArray, QByteArray> HierarchyResolver::resolve() const
{
  QHash<QByteArray, QByteArray> result;

  // first insert all top nodes
  foreach ( const QByteArray &node, mTopNodes )
    result.insert( node, node );

  // search top-most parents for each added relation child node
  QHashIterator<QByteArray, QByteArray> it( mRelations );
  while ( it.hasNext() ) {
    it.next();

    // check if direct parent is top node
    if ( mTopNodes.contains( it.value() ) ) {
      result.insert( it.key(), it.value() ); // we are done
      continue;
    }

    // iterate up the parent path
    QByteArray parentNode = it.value();
    while ( mRelations.contains( parentNode ) )
      parentNode = mRelations.value( parentNode );

    result.insert( it.key(), parentNode );
  }

  return result;
}
