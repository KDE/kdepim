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
#include <qdatetime.h>

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

	empathDebug("Restoring column sizes");
	
	KConfig * c = kapp->getConfig();
	c->setGroup(EmpathConfig::GROUP_GENERAL);
	
	for (int i = 0 ; i < 5 ; i++) {
		header()->setCellSize(i,
				c->readUnsignedNumEntry(
					EmpathConfig::KEY_MESSAGE_LIST_SIZE_COLUMN +
					QString().setNum(i), 10));
		setColumnWidthMode(i, QListView::Manual);
	}

	_setupMessageMenu();
	
	QObject::connect(this, SIGNAL(currentChanged(QListViewItem *)),
			this, SLOT(s_currentChanged(QListViewItem *)));

	// Connect return press to view.
	QObject::connect(this, SIGNAL(returnPressed(QListViewItem *)),
			this, SLOT(s_messageView()));
	
	// Connect right button up so we can produce the popup context menu.
	QObject::connect(
		this, SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int)),
		this, SLOT(s_rightButtonPressed(QListViewItem *, const QPoint &, int)));
	
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
	c->setGroup(EmpathConfig::GROUP_GENERAL);
	
	for (int i = 0 ; i < 5 ; i++) {

		c->writeEntry(
			EmpathConfig::KEY_MESSAGE_LIST_SIZE_COLUMN + QString().setNum(i),
			header()->cellSize(i));
		
		c->writeEntry(
			EmpathConfig::KEY_MESSAGE_LIST_POS_COLUMN + QString().setNum(i),
			header()->cellPos(i));
	}
}

	EmpathMessageListItem *
EmpathMessageListWidget::findRecursive(
		EmpathMessageListItem * initialItem, RMessageID & msgId)
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
EmpathMessageListWidget::find(RMessageID & msgId)
{
	empathDebug("find (" + msgId.asString() + ") called");
	if (!firstChild()) return 0;
	return findRecursive((EmpathMessageListItem *)firstChild(), msgId);
}

	void
