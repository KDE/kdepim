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

// KDE includes
#include <klocale.h>

// Local includes
#include "EmpathUIUtils.h"
#include "EmpathConfigMaildirDialog.h"
#include "EmpathConfigMboxDialog.h"
#include "EmpathConfigMMDFDialog.h"
#include "EmpathConfigPOP3Dialog.h"
#include "EmpathConfigIMAP4Dialog.h"
#include "EmpathFolderWidget.h"
#include "EmpathMailboxList.h"
#include "EmpathFolderList.h"

#include "EmpathMailboxMaildir.h"
#include "EmpathMailboxMMDF.h"
#include "EmpathMailboxMbox.h"
#include "EmpathMailboxPOP3.h"
#include "EmpathMailboxIMAP4.h"

EmpathFolderWidget::EmpathFolderWidget(QWidget * parent, const char * name)
	:	QListView(parent, name)
{
	empathDebug("ctor");
	
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
	
	QObject::connect(empath, SIGNAL(updateFolderLists()), this, SLOT(s_update()));
	
	folderPopup_.insertItem(i18n("Properties"), this, SLOT(s_folderProperties()));
	mailboxPopup_.insertItem(i18n("Check mail"), this, SLOT(s_mailboxCheck()));
	mailboxPopup_.insertItem(i18n("Properties"), this, SLOT(s_mailboxProperties()));

	update();
}

	void
EmpathFolderWidget::update()
{
	empathDebug("update() called");
	setUpdatesEnabled(false);
	empathDebug("set updates to disabled");
	clear();
	empathDebug("cleared");
	
	QListIterator<EmpathMailbox> mit(empath->mailboxList());

	for (; mit.current(); ++mit) {
		
		empathDebug("Adding mailbox " + mit.current()->name());
		_addMailbox(*mit.current());
	}

	setUpdatesEnabled(true);
	triggerUpdate();
}

	void
EmpathFolderWidget::_addMailbox(const EmpathMailbox & mailbox)
{
	empathDebug("Add mailbox called for mailbox \"" +
		mailbox.name() + "\"");
	
	EmpathFolderListItem * newItem =
		new EmpathFolderListItem(this, mailbox);
	
	CHECK_PTR(newItem);
	
	itemList_.append(newItem);
	
	EmpathFolderListIterator fit(mailbox.folderList());

	for (; fit.current(); ++fit) {

		if (fit.current()->parent() == 0)
			_addChildren(*fit.current());
	}
	
}

	void
EmpathFolderWidget::_addChildren(const EmpathFolder & item)
{
	empathDebug("_addChildren(" + item.url().folderPath() + ") called");
	
	// Add this item first.

	EmpathFolderListItem * newItem = 0;

	EmpathFolderListItem * parentFolderListFolder = _parentFolderListFolder(item);
	
	if (parentFolderListFolder == 0) {
		
		EmpathFolderListItemIterator mit(itemList_);

		for (; mit.current(); ++mit) {

			if (!mit.current()->isFolderItem() &&
				mit.current()->mailbox().name() ==
				item.name()) {

				newItem = new EmpathFolderListItem(mit.current(), item);
				CHECK_PTR(newItem);
			}
		}

		if (newItem == 0) {
			empathDebug("Uh ? Couldn't find folder");
			return;
		}
		
	} else {
		
		empathDebug("Making folder with a folder as its parent");
		newItem =
			new EmpathFolderListItem(parentFolderListFolder, item);
		CHECK_PTR(newItem);
	}

	empathDebug("Appending new item to list");
	itemList_.append(newItem);
	
	empathDebug("Checking item's pixmap");
	if (item.pixmap().isNull())
		newItem->setPixmap(0, empathIcon("mini/folder.xpm"));
	else
		newItem->setPixmap(0, item.pixmap());
	
	empathDebug("Adding children of the new item now");

	EmpathMailbox * m = empath->mailbox(item.url());
	if (m == 0) return;
   	
	EmpathFolderListIterator it(m->folderList());

	for (; it.current(); ++it) {
		if (item.url() == it.current()->url())
			_addChildren(*it.current());
	}
}

	EmpathFolderListItem *
EmpathFolderWidget::_parentFolderListFolder(const EmpathFolder & folder)
{
	empathDebug("_parentFolderListFolder " + folder.url().folderPath() + " called");
	EmpathFolderListItemIterator it(itemList_);

	for (; it.current(); ++it) {
		empathDebug("ARseVogle");
		if (it.current()->isFolderItem() &&
			(it.current()->folder() == *folder.parent())) {
			empathDebug("Parent is: " + it.current()->folder().url().folderPath());
			return it.current();
		}
	}

	empathDebug("Returning 0");
	return 0;
}

	void
EmpathFolderWidget::s_currentChanged(QListViewItem * item)
{
	EmpathFolderListItem * i = (EmpathFolderListItem *)item;

	if (!i->isFolderItem()) { // Item is mailbox.
	
		empathDebug("mailbox " + i->mailbox().name() + " selected");
		
	} else { // Item is folder.
	
		empathDebug("folder " + i->folder().url().folderPath() + " selected");

		emit(showFolder(&(i->folder())));
	}
}

	EmpathFolder *
EmpathFolderWidget::selectedFolder() const
{
	// Is it a mailbox ?
	if (!((EmpathFolderListItem *)currentItem())->isFolderItem()) return 0;
	
	return &(((EmpathFolderListItem *)currentItem())->folder());
}

	void
EmpathFolderWidget::s_rightButtonPressed(QListViewItem * item, const QPoint &, int)
{
	EmpathFolderListItem * i = (EmpathFolderListItem *)item;
	
	if (i->isFolderItem()) {
		
		popupMenuIsOverType = Folder;
		popupMenuIsOverFolder = &i->folder();
		folderPopup_.exec(QCursor::pos());
		
	} else {
		
		popupMenuIsOverType = Mailbox;
		popupMenuIsOverMailbox = &i->mailbox();
		mailboxPopup_.exec(QCursor::pos());
	}
}

	void
EmpathFolderWidget::s_folderProperties()
{
	empathDebug("s_folderProperties() called");
	
	if (!(popupMenuIsOverType == Folder)) {
		empathDebug("The popup menu wasn't over a folder !");
		return;
	}
	
	EmpathFolder * f = popupMenuIsOverFolder;

}

	void
EmpathFolderWidget::s_mailboxCheck()
{
	empathDebug("s_mailboxCheck() called");

	if (!(popupMenuIsOverType == Mailbox)) {
		empathDebug("The popup menu wasn't over a mailbox !");
		return;
	}
	
	EmpathMailbox * m = popupMenuIsOverMailbox;

	m->checkNewMail();
}

	void
EmpathFolderWidget::s_mailboxProperties()
{
	empathDebug("s_mailboxProperties() called");

	if (!(popupMenuIsOverType == Mailbox)) {
		empathDebug("The popup menu wasn't over a mailbox !");
		return;
	}
	
	EmpathMailbox * m = popupMenuIsOverMailbox;

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

		case MMDF:
			{
				EmpathConfigMMDFDialog configDialog(
						(EmpathMailboxMMDF *)m, this, "configDialog");
				dlg_retval = (DialogRetval)configDialog.exec();
			}
			break;
		
		case Mbox:
			{
				EmpathConfigMboxDialog configDialog(
						(EmpathMailboxMbox *)m, this, "configDialog");
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

