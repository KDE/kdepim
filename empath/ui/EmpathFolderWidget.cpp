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
#include <qdragobject.h>
#include <qstringlist.h>

// KDE includes
#include <kapp.h>
#include <klocale.h>
#include <klineeditdlg.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kiconloader.h>

// Local includes
#include "EmpathConfig.h"
#include "EmpathFolderWidget.h"
#include "EmpathMailboxList.h"
#include "EmpathFolderList.h"
#include "EmpathDefines.h"
#include "Empath.h"

EmpathFolderWidget::EmpathFolderWidget(QWidget * parent)
    :   EmpathListView(parent, "FolderWidget")
{
    setFrameStyle(QFrame::NoFrame);
    viewport()->setAcceptDrops(true);
    
    addColumn(i18n("Folder name"));
    addColumn(i18n("Unread"));
    addColumn(i18n("Total"));
    
    setAllColumnsShowFocus(true);
    setRootIsDecorated(false);
    setSorting(0);

    dropItem = 0;
    dragContents_ = 0;
    autoOpenTime = 500;
    autoOpenTimer = new QTimer(this);
    
    autoscrollMargin = 5;
    
    QObject::connect(
        this, SIGNAL(rightButtonPressed(QListViewItem *, 
            const QPoint &, int, Area)),
        this, SLOT(s_rightButtonPressed(QListViewItem *, 
            const QPoint &, int, Area)));
    
    QObject::connect(
        this, SIGNAL(linkChanged(QListViewItem *)),
        this, SLOT(s_linkChanged(QListViewItem *)));

    QObject::connect(
        empath, SIGNAL(updateFolderLists()), this, SLOT(s_update()));
    
    QObject::connect(autoOpenTimer, SIGNAL(timeout()),
            this, SLOT(s_openCurrent()));

    QObject::connect(
        this, SIGNAL(startDrag(const QList<QListViewItem> &)),
        this, SLOT(s_startDrag(const QList<QListViewItem> &)));

    ////////
    
    otherPopup_.insertItem(i18n("Set up accounts"),
        this, SLOT(s_setUpAccounts()));
    
    ////////
    
    folderPopup_.insertItem(i18n("New subfolder"),
        this, SLOT(s_newFolder()));
    
    folderPopup_.insertItem(i18n("Remove folder"),
        this, SLOT(s_removeFolder()));
    
    /////////
    
    mailboxPopup_.insertItem(i18n("New folder"),
        this, SLOT(s_newFolder()));
    
    mailboxPopup_.insertItem(i18n("Check mail"),
        this, SLOT(s_mailboxCheck()));
    
    mailboxPopup_.insertSeparator();
    
    mailboxPopup_.insertItem(i18n("Properties"),
        this, SLOT(s_mailboxProperties()));

    /////////
    
    s_update();
}

EmpathFolderWidget::~EmpathFolderWidget()
{
    // Empty.
}

    void
EmpathFolderWidget::s_update()
{
    EmpathMailboxListIterator mit(*(empath->mailboxList()));

    for (; mit.current(); ++mit)
        if (mit.current()->type() != EmpathMailbox::POP3)
            _addMailbox(mit.current());
}

    void
EmpathFolderWidget::_addMailbox(EmpathMailbox * mailbox)
{
    EmpathFolderListItem * newItem;
    EmpathFolderListItem * found = find(mailbox->url());

    if (found != 0) {
        
        newItem = found;
        
    } else {
    
        newItem = new EmpathFolderListItem(this, mailbox->url());
        
        QObject::connect(newItem, SIGNAL(opened()), SLOT(s_openChanged()));
            
        itemList_.append(newItem);
    }
    
    EmpathFolderListIterator fit(mailbox->folderList());

    for (; fit.current(); ++fit)
        if (fit.current()->parent() == 0)
            _addChildren(mailbox, fit.current(), newItem);
}

    void
