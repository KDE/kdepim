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
 
extern "C"
{
    void *init_libEmpathMessageListWidget()
    {
        return new EmpathMessageListPartFactory;
    }
}

KInstance * EmpathMessageListPartFactory::instance_ = 0L;

EmpathMessageListPartFactory::EmpathMessageListPartFactory()
{
    // Empty.
}

EmpathMessageListPartFactory::~EmpathMessageListPartFactory()
{
    delete instance_;
    instance_ = 0L;
}

    QObject *
EmpathMessageListPartFactory::create(
    QObject * parent,
    const char * name,
    const char *,
    const QStringList &
)
{
    QObject * o = new EmpathMessageListPart((QWidget *)parent, name);
    emit objectCreated(o);
    return o;
}

    KInstance *
EmpathMessageListPartFactory::instance()
{
    if (0 == instance_)
        instance_ = new KInstance("EmpathMessageListWidget");

    return instance_;
}

// -------------------------------------------------------------------------

EmpathMessageListPart::EmpathMessageListPart(
    QWidget * parent,
    const char * name
)
    :   KParts::ReadOnlyPart(parent, name)
{
    setInstance(EmpathMessageListPartFactory::instance());

    widget_ = new EmpathMessageListWidget(parent, this);
    widget_->setFocusPolicy(QWidget::StrongFocus);
    setWidget(widget_);

    QObject::connect(
        widget_,    SIGNAL(messageActivated(const QString &)),
        this,       SIGNAL(messageActivated(const QString &)));

    setXMLFile("EmpathMessageListWidget.rc");
    enableAllActions(false);
}

EmpathMessageListPart::~EmpathMessageListPart()
{
    // Empty.
}

    void
EmpathMessageListPart::enableAllActions(bool)
{
    // STUB
}

    void
EmpathMessageListPart::_initActions()
{
#define CA(visibleName, name, key, slot) \
    new KAction(visibleName, name, key, \
        widget_, slot, actionCollection(), name);

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
    new KToggleAction(visibleName, QIconSet(BarIcon(name)), key, \
        widget_, slot, actionCollection(), name);

CTA(i18n("&Tag"), "messageTag", 0, SLOT(s_messageMark()));
CTA(i18n("&Mark as read"), "messageMarkRead", 0, SLOT(s_messageMarkRead()));
CTA(i18n("Mark as replied"), "messageMarkReplied", 0, SLOT(s_messageMarkReplied()));
CTA(i18n("Hide Read"), "hideRead", 0, SLOT(s_toggleHideRead()));
CTA(i18n("Thread Messages"), "thread", 0, SLOT(s_toggleThread()));

#undef CTA
}

    void
EmpathMessageListPart::s_setIndex(const QDict<EmpathIndexRecord> & l)
{ 
    qDebug("EmpathMessageListPart::setIndex");
    widget_->setIndex(l);
}


// -------------------------------------------------------------------------

   
EmpathMessageListWidget::EmpathMessageListWidget(
    QWidget * parent,
    EmpathMessageListPart * part
)
    :   EmpathListView      (parent, "EmpathMessageListWidget"),
        part_               (part),
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
#if 0
    QStringList affectedActions;

    affectedActions
        << "messageView"    << "messageReply"   << "messageReplyAll"
        << "messageForward" << "messageDelete"  << "messageSaveAs"
        << "messageCopyTo"  << "messageMoveTo"  << "messagePrint"
        << "messageFilter"; // TODO: The rest

    QStringList::ConstIterator it(affectedActions.begin());

    for (; it != affectedActions.end(); ++it)
        part_->actionCollection()->action((*it).utf8())->setEnabled(0 != item);

    // XXX Decide who owns which actions.
    if (0 != item) {

        EmpathMessageListItem * i = static_cast<EmpathMessageListItem *>(item);

        part_->actionCollection()->action("messageMarkRead")->setText(
            i->status() & EmpathIndexRecord::Read ?
            i18n("Mark as unread") : i18n("Mark as read"));

        part_->actionCollection()->action("messageMarkReplied")->setText(
            i->status() & EmpathIndexRecord::Replied ?
            i18n("Mark as not replied to") : i18n("Mark as replied to"));

        part_->actionCollection()->action("messageTag")->setText(
            i->status() & EmpathIndexRecord::Marked ?
            i18n("Untag") : i18n("Tag"));
    }
#endif
}

    void
EmpathMessageListWidget::s_doubleClicked(QListViewItem *)
{ emit(messageActivated(firstSelected())); }

    void
EmpathMessageListWidget::s_linkChanged(QListViewItem * item)
{
    qDebug("linkChanged");

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

    qDebug("emitting messageActivated()");
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
EmpathMessageListWidget::setIndex(const QDict<EmpathIndexRecord> & l)
{
    qDebug("EmpathMessageListWidget::setIndex");
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
EmpathMessageListWidget::s_startDrag(const QList<QListViewItem> & items)
{
    QStrList uriList;
    
    QListIterator<QListViewItem> it(items);
    
    while (it.current()) {
        
//        EmpathMessageListItem * i =
//            static_cast<EmpathMessageListItem *>(it.current());

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
EmpathMessageListPart::s_messageCompose()
{ emit(compose()); }

    void
EmpathMessageListPart::s_messageReply()
{ emit(reply(widget_->firstSelected())); }

    void
EmpathMessageListPart::s_messageReplyAll()
{ emit(replyAll(widget_->firstSelected())); }

    void
EmpathMessageListPart::s_messageForward()
{ emit(forward(widget_->firstSelected())); }

    void
EmpathMessageListPart::s_messageBounce()
{ emit(bounce(widget_->firstSelected())); }

    void
EmpathMessageListPart::s_messageDelete()
{ emit(remove(widget_->selection())); }

    void
EmpathMessageListPart::s_messageSaveAs()
{ emit(save(widget_->firstSelected())); }

    void
EmpathMessageListPart::s_messageCopyTo()
{ emit(copy(widget_->selection())); }

    void
EmpathMessageListPart::s_messageMoveTo()
{ emit(move(widget_->selection())); }

    void
EmpathMessageListPart::s_messagePrint()
{ emit(print(widget_->selection())); }

    void
EmpathMessageListPart::s_messageFilter()
{ emit(filter(widget_->selection())); }

    void
EmpathMessageListPart::s_messageView()
{ emit(view(widget_->firstSelected())); }

    QStringList
EmpathMessageListWidget::selection()
{
    qDebug("STUB");
    QStringList l;
    return l;
}

// vim:ts=4:sw=4:tw=78
