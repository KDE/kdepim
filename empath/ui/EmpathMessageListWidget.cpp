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
#include <stdio.h>

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
    
QList<EmpathMessageListItem> * EmpathMessageListWidget::listItemPool_ = 0L;
QListViewItem * EmpathMessageListWidget::lastSelected_ = 0L;

EmpathMessageListWidget::EmpathMessageListWidget(QWidget * parent)
    :   EmpathListView      (parent, "MessageListWidget"),
        listenTo_           (0),
        filling_            (false),
        lastHeaderClicked_  (-1),
        hideRead_           (false)
{
    _init();

    setFrameStyle(QFrame::NoFrame);
    setShowSortIndicator(true);
    setAllColumnsShowFocus(true);
    setSorting(-1);
    setSelectionMode(QListView::Extended);

    addColumn(i18n("Subject"));
    addColumn(i18n("S"));
    addColumn(i18n("T"));
    addColumn(i18n("R"));
    addColumn(i18n("A"));
    addColumn(i18n("Sender"));
    addColumn(i18n("Date"));
    addColumn(i18n("Size"));
    
    _initActions();
    _setupMessageMenu();
    _setupThreadMenu();
    _restoreColumnSizes();
    _connectUp();
}

EmpathMessageListWidget::~EmpathMessageListWidget()
{
    _saveColumnSizes();
    delete markAsReadTimer_;
    markAsReadTimer_ = 0;
}

    void
EmpathMessageListWidget::_init()
{
    markAsReadTimer_ = new EmpathMarkAsReadTimer(this);
    listItemPool_ = new QList<EmpathMessageListItem>;

    px_unread_      = empathIcon("tree-unread");
    px_read_        = empathIcon("tree-read");
    px_marked_      = empathIcon("tree-marked");
    px_attachments_ = empathIcon("tree-attachments");
    px_replied_     = empathIcon("tree-replied");
}

    void
EmpathMessageListWidget::_connectUp()
{
    QObject::connect(
            this, SIGNAL(linkChanged(QListViewItem *)),
            this, SLOT(s_linkChanged(QListViewItem *)));

    QObject::connect(
            this, SIGNAL(startDrag(const QList<QListViewItem> &)),
            this, SLOT(s_startDrag(const QList<QListViewItem> &)));
    
    QObject::connect(this, SIGNAL(currentChanged(QListViewItem *)),
            this, SLOT(s_updateActions(QListViewItem *)));

    // Connect return press to view.
    QObject::connect(this, SIGNAL(returnPressed(QListViewItem *)),
            this, SLOT(s_linkChanged(QListViewItem *)));
    
    // Connect right button up so we can produce the popup context menu.
    QObject::connect(
        this,
        SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int, Area)),
        this,
        SLOT(s_rightButtonPressed(QListViewItem *, const QPoint &, int, Area)));
    
    //XXX Is this now necessary ?
    //Yes ! We want to know which column we're sorting by.
    QObject::connect(header(), SIGNAL(sectionClicked(int)),
        this, SLOT(s_headerClicked(int)));

    QObject::connect(
        empath, SIGNAL(showFolder(const EmpathURL &, unsigned int)),
        this,   SLOT(s_showFolder(const EmpathURL &, unsigned int))); 
}    

    void
EmpathMessageListWidget::_saveColumnSizes()
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
}
 
    void
