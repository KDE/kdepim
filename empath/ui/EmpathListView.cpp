/*
    Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// TODO: Make shift key work, document this class, have an indicator
// to show when the mouse pointer is over an item. 

// QT includes
#include <qapplication.h>

// Local includes
#include "EmpathListView.h"

EmpathListView::EmpathListView(
    QWidget * parent, const char * name)
    :   QListView(parent, name)
{
}

EmpathListView::~EmpathListView()
{
}

    QListViewItem *
EmpathListView::itemAt(const QPoint & screenPos, 
    EmpathListView::Area & areaAtPos) const
{
    QListViewItem * i = QListView::itemAt(screenPos);
    if (!i) { 
        areaAtPos = Void;
        return i;
    }

    int xrel = screenPos.x() - ( i->depth() - !rootIsDecorated() ) * treeStepSize();

    if (xrel >= treeStepSize()) 
        areaAtPos = Item;
    else if (xrel >= 0 && i->childCount() > 0)
        areaAtPos = OpenClose;
    else 
        areaAtPos = Void;

    return i;
}

    QList<QListViewItem>
EmpathListView::thread(QListViewItem * item)
{
    // find the topmost item of this thread.
    QListViewItem * top = item;

    while (top->parent())
        top = top->parent();
    
    return subThread(top);
}

    QList<QListViewItem>
EmpathListView::subThread(QListViewItem * item)
{
    QList<QListViewItem> subThreadList;
    QList<QListViewItem> childThreadList;
    
    if (!item)
        return subThreadList;
    
    subThreadList.append(item);
    
    QListViewItem * child = item->firstChild();
    
    while (child) {
        childThreadList = subThread(child);
        
        QListIterator<QListViewItem> it(childThreadList);
        for (; it.current(); ++it) 
            subThreadList.append(it.current());
        
        child = child->nextSibling();
    }

    return subThreadList;
}

// vim:ts=4:sw=4:tw=78
