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

// System includes
#include <stdlib.h>

// Qt includes
#include <qheader.h>
#include <qstring.h>
#include <qfile.h>
#include <qdatetime.h>
#include <qevent.h>

// KDE includes
#include <klocale.h>
#include <kmsgbox.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <kapp.h>

// Local includes
#include "EmpathMessageList.h"
#include "EmpathMessageListWidget.h"
#include "EmpathMessageSourceView.h"
#include "EmpathIndexRecord.h"
#include "EmpathMessageViewWindow.h"
#include "EmpathMenuMaker.h"
#include "EmpathConfig.h"
#include "Empath.h"
#include "EmpathUIUtils.h"
#include "EmpathTask.h"
	
QListViewItem * EmpathMessageListWidget::lastSelected_ = 0;

EmpathMessageListWidget::EmpathMessageListWidget(
		QWidget * parent, const char * name)
	:	QListView(parent, name)
{
	parent_ = (EmpathMainWindow *)parent;
	wantScreenUpdates_ = false;
	setMultiSelection(true);
	
	maybeDrag_ = false;
	
	lastHeaderClicked_ = -1;

	setAllColumnsShowFocus(true);
	setMultiSelection(false);
	setRootIsDecorated(true);
	
	setSorting(-1);

	addColumn("Subject");
	addColumn("Sender");
	addColumn("Date");
	addColumn("Size");
	
	px_						= empathIcon("tree.png");
	pxRead_					= empathIcon("tree-read.png");
	pxMarked_				= empathIcon("tree-marked.png");
	pxReplied_				= empathIcon("tree-replied.png");
	pxReadMarked_			= empathIcon("tree-read-marked.png");
	pxReadReplied_			= empathIcon("tree-read-replied.png");
	pxMarkedReplied_		= empathIcon("tree-marked-replied.png");
	pxReadMarkedReplied_	= empathIcon("tree-read-marked-replied.png");

	empathDebug("Restoring column sizes");
	
	KConfig * c = kapp->getConfig();
	c->setGroup(EmpathConfig::GROUP_GENERAL);
	
	for (int i = 0 ; i < 4 ; i++) {
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
	
	markAsReadTimer_ = new EmpathMarkAsReadTimer(this);
}

EmpathMessageListWidget::~EmpathMessageListWidget()
{
	empathDebug("dtor");
	empathDebug("Saving column sizes and positions");
	empathDebug("XXX: Sort this so that positions can be restored");
	
	KConfig * c = kapp->getConfig();
	c->setGroup(EmpathConfig::GROUP_GENERAL);
	
	for (int i = 0 ; i < 4 ; i++) {

		c->writeEntry(
			EmpathConfig::KEY_MESSAGE_LIST_SIZE_COLUMN + QString().setNum(i),
			header()->cellSize(i));
		
		c->writeEntry(
			EmpathConfig::KEY_MESSAGE_LIST_POS_COLUMN + QString().setNum(i),
			header()->cellPos(i));
	}
	
	delete markAsReadTimer_;
	markAsReadTimer_ = 0;
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
EmpathMessageListWidget::addItem(EmpathIndexRecord * item)
{
	// Put the item into the master list.
	empathDebug("addItem called");
	if (item == 0) {
		empathDebug("item == 0 !");
		return;
	}
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
	
	if (! item->parentID().localPart().isEmpty() &&
		! item->messageID().localPart().isEmpty()) {
		// Child of other item
		empathDebug("Message has parentID, looking for parent");
//		empathDebug("MESSAGE ID: \"" + messageID.asString() + "\"");
//		empathDebug("PARENTID: \"" + parentID.asString() + "\"");
		
		EmpathMessageListItem * parentItem = 0;
		
		parentItem = this->find(item->parentID());

		if (parentItem == 0) {
			
			empathDebug("No parent for this item");
			newItem = new EmpathMessageListItem(this, *item);
			CHECK_PTR(newItem);
			empathDebug("Created OK");
			
		} else {
			
			empathDebug("There's parent for this item");
			newItem = new EmpathMessageListItem(parentItem, *item);
			CHECK_PTR(newItem);
			empathDebug("Created OK");
		}
		
	} else {
		// Root item
		empathDebug("Message is root item");
		newItem = new EmpathMessageListItem(this, *item);
		CHECK_PTR(newItem);
		empathDebug("Created OK");
	}
	itemList_.append(newItem);

	setStatus(newItem, item->status());
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

	void
EmpathMessageListWidget::mark(RMM::MessageStatus status)
{
	empathDebug("mark() called");
	// Don't bother auto-marking this as the user's done it.
	markAsReadTimer_->cancel();
	
	EmpathURL u(url_.mailboxName(), url_.folderPath(), QString::null);
	
	EmpathTask * t(empath->addTask("Marking messages"));
	t->setMax(itemList_.count());
	
	QListIterator<EmpathMessageListItem> it(itemList_);
	
	for (; it.current(); ++it) {
	
		t->doneOne();

		if (!it.current()->isSelected())
			continue;

		u.setMessageID(it.current()->id());
	
		if (empath->mark(u,
				RMM::MessageStatus(it.current()->status() ^ status)))
			setStatus(it.current(),
				RMM::MessageStatus(it.current()->status() ^ status));
	}
	t->done();

}

	void
EmpathMessageListWidget::s_messageMark()
{
	mark(RMM::Marked);
}

	void
EmpathMessageListWidget::s_messageMarkRead()
{
	mark(RMM::Read);
}

	void
EmpathMessageListWidget::s_messageMarkReplied()
{
	mark(RMM::Replied);
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
	if (!empath->remove(firstSelectedMessage())) {
		empathDebug("Couldn't remove message !");
		return;
	}
	
	QListViewItem * i(selectedItem());
	if (i == 0) {
		empathDebug("No currently selected item to remove !");
		return;
	}
	
	delete i;
}

	void
EmpathMessageListWidget::s_messageSaveAs()
{
	empathDebug("s_messageSaveAs called");
	QString saveFilePath =
		KFileDialog::getSaveFileName(
			QString::null, QString::null, this,
			i18n("Empath: Save Message").ascii());
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
	
	QCString s =
		message->asString();
	
	unsigned int blockSize = 1024; // 1k blocks
	
	unsigned int fileLength = s.length();

	for (unsigned int i = 0 ; i < s.length() ; i += blockSize) {
		
		QCString outStr;
		
		if ((fileLength - i) < blockSize)
			outStr = QCString(s.right(fileLength - i));
		else
			outStr = QCString(s.mid(i, blockSize));
		
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
			i18n("Empath: Message View").ascii());
	
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
	
	EmpathMessageListItem * i = (EmpathMessageListItem *)item;

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
	wantScreenUpdates_ = true;
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
EmpathMessageListWidget::s_currentChanged(QListViewItem * i)
{
	empathDebug("Current message changed - updating message widget");
	markAsReadTimer_->cancel();
	
	// Make sure we highlight the current item.
	kapp->processEvents();
	
	if (wantScreenUpdates_) {
		emit(changeView(firstSelectedMessage()));
		markAsReadTimer_->go((EmpathMessageListItem *)i);
	}
}

	void
EmpathMessageListWidget::setSignalUpdates(bool yn)
{
	empathDebug("Setting signal updates to " + QString(yn ? "true" : "false"));
	wantScreenUpdates_ = yn;
}

	void
EmpathMessageListWidget::markAsRead(EmpathMessageListItem * item)
{
	EmpathURL u = EmpathURL(url_.mailboxName(), url_.folderPath(), item->id());
	if (empath->mark(u, RMM::MessageStatus(item->status() ^ RMM::Read))) {
		setStatus(item, RMM::MessageStatus(item->status() ^ RMM::Read));
	}
}

	void
EmpathMessageListWidget::setStatus(
		EmpathMessageListItem * item, RMM::MessageStatus status)
{
	empathDebug("setStatus() called with " + QString().setNum(status));
	
	item->setStatus(status);
	
	if (status & RMM::Read)
		if (status & RMM::Marked)
			if (status & RMM::Replied)
				item->setPixmap(0, pxReadMarkedReplied_);
			else
				item->setPixmap(0, pxReadMarked_);
		else
			if (status & RMM::Replied)
				item->setPixmap(0, pxReadReplied_);
			else
				item->setPixmap(0, pxRead_);
	else
		if (status & RMM::Marked)
			if (status & RMM::Replied)
				item->setPixmap(0, pxMarkedReplied_);
			else
				item->setPixmap(0, pxMarked_);
		else
			if (status & RMM::Replied)
				item->setPixmap(0, pxReplied_);
			else
				item->setPixmap(0, px_);
	
	return;
}

	void
EmpathMessageListWidget::s_showFolder(const EmpathURL & url)
{
	empathDebug("s_showFolder(" + url.asString() + ") called");
	
	if (url_ == url) {
		emit(showing());
		return;
	}
	
	url_ = url;
	
	EmpathFolder * f = empath->folder(url_);
	if (f == 0) {
		emit(showing());
	   	return;
	}
	
	clear();
	masterList_.clear();
	
	f->messageList().sync();
	
	EmpathTask * t(empath->addTask("Sorting messages"));
	CHECK_PTR(t);
	t->setMax(f->messageCount());
	
	EmpathIndexIterator it(f->messageList());
	
	KConfig * c(kapp->getConfig());
	c->setGroup(EmpathConfig::GROUP_DISPLAY);
	
	empathDebug("....................");
	
	// If we're not threading, just fire 'em in.
	if (!c->readBoolEntry(EmpathConfig::KEY_THREAD_MESSAGES, false)) {
		setRootIsDecorated(false);
		setUpdatesEnabled(false);
		for (; it.current(); ++it) {
			EmpathMessageListItem * newItem =
				new EmpathMessageListItem(this, *it.current());
			CHECK_PTR(newItem);
			itemList_.append(newItem);
			setStatus(newItem, it.current()->status());
			t->doneOne();
		}
		
		setSorting(
			c->readNumEntry(EmpathConfig::KEY_MESSAGE_SORT_COLUMN, 2),
			c->readNumEntry(EmpathConfig::KEY_MESSAGE_SORT_ASCENDING, true));
		
		setUpdatesEnabled(true);
		triggerUpdate();
		t->done();
		
		emit(showing());
	
		return;
	}
	setRootIsDecorated(true);

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
	
	
	QTime begin(QTime::currentTime());
	QTime begin2(begin);
	QTime now;
	
	setUpdatesEnabled(false);

	for (; mit.current(); ++mit) {
		
		empathDebug("in s_showFolder() mit.current()->status == " +
			QString().setNum(mit.current()->status()));
		addItem(mit.current());
		t->doneOne();
		
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
	
	sortType_ = c->readNumEntry(EmpathConfig::KEY_MESSAGE_SORT_ASCENDING, true);
	
	setSorting(
		c->readNumEntry(EmpathConfig::KEY_MESSAGE_SORT_COLUMN, 3), sortType_);
	
	setUpdatesEnabled(true);
	triggerUpdate();
	
	emit(showing());
	t->done();
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
	
	KConfig * c(kapp->getConfig());
	c->setGroup(EmpathConfig::GROUP_DISPLAY);
	
	c->writeEntry(EmpathConfig::KEY_MESSAGE_SORT_COLUMN, i);
	c->writeEntry(EmpathConfig::KEY_MESSAGE_SORT_ASCENDING, sortType_);
	
	lastHeaderClicked_ = i;
}

	void
EmpathMessageListWidget::_setupMessageMenu()
{
	messageMenu_.insertItem(empathIcon("mini-view.png"), i18n("View"),
		this, SLOT(s_messageView()));
	
	messageMenu_.insertSeparator();
	
	messageMenuItemMark =
		messageMenu_.insertItem(
			empathIcon("tree-marked.png"), i18n("Tag"),
			this, SLOT(s_messageMark()));
	
	messageMenuItemMarkRead =
		messageMenu_.insertItem(
			empathIcon("tree-read.png"), i18n("Mark as Read"),
			this, SLOT(s_messageMarkRead()));
	
	messageMenuItemMarkReplied =
		messageMenu_.insertItem(
			empathIcon("tree-replied.png"), i18n("Mark as Replied"),
			this, SLOT(s_messageMarkReplied()));
		
	messageMenu_.insertSeparator();

	messageMenu_.insertItem(empathIcon("mini-reply.png"), i18n("Reply"),
		this, SLOT(s_messageReply()));

	messageMenu_.insertItem(empathIcon("mini-reply.png"),i18n("Reply to A&ll"),
		this, SLOT(s_messageReplyAll()));

	messageMenu_.insertItem(empathIcon("mini-forward.png"), i18n("Forward"),
		this, SLOT(s_messageForward()));

	messageMenu_.insertItem(empathIcon("mini-delete.png"), i18n("Delete"),
		this, SLOT(s_messageDelete()));

	messageMenu_.insertItem(empathIcon("mini-save.png"), i18n("Save As"),
		this, SLOT(s_messageSaveAs()));
}

EmpathMarkAsReadTimer::EmpathMarkAsReadTimer(EmpathMessageListWidget * parent)
	:	QObject(),
		parent_(parent)
{
	empathDebug("ctor");

	QObject::connect(
		&timer_,	SIGNAL(timeout()),
		this,		SLOT(s_timeout()));
}

EmpathMarkAsReadTimer::~EmpathMarkAsReadTimer()
{
	empathDebug("dtor");
}

	void
EmpathMarkAsReadTimer::go(EmpathMessageListItem * i)
{
	item_ = i;
	// Don't bother if it's already read.
	if (i->status() & RMM::Read) return;

	KConfig * c(kapp->getConfig());
	c->setGroup(EmpathConfig::GROUP_DISPLAY);
	
	if (!c->readBoolEntry(EmpathConfig::KEY_MARK_AS_READ)) return;
	int waitTime(c->readNumEntry(EmpathConfig::KEY_MARK_AS_READ_TIME, 2));
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

	bool
EmpathMessageListWidget::eventFilter(QObject * o, QEvent * e)
{
	if (!o || !e)
		return false;
	
	QMouseEvent * me = (QMouseEvent *) e;
	
	switch (me->type()) {
		
		case QEvent::MouseButtonPress:
			mousePressEvent(me);
			return true;
			break;
			
		case QEvent::MouseButtonRelease:
			mouseReleaseEvent(me);
			return true;
			break;

		case QEvent::MouseMove:
			mouseMoveEvent(me);
			return true;
			break;

		default:
			break;
	}
	return QListView::eventFilter(o, e);
}

	void
EmpathMessageListWidget::mousePressEvent(QMouseEvent * e)
{
	empathDebug("MOUSE PRESS EVENT");
	// Ok, here's the method:
	// 
	// CASE 0:
	// If a button other than right or left has been used, ignore.
	// 
	// CASE 1:
	// If the right button has been used, we popup the menu.
	// 
	// So, we've got the left button.
	// 
	// CASE 2:
	// If no modifier keys were used, deselect all, and select under
	// cursor.
	// 
	// CASE 3:
	// If just control key is pressed, toggle selection state of
	// item under cursor.
	// 
	// CASE 4:
	// If just shift key is pressed, find item above the one under cursor
	// that's selected. If there is none, deselect all items (that means
	// ones below that under cursor) and select item under cursor.
	// 
	// CASE 5:
	// If there IS an item selected above that under cursor, deselect all
	// items but that one, and then select all items from that one down to
	// the one under cursor.
	// 
	// CASE 6:
	// If control AND shift keys are pressed, find item above one under
	// cursor that's selected. If there is none, clear all and select item
	// under cursor.
	// 
	// CASE 7:
	// If there IS an item selected above that under cursor, select all
	// items from that above, to that under cursor.
	
	
	// CASE 0: Neither right nor left buttons pressed
	if (e->button() != LeftButton && e->button() != RightButton) {
		empathDebug("CASE 0");
		return;
	}
	
	QListViewItem * item = itemAt(e->pos());

	if (!item) {
		empathDebug("No item under cursor");
		return;
	}
	
	// CASE 1: Right button pressed
	
	if (e->button() == RightButton) {
		empathDebug("CASE 1");
		s_rightButtonPressed(itemAt(e->pos()), QCursor::pos(), 0); 
		return;
	}
	
	// Ok, it's the left button. We'll interject here and just set the
	// flag to say we may be about to drag.
	maybeDrag_ = true;
	dragStart_ = e->pos();
	

	// CASE 2: Left button pressed, but no modifier keys.
	
	if (e->state() == 0) {
		
		empathDebug("CASE 2");
		
		clearSelection();
		
		setMultiSelection(false);
		
		setSelected(item, true);
		lastSelected_ = item;
		
		return;
	}
		
	// CASE 3: Left button + control pressed
	if (e->state() == ControlButton) {
		empathDebug("CASE 3");
		setMultiSelection(true);
		
		if (!item->isSelected())
			lastSelected_ = item;

		setSelected(item, !(item->isSelected()));

		return;
	}
	
	if ((e->state() & ShiftButton) && (lastSelected_ == 0)) {
		
		// CASE 4 and CASE 6:
		// Shift button pressed, but no prior selection.
		// Clear all selections, and select this only.
		// For CASE 6, if control is pressed, toggle instead.
		
		empathDebug("CASE 4/6");
		
		clearSelection();
		
		setMultiSelection(false);
		
		// For CASE 6:
		if (e->state() & ControlButton) {
		
			if (!item->isSelected())
				lastSelected_ = item;

			setSelected(item, !(item->isSelected()));
			
		} else {
		
			// For CASE 4:
			setSelected(item, true);
			lastSelected_ = item;
		}
		
		return;
	}
	
	if (e->state() & ShiftButton) {
		
		// CASE 5, CASE 7:
		// There is an item already selected, as the above test didn't
		// hold.
		
		setMultiSelection(true);
		
		
		if (!(e->state() & ControlButton)) {
			
			// CASE 5:
			// Control button has not been held, so we must clear the
			// selection.
			empathDebug("CASE 5");
			clearSelection();
		}
			
		QListViewItem * i = lastSelected_;
		
		// First see if the item we're looking at is below the last selected.
		// If so, work down. Otherwise, er... work up.
		if (i->itemPos() < item->itemPos()) {
		
			while (i && i != item) {
				
				setSelected(i, true);

				i = i->itemBelow();
			}
			
		} else {
	
			while (i && i != item) {
				
				setSelected(i, true);

				i = i->itemAbove();
			}
		}

		if (e->state() & ControlButton) {
			
			empathDebug("CASE 7");
			lastSelected_ = item;
		}

		setSelected(item, true);
		return;
	}
	QListView::contentsMousePressEvent(e);
}

	void
EmpathMessageListWidget::mouseReleaseEvent(QMouseEvent *)
{
	maybeDrag_ = false;
}

	void
EmpathMessageListWidget::mouseMoveEvent(QMouseEvent * e)
{
	empathDebug("Mouse move event in progress");
	
	return; // We're broken. Sod it.
	
	if (!maybeDrag_) {
		QListView::contentsMouseMoveEvent(e);
		return;
	}
	
	empathDebug("We may be dragging");
	
	QPoint p = e->pos();
	
	int deltax = abs(dragStart_.x() - p.x());
	int deltay = abs(dragStart_.y() - p.y());
	
	empathDebug("The delta is " + QString().setNum(deltax + deltay));
	
	if ((deltax + deltay) < 30) { // FIXME: Hardcoded
		// Ignore, we haven't moved the cursor far enough.
		QListView::contentsMouseMoveEvent(e);
		return;
	}
	
	empathDebug("Ok, we're dragging");
	
	maybeDrag_ = false;
	
	QListViewItem * item = itemAt(dragStart_);
	
	if (item == 0) {
		empathDebug("Not over anything to drag");
		QListView::contentsMouseMoveEvent(e);
		return;
	}
	
	EmpathMessageListItem * i = (EmpathMessageListItem *)item;
		
	empathDebug("Starting a drag");
	char * c = new char[i->id().length() + 1];
	strcpy(c, i->id().ascii());
	QTextDrag * u  = new QTextDrag(c, this, "urlDrag");
	CHECK_PTR(u);
	
	u->setPixmap(empathIcon("tree.png")); 
	
	empathDebug("Starting the drag copy");
	u->dragCopy();
	QWidget::mouseMoveEvent(e);
}

	void
EmpathMessageListWidget::selectTagged()
{
	clearSelection();
	setMultiSelection(true);
	setUpdatesEnabled(false);
	wantScreenUpdates_ = false;

	QListIterator<EmpathMessageListItem> it(itemList_);
	
	for (; it.current(); ++it)
		if (it.current()->status() & RMM::Marked)
			it.current()->setSelected(true);
	wantScreenUpdates_ = true;
	setUpdatesEnabled(true);
	triggerUpdate();
}

	void
EmpathMessageListWidget::selectRead()
{
	clearSelection();
	setMultiSelection(true);
	setUpdatesEnabled(false);
	wantScreenUpdates_ = false;

	QListIterator<EmpathMessageListItem> it(itemList_);
	
	for (; it.current(); ++it)
		if (it.current()->status() & RMM::Read)
			it.current()->setSelected(true);
	wantScreenUpdates_ = true;
	setUpdatesEnabled(true);
	triggerUpdate();
}

	void
EmpathMessageListWidget::selectAll()
{
	setMultiSelection(true);
	setUpdatesEnabled(false);
	QListIterator<EmpathMessageListItem> it(itemList_);
	wantScreenUpdates_ = false;
	
	for (; it.current(); ++it)
		it.current()->setSelected(true);
	wantScreenUpdates_ = true;
	setUpdatesEnabled(true);
	triggerUpdate();
}	

	void
EmpathMessageListWidget::selectInvert()
{
	clearSelection();
	setMultiSelection(true);
	setUpdatesEnabled(false);
	wantScreenUpdates_ = false;

	QListIterator<EmpathMessageListItem> it(itemList_);
	
	for (; it.current(); ++it)
		it.current()->setSelected(!it.current()->isSelected());
	wantScreenUpdates_ = true;
	setUpdatesEnabled(true);
	triggerUpdate();
}

