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

#ifndef HIERARCHYRESOLVER_H
#define HIERARCHYRESOLVER_H

#include "mobileui_export.h"

#include <QtCore/QByteArray>
#include <QtCore/QSet>

/**
 * @short A utility class to resolve single parent-child relationships to a tree.
 *
 * This class allows the user to specify a couple of parent child relation ships
 * which it will resolve to child -> top-most parent relation ships.
 *
 * @author Tobias Koenig <tokoe@kde.org>
 */
class MOBILEUI_EXPORT HierarchyResolver
{
  public:
    /**
     * Adds a single node to the resolver.
     *
     * The node is known to have no relation, so it must
     * be a top node.
     *
     * @param identifier The unique identifier of this node
     */
    void addNode( const QByteArray &identifier );

    /**
     * Adds a child -> parent relation to the resolver.
     *
     * @param child The unique identifier of the child node.
     * @param parent The unique identifier of the parent node.
     */
    void addRelation( const QByteArray &child, const QByteArray &parent );

    /**
     * Resolves the hierarchy.
     */
    void resolve( const QSet<QByteArray> &existingIdentifiers );

    /**
     * Returns a hash with the child node as key and its resolved top-most parent
     * node as value.
     *
     * The top-most parent of a parent node is the node itself.
     */
    QHash<QByteArray, QByteArray> childParentMap() const;

    /**
     * Returns a hash with the top-most parent node as key and a set of all its descendant
     * nodes as value.
     */
    QHash<QByteArray, QSet<QByteArray> > parentChildrenMap() const;

  private:
    QSet<QByteArray> mTopNodes;
    QHash<QByteArray, QByteArray> mRelations;

    QHash<QByteArray, QByteArray> mChildParentResultMap;
    QHash<QByteArray, QSet<QByteArray> > mParentChildrenResultMap;
};

#endif
