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

#ifdef __GNUG__
# pragma implementation "EmpathComposeWindow.h"
#endif

// Qt includes
#include <qmessagebox.h>

// KDE includes
#include <kmsgbox.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kfiledialog.h>
#include <kapp.h>

// Local includes
#include "EmpathDefines.h"
#include "EmpathUIUtils.h"
#include "EmpathComposeWindow.h"
#include "EmpathComposeWidget.h"
#include "EmpathMessageWidget.h"
#include "EmpathFolderChooserDialog.h"
#include "EmpathMailSender.h"
#include "EmpathConfig.h"
#include "Empath.h"
#include <RMM_Message.h>

EmpathComposeWindow::EmpathComposeWindow()
	: KTMainWindow()
{
	empathDebug("ctor");
	composeWidget_	=
		new EmpathComposeWidget(this, "composeWidget");
	CHECK_PTR(composeWidget_);
	
	_init();
}

EmpathComposeWindow::EmpathComposeWindow(
		Empath::ComposeType t, const EmpathURL & m)
	:	KTMainWindow()
{
	empathDebug("ctor");
	composeWidget_	=
		new EmpathComposeWidget(t, m, this, "composeWidget");
	CHECK_PTR(composeWidget_);
	
	_init();
}

EmpathComposeWindow::EmpathComposeWindow(const QString & recipient)
	: KTMainWindow()
{
	empathDebug("ctor");
	composeWidget_	=
		new EmpathComposeWidget(recipient, this, "composeWidget");
	CHECK_PTR(composeWidget_);

	_init();
}

	void
EmpathComposeWindow::_init()
{
	hide();
	setupMenuBar();
	setupToolBar();
	setupStatusBar();
	composeWidget_->init();
	setCaption(i18n("Compose Message - ") + kapp->getCaption());
	setView(composeWidget_, false);
	updateRects();

	// Resize to main window size for now.
	KConfig * c = KGlobal::config();
	c->setGroup(EmpathConfig::GROUP_DISPLAY);
	
	int x = c->readNumEntry(EmpathConfig::KEY_MAIN_WINDOW_X_SIZE, 600);
	int y = c->readNumEntry(EmpathConfig::KEY_MAIN_WINDOW_Y_SIZE, 400);
	resize(x, y);
	
	QObject::connect(
		this,			SIGNAL(cut()),
		composeWidget_,	SLOT(s_cut()));

	QObject::connect(
		this,			SIGNAL(copy()),
		composeWidget_,	SLOT(s_copy()));

	QObject::connect(
		this,			SIGNAL(paste()),
		composeWidget_,	SLOT(s_paste()));

	QObject::connect(
		this,			SIGNAL(selectAll()),
		composeWidget_,	SLOT(s_selectAll()));
	
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
	
	tb->setFullWidth(false);

	this->addToolBar(tb, 0);

	tb->insertButton(empathIcon("send.png"), 0, SIGNAL(clicked()),
			this, SLOT(s_fileSendMessage()), true, i18n("Send"));
	
	tb->insertButton(empathIcon("sendlater.png"), 0, SIGNAL(clicked()),
			this, SLOT(s_fileSendLater()), true, i18n("Send Later"));
	
	tb->insertButton(empathIcon("save.png"), 0, SIGNAL(clicked()),
			this, SLOT(s_fileSaveAs()), true, i18n("Save"));
	
	KToolBar * tb2 = new KToolBar(this, "tooly2", i + 4 );
	CHECK_PTR(tb2);

	tb2->setFullWidth(false);
	
	this->addToolBar(tb2, 1);
		
	id_confirmDelivery_		= 8;
	id_confirmReading_		= 9;
	id_addSignature_		= 10;
	id_digitallySign_		= 11;
	id_encrypt_				= 12;
	
	tb2->insertButton(
		empathIcon("confirm-delivery.png"),
		id_confirmDelivery_, SIGNAL(toggled(bool)),
		this, SLOT(s_confirmDelivery(bool)), true, i18n("Confirm Delivery"));
	
	tb2->insertButton(
		empathIcon("confirm-reading.png"),
		id_confirmReading_, SIGNAL(toggled(bool)),
		this, SLOT(s_confirmReading(bool)), true, i18n("Confirm Reading"));
	
	tb2->insertButton(
		empathIcon("encrypt.png"),
		id_encrypt_, SIGNAL(toggled(bool)),
		this, SLOT(s_encrypt(bool)), true, i18n("Encrypt"));
	
	tb2->insertButton(
		empathIcon("dig-sign.png"),
		id_digitallySign_, SIGNAL(toggled(bool)),
		this, SLOT(s_digSign(bool)), true, i18n("Digitally Sign"));

	tb2->insertButton(
		empathIcon("sign.png"),
		id_addSignature_, SIGNAL(toggled(bool)),
		this, SLOT(s_sign(bool)), true, i18n("Add Signature"));
	
	KConfig * c(KGlobal::config());

	tb2->setToggle(id_confirmDelivery_);
	tb2->setToggle(id_confirmReading_);
	tb2->setToggle(id_digitallySign_);
	tb2->setToggle(id_encrypt_);
	tb2->setToggle(id_addSignature_);
	
	c->setGroup(EmpathConfig::GROUP_COMPOSE);
	
	tb2->setButton(id_confirmDelivery_,
		c->readBoolEntry(EmpathConfig::KEY_CONFIRM_DELIVERY, false));
	tb2->setButton(id_confirmReading_,
		c->readBoolEntry(EmpathConfig::KEY_CONFIRM_READ, false));
	tb2->setButton(id_digitallySign_,
		c->readBoolEntry(EmpathConfig::KEY_ADD_DIG_SIG, false));
	tb2->setButton(id_encrypt_,
		c->readBoolEntry(EmpathConfig::KEY_ENCRYPT, false));
	tb2->setButton(id_addSignature_,
		c->readBoolEntry(EmpathConfig::KEY_ADD_SIG, false));
}

	void
