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
#include <qmessagebox.h>

// KDE includes
#include <kmsgbox.h>
#include <klocale.h>

// Local includes
#include "EmpathDefines.h"
#include "EmpathUIUtils.h"
#include "EmpathComposeWindow.h"
#include "EmpathComposeWidget.h"
#include "EmpathMessageWidget.h"
#include "EmpathMailSender.h"
#include "EmpathMenuMaker.h"
#include "EmpathAboutBox.h"
#include "Empath.h"

EmpathComposeWindow::EmpathComposeWindow(
	ComposeType t,
	const EmpathURL & m)
	: KTMainWindow()
{
	empathDebug("ctor");

	menu	= menuBar();
	CHECK_PTR(menu);

	status	= statusBar();
	CHECK_PTR(status);

	composeWidget_	=
		new EmpathComposeWidget(t, m, this, "composeWidget");
	CHECK_PTR(composeWidget_);
	
	setView(composeWidget_, false);
	composeWidget_->show();

	setupMenuBar();
	setupToolBar();
	setupStatusBar();
	
	QString title = i18n("Compose message");
	setCaption(title);
	
	updateRects();
	show();
}

EmpathComposeWindow::~EmpathComposeWindow()
{
	empathDebug("dtor");
}
	
	void
EmpathComposeWindow::setupMenuBar()
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
	
	fileMenu_->insertItem(empathIcon("mini-send.xpm"), i18n("&Send Message"),
		this, SLOT(s_fileSendMessage()));
	
	fileMenu_->insertItem(empathIcon("mini-sendlater.xpm"), i18n("Send &Later"),
		this, SLOT(s_fileSendLater()));
	
	fileMenu_->insertSeparator();

	fileMenu_->insertItem(empathIcon("mini-save.xpm"), i18n("Save &As"),
		this, SLOT(s_fileSaveAs()));
	
	fileMenu_->insertSeparator();
	
	fileMenu_->insertItem(i18n("Attach &File"),
		this, SLOT(s_fileAttachFile()));
	
	fileMenu_->insertSeparator();

	fileMenu_->insertItem(empathIcon("empath-print.xpm"), i18n("&Print"),
		this, SLOT(s_filePrint()));
	
	fileMenu_->insertSeparator();

	fileMenu_->insertItem(empathIcon("blank.xpm"), i18n("&Close"),
		this, SLOT(s_fileClose()));

	// Edit menu
	
	empathDebug("setting up edit menu");

	editMenu_->insertItem(i18n("&Undo"),
		this, SLOT(s_editUndo()));
	
	editMenu_->insertItem(i18n("&Redo"),
		this, SLOT(s_editRedo()));
	
	editMenu_->insertSeparator();

	editMenu_->insertItem(empathIcon("empath-cut.xpm"), i18n("Cu&t"),
		this, SLOT(s_editCut()));
	
	editMenu_->insertItem(empathIcon("empath-copy.xpm"), i18n("&Copy"),
		this, SLOT(s_editCopy()));
	
	editMenu_->insertItem(empathIcon("empath-paste.xpm"), i18n("&Paste"),
		this, SLOT(s_editPaste()));
	
	editMenu_->insertItem(empathIcon("blank.xpm"), i18n("&Delete"),
		this, SLOT(s_editDelete()));

	editMenu_->insertSeparator();
	
	editMenu_->insertItem(empathIcon("blank.xpm"), i18n("&Select All"),
		this, SLOT(s_editSelectAll()));
	
	editMenu_->insertSeparator();
	
	editMenu_->insertItem(empathIcon("blank.xpm"), i18n("Find..."),
		this, SLOT(s_editFind()));
	
	editMenu_->insertItem(empathIcon("blank.xpm"), i18n("Find &Again"),
		this, SLOT(s_editFindAgain()));
	
	// Message Menu
	empathDebug("setting up message menu");

	messageMenu_->insertItem(empathIcon("mini-compose.xpm"), i18n("&New"),
		this, SLOT(s_messageNew()));

	messageMenu_->insertItem(empathIcon("mini-save.xpm"), i18n("Save &As"),
		this, SLOT(s_messageSaveAs()));

	messageMenu_->insertItem(empathIcon("mini-copy.xpm"), i18n("&Copy to..."),
		this, SLOT(s_messageCopyTo()));
	
	messageMenu_->insertSeparator();
		
	messageMenu_->insertItem(empathIcon("blank.xpm"), i18n("&View source"),
		this, SLOT(s_messageViewSource()));

	setupHelpMenu(this, 0, helpMenu_);
	
	empathDebug("Adding submenus to main menu");
	menu->insertItem(i18n("&File"), fileMenu_);
	menu->insertItem(i18n("&Edit"), editMenu_);
	menu->insertItem(i18n("&Message"), messageMenu_);
	menu->insertSeparator();
	menu->insertItem(i18n("&Help"), helpMenu_);
}

	void
