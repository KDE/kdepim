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
#include <qapplication.h>
#include <qmsgbox.h>

// KDE includes
#include <klocale.h>
#include <kfiledialog.h>
#include <kmsgbox.h>
#include <kapp.h>

// Local includes
#include "EmpathUIUtils.h"
#include "EmpathDefines.h"
#include "EmpathMainWidget.h"
#include "EmpathMainWindow.h"
#include "EmpathMessageListWidget.h"
#include "EmpathMessageViewWindow.h"
#include "EmpathMessageWidget.h"
#include "EmpathMessageList.h"
#include "EmpathFolderWidget.h"
#include "EmpathConfig.h"
#include "EmpathFolderChooserDialog.h"
#include "Empath.h"

EmpathMainWindow::EmpathMainWindow(const char * name)
	:	KTMainWindow(name)
{
	empathDebug("ctor");

	// Resize to previous size.
	// XXX: Use session management instead once it's been reworked.
	
	KConfig * c = kapp->getConfig();
	c->setGroup(EmpathConfig::GROUP_DISPLAY);
	
	int x = c->readNumEntry(EmpathConfig::KEY_MAIN_WINDOW_X_SIZE, 600);
	int y = c->readNumEntry(EmpathConfig::KEY_MAIN_WINDOW_Y_SIZE, 400);
	resize(x, y);
	
	menu_	= menuBar();
	CHECK_PTR(menu_);

	status_	= statusBar();	
	CHECK_PTR(status_);
	
	mainWidget_	=
		new EmpathMainWidget(this, "mainWidget");
	CHECK_PTR(mainWidget_);
	
	messageListWidget_ = mainWidget_->messageListWidget();

	setView(mainWidget_, false);
	
	setupMenuBar();
	setupToolBar();
	setupStatusBar();
	
	setCaption(kapp->getCaption());

	updateRects();
	kapp->processEvents();
	show();
}

EmpathMainWindow::~EmpathMainWindow()
{
	empathDebug("dtor");

	KConfig * c = kapp->getConfig();
	c->setGroup(EmpathConfig::GROUP_DISPLAY);
	
	c->writeEntry(EmpathConfig::KEY_MAIN_WINDOW_X_SIZE, width());
	c->writeEntry(EmpathConfig::KEY_MAIN_WINDOW_Y_SIZE, height());
}

	void
EmpathMainWindow::setupToolBar()
{
	empathDebug("setting up tool bar");

	QPixmap p = empathIcon("compose.png");
	int i = QMAX(p.width(), p.height());

	KToolBar * tb = new KToolBar(this, "tooly", i + 4 );
	CHECK_PTR(tb);

	KConfig * c = kapp->getConfig();
	c->setGroup(EmpathConfig::GROUP_DISPLAY);
	
	KToolBar::BarPosition pos =
		(KToolBar::BarPosition)
		c->readNumEntry(EmpathConfig::KEY_MAIN_WINDOW_TOOLBAR_POSITION);

	if	(	pos != KToolBar::Top	||
			pos != KToolBar::Left	||
			pos != KToolBar::Right	||
			pos != KToolBar::Bottom)
		pos = KToolBar::Top;

	tb->setBarPos(pos);
	
	this->addToolBar(tb, 0);

	QObject::connect(tb, SIGNAL(moved(BarPosition)),
			this, SLOT(s_toolbarMoved(BarPosition)));

	tb->insertButton(empathIcon("compose.png"), 0, SIGNAL(clicked()),
			this, SLOT(s_messageNew()), true, i18n("Compose"));
	
	tb->insertButton(empathIcon("reply.png"), 0, SIGNAL(clicked()),
			this, SLOT(s_messageReply()), true, i18n("Reply"));
	
	tb->insertButton(empathIcon("forward.png"), 0, SIGNAL(clicked()),
			this, SLOT(s_messageForward()), true, i18n("Forward"));
	
	tb->insertSeparator();
	
	tb->insertButton(empathIcon("delete.png"), 0, SIGNAL(clicked()),
			this, SLOT(s_messageDelete()), true, i18n("Delete"));
	
	tb->insertButton(empathIcon("save.png"), 0, SIGNAL(clicked()),
			this, SLOT(s_messageSaveAs()), true, i18n("Save"));
	
//	// Debugging
//	tb->insertButton(empathIcon("mini-view.png"), 0, SIGNAL(clicked()),
//			this, SLOT(s_dumpWidgetList()), true, i18n("Debug"));
}

	void
