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

	setupMenuBar();
	setupToolBar();
	setupStatusBar();
	
	QString title = i18n("Compose message");
	setCaption(title);
	
	updateRects();
	kapp->processEvents();
	show();
}

EmpathComposeWindow::~EmpathComposeWindow()
{
	empathDebug("dtor");
}
	
	void
EmpathComposeWindow::setupToolBar() 
{
	empathDebug("setting up tool bar");

	QPixmap p = empathIcon("compose.png");
	int i = QMAX(p.width(), p.height());

	KToolBar * tb = new KToolBar(this, "tooly", i + 4 );
	CHECK_PTR(tb);

	this->addToolBar(tb, 0);

	tb->insertButton(empathIcon("send.png"), 0, SIGNAL(clicked()),
			this, SLOT(s_fileSendMessage()), true, i18n("Send"));
	
	tb->insertButton(empathIcon("sendlater.png"), 0, SIGNAL(clicked()),
			this, SLOT(s_fileSendLater()), true, i18n("Send Later"));
	
	tb->insertButton(empathIcon("save.png"), 0, SIGNAL(clicked()),
			this, SLOT(s_fileSaveAs()), true, i18n("Save"));
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
	delete this;
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
	EmpathAboutBox::create();
}

	void
EmpathComposeWindow::s_aboutQt()
{
	QMessageBox::aboutQt(this, "aboutQt");
}

#include "EmpathComposeWindowMenus.cpp"

