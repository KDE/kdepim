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

#ifdef __GNUG__
# pragma implementation "EmpathMessageListWidget.h"
#endif

// System includes
#include <stdlib.h>

// Qt includes
#include <qheader.h>
#include <qmessagebox.h>
#include <qstring.h>
#include <qfile.h>
#include <qdatetime.h>
#include <qevent.h>

// KDE includes
#include <klocale.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <kapp.h>
#include <kglobal.h>
#include <kconfig.h>

// Local includes
#include "EmpathMessageMarkDialog.h"
#include "EmpathMessageList.h"
#include "EmpathMessageListWidget.h"
#include "EmpathIndexRecord.h"
#include "EmpathMessageViewWindow.h"
#include "EmpathConfig.h"
#include "Empath.h"
#include "EmpathUIUtils.h"
#include "EmpathTask.h"
    
QListViewItem * EmpathMessageListWidget::lastSelected_ = 0;

EmpathMessageListWidget::EmpathMessageListWidget(
        QWidget * parent, const char * name)
    :    QListView            (parent, name),
        maybeDrag_            (false),
        wantScreenUpdates_    (false),
        filling_            (false)
{
    empathDebug("ctor");
    
    setFrameStyle(QFrame::NoFrame);

    wantScreenUpdates_ = false;
    
    maybeDrag_ = false;
    
    lastHeaderClicked_ = -1;

    setAllColumnsShowFocus(true);
    setRootIsDecorated(true);
    
    setSorting(-1);

    addColumn(i18n("Subject"));
    addColumn(i18n("Sender"));
    addColumn(i18n("Date"));
    addColumn(i18n("Size"));
    
    px_xxx_    = empathIcon("tree");
    px_Sxx_    = empathIcon("tree-read");
    px_xMx_    = empathIcon("tree-marked");
    px_xxR_    = empathIcon("tree-replied");
    px_SMx_    = empathIcon("tree-read-marked");
    px_SxR_    = empathIcon("tree-read-replied");
    px_xMR_    = empathIcon("tree-marked-replied");
    px_SMR_    = empathIcon("tree-read-marked-replied");

    KConfig * c = KGlobal::config();
    c->setGroup(EmpathConfig::GROUP_GENERAL);
    
    for (int i = 0 ; i < 4 ; i++) {
        header()->setCellSize(i,
                c->readUnsignedNumEntry(
                    EmpathConfig::KEY_MESSAGE_LIST_SIZE_COLUMN +
                    QString().setNum(i), 80));
        setColumnWidthMode(i, QListView::Manual);
    }

    _setupMessageMenu();
    
    QObject::connect(this, SIGNAL(currentChanged(QListViewItem *)),
            this, SLOT(s_currentChanged(QListViewItem *)));

    // Connect return press to view.
    QObject::connect(this, SIGNAL(returnPressed(QListViewItem *)),
            this, SLOT(s_messageView()));
    
    // Connect right button up so we can produce the popup context menu.
    QObject::connect(
        this, SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int)),
        this, SLOT(s_rightButtonPressed(QListViewItem *, const QPoint &, int)));
    
    // Connect the header's section clicked signal so we can sort properly
    QObject::connect(header(), SIGNAL(sectionClicked(int)),
        this, SLOT(s_headerClicked(int)));
    
    markAsReadTimer_ = new EmpathMarkAsReadTimer(this);
    
}

EmpathMessageListWidget::~EmpathMessageListWidget()
{
    empathDebug("dtor");
    empathDebug("Saving column sizes and positions");
    empathDebug("XXX: Sort this so that positions can be restored");
    
    KConfig * c = KGlobal::config();
    c->setGroup(EmpathConfig::GROUP_GENERAL);
    
    for (int i = 0 ; i < 4 ; i++) {

        c->writeEntry(
            EmpathConfig::KEY_MESSAGE_LIST_SIZE_COLUMN + QString().setNum(i),
            header()->cellSize(i));
        
        c->writeEntry(
            EmpathConfig::KEY_MESSAGE_LIST_POS_COLUMN + QString().setNum(i),
            header()->cellPos(i));
    }
    
    c->sync();
    
    delete markAsReadTimer_;
    markAsReadTimer_ = 0;
}

    EmpathMessageListItem *