EmpathMessageListWidget::_restoreColumnSizes()
{
    KConfig * c = KGlobal::config();

    using namespace EmpathConfig;

    c->setGroup(GROUP_GENERAL);
    
    for (int i = 0 ; i < 4 ; i++) {

        unsigned int sz =
            c->readUnsignedNumEntry(UI_MSG_LIST_SIZE + QString::number(i), 80);

        header()->setCellSize(i, sz);

        setColumnWidthMode(i, QListView::Manual);
    }
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

    if (initialItem->messageID() == QString::fromUtf8(msgId.asString()))
        return initialItem;

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

    EmpathMessageListItem * parentItem = 0L;
    
    // Find parent of this item.
    if (rec.hasParent())

        for (QListViewItemIterator it(this); it.current(); ++it) {
            
            EmpathMessageListItem * i =
                static_cast<EmpathMessageListItem *>(it.current());
        
            if (i->messageID() == rec.parentID()) {
                parentItem = i;
                break;
            }
        }

    return _createListItem(rec, 0L, parentItem);
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
    
    RMM::MessageStatus stat = RMM::MessageStatus(item->status());
    
    RMM::MessageStatus s =
        RMM::MessageStatus(stat & status ? stat ^ status : stat | status);
    
    setStatus(item, s);
    kapp->processEvents();

    empath->mark(u, s);
}

    void
EmpathMessageListWidget::mark(RMM::MessageStatus status)
{
    // Don't bother auto-marking this as the user's done it.
    markAsReadTimer_->cancel();
    
    EmpathURL u(url_.mailboxName(), url_.folderPath(), QString::null);
    
    QStringList l; // Candidates for marking

    EmpathMessageListItemIterator it(selected_);
    
    for (; it.current(); ++it) {
        l.append(it.current()->id());
        setStatus(it.current(), status);
    }

    empath->mark(url_, l, status);
}

    void
EmpathMessageListWidget::s_goPrevious()
{    
    QListViewItem * i = currentItem();
    
    if (0 == i) 
        i = firstChild();
    else if (i->itemAbove())
        i = i->itemAbove();
    
    setDelayedLink(true);
    clearSelection();

    if (0 != i) { // There might be no items
        setCurrentItem(i);
        setSelected(i, true);
        ensureItemVisible(i);
    }
}

    void
EmpathMessageListWidget::s_goNext()
{
    QListViewItem * i = currentItem();
    
    if (0 == i) 
        i = firstChild();
    else if (i->itemBelow())
        i = i->itemBelow();
        
    setDelayedLink(true);
    clearSelection();

    if (0 != i) { // There might be no items
        setCurrentItem(i);
        setSelected(i, true);
        ensureItemVisible(i);
    }
}

    void
EmpathMessageListWidget::s_goNextUnread()
{
    if (0 == currentItem())
        return;

    QListViewItemIterator it;

    // Search the items below the current one.

    for (it=currentItem()->nextSibling(); it.current(); ++it) {
        
        EmpathMessageListItem * i =
            static_cast<EmpathMessageListItem *>(it.current());

        if (!(i->status() & RMM::Read)) {
            setDelayedLink(true);
            clearSelection();
            setCurrentItem(i);
            setSelected(i, true);
            ensureItemVisible(currentItem());
            break;
        }
    }
   
    // If there isn't one below the current item, start again on top.
    
    for (it=firstChild(); it.current()!=currentItem(); ++it) {
        
        if (0 == it.current()) // Maybe we're empty.
            break;
        
        EmpathMessageListItem * i =
            static_cast<EmpathMessageListItem *>(it.current());

        if (!(i->status() & RMM::Read)) {
            setDelayedLink(true);
            clearSelection();
            setCurrentItem(i);
            setSelected(i, true);
            ensureItemVisible(currentItem());
            break;
        }
    }
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
    empathDebug("STUB");
}

    void
EmpathMessageListWidget::s_messageMoveTo()
{
    empathDebug("STUB");
}

    void
EmpathMessageListWidget::s_messagePrint()
{
    empathDebug("STUB");
}

    void
EmpathMessageListWidget::s_messageFilter()
{
    empathDebug("STUB");
}

    void
EmpathMessageListWidget::s_messageView()
{
    (void)new EmpathMessageViewWindow(firstSelectedMessage());
}

    void
EmpathMessageListWidget::s_rightButtonPressed(QListViewItem *, 
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

    messageMenu_.exec(pos);
}

    void
