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
#include <kaction.h>

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
        listenTo_           (0),
        filling_            (false)
{
    setFrameStyle(QFrame::NoFrame);

    setShowSortIndicator(true);

    lastHeaderClicked_ = -1;

    setAllColumnsShowFocus(true);
    setRootIsDecorated(true);
    
    setSorting(-1);

    setSelectionMode(QListView::Extended);

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

    _initActions();
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

    QObject::connect(
        empath, SIGNAL(showFolder(const EmpathURL &, unsigned int)),
        this,   SLOT(s_showFolder(const EmpathURL &, unsigned int))); 
    
    markAsReadTimer_ = new EmpathMarkAsReadTimer(this);
    
    hideRead_ = false; // TODO: KGlobal::config()->readEntry(UI_HIDE_READ, false);
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
    if (initialItem == 0) {
        empathDebug("initial item is 0 !");
        return 0;
    }

    EmpathMessageListItem * fChild =
        static_cast<EmpathMessageListItem *>(initialItem->firstChild());
    
    if (fChild != 0) {
        EmpathMessageListItem * found = findRecursive(fChild, msgId);
        if (found != 0) return found;
    }
    
    EmpathMessageListItem * nextSibling =
        static_cast<EmpathMessageListItem *>(initialItem->nextSibling());
    
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

    return findRecursive(
        static_cast<EmpathMessageListItem *>(firstChild()), msgId);
}

    EmpathMessageListItem *
EmpathMessageListWidget::_threadItem(EmpathIndexRecord & rec)
{
    if (rec.isNull()) {
        empathDebug("record is null !");
        return 0;
    }

    EmpathMessageListItem * parentItem = 0;
    
    // Find parent of this item.
    if (!rec.parentID().localPart().isEmpty())

        for (QListViewItemIterator it(this); it.current(); ++it) {
            
            EmpathMessageListItem * i =
                static_cast<EmpathMessageListItem *>(it.current());

            if (i->messageID() == rec.parentID()) {
                parentItem = i;
                break;
            }
        }

    if (parentItem == 0)
        return _addItem(this, rec);
    else
        return _addItem(parentItem, rec);
}

    EmpathURL
EmpathMessageListWidget::firstSelectedMessage()
{
    EmpathURL u;

    if (currentItem() == 0) return u;
    
    EmpathMessageListItem * item =
        static_cast<EmpathMessageListItem *>(currentItem());
    
    return EmpathURL(url_.mailboxName(), url_.folderPath(), item->id());
}

    void
EmpathMessageListWidget::markOne(RMM::MessageStatus status)
{
    // Don't bother auto-marking this as the user's done it.
    markAsReadTimer_->cancel();
    
    if (!currentItem())
        return;
    
    EmpathMessageListItem * item =
        static_cast<EmpathMessageListItem *>(currentItem());

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
EmpathMessageListWidget::s_messageCompose()
{
    empath->s_compose();
}

    void
EmpathMessageListWidget::s_messageReply()
{
    empath->s_reply(firstSelectedMessage());
}

    void
EmpathMessageListWidget::s_messageReplyAll()
{
    empath->s_replyAll(firstSelectedMessage());
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
    
    EmpathTask * t = new EmpathTask(i18n("Deleting messages"));
    
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
        return;
    }

    if (_nSelected() > 1) {
        multipleMessageMenu_.exec(pos);
        return;
    }

    EmpathMessageListItem * i =
        static_cast<EmpathMessageListItem *>(item);

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
    markAsReadTimer_->go(static_cast<EmpathMessageListItem *>(i));
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
    setStatusPixmap(item, status);
}

    void
