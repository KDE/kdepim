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
#include <qheader.h>
#include <qstring.h>
#include <qfile.h>

// KDE includes
#include <klocale.h>
#include <kmsgbox.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <kapp.h>

// Local includes
#include "HeapPtr.h"
#include "EmpathMessageList.h"
#include "EmpathMessageListWidget.h"
#include "EmpathMessageSourceView.h"
#include "EmpathIndexRecord.h"
#include "EmpathMessageViewWindow.h"
#include "EmpathMenuMaker.h"
#include "EmpathConfig.h"
#include "Empath.h"
#include "EmpathUIUtils.h"

EmpathMessageListWidget::EmpathMessageListWidget(
		QWidget * parent, const char * name)
	:	QListView(parent, name)
{
	parent_ = (EmpathMainWindow *)parent;
	wantScreenUpdates_ = false;
	
	lastHeaderClicked_ = -1;
	sortType_ = 2;

	setAllColumnsShowFocus(true);
	setMultiSelection(false);
	setRootIsDecorated(true);
	setSorting(2);
	addColumn("Subject");
	addColumn("Sender");
	addColumn("Date");
	addColumn("Status");
	addColumn("Size");
	
	px_read_marked		= empathIcon("tree-read-marked.xpm");
	px_unread_marked	= empathIcon("tree-unread-marked.xpm");
	px_read_unmarked	= empathIcon("tree-read.xpm");
	px_unread_unmarked	= empathIcon("tree-unread.xpm");

	empathDebug("Restoring column sizes and positions");
	
	KConfig * c = kapp->getConfig();
	c->setGroup(GROUP_GENERAL);
	
	for (int i = 0 ; i < 5 ; i++) {
		header()->setCellSize(i,
				c->readUnsignedNumEntry(
					KEY_MESSAGE_LIST_SIZE_COL + QString().setNum(i), 10));
	}

	messageMenu_ = new QPopupMenu();
	CHECK_PTR(messageMenu_);

	setupMessageMenu(this, 0, messageMenu_);
	
	// Connect double click. Rather not have this at all.
	QObject::connect(this, SIGNAL(currentChanged(QListViewItem *)),
			this, SLOT(s_currentChanged(QListViewItem *)));

	// Connect double click. Rather not have this at all.
	QObject::connect(this, SIGNAL(doubleClicked(QListViewItem *)),
			this, SLOT(s_doubleClicked(QListViewItem *)));
	
	// Connect return press to double click, a la mutt
	QObject::connect(this, SIGNAL(returnPressed(QListViewItem *)),
			this, SLOT(s_doubleClicked(QListViewItem *)));
	
	// Connect right button up so we can produce the popup context menu.
	QObject::connect(this, SIGNAL(rightButtonClicked(QListViewItem *, const QPoint &, int)),
			this, SLOT(s_rightButtonClicked(QListViewItem *, const QPoint &, int)));
	
	// Connect the header's section clicked signal so we can sort properly
	QObject::connect(header(), SIGNAL(sectionClicked(int)),
		this, SLOT(s_headerClicked(int)));
}

EmpathMessageListWidget::~EmpathMessageListWidget()
{
	empathDebug("dtor");
	empathDebug("Saving column sizes and positions");
	empathDebug("XXX: Sort this so that positions can be restored");
	
	KConfig * c = kapp->getConfig();
	c->setGroup(GROUP_GENERAL);
	
	for (int i = 0 ; i < 5 ; i++) {

		c->writeEntry(
			KEY_MESSAGE_LIST_SIZE_COL + QString().setNum(i),
			header()->cellSize(i));
		
		c->writeEntry(
			KEY_MESSAGE_LIST_POS_COL + QString().setNum(i),
			header()->cellPos(i));
	}
}

	EmpathMessageListItem *
EmpathMessageListWidget::findRecursive(
		EmpathMessageListItem * initialItem, const RMessageID & msgId)
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

	ASSERT(initialItem->msgDesc() != 0);
	if (initialItem->msgDesc()->messageID() == msgId) return initialItem;
	return 0;
}

	EmpathMessageListItem *
EmpathMessageListWidget::find(const RMessageID & msgId)
{
	empathDebug("find (" + msgId.asString() + ") called");
	if (!firstChild()) return 0;
	return findRecursive((EmpathMessageListItem *)firstChild(), msgId);
}

	void