EmpathMessageListWidget::s_updateActions(QListViewItem * item)
{
    QStringList affectedActions;

    affectedActions
        << "messageView"    << "messageReply"   << "messageReplyAll"
        << "messageForward" << "messageDelete"  << "messageSaveAs"
        << "messageCopyTo"  << "messageMoveTo"  << "messagePrint"
        << "messageFilter"  << "messageTag"     << "messageMarkRead"
        << "messageMarkReplied";

    QStringList::ConstIterator it(affectedActions.begin());

    for (; it != affectedActions.end(); ++it)
        actionCollection()->action((*it).utf8())->setEnabled(0 != item);

    if (0 != item) {

        EmpathMessageListItem * i = static_cast<EmpathMessageListItem *>(item);

        actionCollection()->action("messageMarkRead")->setText(
            i->status() & RMM::Read ?
            i18n("Mark as unread") : i18n("Mark as read"));

        actionCollection()->action("messageMarkReplied")->setText(
            i->status() & RMM::Replied ?
            i18n("Mark as not replied to") : i18n("Mark as replied to"));

        actionCollection()->action("messageTag")->setText(
            i->status() & RMM::Marked ?
            i18n("Untag") : i18n("Tag"));
    }
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
    clearSelection();
    setSelected(currentItem(), true);
    kapp->processEvents();
    
    empathDebug("emitting changeView()");
    emit(changeView(firstSelectedMessage()));
    markAsReadTimer_->go(static_cast<EmpathMessageListItem *>(i));
}

    void
EmpathMessageListWidget::markAsRead(EmpathMessageListItem * item)
{
    EmpathURL u(url_.mailboxName(), url_.folderPath(), item->id());
    setStatus(item, RMM::MessageStatus(item->status() ^ RMM::Read));
    // XXX RETVAL ?
    empath->mark(u, RMM::MessageStatus(item->status() ^ RMM::Read));
}

    void
EmpathMessageListWidget::s_hideRead()
{
    filling_ = false;
    hideRead_ = !hideRead_;
    emit(hideReadChanged(hideRead_));
    _fillDisplay();
}

    void
EmpathMessageListWidget::s_showFolder(const EmpathURL & url, unsigned int i)
{
    if (i != listenTo_)
        return;

    filling_ = false;
    _reconnectToFolder(url);
    _fillDisplay();
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
    
    f->index()->sync();
    
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
    
    c->setGroup("Folder_" + url_.mailboxName() + "/" + url_.folderPath());
   
    c->writeEntry(UI_SORT_COLUMN, i);
    c->writeEntry(UI_SORT_ASCENDING, sortType_);
   
    lastHeaderClicked_ = i;
}

    void