EmpathMessageListWidget::setStatusPixmap(
        EmpathMessageListItem * item, RMM::MessageStatus status)
{
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
EmpathMessageListWidget::s_hideRead()
{
    filling_ = false;
    hideRead_ = !hideRead_;
    emit(hideReadChanged(hideRead_));
    clear();
    _fillDisplay(hideRead_);
}

    void
EmpathMessageListWidget::s_showFolder(const EmpathURL & url, unsigned int i)
{
    if (i != listenTo_)
        return;

    filling_ = false;
    _reconnectToFolder(url);
    clear();
    _fillDisplay(hideRead_);
}
    
    void
EmpathMessageListWidget::_reconnectToFolder(const EmpathURL & url)
{
    if (url_ == url)
        return;
    
    EmpathFolder * oldFolder = empath->folder(url_);
    
    if (oldFolder != 0) {

        oldFolder->disconnect(this, SLOT(s_itemGone(const QString &)));
        oldFolder->disconnect(this, SLOT(s_itemCome(const QString &)));
    }
    
    url_ = url;
    
    EmpathFolder * f = empath->folder(url_);
    
    QObject::connect(
        f,       SIGNAL(itemLeft(const QString &)),
        this,    SLOT(s_itemGone(const QString &)));
    
    QObject::connect(
        f,       SIGNAL(itemArrived (const QString &)),
        this,    SLOT(s_itemCome    (const QString &)));
    
    if (f == 0) {
        empathDebug("Can't find folder !");
        return;
    }

    f->syncIndex();
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
EmpathMessageListWidget::_initActions()
{
    messageCompose = new KAction(i18n("&Compose"), empathIconSet("compose"), 0, 
                    this, SLOT(s_messageCompose()), this, "messageCompose");
    messageReply = new KAction(i18n("&Reply"), empathIconSet("reply"), 0,
                    this, SLOT(s_messageReply()), this, "messageReply");
    messageReplyAll = new KAction(i18n("Reply to &All"), empathIconSet("reply"), 0,
                    this, SLOT(s_messageReplyAll()), this, "messageReplyAll");
    messageForward = new KAction(i18n("&Forward"), empathIconSet("forward"), 0,
                    this, SLOT(s_messageForward()), this, "messageForward");
    messageDelete = new KAction(i18n("&Delete"), empathIconSet("delete"), 0,
                    this, SLOT(s_messageDelete()), this, "messageDelete");
    messageSaveAs = new KAction(i18n("Save &As"), empathIconSet("save"), 0,
                    this, SLOT(s_messageSaveAs()), this, "messageSaveAs");
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

    messageReply->plug(&messageMenu_);
    messageReplyAll->plug(&messageMenu_);
    messageForward->plug(&messageMenu_);
    messageDelete->plug(&messageMenu_);
    messageSaveAs->plug(&messageMenu_);

    multipleMessageMenu_.insertItem(i18n("Mark..."),
        this, SLOT(s_messageMarkMany()));
    
    messageForward->plug(&multipleMessageMenu_);
    messageDelete->plug(&multipleMessageMenu_);
    messageSaveAs->plug(&multipleMessageMenu_);

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
        
        EmpathMessageListItem * i =
            static_cast<EmpathMessageListItem *>(it.current());

        EmpathURL messageURL(url_.mailboxName(), url_.folderPath(), i->id());
        uriList.append(messageURL.asString().utf8());
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
    
    QListViewItemIterator it(this);
    
    for (; it.current(); ++it) {
 
        EmpathMessageListItem * i =
            static_cast<EmpathMessageListItem *>(it.current());

        if (i->status() & RMM::Marked)
            _setSelected(i, true);
    }
    
    viewport()->setUpdatesEnabled(true);
    triggerUpdate();
}

    void
EmpathMessageListWidget::selectRead()
{
    clearSelection();

    viewport()->setUpdatesEnabled(false);
    
    QListViewItemIterator it(this);
    
    for (; it.current(); ++it) {
 
        EmpathMessageListItem * i =
            static_cast<EmpathMessageListItem *>(it.current());

        if (i->status() & RMM::Read)
            _setSelected(i, true);
    }
    
    viewport()->setUpdatesEnabled(true);
    triggerUpdate();
}

    void
EmpathMessageListWidget::selectAll()
{
    viewport()->setUpdatesEnabled(false);
    
    QListViewItemIterator it(this);
    
    for (; it.current(); ++it)
        setSelected(it.current(), true);
    
    viewport()->setUpdatesEnabled(true);
    triggerUpdate();
}    

    void
EmpathMessageListWidget::selectInvert()
{
    viewport()->setUpdatesEnabled(false);
    
    QListViewItemIterator it(this);
    
    for (; it.current(); ++it) {
        if (!it.current()->isSelected()) {
            setSelected(it.current(), true);
        } else {
            setSelected(it.current(), false);
        }
    }
    
    viewport()->setUpdatesEnabled(true);
    triggerUpdate();
}

    void
EmpathMessageListWidget::s_itemGone(const QString & s)
{
    empathDebug("");
    if (filling_) return;
    
    QListViewItemIterator it(this);
    
    for (; it.current(); ++it) {
 
        EmpathMessageListItem * i =
            static_cast<EmpathMessageListItem *>(it.current());

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
        
    EmpathIndexRecord rec(f->indexRecord(s));

    if (rec.isNull()) {
        empathDebug("Can't find index record for \"" + s + "\"");
        return;
    }
    
    if (KGlobal::config()->readBoolEntry(EmpathConfig::UI_THREAD))
        _threadItem(rec);
    else
        _addItem(this, rec);
}

    void
EmpathMessageListWidget::_fillDisplay(bool unreadOnly) 
{
    filling_ = true;
    
    selected_.clear();
    clear();
    triggerUpdate();
    kapp->processEvents();
    setUpdatesEnabled(false);
    viewport()->setUpdatesEnabled(false);

    EmpathFolder * f = empath->folder(url_);

    if (!f)
        return;
    
    EmpathTask t(i18n("Sorting messages"));
    t.setMax(f->messageCount());
    
    setSorting(-1);
    
    KConfig * c(KGlobal::config());
    
    using namespace EmpathConfig;

    c->setGroup(GROUP_DISPLAY);
    QStringList index(f->allIndexKeys());
    QStringList::ConstIterator it(index.begin());
    
    if (KGlobal::config()->readBoolEntry(UI_THREAD))
        
        // fill threading
        for (; it != index.end() && filling_; ++it) {

            EmpathIndexRecord rec = f->indexRecord(*it);
            
            if (rec.isNull()) {
                empathDebug("Can't find record, but I'd called allKeys !");
                continue;
            }

            if (!(unreadOnly && (rec.status() & RMM::Read)))
                _threadItem(rec);

            t.doneOne();
        }
    
    else
        
        // fill nonthreading;
        // Rikkus: Hey Wilco, you don't need ';' after comments ;)
        for (; it != index.end() && filling_; ++it) {
            
            EmpathIndexRecord rec = f->indexRecord(*it);

            if (rec.isNull()) {
                empathDebug("Can't find record, but I'd called allKeys !");
                continue;
            }
            
            if (!(unreadOnly && (rec.status() & RMM::Read)))
                _addItem(this, rec);

            t.doneOne();
        }
   
    if (filling_) {
   
        setSorting(
            c->readNumEntry(UI_SORT_COLUMN, DFLT_SORT_COL),
            c->readNumEntry(UI_SORT_ASCENDING, DFLT_SORT_ASCENDING));

        viewport()->setUpdatesEnabled(true);
        setUpdatesEnabled(true);
        
        triggerUpdate();
        
        filling_ = false;
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
        _setSelected(static_cast<EmpathMessageListItem *>(item), b);
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
    EmpathMessageListItem * prt, EmpathIndexRecord & rec)
{
    EmpathMessageListItem * newItem = new EmpathMessageListItem(prt, rec);
    setStatusPixmap(newItem, rec.status());
    return newItem;
}

    EmpathMessageListItem *
EmpathMessageListWidget::_addItem(
    EmpathMessageListWidget * prt, EmpathIndexRecord & rec)
{
    EmpathMessageListItem * newItem = new EmpathMessageListItem(prt, rec);
    setStatusPixmap(newItem, rec.status());
    return newItem;
}

    void
EmpathMessageListWidget::_removeItem(EmpathMessageListItem * i)
{
    selected_.remove(i);
    delete i;
}

    void
EmpathMessageListWidget::listenTo(unsigned int id)
{
    listenTo_ = id;
}

// vim:ts=4:sw=4:tw=78