EmpathMessageListWidget::addItem(EmpathIndexRecord & item)
{
	// Put the item into the master list.
	empathDebug("addItem called");
#if 0	
	QListIterator<EmpathMessageListItem> it(threadItemList_);
	bool needToClearOut = false;
	
	for (; it.current(); ++it) {
		if (it.current()->parentID() == item.messageID()) {
			// This item is a parent of one in list
			empathDebug("Should clear out!");
			needToClearOut = true;
			break;
		}
	}
#endif
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
	
	if (! item.parentID().localPart().isEmpty() &&
		! item.messageID().localPart().isEmpty()) {
		// Child of other item
		empathDebug("Message has parentID, looking for parent");
//		empathDebug("MESSAGE ID: \"" + messageID.asString() + "\"");
//		empathDebug("PARENTID: \"" + parentID.asString() + "\"");
		
		EmpathMessageListItem * parentItem = 0;
		
		parentItem = this->find(item.parentID());

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
		QTime begin(QTime::currentTime());
		newItem = new EmpathMessageListItem(this, item);
		empathDebug("Creation of message list item took " +
			QString().setNum(begin.msecsTo(QTime::currentTime())) + " ms");
		CHECK_PTR(newItem);
		empathDebug("Created OK");
	}

	setStatus(newItem, item.status());
}
/*
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
	EmpathURL
EmpathMessageListWidget::firstSelectedMessage()
{
	EmpathURL u("orphaned");
	if (currentItem() == 0) return u;
	EmpathMessageListItem * item = (EmpathMessageListItem *)currentItem();
	u = EmpathURL(url_.mailboxName(), url_.folderPath(), item->id());
	return u;
}

// Message menu slots


	void
EmpathMessageListWidget::s_messageMark()
{
	if (currentItem() == 0) return;
	EmpathMessageListItem * item = (EmpathMessageListItem *)currentItem();
	EmpathURL u = EmpathURL(url_.mailboxName(), url_.folderPath(), item->id());
	empath->s_mark(u, RMM::MessageStatus(item->status() ^ RMM::Marked));
}

	void
EmpathMessageListWidget::s_messageMarkRead()
{
	if (currentItem() == 0) return;
	EmpathMessageListItem * item = (EmpathMessageListItem *)currentItem();
	EmpathURL u = EmpathURL(url_.mailboxName(), url_.folderPath(), item->id());
	empath->s_mark(u, RMM::MessageStatus(item->status() ^ RMM::Read));
}

	void
EmpathMessageListWidget::s_messageMarkReplied()
{
	if (currentItem() == 0) return;
	EmpathMessageListItem * item = (EmpathMessageListItem *)currentItem();
	EmpathURL u = EmpathURL(url_.mailboxName(), url_.folderPath(), item->id());
	empath->s_mark(u, RMM::MessageStatus(item->status() ^ RMM::Replied));
}

	void
EmpathMessageListWidget::s_messageReply()
{
	empathDebug("s_messageReply called");
	empath->s_reply(firstSelectedMessage());
}

	void
EmpathMessageListWidget::s_messageReplyAll()
{
	empathDebug("s_messageReplyAll called");
	empath->s_reply(firstSelectedMessage());
}

	void
EmpathMessageListWidget::s_messageForward()
{
	empathDebug("s_messageForward called");
	empath->s_forward(firstSelectedMessage());
}

	void
EmpathMessageListWidget::s_messageBounce()
{
	empathDebug("s_messageBounce called");
	empath->s_bounce(firstSelectedMessage());
}

	void
EmpathMessageListWidget::s_messageDelete()
{
	empathDebug("s_messageDelete called");
	empath->s_remove(firstSelectedMessage());
}

	void
EmpathMessageListWidget::s_messageSaveAs()
{
	empathDebug("s_messageSaveAs called");
	QString saveFilePath =
		KFileDialog::getSaveFileName(
			QString::null, QString::null, this, i18n("Empath: Save Message"));
	empathDebug(saveFilePath);
	
	if (saveFilePath.isEmpty()) {
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
	
	EmpathFolder * folder(empath->folder(url_));
	if (folder == 0) return;
	
	RMessage * message(folder->message(firstSelectedMessage()));
	if (message == 0) return;
	
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
			delete message; message = 0;
			return;
		}
		qApp->processEvents();
	}

	f.close();
	
	KMsgBox(this, "Empath", i18n("Message saved to") + QString(" ") + saveFilePath + QString(" ") + i18n("OK"), KMsgBox::INFORMATION, i18n("OK"));
	delete message; message = 0;
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
	
	EmpathMessageViewWindow * messageViewWindow =
		new EmpathMessageViewWindow(
			firstSelectedMessage(),
			i18n("Empath: Message View"));
	
	CHECK_PTR(messageViewWindow);
	
	messageViewWindow->show();
}

	void
EmpathMessageListWidget::s_messageViewSource()
{
	empathDebug("s_messageViewSource called");
	
	EmpathMessageSourceView * sourceView =
		new EmpathMessageSourceView(firstSelectedMessage(), 0);

	CHECK_PTR(sourceView);

	sourceView->show();
}

	void
EmpathMessageListWidget::s_rightButtonPressed(
		QListViewItem * item, const QPoint & pos, int column)
{
	if (item == 0) return;
	wantScreenUpdates_ = false;
	setSelected(item, true);
	messageMenu_.exec(pos);
	wantScreenUpdates_ = true;
	emit(currentChanged(item));
}
/*
	void
EmpathMessageListWidget::s_rightButtonClicked(
		QListViewItem * item, const QPoint & pos, int column)
{
	if (item == 0) return;
	setSelected(item, true);
	messageMenu_.exec(pos);
}
*/
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
	
	// Make sure we highlight the current item.
//	kapp->processEvents();
	
	if (wantScreenUpdates_)
		emit(changeView(firstSelectedMessage()));
}

	void
EmpathMessageListWidget::setSignalUpdates(bool yn)
{
	empathDebug("Setting signal updates to " + QString(yn ? "true" : "false"));
	wantScreenUpdates_ = yn;
}

	void
EmpathMessageListWidget::setStatus(
		EmpathMessageListItem * item, RMM::MessageStatus status)
{
	if (status & RMM::Read)
		if (status & RMM::Marked)
			item->setPixmap(0, px_read_marked);
		else
			item->setPixmap(0, px_read_unmarked);
	else
		if (status & RMM::Marked)
			item->setPixmap(0, px_unread_marked);
		else
			item->setPixmap(0, px_unread_unmarked);

	return;
}

	void
