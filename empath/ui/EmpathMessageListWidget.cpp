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

// Qt includes
#include <qheader.h>
#include <qdragobject.h>

// KDE includes
#include <kconfig.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kaction.h>
#include <kinstance.h>

// Local includes
#include "EmpathMessageMarkDialog.h"
#include "EmpathMessageListWidget.h"
#include "EmpathMessageListItem.h"
#include "EmpathIndexRecord.h"
#include "EmpathCustomEvents.h"
#include "Empath.h"
 
EmpathMessageListWidget::EmpathMessageListWidget(QWidget * parent)
    :   EmpathListView      (parent, "EmpathMessageListWidget"),
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
    
    _connectUp();

    _restoreColumnSizes();
}

EmpathMessageListWidget::~EmpathMessageListWidget()
{
    _saveColumnSizes();
}

    void
EmpathMessageListWidget::_init()
{
    EmpathMessageListItem::initStatic();

    px_unread_      = BarIcon("tree-unread");
    px_read_        = BarIcon("tree-read");
    px_marked_      = BarIcon("tree-marked");
    px_attachments_ = BarIcon("tree-attachments");
    px_replied_     = BarIcon("tree-replied");

    actionCollection_ = new KActionCollection(this);

    (void) new KAction(
        i18n("&View"),
        "empath_message_view",
        CTRL+Key_Return,
        this,
        SLOT(s_messageView()),
        actionCollection(),
        "messageView"
    );
        
    (void) new KAction(
        i18n("&Compose"),
        "empath_message_compose",
        Key_M,
        this,
        SLOT(s_messageCompose()),
        actionCollection(),
        "messageCompose"
    );
        
    (void) new KAction(
        i18n("&Reply"),
        "empath_messageReply",
        Key_R,
        this,
        SLOT(s_messageReply()),
        actionCollection(),
        "messageReply"
    );
        
    (void) new KAction(
        i18n("Reply to &All"),
        "empath_message_reply_all",
        Key_G,
        this,
        SLOT(s_messageReplyAll()),
        actionCollection(),
        "messageReplyAll"
    );
        
    (void) new KAction(
        i18n("&Forward"),
        "empath_message_forward",
        Key_F,
        this,
        SLOT(s_messageForward()),
        actionCollection(),
        "messageForward"
    );
        
    (void) new KAction(
        i18n("&Delete"),
        "empath_message_delete",
        Key_D,
        this,
        SLOT(s_messageDelete()),
        actionCollection(),
        "messageDelete"
    );
        
    (void) new KAction(
        i18n("&Bounce"),
        "empath_message_bounce",
        0,
        this,
        SLOT(s_messageBounce()),
        actionCollection(),
        "messageBounce"
    );
        
    (void) new KAction(
        i18n("Save &As..."),
        "empath_message_save_as",
        0,
        this,
        SLOT(s_messageSaveAs()),
        actionCollection(),
        "messageSaveAs"
    );
        
    (void) new KAction(
        i18n("&Copy To..."),
        "empath_message_copy",
        Key_C,
        this,
        SLOT(s_messageCopyTo()),
        actionCollection(),
        "messageCopyTo"
    );
        
    (void) new KAction(
        i18n("&Move To..."),
        "empath_message_move",
        0,
        this,
        SLOT(s_messageMoveTo()),
        actionCollection(),
        "messageMoveTo"
    );
        
    (void) new KAction(
        i18n("Mark..."),
        "empath_message_mark_many",
        0,
        this,
        SLOT(s_messageMarkMany()),
        actionCollection(),
        "messageMarkMany"
    );
        
    (void) new KAction(
        i18n("&Print"),
        "empath_message_print",
        0,
        this,
        SLOT(s_messagePrint()),
        actionCollection(),
        "messagePrint"
    );
        
    (void) new KAction(
        i18n("&Filter"),
        "empath_message_filter",
        0,
        this,
        SLOT(s_messageFilter()),
        actionCollection(),
        "messageFilter"
    );
        
    (void) new KAction(
        i18n("&Expand"),
        "empath_thread_expand",
        0,
        this,
        SLOT(s_threadExpand()),
        actionCollection(),
        "threadExpand"
    );
        
    (void) new KAction(
        i18n("&Collapse"),
        "empath_thread_collapse",
        0,
        this,
        SLOT(s_threadCollapse()),
        actionCollection(),
        "threadCollapse"
    );
        
    (void) new KAction(
        i18n("&Previous"),
        "empath_go_previous",
        CTRL+Key_P,
        this,
        SLOT(s_goPrevious()),
        actionCollection(),
        "goPrevious"
    );
        
    (void) new KAction(
        i18n("&Next"),
        "empath_go_next",
        CTRL+Key_N,
        this,
        SLOT(s_goNext()),
        actionCollection(),
        "goNext"
    );
        
    (void) new KAction(
        i18n("Next &Unread"),
        "empath_go_next_unread",
        Key_N,
        this,
        SLOT(s_goNextUnread()),
        actionCollection(),
        "goNextUnread"
    );
        

    (void) new KToggleAction(
        i18n("&Tag"),
        "empath_message_tag",
        0,
        this,
        SLOT(s_messageMark()),
        actionCollection(),
        "messageTag"
     );
        
    (void) new KToggleAction(
        i18n("&Mark as read"),
        "empath_message_mark_read",
        0,
        this,
        SLOT(s_messageMarkRead()),
        actionCollection(),
        "messageMarkRead"
     );
        
    (void) new KToggleAction(
        i18n("Mark as replied"),
        "empath_message_mark_replied",
        0,
        this,
        SLOT(s_messageMarkReplied()),
        actionCollection(),
        "messageMarkReplied"
     );
        
    (void) new KToggleAction(
        i18n("Hide Read"),
        "empath_hide_read",
        0,
        this,
        SLOT(s_toggleHideRead()),
        actionCollection(),
        "hideRead"
     );
        
    (void) new KToggleAction(
        i18n("Thread Messages"),
        "empath_thread",
        0,
        this,
        SLOT(s_toggleThread()),
        actionCollection(),
        "thread"
     );

}

    void
