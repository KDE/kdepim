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

#ifndef AKREGATORTREENODEITEM_H
#define AKREGATORTREENODEITEM_H

#include <klistview.h>

class QPoint;
class QString;

namespace Akregator {

class FolderItem;
class TreeNode;

/**
    abstract base class for all items in the feeds tree
*/
class TreeNodeItem : public KListViewItem
{
    
public:
     
    TreeNodeItem(FolderItem* parent, TreeNode* node);
    TreeNodeItem(FolderItem* parent, TreeNodeItem* after, TreeNode* node);
    TreeNodeItem(KListView* parent, TreeNode* node);
    TreeNodeItem(KListView* parent, TreeNodeItem* after, TreeNode* node);
    virtual ~TreeNodeItem();
    virtual TreeNode* node();
    
    virtual void nodeChanged();

    virtual QString toolTip() const;
    virtual TreeNodeItem* firstChild() const;
    virtual TreeNodeItem* nextSibling() const;
    virtual FolderItem* parent() const;

    virtual void showContextMenu(const QPoint& p) = 0;

    protected:
    
    TreeNode* m_node;
    
    virtual void paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int align );

    private:

    void initialize(TreeNode* node);
};

}

#endif
