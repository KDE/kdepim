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

#ifdef __GNUG__
# pragma implementation "EmpathListView.h"
#endif

// QT includes
#include <qapplication.h>
#include <qtimer.h>

// Local includes
#include "EmpathListView.h"

EmpathListView::EmpathListView(
    QWidget * parent, const char * name)
    :   KListView(parent, name),
        waitForLink_(false),
        delayedLink_(false),
        dragEnabled_(true),
        maybeDrag_(false)
{
    viewport()->setMouseTracking(true);

    linkItem_ = 0;

    delayedLinkTimer_ = new QTimer(this);

    QObject::connect(this, SIGNAL(currentChanged(QListViewItem *)),
            this, SLOT(s_currentChanged(QListViewItem *)));

    QObject::connect(delayedLinkTimer_, SIGNAL(timeout()),
            this, SLOT(s_delayedLinkTimeout()));
}

EmpathListView::~EmpathListView()
{
}

    void
EmpathListView::setLinkItem(QListViewItem * i)
{ 
    delayedLinkTimer_->stop();
    if (i != linkItem_) {
        linkItem_ = i;
        emit linkChanged(i);
    }
}

    QListViewItem *
EmpathListView::itemAt(const QPoint & screenPos, 
    EmpathListView::Area & areaAtPos) const
{
    QListViewItem * i = itemAt(screenPos);
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

    void
EmpathListView::s_currentChanged(QListViewItem *)
{
    if (delayedLink_) {
        delayedLinkTimer_->start(400); // XXX: hardcoded
        delayedLink_ = false;
    }
}

    void
EmpathListView::s_delayedLinkTimeout()
{
    delayedLinkTimer_->stop();
    setLinkItem(currentItem());
}

    void
EmpathListView::contentsMousePressEvent(QMouseEvent *e)
{
    // Unfortunately there is a lot of duplicated code here.
    if (!e) 
        return;

    delayedLink_ = false;

    pressPos_ = e->pos();
    QPoint vp = contentsToViewport(pressPos_);
    pressItem_ = itemAt(vp, pressArea_);

    if (e->button() == RightButton) {

        if ( !pressItem_ ) {
            clearSelection();
            emit rightButtonPressed( 0, viewport()->mapToGlobal( vp ), -1);
            emit rightButtonPressed( 0, viewport()->mapToGlobal( vp ), 
                -1, pressArea_ );
            return;
        }
      
        setCurrentItem(pressItem_);
      
        int c = 0;
        int cumColumnWidth = columnWidth(c);
        while (vp.x() > cumColumnWidth) 
            cumColumnWidth += columnWidth(++c);
        
        emit rightButtonPressed( pressItem_, viewport()->mapToGlobal( vp ), c );
        emit rightButtonPressed( pressItem_, viewport()->mapToGlobal( vp ), c, pressArea_ );

        return;
    } 

    if (e->button() != LeftButton) 
        return;

    if (pressArea_ == Void)
        return;

    if (pressArea_ == OpenClose) {
        // setCurrentItem(pressItem_);
        setOpen(pressItem_, !pressItem_->isOpen());
        return;
    }

    // No modifier key pressed
    if (e->state() == 0) { 
        maybeDrag_ = (true & dragEnabled_);
        setCurrentItem(pressItem_);
        if (!pressItem_->isSelected()) {
            clearSelection();
            setSelected(pressItem_, true);
        }
        return;
    }
            
    if (e->state() & ControlButton) {
        return;
    }

    if (e->state() & ShiftButton) {
        return;
    }
}

    void
EmpathListView::contentsMouseReleaseEvent(QMouseEvent *e)
{ 
    maybeDrag_ = false;
    
    if (!e || pressArea_ != Item) 
        return;
    
    if (e->state() & ShiftButton) {

        if (!(e->state() & ControlButton))
            clearSelection();

        if (!currentItem()) {
            setCurrentItem(pressItem_);
            setSelected(pressItem_, true);
            return;
        }
    
        QListViewItem * i1 = 0;
        QListViewItem * i2 = 0;
   
        if (currentItem()->itemPos() < pressItem_->itemPos()) {
            i1 = currentItem();
            i2 = pressItem_;
        } else {
            i1 = pressItem_;
            i2 = currentItem();
        }
        
        while (i1 != i2) {
            setSelected(i1, true);
            i1 = i1->itemBelow();
        }
        setSelected(i2, true);

        setCurrentItem(pressItem_);
        return;
    }
 
    if (e->state() & ControlButton) {
        setCurrentItem(pressItem_);
        setSelected(pressItem_, !isSelected(pressItem_));
        return;
    }

    if (e->button() == LeftButton) {
        clearSelection();
        setSelected(pressItem_, true);
        setLinkItem(pressItem_);
    }
}

    void
EmpathListView::contentsMouseMoveEvent(QMouseEvent *e)
{
    // FIXME: Disable for the moment. Qt crashes !
    // return;
    
    if (!maybeDrag_)
        return;

    if (!e) 
        return;

    qDebug("We may be dragging");

    QPoint p = e->pos();

    if ( (p - pressPos_).manhattanLength() < 
                QApplication::startDragDistance()) {
        // Ignore, we haven't moved the cursor far enough.
        // QListView::contentsMouseMoveEvent(e);
        return;
    }

    qDebug("Ok, we're dragging");

    maybeDrag_ = false;

    if (!pressItem_) {
        qDebug("Not over anything to drag");
        // QListView::contentsMouseMoveEvent(e);
        return;
    }
    
    QList<QListViewItem> selected;   
    
    if (isMultiSelection()) {
        QListViewItemIterator it(this);
        while (it.current()) {
            if (it.current()->isSelected())
                selected.append(it.current());
            ++it;
        }
    } else
        selected.append(selectedItem());
        
    emit startDrag(selected);
}

    void
EmpathListView::keyPressEvent(QKeyEvent *e)
{ 
    /*
    delayedLink_ = true;

    if (e->key() == Key_Escape && isMultiSelection()) {
        clearSelection();
        return;
    }
    */
    
    KListView::keyPressEvent(e);
}

// vim:ts=4:sw=4:tw=78
