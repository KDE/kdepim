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
#include <kapp.h>

// Local includes
#include <RMM_Message.h>
#include "Empath.h"
#include "EmpathUIUtils.h"
#include "EmpathMessageViewWidget.h"
#include "EmpathMessageViewWindow.h"
#include "EmpathMenuMaker.h"

EmpathMessageViewWindow::EmpathMessageViewWindow(
		const EmpathURL & url, const char * name)
	:	KTMainWindow(name)
{
	empathDebug("ctor");
	
	messageView_ = new EmpathMessageViewWidget(url, this, "messageView");
	CHECK_PTR(messageView_);
	
	setupMenuBar();
	setupToolBar();
	
	empathDebug("Finished setup");
	
	QString title = "Empath: Message View";
	
	setCaption(title);
	
	setView(messageView_, false);
	
	updateRects();
	kapp->processEvents();
	messageView_->go();
	kapp->processEvents();
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
	
	helpMenu_	= new QPopupMenu();
	CHECK_PTR(helpMenu_);

	// File menu
	empathDebug("setting up file menu");
	
	fileMenu_->insertItem(empathIcon("empath-save.png"), i18n("Save &As"),
		this, SLOT(s_fileSaveAs()));
	
	fileMenu_->insertSeparator();

	fileMenu_->insertItem(empathIcon("empath-print.png"), i18n("&Print"),
		this, SLOT(s_filePrint()));
	
	fileMenu_->insertSeparator();
	
	fileMenu_->insertItem(i18n("&Close"),
		this, SLOT(s_fileClose()));

	// Edit menu
	
	empathDebug("setting up edit menu");

	editMenu_->insertItem(empathIcon("empath-copy.png"), i18n("&Copy"),
		this, SLOT(s_editCopy()));
	
	editMenu_->insertSeparator();
	
	editMenu_->insertItem(empathIcon("findf.png"), i18n("Find..."),
		this, SLOT(s_editFind()));
	
	editMenu_->insertItem(empathIcon("find.png"), i18n("Find &Again"),
		this, SLOT(s_editFindAgain()));
	
	// Message Menu
	empathDebug("setting up message menu");

	messageMenu_->insertItem(empathIcon("mini-view.png"), i18n("&View source"),
		this, SLOT(s_messageViewSource()));
	
	messageMenu_->insertItem(empathIcon("mini-view.png"), i18n("&New"),
		this, SLOT(s_messageNew()));

	messageMenu_->insertItem(empathIcon("mini-save.png"), i18n("Save &As"),
		this, SLOT(s_messageSaveAs()));

	messageMenu_->insertItem(empathIcon("editcopy.png"), i18n("&Copy to..."),
		this, SLOT(s_messageCopyTo()));
	

	setupHelpMenu(this, 0, helpMenu_);

	menuBar()->insertItem(i18n("&File"), fileMenu_);
	menuBar()->insertItem(i18n("&Edit"), editMenu_);
	menuBar()->insertItem(i18n("&Message"), messageMenu_);
	menuBar()->insertSeparator();
	menuBar()->insertItem(i18n("&Help"), helpMenu_);
}

	void
EmpathMessageViewWindow::setupToolBar() 
{
	empathDebug("setting up tool bar");

	QPixmap p = empathIcon("compose.png");
	int i = QMAX(p.width(), p.height());

	KToolBar * tb = new KToolBar(this, "tooly", i + 4);
	CHECK_PTR(tb);
	
	this->addToolBar(tb, 0);

	tb->insertButton(empathIcon("compose.png"), 0, SIGNAL(clicked()),
			this, SLOT(s_fileNewMessage()), true, i18n("Compose"));
	
	tb->insertButton(empathIcon("reply.png"), 0, SIGNAL(clicked()),
			this, SLOT(s_messageReply()), true, i18n("Reply"));
	
	tb->insertButton(empathIcon("forward.png"), 0, SIGNAL(clicked()),
			this, SLOT(s_messageForward()), true, i18n("Forward"));
	
	tb->insertButton(empathIcon("delete.png"), 0, SIGNAL(clicked()),
			this, SLOT(s_messageDelete()), true, i18n("Delete"));
	
	tb->insertButton(empathIcon("save.png"), 0, SIGNAL(clicked()),
			this, SLOT(s_messageSaveAs()), true, i18n("Save"));
}

	void
EmpathMessageViewWindow::setupStatusBar()
{
	empathDebug("setting up status bar");
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
}

	void
EmpathMessageViewWindow::s_fileClose()
{
	empathDebug("s_fileClose called");
	// FIXME: Check if the user wants to save changes
	delete this;
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