EmpathMessageListWidget::_connectUp()
{
    QObject::connect(
        this, SIGNAL(currentChanged(QListViewItem *)),
        this, SLOT(s_currentChanged(QListViewItem *)));

    QObject::connect(
        this, SIGNAL(startDrag(const QList<QListViewItem> &)),
        this, SLOT(s_startDrag(const QList<QListViewItem> &)));
    
    QObject::connect(
        this,   SIGNAL(currentChanged(QListViewItem *)),
        this,   SLOT(s_updateActions(QListViewItem *)));

    // Connect return press to view.
    QObject::connect(
        this, SIGNAL(returnPressed(QListViewItem *)),
        this, SLOT(s_currentChanged(QListViewItem *)));
    
    // Connect right button up so we can produce the popup context menu.
    QObject::connect(
        this,
        SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int, Area)),
        this,
        SLOT(s_rightButtonPressed(QListViewItem *, const QPoint &, int, Area)));
    
    // We want to remember which column we are sorting by.
    QObject::connect(
        header(),   SIGNAL(sectionClicked(int)),
        this,       SLOT(s_headerClicked(int)));
}

    void
EmpathMessageListWidget::_saveColumnSizes()
{
    KConfig * c = KGlobal::config();
    
    c->setGroup("EmpathMessageListWidget");
    
    for (int i = 0 ; i < header()->count() ; i++)
        c->writeEntry("ColumnSize" + QString::number(i), header()->cellSize(i));
}
 
    void
EmpathMessageListWidget::_restoreColumnSizes()
{
    KConfig * c = KGlobal::config();

    c->setGroup("EmpathMessageListWidget");
    
    for (int i = 0 ; i < header()->count() ; i++) {

        unsigned int sz =
            c->readUnsignedNumEntry("ColumnSize" + QString::number(i), 80);

        header()->setCellSize(i, sz);

        setColumnWidthMode(i, QListView::Manual);
    }
}

    EmpathMessageListItem *