EmpathMessageListWidget::s_showFolder(const EmpathURL & url)
{
	empathDebug("s_showFolder(" + url.asString() + ") called");
	
	if (url_ == url) {
		empathDebug("Same as we're showing");
		return;
	}
	
	url_ = url;
	
	EmpathFolder * f = empath->folder(url_);
	if (f == 0) return;
	
	clear();
	masterList_.clear();
	
	f->messageList().sync();

	empathDebug("Adding " +
		QString().setNum(f->messageCount()) +
			" messages to visual list");
	
	EmpathIndexIterator it(f->messageList());

	// Start by putting everything into our list. This takes care of sorting so
	// hopefully threading will be simpler.
	for (; it.current(); ++it)
		masterList_.inSort(it.current());
	
	QListIterator<EmpathIndexRecord> mit(masterList_);
	
	// What we doing here ?
	// Well, we keep the time we started.
	// When the time since 'begin' is too long (100ms) we do a
	// kapp->processEvents(). This prevents the UI from stalling.
	// We could then have done an update, but this takes a long time, so we
	// don't want to do it so often. Therefore we do it every 10 times that
	// we've had to do a processEvents().
	
	setUpdatesEnabled(false);
	
	QTime begin(QTime::currentTime());
	QTime begin2(begin);
	QTime now;
	
	for (; mit.current(); ++mit) {
		
		addItem(*mit.current());
		
		now = QTime::currentTime();
		if (begin.msecsTo(now) > 100) {
			kapp->processEvents();
			begin = now;
		}
	
//		if (begin2.secsTo(now) > 10) {
//			setUpdatesEnabled(true);
//			triggerUpdate();
//			setUpdatesEnabled(false);
//			begin2 = now;
//		}
	}
	
	setUpdatesEnabled(true);
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

	void
EmpathMessageListWidget::_setupMessageMenu()
{
	messageMenu_.insertItem(empathIcon("mini-view.xpm"), i18n("&View"),
		this, SLOT(s_messageView()));
	
	messageMenu_.insertItem(empathIcon("mini-view.xpm"), i18n("View &Source"),
		this, SLOT(s_messageViewSource()));

	messageMenuItemMark =
		messageMenu_.insertItem(
			empathIcon("mini-mark.xpm"), i18n("&Tag"),
			this, SLOT(s_messageMark()));
	
	messageMenuItemMarkRead =
		messageMenu_.insertItem(
			empathIcon("mini-mark.xpm"), i18n("&Mark as Read"),
			this, SLOT(s_messageMarkRead()));
	
	messageMenuItemMarkReplied =
		messageMenu_.insertItem(
			empathIcon("mini-mark.xpm"), i18n("&Mark as Replied"),
			this, SLOT(s_messageMarkReplied()));
		
	messageMenu_.insertSeparator();

	messageMenu_.insertItem(empathIcon("mini-reply.xpm"), i18n("&Reply"),
		this, SLOT(s_messageReply()));

	messageMenu_.insertItem(empathIcon("mini-reply.xpm"),i18n("Reply to A&ll"),
		this, SLOT(s_messageReplyAll()));

	messageMenu_.insertItem(empathIcon("mini-forward.xpm"), i18n("&Forward"),
		this, SLOT(s_messageForward()));

	messageMenu_.insertItem(empathIcon("mini-bounce.xpm"), i18n("&Bounce"),
		this, SLOT(s_messageBounce()));

	messageMenu_.insertItem(empathIcon("mini-delete.xpm"), i18n("&Delete"),
		this, SLOT(s_messageDelete()));

	messageMenu_.insertItem(empathIcon("mini-save.xpm"), i18n("Save &As"),
		this, SLOT(s_messageSaveAs()));
	
	messageMenu_.insertItem(empathIcon("empath-print.xpm"), i18n("&Print..."),
		this, SLOT(s_messagePrint()));
}

/*
	void
EmpathMessageListWidget::mousePressEvent(QMouseEvent * e)
{
	// Algorithm adapted from one written by Geri H. <ge_ha@yahoo.com>
	
	QListViewItem * lastSel = currentItem();
	
	newSel = itemAt( e->pos() );

	if (e->button() != LeftButton)
		return;
	
	if (lastSel == 0 || !isSelected(lastSel)) {
		
		// new selection
		setMultiSelection(false);
		setSelected (newSel, true);
		return;
	
	}
	
	// if new postion select it
	
	if (isSelected(newSel))
		return;
	
		
	if (e->state() == ControlButton) {
		
		setMultiSelection(TRUE);
		setSelected ( newSel, TRUE);
	
	} else {
		
		if (e->state() == ShiftButton) {
			
			setMultiSelection(true);
			
			if ((itemPos(lastSel)-itemPos(newSel)) > 0) {
				
				QListViewItem *item = lastSel->itemAbove();
				
				do {
					setSelected(item, true);
					
					if (item == newSel)
						break;
					
					item = item->itemAbove();
				
				} while (item);
			
			} else {
			
				QListViewItem *item = lastSel->itemBelow();
				
				do {
					setSelected(item, true);
					
					if (item == newSel)
						break;
					
					item = item->itemBelow();
				
				} while (item);
			}
		} else {
			// unselect current Item
			if (isMultiSelection())
				unselect_all();
			setSelected (newSel, true);
		}
	} else {
		if (e->state() != ControlButton)
			wasSelected = true;
		else
			setSelected (newSel, false);
	}
}

*/