EmpathMainWindow::setupStatusBar()
{
	empathDebug("setting up status bar");
	status_->message("Ready");
}

// If the user presses the close button on the title bar, or tries
// to kill us off another way, handle gracefully

	bool
EmpathMainWindow::queryExit()
{
	// FIXME: Check if the user wants to save changes
	
	s_fileQuit();

	return false;
}

// File menu slots

	void
EmpathMainWindow::s_fileSendNew()
{
	empathDebug("s_fileSendNew called");
	empath->sendQueued();
}

	void
EmpathMainWindow::s_fileAddressBook()
{
	empathDebug("s_fileAddressBook called");
}
	
	void
EmpathMainWindow::s_fileQuit()
{
	empathDebug("s_fileQuit called");
	// FIXME: Check if the user wants to save changes
	hide();
	kapp->processEvents();
	delete this;
}

// Folder menu slots

	void
EmpathMainWindow::s_folderNew()
{
	empathDebug("s_folderNew called");

}

	void
EmpathMainWindow::s_folderEdit()
{
	empathDebug("s_folderEdit called");

}

	void
EmpathMainWindow::s_folderClear()
{
	empathDebug("s_folderClear called");

}

	void
EmpathMainWindow::s_folderDelete()
{
	empathDebug("s_folderDelete called");

}


// Message menu slots

	void
EmpathMainWindow::s_messageNew()
{
	empathDebug("s_messageNew called");
	empath->s_compose();
}

	void
EmpathMainWindow::s_messageReply()
{
	empathDebug("s_messageReply called");
	empath->s_reply(messageListWidget_->firstSelectedMessage());
}

	void
EmpathMainWindow::s_messageReplyAll()
{
	empathDebug("s_messageReplyAll called");
	empath->s_replyAll(messageListWidget_->firstSelectedMessage());
}

	void
EmpathMainWindow::s_messageForward()
{
	empathDebug("s_messageForward called");
	empath->s_forward(messageListWidget_->firstSelectedMessage());
}

	void
EmpathMainWindow::s_messageBounce()
{
	empathDebug("s_messageBounce called");
	empath->s_bounce(messageListWidget_->firstSelectedMessage());
}

	void
EmpathMainWindow::s_messageDelete()
{
	empathDebug("s_messageDelete called");
	empath->remove(messageListWidget_->firstSelectedMessage());
}

	void
EmpathMainWindow::s_messageSaveAs()
{
	empathDebug("s_messageSaveAs called");
	
	RMessage * m = _getFirstSelectedMessage();
	
	if (m == 0) {
		KMsgBox::message(this, "Empath", i18n("Please select a message first"), KMsgBox::EXCLAMATION, i18n("OK"));
		return;
	}
	
	RMessage message(*m);

	QString saveFilePath =
		KFileDialog::getSaveFileName(
			QString::null, QString::null, this, i18n("Empath: Save Message").ascii());
	empathDebug(saveFilePath);
	
	if (saveFilePath.isEmpty()) {
		empathDebug("No filename given");
		return;
	}
	
	QFile f(saveFilePath);
	if (!f.open(IO_WriteOnly)) {
		// Warn user file cannot be opened.
		empathDebug("Couldn't open file for writing");
		KMsgBox::message(this, "Empath", i18n("Sorry I can't write to that file. Please try another filename."), KMsgBox::EXCLAMATION, i18n("OK"));
		return;
	}
	empathDebug("Opened " + saveFilePath + " OK");
	
	QDataStream d(&f);
	
	d << message.asString();

	f.close();
		
//	KMsgBox::message(this, "Empath", i18n("Message saved to") + QString(" ") + saveFilePath + QString(" ") + i18n("OK"), KMsgBox::INFORMATION, i18n("OK"));
}

	void