EmpathMessageListWidget::_threadItem(const EmpathIndexRecord & rec)
{
    if (rec.isNull())
        return 0;

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

    return _createListItem(rec, parentItem);
}

    EmpathURL
EmpathMessageListWidget::firstSelected()
{
    if (currentItem() == 0)
        return EmpathURL();
    
    EmpathURL url(folder_);
    empathDebug("My url is '" + url.asString() + "'");
    url.setMessageID(static_cast<EmpathMessageListItem *>(currentItem())->id());
    empathDebug("url with message id set is '" + url.asString() + "'");
    return url;
}

    void
EmpathMessageListWidget::_markOne(EmpathIndexRecord::Status status)
{
    QListViewItem * item = currentItem();

    if (!item)
        return;

    EmpathMessageListItem * i = static_cast<EmpathMessageListItem *>(item);

    // Don't bother auto-marking this as the user's done it.
    i->cancelAutoMarkTimer();

    EmpathIndexRecord::Status stat = EmpathIndexRecord::Status(i->status());
    
    EmpathIndexRecord::Status s = EmpathIndexRecord::Status(
        stat & status ? stat ^ status : stat | status
    );
    
    i->setStatus(s);
}

    void
EmpathMessageListWidget::s_goPrevious()
{    
    QListViewItem * i = currentItem();
    
    if (0 == i) 
        i = firstChild();
    else if (i->itemAbove())
        i = i->itemAbove();
    
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

        if (!(i->status() & EmpathIndexRecord::Read)) {
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

        if (!(i->status() & EmpathIndexRecord::Read)) {
            clearSelection();
            setCurrentItem(i);
            setSelected(i, true);
            ensureItemVisible(currentItem());
            break;
        }
    }
}

    void
EmpathMessageListWidget::s_rightButtonPressed(QListViewItem *, 
        const QPoint & pos, int, EmpathMessageListWidget::Area area)
{
    if (area == Void)
        return;
    
    if (area == OpenClose)
        threadMenu_.exec(pos);
    else if (selected_.count() == 1)
        messageMenu_.exec(pos);
    else if (selected_.count() > 1)
        multipleMessageMenu_.exec(pos);
}

    void
EmpathMessageListWidget::s_updateActions(QListViewItem * item)
{
    if (0 == actionCollection()) {
        empathDebug("actionCollection() returned NULL !");
        return;
    }

    QStringList affectedActions;

    affectedActions
        << "messageView"    << "messageReply"   << "messageReplyAll"
        << "messageForward" << "messageDelete"  << "messageSaveAs"
        << "messageCopyTo"  << "messageMoveTo"  << "messagePrint"
        << "messageFilter"; // TODO: The rest

    QStringList::ConstIterator it(affectedActions.begin());

    for (; it != affectedActions.end(); ++it) {
        KAction * a = actionCollection()->action((*it).utf8());
        if (0 == a) {
            empathDebug("Action " + *it + " is NULL !");
        }
        else
            a->setEnabled(0 != item);
    }

    // XXX Decide who owns which actions.
    if (0 != item) {

        EmpathMessageListItem * i = static_cast<EmpathMessageListItem *>(item);

        KAction * a = actionCollection()->action("messageMarkRead");

        if (0 == a) {
            empathDebug("Action messageMarkRead is NULL !");
        } else {
            a->setText(
                    i->status() & EmpathIndexRecord::Read ?
                    i18n("Mark as unread") : i18n("Mark as read"));
        }

        a = actionCollection()->action("messageMarkReplied");

        if (0 == a) {
            empathDebug("Action messageMarkReplied is NULL !");
        } else {
            a->setText(
                    i->status() & EmpathIndexRecord::Replied ?
                    i18n("Mark as not replied to") : i18n("Mark as replied to"));
        }

        a = actionCollection()->action("messageTag");

        if (0 == a) {
            empathDebug("Action messageTag is NULL !");
        } else {
            a->setText(
                    i->status() & EmpathIndexRecord::Marked ?
                    i18n("Untag") : i18n("Tag"));
        }
    }
}

    void
