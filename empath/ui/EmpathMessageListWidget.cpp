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
        parent_                ((EmpathMainWindow *)parent),
        nSelected_            (0),
        maybeDrag_            (false),
        wantScreenUpdates_    (false),
        filling_            (false)
{
    empathDebug("ctor");
    
    setFrameStyle(QFrame::NoFrame);

    setUpdatesEnabled(false);
    
    itemList_.setAutoDelete(true);
    
    parent_ = (EmpathMainWindow *)parent;
    wantScreenUpdates_ = false;
    setMultiSelection(true);
    
    maybeDrag_ = false;
    
    lastHeaderClicked_ = -1;

    setAllColumnsShowFocus(true);
    setMultiSelection(false);
    setRootIsDecorated(true);
    
    setSorting(-1);

    addColumn(i18n("Subject"));
    addColumn(i18n("Sender"));
    addColumn(i18n("Date"));
    addColumn(i18n("Size"));
    
    px_xxx_    = empathIcon("tree.png");
    px_Sxx_    = empathIcon("tree-read.png");
    px_xMx_    = empathIcon("tree-marked.png");
    px_xxR_    = empathIcon("tree-replied.png");
    px_SMx_    = empathIcon("tree-read-marked.png");
    px_SxR_    = empathIcon("tree-read-replied.png");
    px_xMR_    = empathIcon("tree-marked-replied.png");
    px_SMR_    = empathIcon("tree-read-marked-replied.png");

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

    EmpathMessageListItem * newItem;
    
    if (item->parentID().localPart().isEmpty()) {
        newItem = new EmpathMessageListItem(this, *item);
        CHECK_PTR(newItem);
        
        itemList_.append(newItem);
        
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

    if (parentItem == 0) {
        
        empathDebug("No parent for this item");
        newItem = new EmpathMessageListItem(this, *item);
        CHECK_PTR(newItem);
        itemList_.append(newItem);
        empathDebug("Created OK");
        
    } else {
        
        empathDebug("There's parent for this item");
        newItem = new EmpathMessageListItem(parentItem, *item);
        CHECK_PTR(newItem);
        itemList_.append(newItem);
        empathDebug("Created OK");
    }

    setStatus(newItem, item->status());
}

/*
    void
EmpathMessageListWidget::getDescendants(
        EmpathMessageListItem * initialItem,
        QList<EmpathMessageListItem> * itemList)
{
    EmpathMessageListItem * firstChild =
    (EmpathMessageListItem *)initialItem->firstChild();
    
    if (firstChild)
        getDescendants(firstChild, itemList);
    
    EmpathMessageListItem * nextSibling =
    (EmpathMessageListItem *)initialItem->nextSibling();
    
    if (nextSibling)
        getDescendants(nextSibling, itemList);

    itemList->append(initialItem);
}
*/

    EmpathURL
EmpathMessageListWidget::firstSelectedMessage()
{
    EmpathURL u("orphaned");
    if (currentItem() == 0) return u;
    EmpathMessageListItem * item = (EmpathMessageListItem *)currentItem();
    u = EmpathURL(url_.mailboxName(), url_.folderPath(), item->id());
    return u;
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

    _updateSelected();

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
    
    _updateSelected();
    t->setMax(selected_.count());
    
    QList<EmpathURL> condemned;

    EmpathMessageListItemIterator it(selected_);
    
    for (; it.current(); ++it) {
                
        empathDebug(((EmpathMessageListItem *)it.current())->id());
        u.setMessageID(((EmpathMessageListItem *)it.current())->id());
        

        
        if (!empath->remove(u))
            empathDebug("Couldn't remove message \"" + u.asString() + "\"");
    
        itemList_.remove(it.current());
        
        t->doneOne();
    }
    t->done();
}

    void
EmpathMessageListWidget::s_messageSaveAs()
{
    empathDebug("s_messageSaveAs called");
    QString saveFilePath =
        KFileDialog::getSaveFileName(
            QString::null, QString::null, this,
            i18n("Empath: Save Message").ascii());
    empathDebug(saveFilePath);
    
    if (saveFilePath.isEmpty()) {
        empathDebug("No filename given");
        return;
    }
    
    QFile f(saveFilePath);
    if (!f.open(IO_WriteOnly)) {
        // Warn user file cannot be opened.
        empathDebug("Couldn't open file for writing");
        QMessageBox::information(this, "Empath",
            i18n("Sorry I can't write to that file. "
                "Please try another filename."), i18n("OK"));
        return;
    }
    empathDebug("Opened " + saveFilePath + " OK");
    
    EmpathFolder * folder(empath->folder(url_));
    if (folder == 0) return;
    
    RMM::RMessage * message(folder->message(firstSelectedMessage()));
    if (message == 0) return;
    
    QCString s =
        message->asString();
    
    unsigned int blockSize = 1024; // 1k blocks
    
    unsigned int fileLength = s.length();

    for (unsigned int i = 0 ; i < s.length() ; i += blockSize) {
        
        QCString outStr;
        
        if ((fileLength - i) < blockSize)
            outStr = QCString(s.right(fileLength - i));
        else
            outStr = QCString(s.mid(i, blockSize));
        
        if (f.writeBlock(outStr, outStr.length()) != (int)outStr.length()) {
            // Warn user file not written.
            QMessageBox::information(this, "Empath",
                i18n("Sorry I couldn't write the file successfully. "
                    "Please try another file."), i18n("OK"));
            delete message; message = 0;
            return;
        }
        qApp->processEvents();
    }

    f.close();
    
    QMessageBox::information(this, "Empath",
        i18n("Message saved to") + " " + saveFilePath + " " + i18n("OK"),
        i18n("OK"));
    delete message; message = 0;
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
    
    if (nSelected_ == 0) {
        setSelected(item, true);
        nSelected_ = 1;
    }
    
    if (nSelected_ != 1) {
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
    if (empath->mark(u, RMM::MessageStatus(item->status() ^ RMM::Read))) {
        setStatus(item, RMM::MessageStatus(item->status() ^ RMM::Read));
    }
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
    
    // Ask the folder we were showing to drop its index.
    // Thanks to Waldo's dandy zone allocator, a few 128K blocks might
    // just get passed back to the OS here.
    EmpathFolder * oldFolder = empath->folder(url_);
    
    if (oldFolder != 0) {
        
        oldFolder->dropIndex();
        
        QObject::disconnect(
            oldFolder,    SIGNAL(itemLeft(const QString &)),
            this,        SLOT(s_itemGone(const QString &)));
        
        QObject::disconnect(
            oldFolder,    SIGNAL(itemArrived    (const QString &)),
            this,        SLOT(s_itemCome        (const QString &)));
    }
    
    url_ = url;
    
    EmpathFolder * f = empath->folder(url_);
    
    QObject::connect(
        f,        SIGNAL(itemLeft(const QString &)),
        this,    SLOT(s_itemGone(const QString &)));
    
    QObject::connect(
        f,        SIGNAL(itemArrived    (const QString &)),
        this,    SLOT(s_itemCome        (const QString &)));
    
    if (f == 0) {
    
        // Can't find folder !
    
        emit(showing());
           return;
    }
    
    clear();
    masterList_.clear();
    
    f->messageList().sync();
    
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
        messageMenu_.insertItem(empathIcon("mini-view.png"), i18n("View"),
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
    messageMenu_.insertItem(empathIcon("mini-reply.png"), i18n("Reply"),
        this, SLOT(s_messageReply()));

    messageMenuItemReplyAll =
    messageMenu_.insertItem(empathIcon("mini-reply.png"),i18n("Reply to A&ll"),
        this, SLOT(s_messageReplyAll()));

    messageMenuItemForward =
    messageMenu_.insertItem(empathIcon("mini-forward.png"), i18n("Forward"),
        this, SLOT(s_messageForward()));

    messageMenuItemDelete =
    messageMenu_.insertItem(empathIcon("mini-delete.png"), i18n("Delete"),
        this, SLOT(s_messageDelete()));

    messageMenuItemSaveAs =
    messageMenu_.insertItem(empathIcon("mini-save.png"), i18n("Save As"),
        this, SLOT(s_messageSaveAs()));
    
    multipleMessageMenu_.insertItem(i18n("Mark..."),
        this, SLOT(s_messageMarkMany()));
    
    multipleMessageMenu_.insertItem(
        empathIcon("mini-forward.png"), i18n("Forward"),
        this, SLOT(s_messageForward()));

    multipleMessageMenu_.insertItem(
        empathIcon("mini-delete.png"), i18n("Delete"),
        this, SLOT(s_messageDelete()));

    multipleMessageMenu_.insertItem(
        empathIcon("mini-save.png"), i18n("Save As"),
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
    empathDebug("MOUSE PRESS EVENT");
    
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
        
        clearSelection();
        
        setMultiSelection(false);
        
        setSelected(item, true);
        lastSelected_ = item;
        
        s_currentChanged(item);
        
        nSelected_ = 1;
        
        return;
    }
        
    // CASE 3: Left button + control pressed
    if (e->state() == ControlButton) {
        empathDebug("CASE 3");
        setMultiSelection(true);
        
        if (!item->isSelected()) {
            lastSelected_ = item;
            nSelected_++;
        }

        setSelected(item, !(item->isSelected()));

        return;
    }
    
    if ((e->state() & ShiftButton) && (lastSelected_ == 0)) {
        
        // CASE 4 and CASE 6:
        // Shift button pressed, but no prior selection.
        // Clear all selections, and select this only.
        // For CASE 6, if control is pressed, toggle instead.
        
        empathDebug("CASE 4/6");
        
        clearSelection();
        
        setMultiSelection(false);
        
        // For CASE 6:
        if (e->state() & ControlButton) {
        
            if (!item->isSelected())
                lastSelected_ = item;

            setSelected(item, !(item->isSelected()));
            
        } else {
        
            // For CASE 4:
            setSelected(item, true);
            lastSelected_ = item;
        }
        
        nSelected_ = 1;
        
        return;
    }
    
    if (e->state() & ShiftButton) {
        
        // CASE 5, CASE 7:
        // There is an item already selected, as the above test didn't
        // hold.
        
        setMultiSelection(true);
        
        
        if (!(e->state() & ControlButton)) {
            
            // CASE 5:
            // Control button has not been held, so we must clear the
            // selection.
            empathDebug("CASE 5");
            clearSelection();
            nSelected_ = 0;
        }
            
        QListViewItem * i = lastSelected_;
        
        // First see if the item we're looking at is below the last selected.
        // If so, work down. Otherwise, er... work up.
        if (i->itemPos() < item->itemPos()) {
        
            while (i && i != item) {
                
                setSelected(i, true);
                nSelected_++;

                i = i->itemBelow();
            }
            
        } else {
    
            while (i && i != item) {
                
                setSelected(i, true);
                nSelected_++;

                i = i->itemAbove();
            }
        }

        if (e->state() & ControlButton) {
            
            empathDebug("CASE 7");
            lastSelected_ = item;
        }

        setSelected(item, true);
        nSelected_++;
        return;
    }
}

    void
EmpathMessageListWidget::contentsMouseReleaseEvent(QMouseEvent *)
{
    maybeDrag_ = false;
}

    void
EmpathMessageListWidget::contentsMouseMoveEvent(QMouseEvent * e)
{
    empathDebug("Mouse move event in progress");
    
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
    
    u->setPixmap(empathIcon("tree.png")); 
    
    empathDebug("Starting the drag copy");
    u->drag();
}

    void
EmpathMessageListWidget::selectTagged()
{
    clearSelection();
    setMultiSelection(true);
    setUpdatesEnabled(false);
    
    nSelected_ = 0;
    
    wantScreenUpdates_ = false;

    QListViewItemIterator it(this);
    
    for (; it.current(); ++it) {
        
        EmpathMessageListItem * i = (EmpathMessageListItem *)it.current();
        
        if (i->status() & RMM::Marked) {
            nSelected_++;
            i->setSelected(true);
        }
    }
    
    wantScreenUpdates_ = true;
    setUpdatesEnabled(true);
    triggerUpdate();
}

    void
EmpathMessageListWidget::selectRead()
{
    clearSelection();
    setMultiSelection(true);
    setUpdatesEnabled(false);
    
    nSelected_ = 0;
    
    wantScreenUpdates_ = false;

    QListViewItemIterator it(this);
    
    for (; it.current(); ++it) {
        
        EmpathMessageListItem * i = (EmpathMessageListItem *)it.current();
        
        if (i->status() & RMM::Read) {
            nSelected_++;
            i->setSelected(true);
        }
    }
    
    wantScreenUpdates_ = true;
    setUpdatesEnabled(true);
    triggerUpdate();
}

    void
EmpathMessageListWidget::selectAll()
{
    setMultiSelection(true);
    setUpdatesEnabled(false);
    
    nSelected_ = 0;
    
    QListViewItemIterator it(this);
    wantScreenUpdates_ = false;
    
    for (; it.current(); ++it) {
        it.current()->setSelected(true);
        nSelected_++;
    }
    
    wantScreenUpdates_ = true;
    setUpdatesEnabled(true);
    triggerUpdate();
}    

    void
EmpathMessageListWidget::selectInvert()
{
    setMultiSelection(true);
    setUpdatesEnabled(false);
    
    wantScreenUpdates_ = false;

    QListViewItemIterator it(this);
    
    for (; it.current(); ++it) {
        if (!it.current()->isSelected()) {
            it.current()->setSelected(true);
            nSelected_++;
        } else {
            it.current()->setSelected(false);
            nSelected_--;
        }
    }
    
    wantScreenUpdates_ = true;
    setUpdatesEnabled(true);
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
            itemList_.remove(i);
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
        
    EmpathIndexRecord * i(f->messageList()[s]);

    if (i == 0) {
        empathDebug("Can't find index record for \"" + s + "\"");
        return;
    }
    
    if (KGlobal::config()->readBoolEntry(EmpathConfig::KEY_THREAD_MESSAGES)) {
    
        addItem(i);
    
    } else {

        EmpathMessageListItem * newItem =
            new EmpathMessageListItem(this, *i);

        itemList_.append(newItem);
    
        CHECK_PTR(newItem);

        setStatus(newItem, i->status());
    }
}

    void
EmpathMessageListWidget::_fillDisplay(EmpathFolder * f)
{
    EmpathIndexIterator it(f->messageList());
    
    filling_ = true;
    
    setUpdatesEnabled(false);
    empath->s_infoMessage(
        "Message list hidden while filling until Qt bug fixed !!!");
    
    hide();
    clear();
    
    KGlobal::config()->setGroup(EmpathConfig::GROUP_DISPLAY);

    if (KGlobal::config()->readBoolEntry(EmpathConfig::KEY_THREAD_MESSAGES))
        _fillThreading(f);
    else
        _fillNonThreading(f);
    
    show();
    
    filling_ = false;
    
    emit(showing());
}

    void
EmpathMessageListWidget::_fillNonThreading(EmpathFolder * f)
{
    setRootIsDecorated(false);

    EmpathTask * t(empath->addTask(i18n("Sorting messages")));
    CHECK_PTR(t);

    t->setMax(f->messageCount());
    
    EmpathIndexIterator it(f->messageList());

    for (; it.current(); ++it) {
        
        EmpathMessageListItem * newItem =
            new EmpathMessageListItem(this, *it.current());

        CHECK_PTR(newItem);
        itemList_.append(newItem);
        
        setStatus(newItem, it.current()->status());

        t->doneOne();
    }
    
    setSorting(
        KGlobal::config()->
            readNumEntry(EmpathConfig::KEY_MESSAGE_SORT_COLUMN, 2),
        KGlobal::config()->
            readNumEntry(EmpathConfig::KEY_MESSAGE_SORT_ASCENDING, true));
    
    t->done();
    setUpdatesEnabled(true);
//    triggerUpdate();
    empath->s_infoMessage(
        i18n("Finished reading mailbox") + " " + url_.asString());
}

    void
EmpathMessageListWidget::_fillThreading(EmpathFolder * f)
{
    setRootIsDecorated(true);
    
    // Start by putting everything into our list. This takes care of sorting so
    // hopefully threading will be simpler.
    
    EmpathTask * t(empath->addTask(i18n("Sorting messages")));
    CHECK_PTR(t);

    t->setMax(f->messageCount());
    
    EmpathIndexIterator it(f->messageList());
    
    for (; it.current(); ++it)
        masterList_.inSort(it.current());
    
    QListIterator<EmpathIndexRecord> mit(masterList_);
    
    // What we doing here ?
    // Well, we keep the time we started.
    // When the time since 'begin' is too long (100ms) we do a
    // kapp->processEvents(). This prevents the UI from stalling.
    // We could then have done an update, but this takes a long time, so we
    // don't want to do it so often. Therefore we do it every 10 times that
    // we've had to do a processEvents().
    
    
    QTime begin(QTime::currentTime());
//    QTime begin2(begin);
    QTime now;
    
    sortType_ = KGlobal::config()->
        readNumEntry(EmpathConfig::KEY_MESSAGE_SORT_ASCENDING, true);
    
    setSorting(
        KGlobal::config()->
        readNumEntry(EmpathConfig::KEY_MESSAGE_SORT_COLUMN, 3), sortType_);
    
    for (; mit.current(); ++mit) {
        
        addItem(mit.current());
        
        t->doneOne();
        
        now = QTime::currentTime();
        
        if (begin.msecsTo(now) > 100) {
        
            kapp->processEvents();
            begin = now;
        }
    
//        if (begin2.secsTo(now) > 10) {
//            setUpdatesEnabled(true);
//            triggerUpdate();
//            setUpdatesEnabled(false);
//            begin2 = now;
//        }
    }
    
    t->done();
    setUpdatesEnabled(true);
//    triggerUpdate();
    empath->s_infoMessage(
        i18n("Finished reading mailbox") + " " + url_.asString());
}

    void
EmpathMessageListWidget::_updateSelected()
{
    empathDebug("_updateSelected() called");
    selected_.clear();

    EmpathMessageListItemIterator it(itemList_);
    
    for (; it.current(); ++it)
        if (it.current()->isSelected()) {
            selected_.append(it.current());
        }
}

    void
EmpathMessageListWidget::s_messageMarkMany()
{
    EmpathMessageMarkDialog d;
    
    if (d.exec() != QDialog::Accepted)
        return;
    
    EmpathMessageMarkDialog::MarkType t = d.markType();

    RMM::MessageStatus s = d.status();
    
    if (s == RMM::Marked)
        empathDebug("Tag");
    
    if (s == RMM::Replied)
        empathDebug("Replied");
    
    if (s == RMM::Read)
        empathDebug("Read");
    
    if (t == EmpathMessageMarkDialog::On)
        empathDebug("On");
    
    if (t == EmpathMessageMarkDialog::Off)
        empathDebug("Off");
    
    if (t == EmpathMessageMarkDialog::Toggle)
        empathDebug("Toggle");

    _updateSelected();
    
    QStringList l;

    empathDebug("There are " + QString().setNum(selected_.count()) + " selected messages");

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

// vim:ts=4:sw=4:tw=78