EmpathMessageListWidget::addItem(EmpathIndexRecord * item)
{
	// Put the item into the master list.
	empathDebug("addItem called");
	empathDebug("InSorting into list");
	item->sender().assemble();
	item->date().assemble();
	item->messageID().assemble();
	item->parentID().assemble();
	
	QListIterator<EmpathMessageListItem> it(threadItemList_);
	bool needToClearOut = false;
	
	for (; it.current(); ++it) {
		if (it.current()->parentID() == item->messageID()) {
			// This item is a parent of one in list
			empathDebug("Should clear out!");
			needToClearOut = true;
			break;
		}
	}
	
	EmpathMessageListItem * newItem;
	
	// TODO: If the item is a parent of an item in our list, remove the tree
	// from its child down, add the parent, then re-add the child and its
	// descendants as children of the new parent.
	// XXX: Actually I think we don't need to do this. We should really, but it's
	// probably not necessary. I will sort the entries in the description list by
	// date sent. It's theoretically impossible to send a reply to a message you
	// haven't received yet. The only reason this will fail is when someone's
	// clock is set wrongly. It remains to be seen how far out of sync the world's
	// clocks are and how quickly people can reply to messages.
	
	if (!item->parentID().asString().isEmpty() &&
		!(item->parentID().asString() == "<@>") &&
		!item->messageID().asString().isEmpty()) {
		// Child of other item
		empathDebug("Message has parentID, looking for parent");
		empathDebug("MESSAGE ID: \"" + item->messageID().asString() + "\"");
		empathDebug("PARENT  ID: \"" + item->parentID().asString() + "\"");
		
		EmpathMessageListItem * parentItem = 0;
		
		parentItem = this->find(item->parentID());

		if (parentItem == 0) {
			
			empathDebug("No parent for this item");
			newItem = new EmpathMessageListItem(this, item);
			CHECK_PTR(newItem);
			empathDebug("Created OK");
			
		} else {
			
			empathDebug("There's parent for this item");
			newItem = new EmpathMessageListItem(parentItem, item);
			CHECK_PTR(newItem);
			empathDebug("Created OK");
		}
		
	} else {
		// Root item
		empathDebug("Message is root item");
		newItem = new EmpathMessageListItem(this, item);
		CHECK_PTR(newItem);
		empathDebug("Created OK");
	}

	setStatus(newItem, item->status());
}
/*
	void
EmpathMessageListWidget::append(EmpathMessageListItem * item)
{
	QListIterator<EmpathMessageListItem> parentIt(threadItemList_);

	EmpathMessageListItem * parentItem = 0;
			
	// See if our item has a parent in the list
	for (; parentIt.current(); ++parentIt) {
		
		empathDebug("Looking at item with id " + parentIt.current()->messageID());
		
		if (parentIt.current()->messageID() == item->msgDesc()->parentID()) {
			parentItem = (EmpathMessageListItem *)parentIt.current();
			break;
		}
	}

	EmpathMessageListItem * i;
	
	if (parentItem != 0) {
		empathDebug("Adding item with parent as list item");
		i = new EmpathMessageListItem(parentItem, item->msgDesc());
		i->setPixmap(0, px_unread_unmarked);
	} else {
		empathDebug("Adding item with parent as list itself");
		i = new EmpathMessageListItem( this, item->msgDesc());
		i->setPixmap(0, px_unread_unmarked);
	}
}

// We have the top item, #1
//
// 1
// `--2
// |  `--3
// |  |
// |  `--4
// |
// `--5
//
// So ... if there's a first child, call this function with that as initialItem
//
// if there's a next sibling, call this function with that sibling
// 
// add this item, return
//
// This will do:
//
// start at 1
// has first child - 2
// 		start at 2
// 		has first child - 3
// 			start at 3
// 			has no first child
// 			has next sibling - 4
// 				start at 4
// 				no first child, no next sibling
// 				add 4, return
// 			add 3, return
// 		has sibling - 5
// 			start at 5
// 			no child, no sibling
// 			add 5, return
// 		return
// 	no sibling
// 	add 1, return
// 

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
	EmpathIndexRecord *
EmpathMessageListWidget::firstSelectedMessage() const
{
	if (currentItem() == 0) return 0;
	return ((EmpathMessageListItem *)currentItem())->msgDesc();
}

// Message menu slots

	void
EmpathMessageListWidget::s_messageNew()
{
	empathDebug("s_messageNew called");

}

	void
EmpathMessageListWidget::s_messageReply()
{
	empathDebug("s_messageReply called");
	
	RMessage * message(
		currentFolder_->message(firstSelectedMessage()->messageID()));
	
	empath->reply(message);
	empathDebug("Blah !");

}

	void
EmpathMessageListWidget::s_messageReplyAll()
{
	empathDebug("s_messageReplyAll called");

}

	void
EmpathMessageListWidget::s_messageForward()
{
	empathDebug("s_messageForward called");

}

	void
EmpathMessageListWidget::s_messageBounce()
{
	empathDebug("s_messageBounce called");

}

	void
EmpathMessageListWidget::s_messageDelete()
{
	empathDebug("s_messageDelete called");

}

	void
EmpathMessageListWidget::s_messageSaveAs()
{
	empathDebug("s_messageSaveAs called");
	empathDebug("!!!");
	QString saveFilePath = KFileDialog::getSaveFileName("", "", this, i18n("Empath: Save Message"));
	empathDebug(saveFilePath);
	
	if (saveFilePath == "") {
		empathDebug("No filename given");
		return;
	}
	
	QFile f(saveFilePath);
	if (!f.open(IO_WriteOnly)) {
		// Warn user file cannot be opened.
		empathDebug("Couldn't open file for writing");
		KMsgBox(this, "Empath", i18n("Sorry I can't write to that file. Please try another filename."), KMsgBox::EXCLAMATION, i18n("OK"));
		return;
	}
	empathDebug("Opened " + saveFilePath + " OK");
	
	RMessageID id = firstSelectedMessage()->messageID();
	
	empathDebug("Using message " + id.asString());

	HeapPtr<RMessage> message(currentFolder_->message(id));
	
	QString s =
		message->asString();
	
	unsigned int blockSize = 1024; // 1k blocks
	
	unsigned int fileLength = s.length();

	for (unsigned int i = 0 ; i < s.length() ; i += blockSize) {
		
		QString outStr;
		
		if ((fileLength - i) < blockSize)
			outStr = QString(s.right(fileLength - i));
		else
			outStr = QString(s.mid(i, blockSize));
		
		if (f.writeBlock(outStr, outStr.length()) != outStr.length()) {
			// Warn user file not written.
			KMsgBox(this, "Empath", i18n("Sorry I couldn't write the file successfully. Please try another file."), KMsgBox::EXCLAMATION, i18n("OK"));
			return;
		}
		qApp->processEvents();
	}

	f.close();
	
	KMsgBox(this, "Empath", i18n("Message saved to") + QString(" ") + saveFilePath + QString(" ") + i18n("OK"), KMsgBox::INFORMATION, i18n("OK"));
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
	
	RMessageID id = firstSelectedMessage()->messageID();
	
	empathDebug("Using message " + id.asString());
	
	RMessage * message(currentFolder_->message(id));
	
	EmpathMessageViewWindow * messageViewWindow =
		new EmpathMessageViewWindow(message, i18n("Empath: Message view"));

	empathDebug("finished s_messageView()");
}

	void
EmpathMessageListWidget::s_messageViewSource()
{
	empathDebug("s_messageViewSource called");
	
	RMessageID id = firstSelectedMessage()->messageID();
	
	empathDebug("Using message " + id.asString());
	
	RMessage * message(currentFolder_->message(id));
	
	QString s =
		message->asString();
	
	EmpathMessageSourceView * sourceView = new EmpathMessageSourceView(s, 0);
	CHECK_PTR(sourceView);

	sourceView->show();
	
	delete message;
}

	void
EmpathMessageListWidget::s_rightButtonClicked(
		QListViewItem * item, const QPoint & pos, int column)
{
	if (item == 0) return;
	setSelected(item, true);
	messageMenu_->exec(pos);
}

	void
EmpathMessageListWidget::s_doubleClicked(QListViewItem *)
{
	empathDebug("s_messageDoubleClicked called");
	s_messageView();
}

	void
EmpathMessageListWidget::s_currentChanged(QListViewItem *)
{
	empathDebug("Current message changed - updating message widget");
	
	RMessageID id = firstSelectedMessage()->messageID();
	
	empathDebug("Using message " + id.asString());
	
	RMessage * message = currentFolder_->message(id);
	
	if (message == 0) {
		empathDebug("Couldn't get data !");
		return;
	}
	
	empathDebug("Message data:\n" + message->asString());
	
//	empathDebug("emitting(currentChanged(" + QString().setNum(message->id()) + ")");
	
	if (wantScreenUpdates_) {
		emit(changeView(message));
	}
}

	void
EmpathMessageListWidget::setSignalUpdates(bool yn)
{
	empathDebug("Setting signal updates to " + QString(yn ? "true" : "false"));
	wantScreenUpdates_ = yn;
}

	void
EmpathMessageListWidget::setStatus(
		EmpathMessageListItem * item, MessageStatus status)
{
	if (status && Read)
		if (status && Marked)
			item->setPixmap(0, px_read_marked);
		else
			item->setPixmap(0, px_read_unmarked);
	else
		if (status && Marked)
			item->setPixmap(0, px_unread_marked);
		else
			item->setPixmap(0, px_unread_unmarked);

	return;
}

	EmpathFolder *
EmpathMessageListWidget::currentFolder()
{
	return currentFolder_;
}

	void
EmpathMessageListWidget::s_showFolder(EmpathFolder * folder)
{
	empathDebug("s_showFolder(" + folder->name() + ") called");
	
	if (currentFolder_ == folder) {
		empathDebug("Same as we're showing");
		return;
	}
	currentFolder_ = folder;

	clear();
	
	currentFolder_->messageList().sync();

	empathDebug("Adding " +
		QString().setNum(currentFolder_->messageList().count()) +
			" messages to visual list");
	
	QDictIterator<EmpathIndexRecord> it(currentFolder_->messageList());

	// Start by putting everything into our list. This takes care of sorting so
	// hopefully threading will be simpler.
	for (; it.current(); ++it)
		masterList_.inSort(it.current());
	
	QListIterator<EmpathIndexRecord> mit(masterList_);
	
	for (; mit.current(); ++mit)
		addItem(mit.current());
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
	
	lastHeaderClicked_ = i;
}

