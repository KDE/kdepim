/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
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

// TODO: Make shift key work, make dragging of multiple items work.
//       document this class

#ifdef __GNUG__
# pragma implementation "EmpathListView.h"
#endif

// QT includes
#include <qtimer.h>

// Local includes
#include "EmpathListView.h"

EmpathListView::EmpathListView(
    QWidget * parent, const char * name)
    :    QListView(parent, name),
        updateLinks_(false),
        delayedLink_(false),
        waitForLink_(false),
        dragEnabled_(true),
        maybeDrag_(false)
{
    empathDebug("ctor");

    linkedItem_ = 0;

    delayedLinkTimer_ = new QTimer; 

    QObject::connect(this, SIGNAL(currentChanged(QListViewItem *)),
            this, SLOT(s_currentChanged(QListViewItem *)));

    QObject::connect(delayedLinkTimer_, SIGNAL(timeout()),
            this, SLOT(s_updateLink()));
}

EmpathListView::~EmpathListView()
{
}

    void
EmpathListView::setUpdateLinks(bool flag, UpdateAction actionOnUpdate )
{
    empathDebug("");
    updateLinks_ = flag;
    if (actionOnUpdate == Revert && linkedItem_) 
        setCurrentItem(linkedItem_);
    else if (actionOnUpdate == Update && currentItem())
        s_updateLink(currentItem());
}

    void
EmpathListView::s_currentChanged(QListViewItem * item)
{
    delayedLinkTimer_->stop();
    
    // The current item should always be selected in
    // single selection mode.
    if (!isMultiSelection()) setSelected(item, true);
    
    if (updateLinks_) { 
    
        if (delayedLink_) {
            delayedLinkTimer_->start(400); // XXX: hardcoded
            empathDebug("delayedLinkTimer started");
            delayedLink_ = false;
            return;
        } else
            s_updateLink(item);
    }
}

    void
EmpathListView::s_updateLink()
{
    s_updateLink(currentItem());
}

    void
EmpathListView::s_updateLink(QListViewItem *item)
{
    if (item && item != linkedItem_) {
        if (waitForLink_) {
            setEnabled(false);
        }
        // setCursor(waitCursor);
        s_showLink(item);
        linkedItem_ = item;
    }
}

    void
EmpathListView::s_showing()
{
    setCursor(arrowCursor);
    setEnabled(true);
}

    void
EmpathListView::s_showLink(QListViewItem * i)
{
    s_showing();
}

    void 
EmpathListView::startDrag(QListViewItem *i)
{
}
 
    void
EmpathListView::contentsMousePressEvent(QMouseEvent *e)
{
    if (!e) return;
    
    updateLinks_ = false;
            
    if (e->button() == RightButton) {
        QListView::contentsMousePressEvent(e);
        return;
    } 

    if (e->button() != LeftButton) return;
            
    if (!isMultiSelection()) {
        maybeDrag_ = (true & dragEnabled_);
        pressPos_ = e->pos();
        QListView::contentsMousePressEvent(e);
        return;
    }
            
    // No modifier key pressed
    if (e->state() == 0) { 

        maybeDrag_ = (true & dragEnabled_);
        pressPos_ = e->pos();
                
        // Behaviour in multiple selection mode should be the
        // same as in single selection mode:
        clearSelection();
        setMultiSelection(false);
                
        QListView::contentsMousePressEvent(e);

        setMultiSelection(true);                
                
        return;
    }
            
    if (e->state() & ControlButton ) {
        QListView::contentsMousePressEvent(e);
        return;
    }
}

    void
EmpathListView::contentsMouseReleaseEvent(QMouseEvent *e)
{  
    if (!e) return;
    
    if (!( e->button() == LeftButton && e->state() & ControlButton ))
        setUpdateLinks(true, Update);

    maybeDrag_ = false;
    
    QListView::contentsMouseReleaseEvent(e);
}

    void
EmpathListView::contentsMouseMoveEvent(QMouseEvent *e)
{   
    if (!maybeDrag_) return;

    if (!e) return;

    empathDebug("We may be dragging");

    QPoint p = e->pos();

    if ( (p - pressPos_).manhattanLength() < 5) { // FIXME: Hardcoded
        // Ignore, we haven't moved the cursor far enough.
        // QListView::contentsMouseMoveEvent(e);
        return;
    }

    empathDebug("Ok, we're dragging");

    maybeDrag_ = false;

    QListViewItem * item = itemAt(contentsToViewport(pressPos_));

    if (!item) {
        empathDebug("Not over anything to drag");
        // QListView::contentsMouseMoveEvent(e);
        return;
    }
    
    // It's up to the children what to do with it.
    startDrag(item);
}

    void
EmpathListView::keyPressEvent(QKeyEvent *e)
{  
    delayedLink_ = true;
    QListView::keyPressEvent(e);
}

// vim:ts=4:sw=4:tw=78