EmpathComposeWindow::setupToolBar() 
{
	empathDebug("setting up tool bar");

	QPixmap p = empathIcon("compose.xpm");
	int i = QMAX(p.width(), p.height());

	KToolBar * tb = new KToolBar(this, "tooly", i + 4 );
	CHECK_PTR(tb);

	this->addToolBar(tb, 0);

	tb->insertButton(empathIcon("send.xpm"), 0, SIGNAL(clicked()),
			this, SLOT(s_fileSendMessage()), true, i18n("Send Message Now"));
	
	tb->insertButton(empathIcon("sendlater.xpm"), 0, SIGNAL(clicked()),
			this, SLOT(s_fileSendLater()), true, i18n("Send Later"));
	
	tb->insertButton(empathIcon("save.xpm"), 0, SIGNAL(clicked()),
			this, SLOT(s_fileSaveAs()), true, i18n("Save message"));
}

	void
EmpathComposeWindow::setupStatusBar()
{
	empathDebug("setting up status bar");
	status->message(i18n("Empath Compose Window"));
}

// File menu slots

	void
EmpathComposeWindow::s_fileSendMessage()
{
	empathDebug("s_fileSendMessage called");

	RMessage outMessage(composeWidget_->messageAsString());

	empathDebug("Checking if message has attachments");
	
//	if (composeWidget_->messageHasAttachments())
//		outMessage.addAttachmentList(composeWidget_->messageAttachments());

	empathDebug("Sending message");

	empath->mailSender().sendOne(outMessage);
}

	void
EmpathComposeWindow::s_fileSendLater()
{
	empathDebug("s_fileSendLater called");
	
	RMessage outMessage(composeWidget_->messageAsString());

	empathDebug("Checking if message has attachments");
	
//	if (composeWidget_->messageHasAttachments())
//		outMessage.addAttachmentList(composeWidget_->messageAttachments());

	empathDebug("Sending message");

	empath->mailSender().addPendingMessage(outMessage);
}

	void
EmpathComposeWindow::s_fileSaveAs()
{
	empathDebug("s_fileSaveAs called");
}

	void
EmpathComposeWindow::s_fileAttachFile()
{
	empathDebug("s_fileAttachFile called");
}

	void
EmpathComposeWindow::s_filePrint()
{
	empathDebug("s_filePrint called");
}

	void
EmpathComposeWindow::s_fileSettings()
{
	empathDebug("s_fileSettings called");
}

	void
EmpathComposeWindow::s_fileClose()
{
	empathDebug("s_fileClose called");
	// FIXME: Check if the user wants to save changes
}

// Edit menu slots
	
	void
EmpathComposeWindow::s_editUndo()
{
	empathDebug("s_editUndo called");
}

	void
EmpathComposeWindow::s_editRedo()
{
	empathDebug("s_editRedo called");
}

	void
EmpathComposeWindow::s_editCut()
{
	empathDebug("s_editCut called");
}

	void
EmpathComposeWindow::s_editCopy()
{
	empathDebug("s_editCopy called");
}

	void
EmpathComposeWindow::s_editPaste()
{
	empathDebug("s_editPaste called");
}

	void
EmpathComposeWindow::s_editDelete()
{
	empathDebug("s_editDelete called");
}

	void
EmpathComposeWindow::s_editSelectAll()
{
	empathDebug("s_editSelectAll called");
}

	void
EmpathComposeWindow::s_editFindInMessage()
{
	empathDebug("s_editFindInMessage called");
}

	void
EmpathComposeWindow::s_editFind()
{
	empathDebug("s_editFind called");
}

	void
EmpathComposeWindow::s_editFindAgain()
{
	empathDebug("s_editFindAgain called");
}

// Message menu slots

	void
EmpathComposeWindow::s_messageNew()
{
	empathDebug("s_messageNew called");

}

	void
EmpathComposeWindow::s_messageSaveAs()
{
	empathDebug("s_messageSaveAs called");

}

	void
EmpathComposeWindow::s_messageCopyTo()
{
	empathDebug("s_messageCopyTo called");

}

	void
EmpathComposeWindow::s_messageViewSource()
{
	empathDebug("s_messageViewSource called");

}

	void
EmpathComposeWindow::s_help()
{
	empathInvokeHelp("", "");
}

	void
EmpathComposeWindow::s_aboutEmpath()
{
	EmpathAboutBox aboutBox(this, "aboutBox");
	aboutBox.exec();
}

	void
EmpathComposeWindow::s_aboutQt()
{
	QMessageBox::aboutQt(this, "aboutQt");
}