EmpathMessageListWidget::findRecursive(
        EmpathMessageListItem * initialItem, RMM::RMessageID & msgId)
{
    ASSERT(initialItem);

    EmpathMessageListItem * fChild =
        (EmpathMessageListItem *)initialItem->firstChild();
    
    if (fChild != 0) {
        EmpathMessageListItem * found = findRecursive(fChild, msgId);
        if (found != 0) return found;
    }
    
    EmpathMessageListItem * nextSibling =
        (EmpathMessageListItem *)initialItem->nextSibling();
    
    if (nextSibling != 0) {
        EmpathMessageListItem * found = findRecursive(nextSibling, msgId);
        if (found != 0) return found;
    }

    if (initialItem->messageID() == msgId) return initialItem;
    return 0;
}

    EmpathMessageListItem *
EmpathMessageListWidget::find(RMM::RMessageID & msgId)
{
    empathDebug("find (" + msgId.asString() + ") called");
    if (!firstChild()) return 0;
    return findRecursive((EmpathMessageListItem *)firstChild(), msgId);
}

    void
EmpathMessageListWidget::addItem(EmpathIndexRecord * item)
{
    empathDebug("addItem called");
    
    if (item == 0) {
        empathDebug("item == 0 !");
        return;
    }

    if (item->parentID().localPart().isEmpty()) {
        EmpathMessageListItem * newItem = _addItem(this, *item);
        setStatus(newItem, item->status());
        return;
    }

    // Find parent of this item.
    
    empathDebug("Message has parentID, looking for parent");
    empathDebug("PARENTID: \"" + item->parentID().asString() + "\"");
        
    EmpathMessageListItem * parentItem = 0;
    
    QListViewItemIterator it(this);
    
    for (; it.current(); ++it) {
//        empathDebug("Looking at message id \"" +
//            it.current()->messageID().asString() + "\"");
        EmpathMessageListItem * i = (EmpathMessageListItem *)it.current();
        if (i->messageID() == item->parentID()) {
            parentItem = (EmpathMessageListItem *)it.current();
            break;
        }
    }

    EmpathMessageListItem * newItem(0);

    if (parentItem == 0)
        newItem = _addItem(this, *item);
    else
        newItem = _addItem(parentItem, *item);

    setStatus(newItem, item->status());
}

    EmpathURL
EmpathMessageListWidget::firstSelectedMessage()
{
    EmpathURL u("orphaned");
    if (currentItem() == 0) return u;
    EmpathMessageListItem * item = (EmpathMessageListItem *)currentItem();
    return EmpathURL(url_.mailboxName(), url_.folderPath(), item->id());
}

    void
EmpathMessageListWidget::markOne(RMM::MessageStatus status)
{
    empathDebug("mark() called");
    // Don't bother auto-marking this as the user's done it.
    markAsReadTimer_->cancel();
    
    if (!currentItem())
        return;
    
    EmpathMessageListItem * item = (EmpathMessageListItem *)currentItem();
    
    EmpathURL u(url_.mailboxName(), url_.folderPath(), item->id());
    
    RMM::MessageStatus stat = item->status();
    
    RMM::MessageStatus s =
        (RMM::MessageStatus)
        (stat & status ? stat ^ status : stat | status);
    
    empath->mark(u, s);
    
    setStatus(item, s);
}

    void
EmpathMessageListWidget::mark(RMM::MessageStatus status)
{
    empathDebug("mark() called");
    // Don't bother auto-marking this as the user's done it.
    markAsReadTimer_->cancel();
    
    EmpathURL u(url_.mailboxName(), url_.folderPath(), QString::null);
    
    QStringList l; // Candidates for marking

    EmpathMessageListItemIterator it(selected_);
    
    for (; it.current(); ++it) {
        empathDebug("Adding " + it.current()->id());
        l.append(it.current()->id());
    }
        
    empath->mark(url_, l, status);
    
    for (it.toFirst(); it.current(); ++it)
        setStatus(it.current(), status);
}

    void
EmpathMessageListWidget::s_messageMark()
{
    markOne(RMM::Marked);
}

    void
EmpathMessageListWidget::s_messageMarkRead()
{
    markOne(RMM::Read);
}

    void
EmpathMessageListWidget::s_messageMarkReplied()
{
    markOne(RMM::Replied);
}

    void
EmpathMessageListWidget::s_messageReply()
{
    empathDebug("s_messageReply called");
    empath->s_reply(firstSelectedMessage());
}

    void