EmpathMainWindow::s_messageCopyTo()
{
	empathDebug("s_messageCopyTo called");

	RMessage * m(_getFirstSelectedMessage());
	
	if (m == 0) {
		KMsgBox::message(this, "Empath", i18n("Please select a message first"), KMsgBox::EXCLAMATION, i18n("OK"));
		return;
	}
	
	RMessage message(*m);
	
	EmpathFolderChooserDialog fcd((QWidget *)0L, "fcd");

	if (fcd.exec() != QDialog::Accepted) {
		empathDebug("copy cancelled");
		return;
	}

	EmpathFolder * copyFolder = empath->folder(fcd.selected());

	if (copyFolder != 0)
		copyFolder->writeMessage(message);
	else {
		empathDebug("Couldn't get copy folder");
	}
}

	void
EmpathMainWindow::s_messageMoveTo()
{
	empathDebug("s_messageMoveTo called");

	RMessage * m(_getFirstSelectedMessage());
	
	if (m == 0) {
		KMsgBox::message(this, "Empath", i18n("Please select a message first"), KMsgBox::EXCLAMATION, i18n("OK"));
		return;
	}
	
	RMessage message(*m);
	
	EmpathFolderChooserDialog fcd((QWidget *)0L, "fcd");

	if (fcd.exec() != QDialog::Accepted) {
		empathDebug("copy cancelled");
		return;
	}


	EmpathFolder * copyFolder = empath->folder(fcd.selected());

	if (copyFolder != 0) {
		if (copyFolder->writeMessage(message)) {
			empath->remove(messageListWidget_->firstSelectedMessage());
		}
	}
}

	void
EmpathMainWindow::s_messagePrint()
{
	empathDebug("s_messagePrint called");
//	mainWidget_->messageViewWidget()->s_print();
}

	void
EmpathMainWindow::s_messageFilter()
{
	empathDebug("s_messageFilter called");
	empath->filter(messageListWidget_->firstSelectedMessage());
}

	void
EmpathMainWindow::s_messageView()
{
	empathDebug("s_messageView called");
	
	EmpathMessageViewWindow * messageViewWindow =
		new EmpathMessageViewWindow(
			messageListWidget_->firstSelectedMessage(), "Empath");

	CHECK_PTR(messageViewWindow);

	messageViewWindow->show();
}

	void
EmpathMainWindow::s_help()
{
	empathInvokeHelp("", "");
}

	void
EmpathMainWindow::s_aboutQt()
{
	QMessageBox::aboutQt(this, "aboutQt");
}

	void
EmpathMainWindow::statusMessage(const QString & messageText, int seconds)
{
	status_->message(messageText, seconds);
}

	void
EmpathMainWindow::clearStatusMessage()
{
	status_->clear();
}

	void
EmpathMainWindow::s_toolbarMoved(BarPosition pos)
{
	KConfig * c = kapp->getConfig();
	c->setGroup(EmpathConfig::GROUP_DISPLAY);
	c->writeEntry(EmpathConfig::KEY_MAIN_WINDOW_TOOLBAR_POSITION, (int)pos);
}

	void
EmpathMainWindow::newMailArrived()
{
	messageListWidget_->update();
}

	EmpathMessageListWidget *
EmpathMainWindow::messageListWidget()
{
	return messageListWidget_;
}

	void
EmpathMainWindow::s_dumpWidgetList()
{
//	EmpathDebugDialog d(this, "debugDialog");
//	d.exec();
}

	RMessage *
EmpathMainWindow::_getFirstSelectedMessage() const
{
	return empath->message(messageListWidget_->firstSelectedMessage());
}

	void
EmpathMainWindow::s_editSelectTagged()
{
	messageListWidget_->selectTagged();
}

	void
EmpathMainWindow::s_editSelectRead()
{
	messageListWidget_->selectRead();
}

	void
EmpathMainWindow::s_editSelectAll()
{
	messageListWidget_->selectAll();
}

	void
EmpathMainWindow::s_editInvertSelection()
{
	messageListWidget_->selectInvert();
}

#include "EmpathMainWindowMenus.cpp"