EmpathFolderWidget::_addChildren(
    EmpathMailbox * m,
    EmpathFolder * item,
    EmpathFolderListItem * parent)
{
    EmpathFolderListItem * newItem;
    
    EmpathFolderListItem * found = find(item->url());

    if (found != 0) {
        
        newItem = found;

    } else {
    
        newItem = new EmpathFolderListItem(parent, item->url());
    
        QObject::connect(newItem, SIGNAL(opened()), SLOT(s_openChanged()));

        itemList_.append(newItem);
    }

    EmpathFolderListIterator it(m->folderList());

    for (; it.current(); ++it)
        if (it.current()->parent() == item)
            _addChildren(m, it.current(), newItem);
}

    void
EmpathFolderWidget::s_linkChanged(QListViewItem * item)
{
    EmpathFolderListItem * i = (EmpathFolderListItem *)item;
    
    if (!i->url().isFolder())
        return;
    
    EmpathFolder * f = empath->folder(i->url());
   
    if (f == 0)
        return;
    
    if (f->isContainer())
        return;

    emit(showFolder(i->url()));
}

    EmpathURL
EmpathFolderWidget::selected() const
{
    EmpathURL url;
    if (!currentItem()) return url;
    
    return (((EmpathFolderListItem *)currentItem())->url());
}

    void
EmpathFolderWidget::s_rightButtonPressed(
    QListViewItem * item, const QPoint &, int, EmpathListView::Area area)
{
    if (area == Void) {
        otherPopup_.exec(QCursor::pos());
        return;
    }

    // setSelected(item, true);
    
    EmpathFolderListItem * i = (EmpathFolderListItem *)item;
    
    empathDebug("Right button pressed over object with url \"" +
        i->url().asString() + "\"");
    
    popupMenuOverType = (i->depth() == 0 ? Mailbox : Folder);
    popupMenuOver = i;
    
    if (popupMenuOverType == Folder)
        folderPopup_.exec(QCursor::pos());
    else
        mailboxPopup_.exec(QCursor::pos());
}

    void
EmpathFolderWidget::s_mailboxCheck()
{
    if (popupMenuOverType != Mailbox) {
        empathDebug("The popup menu wasn't over a mailbox !");
        return;
    }
    
    EmpathMailbox * m = empath->mailbox(popupMenuOver->url());
    
    if (m == 0) return;

    m->s_checkMail();
}

    void
EmpathFolderWidget::s_mailboxProperties()
{
    if (popupMenuOverType != Mailbox) {
        empathDebug("The popup menu wasn't over a mailbox !");
        return;
    }

    empath->s_configureMailbox(popupMenuOver->url(), this);
}

    void
EmpathFolderWidget::s_newFolder()
{
    KLineEditDlg led(i18n("Folder name"), QString::null, this, false);
    led.exec();

    QString name = led.text();
    
    if (name.isEmpty())
        return;
        
    EmpathURL newFolderURL(popupMenuOver->url().asString() + "/" + name + "/");

    if (currentItem() != 0)
        setOpen(currentItem(), true);

    // TODO: Notice when it's done.
    empath->createFolder(newFolderURL, this);
}

    void
EmpathFolderWidget::s_removeFolder()
{
    QString warning =
        i18n("This will remove the folder %1 and ALL subfolders !");
    QString folderPath = popupMenuOver->url().folderPath();
    
    int c =
        KMessageBox::warningYesNo(
            this,
            warning.arg(folderPath),
            i18n("Remove folder"),
            i18n("Remove"),
            i18n("Cancel"));

    // TODO: Notice when it's done.
    if (c == KMessageBox::Yes)
        empath->removeFolder(popupMenuOver->url(), this);
}

    void
EmpathFolderWidget::s_setUpAccounts()
{
}

    EmpathFolderListItem *
EmpathFolderWidget::find(const EmpathURL & url)
{
    QListIterator<EmpathFolderListItem> it(itemList_);
    
    for (; it.current(); ++it)
        if (it.current()->url() == url)
            return it.current();
    
    return 0;
}

    void
EmpathFolderWidget::s_openChanged()
{
    QStringList l;
    
    QListIterator<EmpathFolderListItem> it(itemList_);
    
    for (; it.current(); ++it) 
        l.append(it.current()->url().asString());
    
    KConfig * c(KGlobal::config());
    using namespace EmpathConfig;
    c->setGroup(GROUP_DISPLAY);
    
    c->writeEntry(UI_FOLDERS_OPEN, l);
}

    void 