EmpathMessageListWidget::s_messageReplyAll()
{
    empathDebug("s_messageReplyAll called");
    empath->s_reply(firstSelectedMessage());
}

    void
EmpathMessageListWidget::s_messageForward()
{
    empathDebug("s_messageForward called");
    empath->s_forward(firstSelectedMessage());
}

    void
EmpathMessageListWidget::s_messageBounce()
{
    empathDebug("s_messageBounce called");
    empath->s_bounce(firstSelectedMessage());
}

    void
EmpathMessageListWidget::s_messageDelete()
{
    empathDebug("s_messageDelete called");
    
    EmpathURL u(url_);
    
    EmpathTask * t(empath->addTask(i18n("Deleting messages")));
    
    t->setMax(_nSelected());
    
    QList<EmpathURL> condemned;

    EmpathMessageListItemIterator it(selected_);
    
    for (; it.current(); ++it) {
                
        u.setMessageID(it.current()->id());
        
        // XXX RETVAL ? 
        empath->remove(u);
    
        _removeItem(it.current());
        
        t->doneOne();
    }
    t->done();
}

    void
EmpathMessageListWidget::s_messageSaveAs()
{
    empathDebug("s_messageSaveAs called");
    empath->saveMessage(firstSelectedMessage());
}

    void
EmpathMessageListWidget::s_messageCopyTo()
{
    empathDebug("s_messageCopyTo called");

}

    void
EmpathMessageListWidget::s_messagePrint()
{
    empathDebug("s_messagePrint called");

}

    void
EmpathMessageListWidget::s_messageFilter()
{
    empathDebug("s_messageFilter called");

}

    void
EmpathMessageListWidget::s_messageView()
{
    empathDebug("s_messageView called");
    
    EmpathMessageViewWindow * messageViewWindow =
        new EmpathMessageViewWindow(
            firstSelectedMessage(),
            i18n("Empath: Message View").ascii());
    
    CHECK_PTR(messageViewWindow);
    
    messageViewWindow->show();
}

    void
EmpathMessageListWidget::s_rightButtonPressed(
        QListViewItem * item, const QPoint & pos, int)
{
    if (item == 0) return;
    wantScreenUpdates_ = false;
    
    if (_nSelected() == 0)
        _setSelected(item, true);
    
    if (_nSelected() != 1) {
        multipleMessageMenu_.exec(pos);
        wantScreenUpdates_ = true;
        return;
    }
    
    EmpathMessageListItem * i = (EmpathMessageListItem *)item;

    if (i->status() & RMM::Read)
        messageMenu_.changeItem(messageMenuItemMarkRead,
            i18n("Mark as unread"));
    else
        messageMenu_.changeItem(messageMenuItemMarkRead,
            i18n("Mark as read"));

    if (i->status() & RMM::Replied)
        messageMenu_.changeItem(messageMenuItemMarkReplied,
            i18n("Mark as not replied to"));
    else
        messageMenu_.changeItem(messageMenuItemMarkReplied,
            i18n("Mark as replied to"));
    
    if (i->status() & RMM::Marked)
        messageMenu_.changeItem(messageMenuItemMark,
            i18n("Untag"));
    else
        messageMenu_.changeItem(messageMenuItemMark,
            i18n("Tag"));

    messageMenu_.exec(pos);
    wantScreenUpdates_ = true;
}

    void
EmpathMessageListWidget::s_doubleClicked(QListViewItem *)
{
    empathDebug("s_messageDoubleClicked called");
    s_messageView();
}

    void
EmpathMessageListWidget::s_currentChanged(QListViewItem * i)
{
    empathDebug("Current message changed - updating message widget");
    markAsReadTimer_->cancel();
    
    // Make sure we highlight the current item.
    kapp->processEvents();
    
    if (wantScreenUpdates_) {
        emit(changeView(firstSelectedMessage()));
        markAsReadTimer_->go((EmpathMessageListItem *)i);
    }
}

    void
EmpathMessageListWidget::setSignalUpdates(bool yn)
{
    wantScreenUpdates_ = yn;
}

    void
EmpathMessageListWidget::markAsRead(EmpathMessageListItem * item)
{
    EmpathURL u(url_.mailboxName(), url_.folderPath(), item->id());
    // XXX RETVAL ?
    empath->mark(u, RMM::MessageStatus(item->status() ^ RMM::Read));
    setStatus(item, RMM::MessageStatus(item->status() ^ RMM::Read));
}

    void
