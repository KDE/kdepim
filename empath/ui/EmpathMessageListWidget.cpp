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
#include "EmpathMessageListWidget.h"
#include "EmpathIndexRecord.h"
#include "EmpathIndex.h"
#include "EmpathMessageViewWindow.h"
#include "EmpathConfig.h"
#include "Empath.h"
#include "EmpathUIUtils.h"
#include "EmpathTask.h"
    
QListViewItem * EmpathMessageListWidget::lastSelected_ = 0;

EmpathMessageListWidget::EmpathMessageListWidget(QWidget * parent)
    :   EmpathListView      (parent, "MessageListWidget"),
        wantScreenUpdates_  (false),
        filling_            (false)
{
    setFrameStyle(QFrame::NoFrame);

    wantScreenUpdates_ = false;
    
    lastHeaderClicked_ = -1;

    setAllColumnsShowFocus(true);
    setRootIsDecorated(true);
    
    setSorting(-1);

    setMultiSelection(true);

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
    
    using namespace EmpathConfig;
    
    c->setGroup(GROUP_GENERAL);
    
    for (int i = 0 ; i < 4 ; i++) {
        header()->setCellSize
    (i, c->readUnsignedNumEntry(UI_MSG_LIST_SIZE + QString().setNum(i), 80));
        setColumnWidthMode(i, QListView::Manual);
    }

    _setupMessageMenu();
   
    QObject::connect(
            this, SIGNAL(linkChanged(QListViewItem *)),
            this, SLOT(s_linkChanged(QListViewItem *)));

     QObject::connect(
            this, SIGNAL(startDrag(const QList<QListViewItem> &)),
            this, SLOT(s_startDrag(const QList<QListViewItem> &)));
    
    // QObject::connect(this, SIGNAL(currentChanged(QListViewItem *)),
    //         this, SLOT(s_currentChanged(QListViewItem *)));

    // Connect return press to view.
    QObject::connect(this, SIGNAL(returnPressed(QListViewItem *)),
            this, SLOT(s_messageView()));
    
    // Connect right button up so we can produce the popup context menu.
    QObject::connect(
        this, SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int, Area)),
        this, SLOT(s_rightButtonPressed(QListViewItem *, const QPoint &, int, Area)));
    
    // Connect the header's section clicked signal so we can sort properly
    QObject::connect(header(), SIGNAL(sectionClicked(int)),
        this, SLOT(s_headerClicked(int)));
    
    markAsReadTimer_ = new EmpathMarkAsReadTimer(this);
    
}