EmpathMessageListWidget::_initActions()
{
    actionCollection_ = new QActionCollection(this, "actionCollection");

    ac_goPrevious_ =
        new KAction(i18n("&Previous"), QIconSet(BarIcon("prev")), CTRL+Key_P,
            this, SLOT(s_goPrevious()), actionCollection(),
            "goPrevious");

    ac_goNext_ =
        new KAction(i18n("&Next"), QIconSet(BarIcon("next")), CTRL+Key_N,
            this, SLOT(s_goNext()), actionCollection(),
            "goNext");

    ac_goNextUnread_ =
        new KAction(i18n("Next &Unread"), QIconSet(BarIcon("down")), 0, 
            this, SLOT(s_goNextUnread()), actionCollection(),
            "goNextUnread");
    
    // On/off pixmaps identical until I make the 'off' versions.
    // Small/Large identical until I make the large versions.
    // (rikkus)
 
    QIconSet markedOrNot, readOrNot, repliedOrNot;

    markedOrNot.setPixmap(px_marked_, QIconSet::Small, QIconSet::Normal);
    markedOrNot.setPixmap(px_marked_, QIconSet::Large, QIconSet::Normal);
    readOrNot.setPixmap(px_read_, QIconSet::Small, QIconSet::Normal);
    readOrNot.setPixmap(px_read_, QIconSet::Large, QIconSet::Normal);
    repliedOrNot.setPixmap(px_replied_, QIconSet::Small, QIconSet::Normal);
    repliedOrNot.setPixmap(px_replied_, QIconSet::Large, QIconSet::Normal);


    ac_messageTag_ =
        new KToggleAction(i18n("Tag"), markedOrNot, 0, 
            this, SLOT(s_messageMark()), actionCollection(),
            "messageTag");

       ac_messageMarkRead_ =
        new KToggleAction(i18n("&Mark as read"), readOrNot, 0, 
            this, SLOT(s_messageMarkRead()), actionCollection(),
            "messageMarkRead");

    ac_messageMarkReplied_ =
        new KToggleAction(i18n("Mark as replied"), repliedOrNot, 0, 
            this, SLOT(s_messageMarkReplied()), actionCollection(),
            "messageMarkReplied");
                    
    ac_messageMarkMany_ =
        new KAction(i18n("Mark..."), 0, 
            this, SLOT(s_messageMarkMany()), actionCollection(),
            "messageMarkMany");
                   
    ac_messageView_ =
        new KAction(i18n("&View"), empathIconSet("view"), 0, 
            this, SLOT(s_messageView()), actionCollection(),
            "messageView");

    ac_messageReply_ =
        new KAction(i18n("&Reply"), empathIconSet("reply"), Key_R,
            this, SLOT(s_messageReply()), actionCollection(),
            "messageReply");

    ac_messageReply_->setEnabled(false);

    ac_messageReplyAll_ =
        new KAction(i18n("Reply to &All"), empathIconSet("reply"), Key_G,
            this, SLOT(s_messageReplyAll()), actionCollection(),
            "messageReplyAll");

    ac_messageForward_ =
        new KAction(i18n("&Forward"), empathIconSet("forward"), Key_F,
            this, SLOT(s_messageForward()), actionCollection(),
            "messageForward");

    ac_messageDelete_ =
        new KAction(i18n("&Delete"), empathIconSet("delete"), Key_D,
            this, SLOT(s_messageDelete()), actionCollection(),
            "messageDelete");

    ac_messageSaveAs_ =
        new KAction(i18n("Save &As..."), empathIconSet("save"), 0,
            this, SLOT(s_messageSaveAs()), actionCollection(),
            "messageSaveAs");

    ac_messageCopyTo_ =
        new KAction(i18n("&Copy To..."), empathIconSet("copy"), Key_C,
            this, SLOT(s_messageCopyTo()), actionCollection(),
            "messageCopyTo");

    ac_messageMoveTo_ =
        new KAction(i18n("&Move To..."), empathIconSet("save"), 0,
            this, SLOT(s_messageMoveTo()), actionCollection(),
            "messageMoveTo");

    ac_messagePrint_ =
        new KAction(i18n("&Print"), empathIconSet("print"), 0,
            this, SLOT(s_messagePrint()), actionCollection(),
            "messagePrint");

    ac_messageFilter_ =
        new KAction(i18n("&Filter"), empathIconSet("filter"), 0,
            this, SLOT(s_messageFilter()), actionCollection(),
            "messageFilter");

    ac_threadExpand_ =
        new KAction(i18n("&Expand"), 0,
            this, SLOT(s_threadExpand()), actionCollection(),
            "threadExpand");

    ac_threadCollapse_ =
        new KAction(i18n("&Collapse"), 0,
            this, SLOT(s_threadCollapse()), actionCollection(),
            "threadCollapse");
}

    void