EmpathMessageListWidget::setStatus(
        EmpathMessageListItem * item, RMM::MessageStatus status)
{
    item->setStatus(status);

    if (status & RMM::Read)
        if (status & RMM::Marked)
            if (status & RMM::Replied)
                item->setPixmap(0, px_SMR_);
            else
                item->setPixmap(0, px_SMx_);
        else
            if (status & RMM::Replied)
                item->setPixmap(0, px_SxR_);
            else
                item->setPixmap(0, px_Sxx_);
    else
        if (status & RMM::Marked)
            if (status & RMM::Replied)
                item->setPixmap(0, px_xMR_);
            else
                item->setPixmap(0, px_xMx_);
        else
            if (status & RMM::Replied)
                item->setPixmap(0, px_xxR_);
            else
                item->setPixmap(0, px_xxx_);
    
    return;
}

    void
EmpathMessageListWidget::s_showFolder(const EmpathURL & url)
{
    empathDebug("s_showFolder(" + url.asString() + ") called");
    
    if (url_ == url) {
        emit(showing());
        return;
    }
    
    empath->s_infoMessage(
        i18n("Reading mailbox") + " " + url.asString());
    
    EmpathFolder * oldFolder = empath->folder(url_);
    
    if (oldFolder != 0) {
        
        QObject::disconnect(
            oldFolder,   SIGNAL(itemLeft(const QString &)),
            this,        SLOT(s_itemGone(const QString &)));
        
        QObject::disconnect(
            oldFolder,   SIGNAL(itemArrived    (const QString &)),
            this,        SLOT(s_itemCome        (const QString &)));
    }
    
    url_ = url;
    
    empathDebug("calling empath->folder(" + url_.asString() + ")");
    EmpathFolder * f = empath->folder(url_);
    
    QObject::connect(
        f,       SIGNAL(itemLeft(const QString &)),
        this,    SLOT(s_itemGone(const QString &)));
    
    QObject::connect(
        f,       SIGNAL(itemArrived    (const QString &)),
        this,    SLOT(s_itemCome        (const QString &)));
    
    if (f == 0) {
    
        empathDebug("Can't find folder !");
        emit(showing());
        return;
    }
    
    clear();
    masterList_.clear();
    
    f->index().sync();
    
    _fillDisplay(f);
}
    void
EmpathMessageListWidget::s_headerClicked(int i)
{
    // If the last header clicked on is the same as the one we're given, change
    // the sort order for the column. Otherwise, revert back to ascending order.

    if (lastHeaderClicked_ == i)
        sortType_ = !sortType_;
    
    else sortType_ = true; // revert
    
    setSorting(i, sortType_);
    
    KConfig * c(KGlobal::config());
    c->setGroup(EmpathConfig::GROUP_DISPLAY);
    
    c->writeEntry(EmpathConfig::KEY_MESSAGE_SORT_COLUMN, i);
    c->writeEntry(EmpathConfig::KEY_MESSAGE_SORT_ASCENDING, sortType_);
    
    lastHeaderClicked_ = i;
}

    void
EmpathMessageListWidget::_setupMessageMenu()
{
    messageMenuItemView =
        messageMenu_.insertItem(empathIcon("mini-view"), i18n("View"),
        this, SLOT(s_messageView()));
    
    messageMenu_.insertSeparator();
    
    messageMenuItemMark =
        messageMenu_.insertItem(
            px_xMx_, i18n("Tag"),
            this, SLOT(s_messageMark()));
    
    messageMenuItemMarkRead =
        messageMenu_.insertItem(
            px_Sxx_, i18n("Mark as Read"),
            this, SLOT(s_messageMarkRead()));
    
    messageMenuItemMarkReplied =
        messageMenu_.insertItem(
            px_xxR_, i18n("Mark as Replied"),
            this, SLOT(s_messageMarkReplied()));
        
    messageMenu_.insertSeparator();

    messageMenuItemReply =
    messageMenu_.insertItem(empathIcon("mini-reply"), i18n("Reply"),
        this, SLOT(s_messageReply()));

    messageMenuItemReplyAll =
    messageMenu_.insertItem(empathIcon("mini-reply"),i18n("Reply to A&ll"),
        this, SLOT(s_messageReplyAll()));

    messageMenuItemForward =
    messageMenu_.insertItem(empathIcon("mini-forward"), i18n("Forward"),
        this, SLOT(s_messageForward()));

    messageMenuItemDelete =
    messageMenu_.insertItem(empathIcon("mini-delete"), i18n("Delete"),
        this, SLOT(s_messageDelete()));

    messageMenuItemSaveAs =
    messageMenu_.insertItem(empathIcon("mini-save"), i18n("Save As"),
        this, SLOT(s_messageSaveAs()));
    
    multipleMessageMenu_.insertItem(i18n("Mark..."),
        this, SLOT(s_messageMarkMany()));
    
    multipleMessageMenu_.insertItem(
        empathIcon("mini-forward"), i18n("Forward"),
        this, SLOT(s_messageForward()));

    multipleMessageMenu_.insertItem(
        empathIcon("mini-delete"), i18n("Delete"),
        this, SLOT(s_messageDelete()));

    multipleMessageMenu_.insertItem(
        empathIcon("mini-save"), i18n("Save As"),
        this, SLOT(s_messageSaveAs()));
}