EmpathComposeWindow::setupStatusBar()
{
	empathDebug("setting up status bar");
	statusBar()->message(i18n("Empath Compose Window"));
}

// File menu slots

	void
EmpathComposeWindow::s_fileSendMessage()
{
	empathDebug("s_fileSendMessage called");

	if (!composeWidget_->haveTo()) {
		_askForRecipient();
		return;
	}
	
	if (!composeWidget_->haveSubject()) {
		_askForSubject();
		return;
	}
	
	RMM::RMessage outMessage(composeWidget_->message());

	hide();
	empath->send(outMessage);
	delete this;
}

	void
EmpathComposeWindow::s_fileSendLater()
{
	empathDebug("s_fileSendLater called");
	
	if (!composeWidget_->haveTo()) {
		_askForRecipient();
		return;
	}
	
	if (!composeWidget_->haveSubject()) {
		_askForSubject();
		return;
	}
	
	RMM::RMessage outMessage(composeWidget_->message());

	empathDebug("Checking if message has attachments");
	
//	if (composeWidget_->messageHasAttachments())
//		outMessage.addAttachmentList(composeWidget_->messageAttachments());

	empathDebug("Sending message");

	empath->queue(outMessage);
}

	void
EmpathComposeWindow::s_fileSaveAs()
{
	empathDebug("s_fileSaveAs called");

	RMM::RMessage message(composeWidget_->message());
	
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
		KMsgBox::message(this, "Empath",
			i18n("Sorry I can't write to that file. "
				"Please try another filename."),
			KMsgBox::EXCLAMATION, i18n("OK"));

		return;
	}

	empathDebug("Opened " + saveFilePath + " OK");
	
	QDataStream d(&f);
	
	QCString s = message.asString();
	
	d.writeRawBytes(s, s.length());

	f.close();
	
}

	void
EmpathComposeWindow::s_filePrint()
{
	empathDebug("s_filePrint called");
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
	emit(cut());
}

	void
EmpathComposeWindow::s_editCopy()
{
	empathDebug("s_editCopy called");
	emit(copy());
}

	void
EmpathComposeWindow::s_editPaste()
{
	empathDebug("s_editPaste called");
	emit(paste());
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
	emit(selectAll());
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
EmpathComposeWindow::s_messageSaveAs()
{
	empathDebug("s_messageSaveAs called");

	RMM::RMessage message(composeWidget_->message());
	
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
		KMsgBox::message(this, "Empath",
			i18n("Sorry I can't write to that file. "
				"Please try another filename."),
			KMsgBox::EXCLAMATION, i18n("OK"));

		return;
	}

	empathDebug("Opened " + saveFilePath + " OK");
	
	QDataStream d(&f);
	
	d << message.asString();

	f.close();
	
}

	void
EmpathComposeWindow::s_messageCopyTo()
{
	empathDebug("s_messageCopyTo called");
	
	RMM::RMessage message(composeWidget_->message());
	
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
EmpathComposeWindow::s_help()
{
	//empathInvokeHelp("", "");
}

	void
EmpathComposeWindow::s_aboutEmpath()
{
	empath->s_about();
}

	void
EmpathComposeWindow::s_aboutQt()
{
	QMessageBox::aboutQt(this, "aboutQt");
}

	void
EmpathComposeWindow::s_confirmDelivery(bool)
{
	empathDebug("s_confirmDelivery() called");
}
	
	void
EmpathComposeWindow::s_confirmReading(bool)
{
	empathDebug("s_confirmReading() called");
}
	
	void
EmpathComposeWindow::s_addSignature(bool)
{
	empathDebug("s_addSignature() called");
}
	
	void
EmpathComposeWindow::s_digitallySign(bool)
{
	empathDebug("s_digitallySign() called");
}

	void
EmpathComposeWindow::s_encrypt(bool)
{
	empathDebug("s_encrypt() called");
}

	void
EmpathComposeWindow::bugReport()
{
	setCaption(i18n("Bug Report - ") + kapp->getCaption());
	composeWidget_->bugReport();
}

	void
EmpathComposeWindow::_askForRecipient()
{
	KMsgBox::message(this, "Empath",
		i18n("Please specify at least one recipient"),
		KMsgBox::EXCLAMATION, i18n("OK"));
}

	void
EmpathComposeWindow::_askForSubject()
{
	KMsgBox::message(this, "Empath",
		i18n("Please specify a subject"),
		KMsgBox::EXCLAMATION, i18n("OK"));
}

#include "EmpathComposeWindowMenus.cpp"