EmpathMessageListWidget::_setupMessageMenu()
{
    actionCollection()->action("messageView")->plug(&messageMenu_);
    
    messageMenu_.insertSeparator();
    
    actionCollection()->action("messageTag")->plug(&messageMenu_);
    actionCollection()->action("messageMarkRead")->plug(&messageMenu_);
    actionCollection()->action("messageMarkReplied")->plug(&messageMenu_);

    messageMenu_.insertSeparator();

    actionCollection()->action("messageReply")->plug(&messageMenu_);
    actionCollection()->action("messageReplyAll")->plug(&messageMenu_);
    actionCollection()->action("messageForward")->plug(&messageMenu_);
    actionCollection()->action("messageDelete")->plug(&messageMenu_);
    actionCollection()->action("messageSaveAs")->plug(&messageMenu_);

    actionCollection()->action("messageMarkMany")->plug(&multipleMessageMenu_);
    actionCollection()->action("messageForward")->plug(&multipleMessageMenu_);
    actionCollection()->action("messageDelete")->plug(&multipleMessageMenu_);
    actionCollection()->action("messageSaveAs")->plug(&multipleMessageMenu_);
}

    void
EmpathMessageListWidget::_setupThreadMenu()
{
    actionCollection()->action("threadExpand")->plug(&threadMenu_);
    actionCollection()->action("threadCollapse")->plug(&threadMenu_);
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
        
    EmpathIndexRecord rec(f->index()->record(s));

    if (rec.isNull()) {
        empathDebug("Can't find index record for \"" + s + "\"");
        return;
    }
    
    KConfig * c(KGlobal::config());
    
    using namespace EmpathConfig;

    c->setGroup(GROUP_DISPLAY);
 
    if (c->readBoolEntry(UI_THREAD))
        _threadItem(rec);
    else
        _createListItem(rec);
}

    void