EmpathMarkAsReadTimer::EmpathMarkAsReadTimer(EmpathMessageListWidget * parent)
    :    QObject(),
        parent_(parent)
{
    QObject::connect(
        &timer_,    SIGNAL(timeout()),
        this,        SLOT(s_timeout()));
}

EmpathMarkAsReadTimer::~EmpathMarkAsReadTimer()
{
    empathDebug("dtor");
}

    void
EmpathMarkAsReadTimer::go(EmpathMessageListItem * i)
{
    item_ = i;
    // Don't bother if it's already read.
    if (i->status() & RMM::Read) return;

    KConfig * c(KGlobal::config());
    c->setGroup(EmpathConfig::GROUP_DISPLAY);
    
    if (!c->readBoolEntry(EmpathConfig::KEY_MARK_AS_READ)) return;
    int waitTime(c->readNumEntry(EmpathConfig::KEY_MARK_AS_READ_TIME, 2));
    timer_.start(waitTime * 1000, true);
}

    void
EmpathMarkAsReadTimer::cancel()
{
    timer_.stop();
}

    void
EmpathMarkAsReadTimer::s_timeout()
{
    if (!item_) return;

    parent_->markAsRead(item_);    
}

    void
EmpathMessageListWidget::contentsMousePressEvent(QMouseEvent * e)
{
    QListView::contentsMousePressEvent(e);
    return;
    empathDebug("MOUSE PRESS EVENT");
    
    if (!isUpdatesEnabled()) {
        empathDebug("UPDATES ARE NOT ENABLED!!!!!!!!!!");
    }
    
    QPoint pos = contentsToViewport(e->pos());
    
    // Ok, here's the method:
    // 
    // CASE 0:
    // If a button other than right or left has been used, ignore.
    // 
    // CASE 1:
    // If the right button has been used, we popup the menu.
    // 
    // So, we've got the left button.
    // 
    // CASE 2:
    // If no modifier keys were used, deselect all, and select under
    // cursor.
    // 
    // CASE 3:
    // If just control key is pressed, toggle selection state of
    // item under cursor.
    // 
    // CASE 4:
    // If just shift key is pressed, find item above the one under cursor
    // that's selected. If there is none, deselect all items (that means
    // ones below that under cursor) and select item under cursor.
    // 
    // CASE 5:
    // If there IS an item selected above that under cursor, deselect all
    // items but that one, and then select all items from that one down to
    // the one under cursor.
    // 
    // CASE 6:
    // If control AND shift keys are pressed, find item above one under
    // cursor that's selected. If there is none, clear all and select item
    // under cursor.
    // 
    // CASE 7:
    // If there IS an item selected above that under cursor, select all
    // items from that above, to that under cursor.
    
    
    // CASE 0: Neither right nor left buttons pressed
    if (e->button() != LeftButton && e->button() != RightButton) {
        empathDebug("CASE 0");
        return;
    }
    
    QListViewItem * item = itemAt(pos);

    if (!item) {
        empathDebug("No item under cursor");
        return;
    }
    
    // CASE 1: Right button pressed
    
    if (e->button() == RightButton) {
        empathDebug("CASE 1");
        s_rightButtonPressed(itemAt(pos), QCursor::pos(), 0); 
        return;
    }
    
    // Ok, it's the left button. We'll interject here and just set the
    // flag to say we may be about to drag.
    maybeDrag_ = true;
    dragStart_ = e->pos();
    

    // CASE 2: Left button pressed, but no modifier keys.
    
    if (e->state() == 0) {
        
        empathDebug("CASE 2");
        
        _clearSelection();
        
        _setSelected(item, true);
        lastSelected_ = item;
        
        s_currentChanged(item);
        
        return;
    }
        
    // CASE 3: Left button + control pressed
    if (e->state() == ControlButton) {
        empathDebug("CASE 3");
        
        if (!item->isSelected())
            lastSelected_ = item;

        _setSelected(item, !(item->isSelected()));

        return;
    }
    
    if ((e->state() & ShiftButton) && (lastSelected_ == 0)) {
        
        // CASE 4 and CASE 6:
        // Shift button pressed, but no prior selection.
        // Clear all selections, and select this only.
        // For CASE 6, if control is pressed, toggle instead.
        
        empathDebug("CASE 4/6");
        
        _clearSelection();
        
        // For CASE 6:
        if (e->state() & ControlButton) {
        
            if (!item->isSelected())
                lastSelected_ = item;

            _setSelected(item, !(item->isSelected()));
            
        } else {
        
            // For CASE 4:
            _setSelected(item, true);
            lastSelected_ = item;
        }
        
        return;
    }
    
    if (e->state() & ShiftButton) {
        
        // CASE 5, CASE 7:
        // There is an item already selected, as the above test didn't
        // hold.
        
        if (!(e->state() & ControlButton)) {
            
            // CASE 5:
            // Control button has not been held, so we must clear the
            // selection.
            empathDebug("CASE 5");
            _clearSelection();
        }
            
        QListViewItem * i = lastSelected_;
        
        // First see if the item we're looking at is below the last selected.
        // If so, work down. Otherwise, er... work up.
        if (i->itemPos() < item->itemPos()) {
        
            while (i && i != item) {
                
                _setSelected(i, true);

                i = i->itemBelow();
            }
            
        } else {
    
            while (i && i != item) {
                
                _setSelected(i, true);

                i = i->itemAbove();
            }
        }

        if (e->state() & ControlButton) {
            
            empathDebug("CASE 7");
            lastSelected_ = item;
        }

        _setSelected(item, true);
        return;
    }
}

    void