EmpathMessageListWidget::s_doubleClicked(QListViewItem *)
{ emit(messageActivated(firstSelected())); }

    void
EmpathMessageListWidget::s_currentChanged(QListViewItem * item)
{
    if (0 == item)
        return;

#if 0
    // Make sure we highlight the current item.
    setUpdatesEnabled(false);
    viewport()->setUpdatesEnabled(false);
    
    clearSelection();
    setSelected(item, true);
    static_cast<EmpathMessageListItem *>(item)->startAutoMarkTimer();

    setUpdatesEnabled(true);
    viewport()->setUpdatesEnabled(true);

    triggerUpdate();
#endif

//    qDebug("emitting messageActivated()");
    emit(messageActivated(firstSelected()));
}

    void
EmpathMessageListWidget::s_toggleHideRead()
{
    hideRead_ = !hideRead_;

    KConfig * c = KGlobal::config();
    
    c->setGroup("EmpathMessageListWidget");
 
    c->writeEntry("HideRead", hideRead_);
}

    void
EmpathMessageListWidget::s_toggleThread()
{
    thread_ = !thread_;

    KConfig * c = KGlobal::config();
    
    c->setGroup("EmpathMessageListWidget");
 
    c->writeEntry("Thread", thread_);
}

    void
EmpathMessageListWidget::setIndex(const EmpathURL & url)
{
    qDebug("EmpathMessageListWidget::setIndex");
    waitingForIndex_ = url;
    empathDebug("Asking empath to read index for " + url.asString());
    empath->readIndex(url, this);
}

    void
EmpathMessageListWidget::s_headerClicked(int i)
{
    // If the last header clicked on is the same as the one we're given, change
    // the sort order for the column. Otherwise, revert back to ascending order.

    if (lastHeaderClicked_ == i)
        sortAscending_ = !sortAscending_;
    
    else sortAscending_ = true; // revert
    
    sortColumn_ = i;
    
    lastHeaderClicked_ = i;
}
    void 
EmpathMessageListWidget::s_startDrag(const QList<QListViewItem> & items)
{
    QStrList uriList;
    
    QListIterator<QListViewItem> it(items);
    
    while (it.current()) {
        
        EmpathMessageListItem * i =
            static_cast<EmpathMessageListItem *>(it.current());

        // TODO
//        EmpathURL messageURL(url_.mailboxName(), url_.folderPath(), i->id());
//        uriList.append(messageURL.asString().utf8());
        ++it;
    }

    QUriDrag * u  = new QUriDrag(uriList, this);

    u->setPixmap(BarIcon("tree"));

    u->drag();
}

    void
EmpathMessageListWidget::s_selectTagged()
{
    clearSelection();

    viewport()->setUpdatesEnabled(false);
    
    QListViewItemIterator it(this);
    
    for (; it.current(); ++it) {
 
        EmpathMessageListItem * i =
            static_cast<EmpathMessageListItem *>(it.current());

        if (i->status() & EmpathIndexRecord::Marked)
            _setSelected(i, true);
    }
    
    viewport()->setUpdatesEnabled(true);
    triggerUpdate();
}

    void
EmpathMessageListWidget::s_selectRead()
{
    clearSelection();

    viewport()->setUpdatesEnabled(false);
    
    QListViewItemIterator it(this);
    
    for (; it.current(); ++it) {
 
        EmpathMessageListItem * i =
            static_cast<EmpathMessageListItem *>(it.current());

        if (i->status() & EmpathIndexRecord::Read)
            _setSelected(i, true);
    }
    
    viewport()->setUpdatesEnabled(true);
    triggerUpdate();
}

    void
EmpathMessageListWidget::s_selectAll()
{
    viewport()->setUpdatesEnabled(false);
    
    QListViewItemIterator it(this);
    
    for (; it.current(); ++it)
        setSelected(it.current(), true);
    
    viewport()->setUpdatesEnabled(true);
    triggerUpdate();
}    

    void
