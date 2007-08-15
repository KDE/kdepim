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

#include "treeitem.h"

#include <QtCore/QString>

TreeItem::TreeItem( QString data, TreeItem *parent )
{
    m_parentItem = parent;
    m_itemData = data;
    m_visible = true;
}

TreeItem::~TreeItem()
{
    qDeleteAll( m_childItems );
    m_childItems.clear();
}

void TreeItem::appendChild( TreeItem *item )
{
    m_childItems.append( item );
}

void TreeItem::removeChild( TreeItem *item )
{
    m_childItems.removeAt( m_childItems.indexOf( item ) );
}

void TreeItem::setIcon( QIcon icon ) {
    m_icon = icon;
}

void TreeItem::setVisible( bool visible ) {
    m_visible = visible;
}

bool TreeItem::visible() const {
    return m_visible;
}

TreeItem *TreeItem::child( int row )
{
    return m_childItems.value( row );
}

int TreeItem::childCount( ChildProperty childProperty ) const
{
    int count = 0;

    if( childProperty == AllChildren )
        count = m_childItems.count();
    else if( childProperty == VisibleChildren ) {
        for( int i=0; i<m_childItems.count(); i++ ) {
            if( m_childItems.at( i )->visible() )
                count++;
        }
    }
    else
        count = m_childItems.count();

    return count;
}

QVariant TreeItem::data() const
{
    return m_itemData;
}

QIcon TreeItem::icon() const
{
    return m_icon;
}

TreeItem *TreeItem::parent()
{
    return m_parentItem;
}

int TreeItem::row() const
{
    if( m_parentItem )
        return m_parentItem->m_childItems.indexOf( const_cast<TreeItem*>(this) );

    return 0;
}

#include "treeitem.moc"