EmpathMessageListWidget::contentsMouseReleaseEvent(QMouseEvent * e)
{
    maybeDrag_ = false;
    QListView::contentsMouseReleaseEvent(e);
}

    void
EmpathMessageListWidget::contentsMouseMoveEvent(QMouseEvent * e)
{
    empathDebug("Mouse move event in progress");
    
    QListView::contentsMouseMoveEvent(e);
    return; // XXX: Still broken.
    
    if (!maybeDrag_) {
        return;
    }
    
    empathDebug("We may be dragging");
    
    QPoint p = e->pos();
    
    int deltax = abs(dragStart_.x() - p.x());
    int deltay = abs(dragStart_.y() - p.y());
    
    empathDebug("The delta is " + QString().setNum(deltax + deltay));
    
    if ((deltax + deltay) < 30) { // FIXME: Hardcoded
        // Ignore, we haven't moved the cursor far enough.
        QListView::contentsMouseMoveEvent(e);
        return;
    }
    
    empathDebug("Ok, we're dragging");
    
    maybeDrag_ = false;
    
    QListViewItem * item = itemAt(dragStart_);
    
    if (item == 0) {
        empathDebug("Not over anything to drag");
        QListView::contentsMouseMoveEvent(e);
        return;
    }
    
    EmpathMessageListItem * i = (EmpathMessageListItem *)item;
        
    empathDebug("Starting a drag");
    char * c = new char[i->id().length() + 1];
    strcpy(c, i->id().ascii());
    QTextDrag * u  = new QTextDrag(c, this);
    CHECK_PTR(u);
    
    u->setPixmap(empathIcon("tree")); 
    
    empathDebug("Starting the drag copy");
    u->drag();
}

    void
EmpathMessageListWidget::selectTagged()
{
    _clearSelection();

    viewport()->setUpdatesEnabled(false);
    
    wantScreenUpdates_ = false;

    QListViewItemIterator it(this);
    
    for (; it.current(); ++it) {
        
        EmpathMessageListItem * i = (EmpathMessageListItem *)it.current();
        
        if (i->status() & RMM::Marked) {
            _setSelected(i, true);
        }
    }
    
    wantScreenUpdates_ = true;
    viewport()->setUpdatesEnabled(true);
    triggerUpdate();
}

    void