EmpathMessageListWidget::_fillDisplay() 
{
    filling_ = true;
    
    _clear();
    triggerUpdate();
    kapp->processEvents();

    setUpdatesEnabled(false);
    viewport()->setUpdatesEnabled(false);
    setSorting(-1);

    KConfig * c(KGlobal::config());
    
    using namespace EmpathConfig;

    c->setGroup(GROUP_DISPLAY);
   
    if (c->readBoolEntry(UI_THREAD))
        _fillThreading();
    else
        _fillNormal();

    if (filling_) {
 
        c->setGroup("Folder_" + url_.mailboxName() + "/" + url_.folderPath());
  
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
EmpathMessageListWidget::_fillThreading()
{
    setRootIsDecorated(true);

    EmpathFolder * f = empath->folder(url_);

    if (!f) {
        empathDebug("Can't find folder");
        return;
    }
 

    QStringList index(f->index()->allKeys());
    
    unsigned int numMsgs = index.count();

    EmpathTask t(i18n("Sorting messages"));
    t.setMax(numMsgs);

    unsigned int dictSize = 101;

    if (numMsgs > 100 && numMsgs < 500)
        dictSize = 503;
    else if (numMsgs < 1000)
        dictSize = 1009;
    else if (numMsgs < 5000)
        dictSize = 5003;
    else if (numMsgs < 10000)
        dictSize = 1007;

    QStringList::ConstIterator indexIt(index.begin());
    QDict<ThreadNode> allDict(dictSize);

    // Put all index records into thread nodes, and the nodes into allDict.
    // Two versions, one for unread only.

    QStringList::ConstIterator it(index.begin());
    EmpathIndexRecord * rec = static_cast<EmpathIndexRecord *>(0L);

    if (hideRead_) {

        for (; it != index.end(); ++it)
        {
            rec = new EmpathIndexRecord(f->index()->record(*it));
            
            if (!(rec->status() & RMM::Read))
                allDict.insert(rec->messageID(), new ThreadNode(rec));
        }

    } else {

        for (; it != index.end(); ++it)
        {
            rec = new EmpathIndexRecord(f->index()->record(*it));

            allDict.insert(rec->messageID(), new ThreadNode(rec));
        }
    }

    QDict<ThreadNode> rootDict(dictSize);

    // Go through every node in allDict.
    // Look for parent of node.
    // If found, set parent's 'next' pointer to this item.
    // If not, add this item to rootDict.
    for (QDictIterator<ThreadNode> it(allDict); it.current(); ++it) {

        if (it.current()->data()->hasParent()) {
    
            ThreadNode * parentNode = allDict[it.current()->data()->parentID()];

            if (parentNode != 0)
                parentNode->addChild(it.current());
            else
                rootDict.insert(it.current()->data()->messageID(), it.current());
        
        } else 
            
            rootDict.insert(it.current()->data()->messageID(), it.current());
    }

    // Now go through every node in rootDict, adding the threads below it.
    for (QDictIterator<ThreadNode> it(rootDict); it.current(); ++it)
        _createThreads(it.current(), &t);
}

    void
EmpathMessageListWidget::_createThreads(
    ThreadNode * root,
    EmpathTask * t,
    EmpathMessageListItem * parent
)
{
    EmpathMessageListItem * i = _createListItem(*(root->data()), t, parent);
   
    for (QListIterator<ThreadNode> it(root->childList()); it.current(); ++it)
        _createThreads(it.current(), t, i);
}

    EmpathMessageListItem *
EmpathMessageListWidget::_createListItem(
    EmpathIndexRecord & rec,
    EmpathTask * t,
    EmpathMessageListItem * parent
)
{
    if (t != 0)
        t->doneOne();

    return _pool(rec, parent);
}

    EmpathMessageListItem *
EmpathMessageListWidget::_pool(
    EmpathIndexRecord & rec,
    EmpathMessageListItem * parent
)
{
    EmpathMessageListItem * retval = 0L;

#if 0
    if (listItemPool_->isEmpty()) {
#endif

        if (0 == parent)
            retval = new EmpathMessageListItem(this, rec);
        else
            retval = new EmpathMessageListItem(parent, rec);

#if 0
    } else {

        empathDebug("reusing");

        empathDebug("getting pointer to first item");
        retval = listItemPool_->getFirst();
        empathDebug("setting record");
        retval->setRecord(rec);

        empathDebug("removing first item from pool");
        listItemPool_->removeFirst();

        empathDebug("inserting item");
        if (0 == parent)
            insertItem(retval);
        else
            parent->insertItem(retval);
    }
#endif

    return retval;
}

    void
EmpathMessageListWidget::_fillNormal()
{
    setRootIsDecorated(false);

    EmpathFolder * f = empath->folder(url_);

    if (!f) {
        empathDebug("Can't find folder");
        return;
    }
 
    EmpathTask t(i18n("Sorting messages"));
    t.setMax(f->index()->count());
    
    QStringList index(f->index()->allKeys());
    QStringList::ConstIterator it(index.begin());
    
    EmpathIndexRecord rec;

    for (; it != index.end(); ++it) {

        rec = f->index()->record(*it);

        if (!rec.isNull())
            if (!hideRead_ || !(rec.status() & RMM::Read))
                _createListItem(rec, &t);
    }
    
    t.done();
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
EmpathMessageListWidget::s_threadExpand()
{
    QList<QListViewItem> threadList;
    
    threadList = subThread(currentItem());
    
    QListIterator<QListViewItem> it(threadList);
    for (; it.current(); ++it)
        setOpen(it.current(), true);
}

    void
EmpathMessageListWidget::s_threadCollapse()
{
    QList<QListViewItem> threadList;
    
    threadList = subThread(currentItem());
    
    QListIterator<QListViewItem> it(threadList);
    for (; it.current(); ++it)
        setOpen(it.current(), false);
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

    void
EmpathMessageListWidget::_clear()
{
    setUpdatesEnabled(false);
    clear();
    setUpdatesEnabled(true);
    return;
    // STUB


    empathDebug("clearing pool");
    listItemPool_->clear();

    empathDebug("taking items and placing in pool");
    QListViewItemIterator it(this);
    
    while (it.current()) {

        empathDebug(".");

        listItemPool_
            ->append(static_cast<EmpathMessageListItem *>(it.current()));

        takeItem(it.current());

        ++it;
    }

    empathDebug("clearing");
    clear();
    setUpdatesEnabled(true);
}

// vim:ts=4:sw=4:tw=78
