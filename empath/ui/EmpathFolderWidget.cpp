/*
	Empath - Mailer for KDE
	
	Copyright (C) 1998 Rik Hemsley rik@kde.org
	
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

// KDE includes
#include <klocale.h>
#include <klineeditdlg.h>

// Local includes
#include "EmpathUIUtils.h"
#include "EmpathConfigMaildirDialog.h"
#include "EmpathConfigPOP3Dialog.h"
#include "EmpathConfigIMAP4Dialog.h"
#include "EmpathFolderWidget.h"
#include "EmpathMailboxList.h"
#include "EmpathFolderList.h"

#include "EmpathMailboxMaildir.h"
#include "EmpathMailboxPOP3.h"
#include "EmpathMailboxIMAP4.h"

EmpathFolderWidget::EmpathFolderWidget(
	QWidget * parent, const char * name, bool wait)
	:	QListView(parent, name),
		waitForShown_(wait)
{
	empathDebug("ctor");
	
	setAcceptDrops(true);
	
	addColumn(i18n("Folder name"));
	addColumn(i18n("Unread"));
	addColumn(i18n("Total"));
	
	setAllColumnsShowFocus(true);
	setRootIsDecorated(true);
	setSorting(-1); // Don't sort this.

	QObject::connect(this, SIGNAL(currentChanged(QListViewItem *)),
			this, SLOT(s_currentChanged(QListViewItem *)));
	
	QObject::connect(
		this,
		SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int)),
		this,
		SLOT(s_rightButtonPressed(QListViewItem *, const QPoint &, int)));
	
	QObject::connect(
		empath, SIGNAL(updateFolderLists()), this, SLOT(s_update()));
	
	////////
	
	otherPopup_.insertItem(i18n("Set up accounts"),
		this, SLOT(s_setUpAccounts()));
	
	////////
	
	folderPopup_.insertItem(i18n("New subfolder"),
		this, SLOT(s_newFolder()));
	
	folderPopup_.insertItem(i18n("Remove folder"),
		this, SLOT(s_removeFolder()));
	
	folderPopup_.insertSeparator();
	
	folderPopup_.insertItem(i18n("Properties"),
		this, SLOT(s_folderProperties()));
	
	/////////
	
	mailboxPopup_.insertItem(i18n("New folder"),
		this, SLOT(s_newFolder()));
	
	mailboxPopup_.insertItem(i18n("Check mail"),
		this, SLOT(s_mailboxCheck()));
	
	mailboxPopup_.insertSeparator();
	
	mailboxPopup_.insertItem(i18n("Properties"),
		this, SLOT(s_mailboxProperties()));

	/////////
	
	update();
}

	void
EmpathFolderWidget::update()
{
	empathDebug("update() called");
	setUpdatesEnabled(false);
	empathDebug("set updates to disabled");
	
	EmpathMailboxListIterator mit(empath->mailboxList());

	for (; mit.current(); ++mit) {
		
		empathDebug("Adding mailbox " + mit.current()->name());
		_addMailbox(*mit.current());
	}
	
	QListIterator<EmpathFolderListItem> it(itemList_);
	
	for (; it.current(); ++it)
		if (!it.current()->isTagged()) {
			itemList_.remove(it.current());
			QListView::removeItem((QListViewItem *)it.current());
		}

	setUpdatesEnabled(true);
	triggerUpdate();
}

	void
EmpathFolderWidget::_addMailbox(const EmpathMailbox & mailbox)
{
	empathDebug("============================================================");
	empathDebug("Add mailbox called for mailbox \"" +
		mailbox.name() + "\"");
	
	EmpathFolderListItem * newItem;
	EmpathFolderListItem * found = find(mailbox.url());

	if (found != 0) {
		
		found->tag(true);
		newItem = found;
		
	} else {
	
		newItem =
			new EmpathFolderListItem(this, mailbox.url());
		CHECK_PTR(newItem);
		newItem->tag(true);
	
		itemList_.append(newItem);
	}
	
	empathDebug("============================================================");
	empathDebug("Adding folders of mailbox \"" + mailbox.name() + "\"");
	EmpathFolderListIterator fit(mailbox.folderList());

	for (; fit.current(); ++fit) {

		if (fit.current()->parent() == 0)
			_addChildren(fit.current(), newItem);
	}
	empathDebug("============================================================");
}

	void
EmpathFolderWidget::_addChildren(
	EmpathFolder * item,
	EmpathFolderListItem * parent)
{
	empathDebug("_addChildren(" + item->url().folderPath() + ") called");
	
	// Add this item first.

	EmpathFolderListItem * newItem;
	
	EmpathFolderListItem * found = find(item->url());

	if (found != 0) {
		
		found->tag(true);
		newItem = found;

	} else {
	
		newItem =
			new EmpathFolderListItem(parent, item->url());
		CHECK_PTR(newItem);
		newItem->tag(true);
	
		itemList_.append(newItem);
	}

	EmpathMailbox * m = empath->mailbox(item->url());
	if (m == 0) return;
   	
	EmpathFolderListIterator it(m->folderList());

	for (; it.current(); ++it) {
		if (it.current()->parent() == item)
			_addChildren(it.current(), newItem);
	}
}

	void
EmpathFolderWidget::s_currentChanged(QListViewItem * item)
{
	EmpathFolderListItem * i = (EmpathFolderListItem *)item;

	if (!i->url().hasFolder()) { // Item is mailbox.
	
		empathDebug("mailbox " + i->url().mailboxName() + " selected");
		
	} else { // Item is folder.
	
		empathDebug("folder " + i->url().folderPath() + " selected");

		if (waitForShown_)
		{
			setEnabled(false);
			setCursor(waitCursor);
			emit(showFolder(i->url()));
		}
	}
}

	void
EmpathFolderWidget::s_showing()
{
	setCursor(arrowCursor);
	setEnabled(true);
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
	QListViewItem * item, const QPoint &, int)
{
	if (item == 0) {
	
		otherPopup_.exec(QCursor::pos());
		return;
	}
	setSelected(item, true);
	
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
EmpathFolderWidget::s_folderProperties()
{
	empathDebug("s_folderProperties() called");
	
	if (popupMenuOverType != Folder) {
		empathDebug("The popup menu wasn't over a folder !");
		return;
	}
	
	empathDebug("Nothing to do for folder properties as yet");
	return;
}

	void
EmpathFolderWidget::s_mailboxCheck()
{
	empathDebug("s_mailboxCheck() called");

	if (popupMenuOverType != Mailbox) {
		empathDebug("The popup menu wasn't over a mailbox !");
		return;
	}
	
	EmpathMailbox * m = empath->mailbox(popupMenuOver->url());
	
	if (m == 0) return;

	m->s_checkNewMail();
}

	void
EmpathFolderWidget::s_mailboxProperties()
{
	empathDebug("s_mailboxProperties() called");

	if (popupMenuOverType != Mailbox) {
		empathDebug("The popup menu wasn't over a mailbox !");
		return;
	}
	
	EmpathMailbox * m = empath->mailbox(popupMenuOver->url());

	if (m == 0) return;
	
	DialogRetval dlg_retval = Cancel;
	
	empathDebug("Mailbox name = " + m->name());

	switch (m->type()) {

		case Maildir:
			{
				EmpathConfigMaildirDialog configDialog(
						(EmpathMailboxMaildir *)m, this, "configDialog");
				dlg_retval = (DialogRetval)configDialog.exec();
			}
			break;

		case POP3:
			{
				EmpathConfigPOP3Dialog configDialog(
						(EmpathMailboxPOP3 *)m, true, this, "configDialog");
				dlg_retval = (DialogRetval)configDialog.exec();
			}
			break;

		case IMAP4:
			{
				EmpathConfigIMAP4Dialog configDialog(
						(EmpathMailboxIMAP4 *)m, this, "configDialog");
				dlg_retval = (DialogRetval)configDialog.exec();
			}
			break;

		default:
			break;
	}

}

	void
EmpathFolderWidget::s_update()
{
	update();
}

	void
EmpathFolderWidget::s_newFolder()
{
	empathDebug("s_newFolder() called");
	KLineEditDlg led(i18n("Folder name"), QString::null, this, false);
	led.exec();

	QString name = led.text();
	
	if (name.isEmpty()) return;
		
	EmpathMailbox * m = empath->mailbox(popupMenuOver->url());
	if (m == 0) return;
	
	EmpathURL newFolderURL(popupMenuOver->url().asString() + "/" + name + "/");

	if (!m->addFolder(newFolderURL)) return;
	
	empathDebug("popupMenuOver == " + popupMenuOver->url().asString());
	empathDebug("newFolderURL  == " + newFolderURL.asString());

	EmpathFolderListItem * item =
		new EmpathFolderListItem(popupMenuOver, newFolderURL);
	
	itemList_.append(item);
}

	void
EmpathFolderWidget::s_removeFolder()
{
	empathDebug("s_removeFolder \"" +
		popupMenuOver->url().asString() + "\" called");

	EmpathMailbox * m = empath->mailbox(popupMenuOver->url());
	if (m == 0) return;
	if (!m->removeFolder(popupMenuOver->url())) {
		empathDebug("Couldn't remove folder \"" +
			popupMenuOver->url().asString() + "\"");
		return;
	}
	
	delete popupMenuOver;
}

	void
EmpathFolderWidget::s_setUpAccounts()
{
}

	void
EmpathFolderWidget::dragMoveEvent(QDragMoveEvent * e)
{
	if (itemAt(e->pos()) != 0) {
		itemAt(e->pos())->setSelected(true);
		if (QTextDrag::canDecode(e))
			e->accept();
	}
}

	void
EmpathFolderWidget::dropEvent(QDropEvent * e)
{
	if (itemAt(e->pos()) == 0) {
		empathDebug("Item was dropped on nothing");
		return;
	}
	
    QString str;
    
	if ( QTextDrag::decode( e, str ) ) {
		itemAt(e->pos())->setText(0, str);
		return;
	}
}

	EmpathFolderListItem *
EmpathFolderWidget::find(const EmpathURL & url)
{
	QListIterator<EmpathFolderListItem> it(itemList_);
	
	for (; it.current(); ++it)
		if (it.current()->url() == url) {
			empathDebug("Found a folder list item with url \""
				+ url.asString() + "\" already in list");
			return it.current();
		}
	
	return 0;
}