EmpathMessageListWidget::selectRead()
{
    _clearSelection();

    viewport()->setUpdatesEnabled(false);
    
    wantScreenUpdates_ = false;

    QListViewItemIterator it(this);
    
    for (; it.current(); ++it) {
        
        EmpathMessageListItem * i = (EmpathMessageListItem *)it.current();
        
        if (i->status() & RMM::Read) {
            _setSelected(i, true);
        }
    }
    
    wantScreenUpdates_ = true;
    viewport()->setUpdatesEnabled(true);
    triggerUpdate();
}

    void
EmpathMessageListWidget::selectAll()
{
    viewport()->setUpdatesEnabled(false);
    
    QListViewItemIterator it(this);
    wantScreenUpdates_ = false;
    
    for (; it.current(); ++it)
        _setSelected(it.current(), true);
    
    wantScreenUpdates_ = true;
    viewport()->setUpdatesEnabled(true);
    triggerUpdate();
}    

    void
EmpathMessageListWidget::selectInvert()
{
    viewport()->setUpdatesEnabled(false);
    
    wantScreenUpdates_ = false;

    QListViewItemIterator it(this);
    
    for (; it.current(); ++it) {
        if (!it.current()->isSelected()) {
            _setSelected(it.current(), true);
        } else {
            _setSelected(it.current(), false);
        }
    }
    
    wantScreenUpdates_ = true;
    viewport()->setUpdatesEnabled(true);
    triggerUpdate();
}

    void
EmpathMessageListWidget::s_itemGone(const QString & s)
{
    empathDebug("itemGone(" + s + ")");
    if (filling_) return;
    
    QListViewItemIterator it(this);
    
    for (; it.current(); ++it) {
        
        EmpathMessageListItem * i = (EmpathMessageListItem *)it.current();
        
        if (i->id() == s)
            _removeItem(i);
    }
}

    void
EmpathMessageListWidget::s_itemCome(const QString & s)
{
    empathDebug("itemCome(" + s + ")");
    
    if (filling_) return;

    EmpathFolder * f(empath->folder(url_));
    
    if (f == 0)
        return;
        
    EmpathIndexRecord * i(f->index().record(s.ascii()));

    if (i == 0) {
        empathDebug("Can't find index record for \"" + s + "\"");
        return;
    }
    
    if (KGlobal::config()->readBoolEntry(EmpathConfig::KEY_THREAD_MESSAGES)) {
    
        addItem(i);
    
    } else {

        EmpathMessageListItem * newItem = _addItem(this, *i);
        setStatus(newItem, i->status());
    }
}

    void
EmpathMessageListWidget::_fillDisplay(EmpathFolder * f)
{
    filling_ = true;
    viewport()->setUpdatesEnabled(false);
    
    selected_.clear();
    clear();
    
    KGlobal::config()->setGroup(EmpathConfig::GROUP_DISPLAY);

    if (KGlobal::config()->readBoolEntry(EmpathConfig::KEY_THREAD_MESSAGES))
        _fillThreading(f);
    else
        _fillNonThreading(f);
    
    viewport()->setUpdatesEnabled(true);
    triggerUpdate();
    filling_ = false;
    
    emit(showing());
}

    void
EmpathMessageListWidget::_fillNonThreading(EmpathFolder * f)
{
    setRootIsDecorated(false);

    EmpathTask * t(empath->addTask(i18n("Sorting messages")));
    CHECK_PTR(t);
    
    setSorting(-1);

    t->setMax(f->messageCount());
    
    QStrList l(f->index().allKeys());
    
    empathDebug("There are " + QString().setNum(l.count()) + " keys");

    QStrListIterator it(l);

    for (; it.current(); ++it) {
        
        EmpathIndexRecord * rec = f->index().record(it.current());

        if (rec == 0) {
            empathDebug("Can't find index record.");
            continue;
        }
        
        EmpathMessageListItem * newItem = _addItem(this, *rec);
        
        setStatus(newItem, rec->status());
        
        t->doneOne();
        kapp->processEvents();
    }
    
    setSorting(
        KGlobal::config()->
            readNumEntry(EmpathConfig::KEY_MESSAGE_SORT_COLUMN, 2),
        KGlobal::config()->
            readNumEntry(EmpathConfig::KEY_MESSAGE_SORT_ASCENDING, true));
    
    t->done();

    empath->s_infoMessage(
        i18n("Finished reading mailbox") + " " + url_.asString());
}

    void
