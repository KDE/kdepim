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
#include <qapplication.h>
#include <qtimer.h>
#include <qstring.h>

// KDE includes
#include <klocale.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kaction.h>
#include <kapp.h>

// Local includes
#include "EmpathMessageMarkDialog.h"
#include "EmpathMessageListWidget.h"
#include "EmpathMessageListItem.h"
#include "EmpathIndexRecord.h"
    
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
    
    _initActions();
//    _setupMessageMenu();
//    _setupThreadMenu();
    _connectUp();
}

EmpathMessageListWidget::~EmpathMessageListWidget()
{
    // Empty.
}

    void
EmpathMessageListWidget::_init()
{
    px_unread_      = BarIcon("tree-unread");
    px_read_        = BarIcon("tree-read");
    px_marked_      = BarIcon("tree-marked");
    px_attachments_ = BarIcon("tree-attachments");
    px_replied_     = BarIcon("tree-replied");
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
    
    QObject::connect(
        this,   SIGNAL(currentChanged(QListViewItem *)),
        this,   SLOT(s_updateActions(QListViewItem *)));

    // Connect return press to view.
    QObject::connect(
        this, SIGNAL(returnPressed(QListViewItem *)),
        this, SLOT(s_linkChanged(QListViewItem *)));
    
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

    QString
EmpathMessageListWidget::firstSelected()
{
    if (currentItem() == 0)
        return QString::null;
    
    return static_cast<EmpathMessageListItem *>(currentItem())->id();
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

        if (!(i->status() & EmpathIndexRecord::Read)) {
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

        if (!(i->status() & EmpathIndexRecord::Read)) {
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
            i->status() & EmpathIndexRecord::Read ?
            i18n("Mark as unread") : i18n("Mark as read"));

        actionCollection()->action("messageMarkReplied")->setText(
            i->status() & EmpathIndexRecord::Replied ?
            i18n("Mark as not replied to") : i18n("Mark as replied to"));

        actionCollection()->action("messageTag")->setText(
            i->status() & EmpathIndexRecord::Marked ?
            i18n("Untag") : i18n("Tag"));
    }
}

    void
EmpathMessageListWidget::s_doubleClicked(QListViewItem *)
{
    s_messageView();
}

    void
EmpathMessageListWidget::s_linkChanged(QListViewItem * item)
{
    if (0 == item)
        return;

    // Make sure we highlight the current item.
    setUpdatesEnabled(false);
    viewport()->setUpdatesEnabled(false);
    
    clearSelection();
    setSelected(item, true);
    static_cast<EmpathMessageListItem *>(item)->startAutoMarkTimer();

    setUpdatesEnabled(true);
    viewport()->setUpdatesEnabled(true);

    triggerUpdate();

    kapp->processEvents();
    
    emit(changeView(firstSelected()));
}

    void
EmpathMessageListWidget::s_hideRead()
{
    filling_ = false;
    hideRead_ = !hideRead_;
    _fillDisplay();
}

    void
EmpathMessageListWidget::s_showFolder(const QDict<EmpathIndexRecord> & l)
{
    qDebug("Hi");
    filling_ = false;
    index_ = l;
    _fillDisplay();
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
EmpathMessageListWidget::_initActions()
{
    actionCollection_ = new QActionCollection(this, "actionCollection");

#define CA(visibleName, name, key, slot) \
    new KAction(visibleName, QIconSet(BarIcon(name)), key, \
        this, slot, actionCollection(), name);


CA(i18n("&View"),       "messageView", CTRL+Key_Return, SLOT(s_messageView()));
CA(i18n("&Compose"),    "messageCompose",   Key_M,  SLOT(s_messageCompose()));
CA(i18n("&Reply"),      "messageReply",     Key_R,  SLOT(s_messageReply()));
CA(i18n("Reply to &All"),"messageReplyAll", Key_G,  SLOT(s_messageReplyAll()));
CA(i18n("&Forward"),    "messageForward",   Key_F,  SLOT(s_messageForward()));
CA(i18n("&Delete"),     "messageDelete",    Key_D,  SLOT(s_messageDelete()));
CA(i18n("&Bounce"),     "messageBounce",    0,      SLOT(s_messageBounce()));
CA(i18n("Save &As..."), "messageSaveAs",    0,      SLOT(s_messageSaveAs()));
CA(i18n("&Copy To..."), "messageCopyTo",    Key_C,  SLOT(s_messageCopyTo()));
CA(i18n("&Move To..."), "messageMoveTo",    0,      SLOT(s_messageMoveTo()));
CA(i18n("Mark..."),     "messageMarkMany",  0,      SLOT(s_messageMarkMany()));
CA(i18n("&Print"),      "messagePrint",     0,      SLOT(s_messagePrint()));
CA(i18n("&Filter"),     "messageFilter",    0,      SLOT(s_messageFilter()));
CA(i18n("&Expand"),     "threadExpand",     0,      SLOT(s_threadExpand()));
CA(i18n("&Collapse"),   "threadCollapse",   0,      SLOT(s_threadCollapse()));
CA(i18n("&Previous"),   "goPrevious",  CTRL+Key_P,  SLOT(s_goPrevious()));
CA(i18n("&Next"),       "goNext",      CTRL+Key_N,  SLOT(s_goNext()));
CA(i18n("Next &Unread"),"goNextUnread",     Key_N,  SLOT(s_goNextUnread()));

#undef CA

#define CTA(visibleName, name, key, slot) \
    new KToggleAction(visibleName, QIconSet(BarIcon(name)), \
        key, this, slot, actionCollection(), name);

CTA(i18n("&Tag"), "messageTag", 0, SLOT(s_messageMark()));
CTA(i18n("&Mark as read"), "messageMarkRead", 0, SLOT(s_messageMarkRead()));
CTA(i18n("Mark as replied"), "messageMarkReplied", 0, SLOT(s_messageMarkReplied()));
                
#undef CTA

    s_updateActions(static_cast<QListViewItem *>(0));
}

    void
EmpathMessageListWidget::_setupMessageMenu()
{

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

    s_updateActions(static_cast<QListViewItem *>(0));
}

    void
EmpathMessageListWidget::_fillThreading()
{
    setRootIsDecorated(true);

    unsigned int numMsgs = index_.count();
    qDebug("Number of msgs to show: %d", numMsgs);

    unsigned int dictSize = 101;

    if (numMsgs > 100 && numMsgs < 500)
        dictSize = 503;
    else if (numMsgs < 1000)
        dictSize = 1009;
    else if (numMsgs < 5000)
        dictSize = 5003;
    else if (numMsgs < 10000)
        dictSize = 1007;

    QDict<ThreadNode> allDict(dictSize);

    // Put all index records into thread nodes, and the nodes into allDict.
    // Two versions, one for unread only.

    QDictIterator<EmpathIndexRecord> it(index_);

    if (hideRead_) {

        for (; it.current(); ++it)
            if (!(it.current()->status() & EmpathIndexRecord::Read))
                allDict.insert(
                    it.current()->messageID(),
                    new ThreadNode(it.current()));

    } else {

        for (; it.current(); ++it)
            allDict.insert(
                it.current()->messageID(),
                new ThreadNode(it.current()));
    }
    
    qDebug("Number of msgs in allDict: %d", allDict.count());

    QDict<ThreadNode> rootDict(dictSize);

    // Go through every node in allDict.
    // Look for parent of node.
    // If found, set parent's 'next' pointer to this item.
    // If not, add this item to rootDict.
    for (QDictIterator<ThreadNode> allIt(allDict); allIt.current(); ++allIt) {

        if (allIt.current()->data()->hasParent()) {
    
            ThreadNode * parentNode =
                allDict[allIt.current()->data()->parentID()];

            if (parentNode != 0)
                parentNode->addChild(allIt.current());
            else
                rootDict.insert(
                    allIt.current()->data()->messageID(),
                    allIt.current());
        
        } else {
            
            rootDict.insert(
                allIt.current()->data()->messageID(),
                allIt.current()
            );
        }
    }
    
    qDebug("Number of msgs in rootDict: %d", rootDict.count());

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
    _createListItem(*root->data());

    for (QListIterator<ThreadNode> it(root->childList()); it.current(); ++it)
        _createThreads(it.current(), _createListItem((*root->data()), parent));
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

    qDebug("Number of msgs to show: %d", index_.count());
    QDictIterator<EmpathIndexRecord> it(index_);

    if (hideRead_) {
        for (; it.current(); ++it)
            if (!hideRead_ || !(it.current()->status() & EmpathIndexRecord::Read))
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
EmpathMessageListWidget::s_messageReply()
{ emit(reply(firstSelected())); }

    void
EmpathMessageListWidget::s_messageReplyAll()
{ emit(replyAll(firstSelected())); }

    void
EmpathMessageListWidget::s_messageForward()
{ emit(forward(firstSelected())); }

    void
EmpathMessageListWidget::s_messageBounce()
{ emit(bounce(firstSelected())); }

    void
EmpathMessageListWidget::s_messageDelete()
{
    QStringList condemned;

    EmpathMessageListItemIterator it(selected_);
    
    for (; it.current(); ++it)
        condemned.append(it.current()->id());

    emit(remove(condemned));
}

    void
EmpathMessageListWidget::s_messageSaveAs()
{ emit(save(firstSelected())); }

    void
EmpathMessageListWidget::s_messageCopyTo()
{ emit(copy(selection())); }

    void
EmpathMessageListWidget::s_messageMoveTo()
{ emit(move(selection())); }

    void
EmpathMessageListWidget::s_messagePrint()
{ emit(print(selection())); }

    void
EmpathMessageListWidget::s_messageFilter()
{ emit(filter(selection())); }

    void
EmpathMessageListWidget::s_messageView()
{ emit(view(firstSelected())); }

    QStringList
EmpathMessageListWidget::selection()
{
    qDebug("STUB");
    QStringList l;
    return l;
}

// vim:ts=4:sw=4:tw=78