EmpathMessageListWidget::s_selectInvert()
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
EmpathMessageListWidget::s_itemGone(const EmpathIndexRecord & rec)
{
    if (filling_) return;
    
    QListViewItemIterator it(this);
    
    for (; it.current(); ++it) {
 
        EmpathMessageListItem * i =
            static_cast<EmpathMessageListItem *>(it.current());

        if (i->id() == rec.id()) {
            selected_.remove(i);
            delete i;
        }
    }
}

    void
EmpathMessageListWidget::s_itemCome(const EmpathIndexRecord & rec)
{
    if (filling_)
        return;

    if (thread_)
        _threadItem(rec);
    else
        _createListItem(rec);
}

    void
EmpathMessageListWidget::_fillDisplay() 
{
    filling_ = true;
    
    setUpdatesEnabled(false);
    viewport()->setUpdatesEnabled(false);

    clear();
    selected_.clear();

    setSorting(-1);

    if (thread_)
        _fillThreading();
    else
        _fillNormal();

    if (filling_) {

        setSorting(sortColumn_, sortAscending_);

        viewport()->setUpdatesEnabled(true);
        setUpdatesEnabled(true);
        
        triggerUpdate();
        
        filling_ = false;
    }

    s_updateActions(static_cast<QListViewItem *>(0L));
}

    void
EmpathMessageListWidget::_fillThreading()
{
    setRootIsDecorated(true);

    unsigned int numMsgs = index_.count();

    unsigned int dictSize = 101;

    if (numMsgs > 100 && numMsgs < 500)
        dictSize = 503;
    else if (numMsgs < 1000)
        dictSize = 1009;
    else if (numMsgs < 5000)
        dictSize = 5003;
    else if (numMsgs < 10000)
        dictSize = 10007;

    // Anyone noticed that any even number is the sum of two primes ?

    QDict<ThreadNode> allDict(dictSize);

    // Put all index records into thread nodes, and the nodes into allDict.
    // Two versions, one for unread only.

    if (hideRead_) {

        for (QDictIterator<EmpathIndexRecord> it(index_); it.current(); ++it)
            if (!(it.current()->status() & EmpathIndexRecord::Read))
                allDict.insert(
                    it.current()->messageID(),
                    new ThreadNode(it.current()));

    } else {

        for (QDictIterator<EmpathIndexRecord> it(index_); it.current(); ++it)
            allDict.insert(
                it.current()->messageID(),
                new ThreadNode(it.current()));
    }
    

    QDict<ThreadNode> rootDict(dictSize);

    // Go through every node in allDict.
    // Look for parent of node.
    // If found, set parent's 'next' pointer to this item.
    // If not, add this item to rootDict.
    for (QDictIterator<ThreadNode> allIt(allDict); allIt.current(); ++allIt) {

        ThreadNode * i = allIt.current();

        if (!(i->data()->hasParent()))
            rootDict.insert(i->data()->messageID(), i);

        else {
    
            ThreadNode * parentNode = allDict[i->data()->parentID()];

            if (0 != parentNode)
                parentNode->addChild(i);
            else
                rootDict.insert(i->data()->messageID(), i);
        
        }
    }
    
    // Now go through every node in rootDict, adding the threads below it.
    for (QDictIterator<ThreadNode> rootIt(rootDict); rootIt.current(); ++rootIt)
        _createThreads(rootIt.current());
}

    void
EmpathMessageListWidget::_createThreads(
    ThreadNode * root,
    EmpathMessageListItem * parent
)
{
    _createListItem(*(root->data()));

    for (QListIterator<ThreadNode> it(root->childList()); it.current(); ++it)
        _createThreads(it.current(), _createListItem(*(root->data()), parent));
}

    EmpathMessageListItem *
EmpathMessageListWidget::_createListItem(
    const EmpathIndexRecord & rec,
    EmpathMessageListItem * parent
)
{
    if (0 == parent)
        return new EmpathMessageListItem(this, rec);
    else
        return new EmpathMessageListItem(parent, rec);
}

    void