EmpathMessageListWidget::_fillThreading(EmpathFolder * f)
{
    setRootIsDecorated(true);
    
    
    EmpathTask * t(empath->addTask(i18n("Sorting messages")));
    CHECK_PTR(t);

    t->setMax(f->messageCount());
    
    QStrList l(f->index().allKeys());
    
    QStrListIterator it(l);
    
    for (; it.current(); ++it) {
        
        EmpathIndexRecord * rec = f->index().record(it.current());
        
        if (rec == 0) {
            continue;
        }
        
        masterList_.inSort(rec);
    }
    
    QListIterator<EmpathIndexRecord> mit(masterList_);
    
    setSorting(-1);

    
    
    for (; mit.current(); ++mit) {

        addItem(mit.current());
        t->doneOne();
        kapp->processEvents();
    }
    
    sortType_ = KGlobal::config()->
        readNumEntry(EmpathConfig::KEY_MESSAGE_SORT_ASCENDING, true);

    setSorting(
        KGlobal::config()->
        readNumEntry(EmpathConfig::KEY_MESSAGE_SORT_COLUMN, 3), sortType_);
    
    t->done();

    empath->s_infoMessage(
        i18n("Finished reading mailbox") + " " + url_.asString());
}

    void
EmpathMessageListWidget::s_messageMarkMany()
{
    EmpathMessageMarkDialog d;
    
    if (d.exec() != QDialog::Accepted)
        return;
    
    EmpathMessageMarkDialog::MarkType t = d.markType();

    RMM::MessageStatus s = d.status();
    
    QStringList l;

    empathDebug("There are " + QString().setNum(_nSelected()) +
        " selected messages");

    EmpathMessageListItemIterator it(selected_);
    
    for (; it.current(); ++it)
        l.append(it.current()->id());    
        
    empath->mark(url_, l, s);

    switch (t) {
        
        case EmpathMessageMarkDialog::On:

            for (it.toFirst(); it.current(); ++it)
                setStatus(it.current(),
                    RMM::MessageStatus(it.current()->status() | s));

            break;

        case EmpathMessageMarkDialog::Off:

            for (it.toFirst(); it.current(); ++it)
                setStatus(it.current(),
                    RMM::MessageStatus(it.current()->status() & (~s)));
        
            break;

        case EmpathMessageMarkDialog::Toggle:
            
            for (it.toFirst(); it.current(); ++it)
                setStatus(it.current(),
                    RMM::MessageStatus(
                        (it.current()->status() & s) ?
                        (it.current()->status() ^ s) :
                        (it.current()->status() | s)));

            break;
        
        default:
            break;
    }
}

    void
EmpathMessageListWidget::_clearSelection()
{
    viewport()->setUpdatesEnabled(false);
    clearSelection();
    selected_.clear();
    viewport()->setUpdatesEnabled(true);
    triggerUpdate();
}

    void
EmpathMessageListWidget::_setSelected(QListViewItem * item, bool b)
{
    _setSelected((EmpathMessageListItem *)item, b);
}

    void
EmpathMessageListWidget::_setSelected(EmpathMessageListItem * item, bool b)
{
    if (b) {
        
        if (!item->isSelected())
            selected_.append(item);
        
        item->setSelected(true);
        
    } else {
        
        selected_.remove(item);
        item->setSelected(false);
    }
    
}

    Q_UINT32
EmpathMessageListWidget::_nSelected()
{
    return selected_.count();
}

    EmpathMessageListItem *
EmpathMessageListWidget::_addItem(
    EmpathMessageListItem * prt, EmpathIndexRecord & d)
{
    EmpathMessageListItem * i = new EmpathMessageListItem(prt, d);
    itemList_.append(i);
    return i;
}

    EmpathMessageListItem *
EmpathMessageListWidget::_addItem(
    EmpathMessageListWidget * prt, EmpathIndexRecord & d)
{
    EmpathMessageListItem * i = new EmpathMessageListItem(prt, d);
    itemList_.append(i);
    return i;
}

    void
EmpathMessageListWidget::_removeItem(EmpathMessageListItem * i)
{
    itemList_.remove(i);
    delete i;
    selected_.remove(i);
}

// vim:ts=4:sw=4:tw=78
