/*
    This file is part of Akregator.

    Copyright (C) 2004 Frank Osterfeld <frank.osterfeld at kdemail.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "actionmanager.h"
#include "feed.h"
#include "feeditem.h"

#include <qpopupmenu.h>
#include <kaction.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <qstring.h>

namespace Akregator {

FeedItem::FeedItem(FolderItem* parent, Feed* node) : TreeNodeItem(parent, node)
{
    initialize(node);
}

FeedItem::FeedItem(KListView* parent, Feed* node) : TreeNodeItem(parent, node)
{
    initialize(node);
}

FeedItem::FeedItem(KListView* parent, TreeNodeItem* after, Feed* node) : TreeNodeItem(parent, after, node)
{
    initialize(node);
}


FeedItem::FeedItem(FolderItem* parent, TreeNodeItem* after, Feed* node) : TreeNodeItem(parent, after, node)
{
    initialize(node);
}

FeedItem::~FeedItem()
{
}

Feed* FeedItem::node() 
{ 
    return static_cast<Feed*> (m_node); 
}

void FeedItem::nodeChanged()
{
    if ( node()->fetchErrorOccurred() )
        setPixmap(0, errorPixmap());
    else
    {
        if (!node()->favicon().isNull())
             setPixmap(0, node()->favicon());
        else
        {
            setPixmap( 0, defaultPixmap() );
            node()->loadFavicon();
        }
    }
    
    TreeNodeItem::nodeChanged();
}

QPixmap FeedItem::errorPixmap()
{
    return KGlobal::iconLoader()->loadIcon("error", KIcon::Small);
}

QPixmap FeedItem::defaultPixmap()
{
    return KGlobal::iconLoader()->loadIcon("txt", KIcon::Small);
}

void FeedItem::initialize(Feed* node)
{
    setExpandable(false);
    if (node)
    {
        setText(0, node->title());
        if (!node->favicon().isNull())
            setPixmap( 0, node->favicon() );
        else
        {
            setPixmap( 0, defaultPixmap() );
            node->loadFavicon();
        }
    }
}

void FeedItem::showContextMenu(const QPoint& p)
{
    QWidget* w = ActionManager::getInstance()->container("feeds_popup");
    if (w)
        static_cast<QPopupMenu *>(w)->exec(p);
}

} // namespace Akregator