EmpathMessageListWidget::_fillNormal()
{
    setRootIsDecorated(false);

    QDictIterator<EmpathIndexRecord> it(index_);

    if (hideRead_) {
        for (; it.current(); ++it)
            if (!hideRead_ ||
                !(it.current()->status() & EmpathIndexRecord::Read))
                _createListItem(*it.current());
    } else
        for (; it.current(); ++it)
            _createListItem(*it.current());
}

    void
EmpathMessageListWidget::s_messageMarkMany()
{
    EmpathMessageMarkDialog d;
    
    if (d.exec() != QDialog::Accepted)
        return;
    
    EmpathMessageMarkDialog::MarkType t = d.markType();

    EmpathIndexRecord::Status s = d.status();
    
    QStringList l;

    EmpathMessageListItemIterator it(selected_);
    
    for (; it.current(); ++it)
        l.append(it.current()->id());    
        
    switch (t) {
        
        case EmpathMessageMarkDialog::On:

            for (it.toFirst(); it.current(); ++it)
                it.current()->setStatus(
                    EmpathIndexRecord::Status(it.current()->status() | s));

            break;

        case EmpathMessageMarkDialog::Off:

            for (it.toFirst(); it.current(); ++it)
                it.current()->setStatus(
                    EmpathIndexRecord::Status(it.current()->status() & (~s)));
        
            break;

        case EmpathMessageMarkDialog::Toggle:
            
            for (it.toFirst(); it.current(); ++it)
                it.current()->setStatus(
                    EmpathIndexRecord::Status(
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
    QList<QListViewItem> threadList(subThread(currentItem()));
    
    for (QListIterator<QListViewItem> it(threadList); it.current(); ++it)
        setOpen(it.current(), true);
}

    void
EmpathMessageListWidget::s_threadCollapse()
{
    QList<QListViewItem> threadList(subThread(currentItem()));
    
    for (QListIterator<QListViewItem> it(threadList); it.current(); ++it)
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
}

    bool
EmpathMessageListWidget::event(QEvent * e)
{
    switch (e->type()) {

        case EmpathIndexReadEventT:
            empathDebug("indexreadevent");
            {
                EmpathIndexReadEvent * ev =
                    static_cast<EmpathIndexReadEvent *>(e);

                if (ev->success() && (ev->folder() == waitingForIndex_)) {

                    folder_ = waitingForIndex_;
                    waitingForIndex_ = EmpathURL();
                    filling_ = false;
                    index_ = ev->index()->dict();
                    empathDebug("Dict count: " +
                            QString::number(index_.count()));
                    _fillDisplay();
                }
            }

            return true;
            break;

        default:
            break;
    }

    return QListView::event(e);
}

    void
EmpathMessageListWidget::s_messageMark()
{ _markOne(EmpathIndexRecord::Marked); }

    void
EmpathMessageListWidget::s_messageMarkRead()
{ _markOne(EmpathIndexRecord::Read); }

    void
EmpathMessageListWidget::s_messageMarkReplied()
{ _markOne(EmpathIndexRecord::Replied); }

    void
EmpathMessageListWidget::s_forward()
{
    empathDebug("STUB");
//    empath->forward(selection());
}

    void
EmpathMessageListWidget::s_bounce()
{
    empathDebug("STUB");
//    empath->bounce(selection());
}

    void
EmpathMessageListWidget::s_remove()
{
    empathDebug("STUB");
//    empath->remove(selection());
}

    void
EmpathMessageListWidget::s_copyTo()
{
    empathDebug("STUB");
//    empath->copy(selection());
}

    void
EmpathMessageListWidget::s_moveTo()
{
    empathDebug("STUB");
//    empath->move(selection());
}

    void
EmpathMessageListWidget::s_print()
{
    empathDebug("STUB");
//    empath->print(selection());
}

    void
EmpathMessageListWidget::s_filter()
{
    empathDebug("STUB");
//    empath->filter(selection());
}

    EmpathURLList
EmpathMessageListWidget::selection()
{
    empathDebug("STUB");
    QValueList<EmpathURL> l;
    return l;
}

// vim:ts=4:sw=4:tw=78
