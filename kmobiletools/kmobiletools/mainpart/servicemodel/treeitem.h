/***************************************************************************
   Copyright (C) 2007 by Matthias Lechner <matthias@lmme.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef TREEITEM_H
#define TREEITEM_H

#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QList>
#include <QtGui/QIcon>

/**
 * This class represents a generic tree item (inspired from a Qt example)
 *
 * @author Matthias Lechner <matthias@lmme.de>
 */
class TreeItem : public QObject {
    Q_OBJECT
public:
    enum ChildProperty { VisibleChildren = 0, AllChildren = 1 };

    TreeItem( QString data, TreeItem *parent = 0 );
    ~TreeItem();

    /**
     * Appends a child item to the current item
     *
     * @param child the child to append
     */
    void appendChild( TreeItem* child );

    /**
     * Removes a child from the current item
     * Please note that the item is NOT deleted.
     *
     * @param child the child to remove
     */
    void removeChild( TreeItem* child );

    /**
     * Sets the item's icon
     *
     * @param icon the icon
     */
    void setIcon( QIcon icon );

    /**
     * Sets whether the item should be displayed in a view
     *
     * @param visible true if the item
     */ 
    void setVisible( bool visible );

    /**
     * Returns the child at the given @p row
     *
     * @param row the row where the item is located
     * @return the child item
     */
    TreeItem* child( int row );

    /**
     * Returns the data of the current item
     *
     * @return the item data
     */
    QVariant data() const;

    /**
     * Returns the icon of the current item
     *
     * @return the item's icon
     */
    QIcon icon() const;

    /**
     * Returns whether the item is visible to a view
     *
     * @return true if the item is visible
     */
    bool visible() const;

    /**
     * Returns how many children the item has
     *
     * @param childProperty include items with the property only
     * @return the child count
     */
    int childCount( ChildProperty childProperty = VisibleChildren ) const;

    /**
     * Returns the position of this item within its parent
     *
     * @return the position
     */
    int row() const;

    /**
     * Returns the item's parent
     *
     * @return the parent
     */
    TreeItem* parent();

private:
    TreeItem* m_parentItem;
    QList<TreeItem*> m_childItems;
    QString m_itemData;
    QIcon m_icon;
    bool m_visible;
};

#endif