EmpathMessageListWidget::~EmpathMessageListWidget()
{
    KConfig * c = KGlobal::config();
    
    using namespace EmpathConfig;
    
    c->setGroup(GROUP_GENERAL);
    
    for (int i = 0 ; i < 4 ; i++) {

        c->writeEntry
            (UI_MSG_LIST_SIZE + QString().setNum(i), header()->cellSize(i));
        
        c->writeEntry
            (UI_MSG_LIST_POS + QString().setNum(i), header()->cellPos(i));
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
    if (!firstChild())
        return 0;

    return findRecursive((EmpathMessageListItem *)firstChild(), msgId);
}

    void
EmpathMessageListWidget::addItem(EmpathIndexRecord * item)
{
    if (item == 0) {
        empathDebug("item == 0 !");
        return;
    }

    EmpathMessageListItem * parentItem = 0;
    
    if (!item->parentID().localPart().isEmpty()) {
        // Find parent of this item.
        QListViewItemIterator it(this); 
        // should try to use EmpathMessageListItemIterator to avoid ugly casts.
        while (it.current() && 
            ((EmpathMessageListItem *)it.current())->messageID() != item->parentID()) ++it;
        parentItem = (EmpathMessageListItem *)it.current();
    }

    EmpathMessageListItem * newItem;
    
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
    // Don't bother auto-marking this as the user's done it.
    markAsReadTimer_->cancel();
    
    EmpathURL u(url_.mailboxName(), url_.folderPath(), QString::null);
    
    QStringList l; // Candidates for marking

    EmpathMessageListItemIterator it(selected_);
    
    for (; it.current(); ++it)
        l.append(it.current()->id());
        
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
    empath->s_reply(firstSelectedMessage());
}

    void
EmpathMessageListWidget::s_messageReplyAll()
{
    empath->s_reply(firstSelectedMessage());
}

    void
EmpathMessageListWidget::s_messageForward()
{
    empath->s_forward(firstSelectedMessage());
}

    void
EmpathMessageListWidget::s_messageBounce()
{
    empath->s_bounce(firstSelectedMessage());
}

    void
EmpathMessageListWidget::s_messageDelete()
{
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
    empath->saveMessage(firstSelectedMessage(), this);
}

    void
EmpathMessageListWidget::s_messageCopyTo()
{
    // STUB
}

    void
EmpathMessageListWidget::s_messagePrint()
{
    // STUB
}

    void
EmpathMessageListWidget::s_messageFilter()
{
    // STUB
}

    void
EmpathMessageListWidget::s_messageView()
{
    empathDebug("");
    new EmpathMessageViewWindow(firstSelectedMessage());
}

    void
EmpathMessageListWidget::s_rightButtonPressed(QListViewItem * item, 
        const QPoint & pos, int, EmpathListView::Area area)
{
    if (area == Void) return;

    if (area == OpenClose) {
        threadMenu_.exec(pos);
        wantScreenUpdates_ = true;
        return;
    }

    if (_nSelected() > 1) {
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
}

    void
EmpathMessageListWidget::s_doubleClicked(QListViewItem *)
{
    s_messageView();
}

    void
EmpathMessageListWidget::s_linkChanged(QListViewItem *i)
{
    markAsReadTimer_->cancel();

    // Make sure we highlight the current item.
    kapp->processEvents();

    emit(changeView(firstSelectedMessage()));
    markAsReadTimer_->go((EmpathMessageListItem *)i);
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
    
    EmpathFolder * f = empath->folder(url_);
    f->setStatus(item->id(), status);

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
}

    void
EmpathMessageListWidget::s_showFolder(const EmpathURL & url)
{
    empathDebug(url.asString());
    if (url_ == url) {
        emit(showing());
        return;
    }
    
    empath->s_infoMessage(i18n("Reading mailbox") + " " + url.asString());
    
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
    
    f->index()->sync();
    
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

    using namespace EmpathConfig;

    c->setGroup(GROUP_DISPLAY);
    
    c->writeEntry(UI_SORT_COLUMN, i);
    c->writeEntry(UI_SORT_ASCENDING, sortType_);
    
    lastHeaderClicked_ = i;
}

    void
EmpathMessageListWidget::_setupMessageMenu()
{
    messageMenuItemView =
        messageMenu_.insertItem(empathIcon("menu-view"), i18n("View"),
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
    messageMenu_.insertItem(empathIcon("menu-reply"), i18n("Reply"),
        this, SLOT(s_messageReply()));

    messageMenuItemReplyAll =
    messageMenu_.insertItem(empathIcon("menu-reply"),i18n("Reply to A&ll"),
        this, SLOT(s_messageReplyAll()));

    messageMenuItemForward =
    messageMenu_.insertItem(empathIcon("menu-forward"), i18n("Forward"),
        this, SLOT(s_messageForward()));

    messageMenuItemDelete =
    messageMenu_.insertItem(empathIcon("menu-delete"), i18n("Delete"),
        this, SLOT(s_messageDelete()));

    messageMenuItemSaveAs =
    messageMenu_.insertItem(empathIcon("menu-save"), i18n("Save As"),
        this, SLOT(s_messageSaveAs()));
    
    multipleMessageMenu_.insertItem(i18n("Mark..."),
        this, SLOT(s_messageMarkMany()));
    
    multipleMessageMenu_.insertItem(
        empathIcon("menu-forward"), i18n("Forward"),
        this, SLOT(s_messageForward()));

    multipleMessageMenu_.insertItem(
        empathIcon("menu-delete"), i18n("Delete"),
        this, SLOT(s_messageDelete()));

    multipleMessageMenu_.insertItem(
        empathIcon("menu-save"), i18n("Save As"),
        this, SLOT(s_messageSaveAs()));

    threadMenu_.insertItem(i18n("Expand"),
        this, SLOT(s_expandThread()));
        
    threadMenu_.insertItem(i18n("Collapse"),
        this, SLOT(s_collapseThread()));
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
    // Empty.
}

    void
EmpathMarkAsReadTimer::go(EmpathMessageListItem * i)
{
    item_ = i;
    // Don't bother if it's already read.
    if (i->status() & RMM::Read) return;

    KConfig * c(KGlobal::config());
    
    using namespace EmpathConfig;
    
    c->setGroup(GROUP_DISPLAY);
    
    if (!c->readBoolEntry(UI_MARK_READ)) return;

    int waitTime(c->readNumEntry(UI_MARK_TIME, 2));

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
EmpathMessageListWidget::s_startDrag(const QList<QListViewItem> & items)
{
    empathDebug("Starting a drag");
    QStrList uriList;
    
    QListIterator<QListViewItem> it(items);
    while (it.current()) {
        EmpathMessageListItem * i = (EmpathMessageListItem *)it.current();
        EmpathURL messageURL(url_.mailboxName(), url_.folderPath(), i->id());
        uriList.append(messageURL.asString());
        ++it;
    }

    // char * c = new char[i->id().length() + 1];
    // strcpy(c, i->id().latin1());
    
    QUriDrag * u  = new QUriDrag(uriList, this);

    u->setPixmap(empathIcon("tree"));

    u->drag();
}

    void
EmpathMessageListWidget::selectTagged()
{
    clearSelection();

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
    clearSelection();

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
        setSelected(it.current(), true);
    
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
            setSelected(it.current(), true);
        } else {
            setSelected(it.current(), false);
        }
    }
    
    wantScreenUpdates_ = true;
    viewport()->setUpdatesEnabled(true);
    triggerUpdate();
}

    void
EmpathMessageListWidget::s_itemGone(const QString & s)
{
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
    if (filling_) return;

    EmpathFolder * f(empath->folder(url_));
    
    if (f == 0)
        return;
        
    EmpathIndexRecord * i(f->index()->record(s.latin1()));

    if (i == 0) {
        empathDebug("Can't find index record for \"" + s + "\"");
        return;
    }
    
    if (KGlobal::config()->readBoolEntry(EmpathConfig::UI_THREAD)) {
    
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

    if (KGlobal::config()->readBoolEntry(EmpathConfig::UI_THREAD))
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
    
    setSorting(-1);

    t->setMax(f->messageCount());
    
    QStrList l(f->index()->allKeys());
    
    QStrListIterator it(l);

    for (; it.current(); ++it) {
        
        EmpathIndexRecord * rec = f->index()->record(it.current());

        if (rec == 0) {
            empathDebug("Can't find index record.");
            continue;
        }
        
        EmpathMessageListItem * newItem = _addItem(this, *rec);
        
        setStatus(newItem, rec->status());
        
        t->doneOne();
        kapp->processEvents();
    }

    KConfig * c(KGlobal::config());

    using namespace EmpathConfig;
    
    c->setGroup(GROUP_DISPLAY);
    
    setSorting(
        c->readNumEntry(UI_SORT_COLUMN, DFLT_SORT_COL),
        c->readNumEntry(UI_SORT_ASCENDING, DFLT_SORT_ASCENDING));
    
    t->done();

    empath->s_infoMessage(
        i18n("Finished reading mailbox") + " " + url_.asString());
}

    void
EmpathMessageListWidget::_fillThreading(EmpathFolder * f)
{
    setRootIsDecorated(true);
    
    EmpathTask * t(empath->addTask(i18n("Sorting messages")));

    t->setMax(f->messageCount());
    
    QStrList l(f->index()->allKeys());
    
    QStrListIterator it(l);
    
    for (; it.current(); ++it) {
        
        EmpathIndexRecord * rec = f->index()->record(it.current());
        
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
    
    KConfig * c(KGlobal::config());

    using namespace EmpathConfig;
    
    c->setGroup(GROUP_DISPLAY);
    
    setSorting(
        c->readNumEntry(UI_SORT_COLUMN, DFLT_SORT_COL),
        c->readNumEntry(UI_SORT_ASCENDING, DFLT_SORT_ASCENDING));
 
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
EmpathMessageListWidget::s_expandThread()
{
    expand(currentItem());   
}

    void
EmpathMessageListWidget::s_collapseThread()
{
    collapse(currentItem());
}

    void
EmpathMessageListWidget::clearSelection()
{
    QListView::clearSelection();
    selected_.clear();
}

    void
EmpathMessageListWidget::setSelected(QListViewItem * item, bool b)
{
    if (item)
        _setSelected((EmpathMessageListItem *)item, b);
}

    void
EmpathMessageListWidget::_setSelected(EmpathMessageListItem * item, bool b)
{
    if (b) {
        
        if (!item->isSelected())
            selected_.append(item);
        
        QListView::setSelected(item, true);
        
    } else {
        
        selected_.remove(item);
        QListView::setSelected(item, false);
    }

    // triggerUpdate();
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
    selected_.remove(i);
    delete i;
}

// vim:ts=4:sw=4:tw=78