EmpathFolderWidget::s_openCurrent()
{
    setOpen(currentItem(), true);
}

    void
EmpathFolderWidget::s_startDrag(const QList<QListViewItem> & items)
{
    // We don't want to drag anything right now !
    return;

    EmpathFolderListItem * i = (EmpathFolderListItem *)items.getFirst();
    
    // We don't want to drag Mailboxes.
    if (i->url().isMailbox())
        return;
             
    QStrList uriList;

    uriList.append(i->url().asString().utf8());
    
    QUriDrag * u = new QUriDrag(uriList, this);
    empathDebug("Drag folder: " + i->url().asString());
    CHECK_PTR(u);

    u->setPixmap(BarIcon("folder-normal"));

    u->drag();
}
    
    void
EmpathFolderWidget::contentsDragEnterEvent(QDragEnterEvent * e)
{
    if (!e)
        return;

    empathDebug("");
    
    if (!QUriDrag::canDecode(e)) {
        e->ignore(); 
        return;
    } else {
        // what's inside the packet?
        QUriDrag::decode(e, dragContents_);
        EmpathURL contentURL(QString(dragContents_.first()));
        if (!contentURL.isValid())
            e->ignore();
    }
}

    void
EmpathFolderWidget::contentsDragMoveEvent(QDragMoveEvent * e)
{
    if (!e)
        return;

    empathDebug("");

    QPoint vp = contentsToViewport(e->pos());
    
    QRect insideMargin(
        autoscrollMargin, autoscrollMargin,
        visibleWidth()  - autoscrollMargin * 2,
        visibleHeight() - autoscrollMargin * 2);
  
    QListViewItem * i = itemAt(vp);
    
    if (!i) {
        e->ignore();
        autoOpenTimer->stop();
        dropItem = 0;
        return;
    }
   
    if (!insideMargin.contains(vp)) {
        startAutoScroll();
        e->accept(QRect(0,0,0,0));
        autoOpenTimer->stop();
    } else { 
        e->accept(itemRect(i)); 
        
        if (i != dropItem) {
            autoOpenTimer->stop();
            dropItem = i;
            autoOpenTimer->start(autoOpenTime);
        }
        
        switch (e->action()) {
            
            case QDropEvent::Copy:
                break;
                
            case QDropEvent::Move:
                e->acceptAction();
                break;
                
            case QDropEvent::Link:
                e->acceptAction();
                break;
                
            default:
                break;
        }
    }
}

    void
EmpathFolderWidget::contentsDragLeaveEvent(QDragLeaveEvent *)
{
    empathDebug("");
    autoOpenTimer->stop();
    stopAutoScroll();
    setCurrentItem(linkItem());
    dropItem = 0;
}

    void
EmpathFolderWidget::contentsDropEvent(QDropEvent * e)
{
    if (!e)
        return;

    empathDebug("");
    autoOpenTimer->stop();
    stopAutoScroll();
    
    QListViewItem * item = itemAt(contentsToViewport(e->pos()));
    
    if (!item) {
        e->ignore();
        return;
    }
    
    QStrList l;
    
    l.setAutoDelete(false);
    
    QUriDrag::decode(e, l);

    QString s;
    
    switch (e->action()) {
        
        case QDropEvent::Copy:
            s = "Copy";
            break;
    
        case QDropEvent::Move:
            s = "Move";
            break;
    
        case QDropEvent::Link:
            s = "Link";
            break;
    
        default:
            // str = "Unknown";
            break;
    }
        
    EmpathFolderListItem * i = static_cast<EmpathFolderListItem *>(item);
    
    if (!i->url().isFolder()) {
        empathDebug("Funny, url of EmpathFolderListItem is not a folder");
        return;
    }
    
    // str += "\n\n";
    QStrListIterator it(l);

    while (it.current()) {

        empathDebug("Got: " + QString(it.current()) + " " + s );

        EmpathURL u(QString(it.current()));

        if (u.isMessage())
            empath->move(u, i->url());

        ++it;
    }
    
    e->accept();

    setCurrentItem(linkItem());
}

    void
EmpathFolderWidget::startAutoScroll()
{
}

    void
EmpathFolderWidget::stopAutoScroll()
{
}

// vim:ts=4:sw=4:tw=78
