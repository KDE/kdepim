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
#include <RMM_Message.h>
#include "Empath.h"
#include "EmpathUIUtils.h"
#include "EmpathMessageViewWidget.h"
#include "EmpathMessageViewWindow.h"
#include "EmpathSettingsDialog.h"
#include "EmpathMenuMaker.h"

EmpathMessageViewWindow::EmpathMessageViewWindow(
		const EmpathURL & url, const char * name)
	:	KTMainWindow(name)
{
	empathDebug("ctor");
	
	messageView_ = new EmpathMessageViewWidget(url, this, "messageView");
	CHECK_PTR(messageView_);
	
	menu = menuBar();
	CHECK_PTR(menu);
	
	status = statusBar();
	CHECK_PTR(status);
	
	setupMenuBar();
	setupToolBar();
	setupStatusBar();
	
	empathDebug("Finished setup");
	
	QString title = "Empath: Message View";
	
	setCaption(title);
	
	setView(messageView_, false);
	
	updateRects();
	qApp->processEvents();
	messageView_->go();
	qApp->processEvents();
	show();
	empathDebug("ctor finished");
}

EmpathMessageViewWindow::~EmpathMessageViewWindow()
{
	empathDebug("dtor");
}

	void
EmpathMessageViewWindow::setupMenuBar()
{
	empathDebug("setting up menu bar");

	fileMenu_	= new QPopupMenu();
	CHECK_PTR(fileMenu_);

	editMenu_	= new QPopupMenu();
	CHECK_PTR(editMenu_);

	messageMenu_	= new QPopupMenu();
	CHECK_PTR(messageMenu_);

	// File menu
	empathDebug("setting up file menu");
	
	fileMenu_->insertItem(empathIcon("empath-save.xpm"), i18n("Save &As"),
		this, SLOT(s_fileSaveAs()));
	
	fileMenu_->insertSeparator();

	fileMenu_->insertItem(empathIcon("empath-print.xpm"), i18n("&Print"),
		this, SLOT(s_filePrint()));
	
	fileMenu_->insertSeparator();
	
	fileMenu_->insertItem(i18n("&Close"),
		this, SLOT(s_fileClose()));

	// Edit menu
	
	empathDebug("setting up edit menu");

	editMenu_->insertItem(empathIcon("empath-copy.xpm"), i18n("&Copy"),
		this, SLOT(s_editCopy()));
	
	editMenu_->insertSeparator();
	
	editMenu_->insertItem(empathIcon("findf.xpm"), i18n("Find..."),
		this, SLOT(s_editFind()));
	
	editMenu_->insertItem(empathIcon("find.xpm"), i18n("Find &Again"),
		this, SLOT(s_editFindAgain()));
	
	// Message Menu
	empathDebug("setting up message menu");

	messageMenu_->insertItem(empathIcon("mini-view.xpm"), i18n("&View source"),
		this, SLOT(s_messageViewSource()));
	
	messageMenu_->insertItem(empathIcon("mini-view.xpm"), i18n("&New"),
		this, SLOT(s_messageNew()));

	messageMenu_->insertItem(empathIcon("mini-save.xpm"), i18n("Save &As"),
		this, SLOT(s_messageSaveAs()));

	messageMenu_->insertItem(empathIcon("editcopy.xpm"), i18n("&Copy to..."),
		this, SLOT(s_messageCopyTo()));
	

	setupHelpMenu(this, 0, helpMenu_);

	menu->insertItem(i18n("&File"), fileMenu_);
	menu->insertItem(i18n("&Edit"), editMenu_);
	menu->insertItem(i18n("&Message"), messageMenu_);
	menu->insertSeparator();
	menu->insertItem(i18n("&Help"), helpMenu_);
}

	void
EmpathMessageViewWindow::setupToolBar() 
{
	empathDebug("setting up tool bar");

	KToolBar * tb = new KToolBar(this, "tooly", 42);
	CHECK_PTR(tb);
	
	this->addToolBar(tb, 0);

	tb->insertButton(empathIcon("compose.xpm"), 0, SIGNAL(clicked()),
			this, SLOT(s_fileNewMessage()), true, i18n("Compose message"));
	
	tb->insertButton(empathIcon("reply.xpm"), 0, SIGNAL(clicked()),
			this, SLOT(s_messageReply()), true, i18n("Reply to message"));
	
	tb->insertButton(empathIcon("forward.xpm"), 0, SIGNAL(clicked()),
			this, SLOT(s_messageForward()), true, i18n("Forward message"));
	
	tb->insertButton(empathIcon("delete.xpm"), 0, SIGNAL(clicked()),
			this, SLOT(s_messageDelete()), true, i18n("Delete message"));
	
	tb->insertButton(empathIcon("save.xpm"), 0, SIGNAL(clicked()),
			this, SLOT(s_messageSaveAs()), true, i18n("Save message"));
	
	tb->insertButton(empathIcon("bounce.xpm"), 0, SIGNAL(clicked()),
			this, SLOT(s_messageBounce()), true, i18n("Bounce message"));
}

	void
EmpathMessageViewWindow::setupStatusBar()
{
	empathDebug("setting up status bar");
	status->message(i18n("Empath Message View Window"));
}

	void
EmpathMessageViewWindow::s_fileNewMessage()
{
	empathDebug("s_fileNewMessage called");
}

	void
EmpathMessageViewWindow::s_fileSaveAs()
{
	empathDebug("s_fileSaveAs called");
}

	void
EmpathMessageViewWindow::s_filePrint()
{
	empathDebug("s_filePrint called");
}

	void
EmpathMessageViewWindow::s_fileSettings()
{
	empathDebug("s_fileSettings called");
	EmpathSettingsDialog settingsDialog(AllSettings,
			(QWidget *)0L, "settingsDialog");
	settingsDialog.exec();
}

	void
EmpathMessageViewWindow::s_fileClose()
{
	empathDebug("s_fileClose called");
	// FIXME: Check if the user wants to save changes
}

// Edit menu slots
	
	void
EmpathMessageViewWindow::s_editCopy()
{
	empathDebug("s_editCopy called");
}

	void
EmpathMessageViewWindow::s_editFindInMessage()
{
	empathDebug("s_editFindInMessage called");
}

	void
EmpathMessageViewWindow::s_editFind()
{
	empathDebug("s_editFind called");
}

	void
EmpathMessageViewWindow::s_editFindAgain()
{
	empathDebug("s_editFindAgain called");
}

// Message menu slots

	void
EmpathMessageViewWindow::s_messageNew()
{
	empathDebug("s_messageNew called");

}

	void
EmpathMessageViewWindow::s_messageSaveAs()
{
	empathDebug("s_messageSaveAs called");

}

	void
EmpathMessageViewWindow::s_messageCopyTo()
{
	empathDebug("s_messageCopyTo called");

}

	void
EmpathMessageViewWindow::s_messageViewSource()
{
	empathDebug("s_messageViewSource called");

}

	void
EmpathMessageViewWindow::s_messageForward()
{
	empathDebug("s_messageForward called");
}

	void
EmpathMessageViewWindow::s_messageBounce()
{
	empathDebug("s_messageBounce called");
}

	void
EmpathMessageViewWindow::s_messageDelete()
{
	empathDebug("s_messageDelete called");
}

	void
EmpathMessageViewWindow::s_messagePrint()
{
	empathDebug("s_messagePrint called");
}

	void
EmpathMessageViewWindow::s_messageReply()
{
	empathDebug("s_messageReply called");
}

	void
EmpathMessageViewWindow::s_messageReplyAll()
{
	empathDebug("s_messageReplyAll called");
}


