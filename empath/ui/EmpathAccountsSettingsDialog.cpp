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
#include <klineeditdlg.h>

// Local includes
#include "EmpathAccountsSettingsDialog.h"
#include "RikGroupBox.h"
#include "EmpathServerTypeDialog.h"
#include "EmpathConfigMaildirDialog.h"
#include "EmpathConfigMboxDialog.h"
#include "EmpathConfigMMDFDialog.h"
#include "EmpathConfigPOP3Dialog.h"
#include "EmpathConfigIMAP4Dialog.h"
#include "EmpathMailboxList.h"
#include "EmpathMailbox.h"
#include "EmpathMailboxMaildir.h"
#include "EmpathMailboxMbox.h"
#include "EmpathMailboxMMDF.h"
#include "EmpathMailboxPOP3.h"
#include "EmpathMailboxIMAP4.h"
#include "Empath.h"

EmpathAccountsSettingsDialog::EmpathAccountsSettingsDialog(
		QWidget * parent,
		const char * name)
	:	QWidget(parent, name)
{
	empathDebug("ctor");

	rgb_account_	= new RikGroupBox(i18n("Account"), 8, this, "rgb_account");
	CHECK_PTR(rgb_account_);

	w_account_		= new QWidget(rgb_account_,	"w_account");
	CHECK_PTR(w_account_);

	rgb_account_->setWidget(w_account_);

	QPushButton	tempButton((QWidget *)0);
	Q_UINT32 h	= tempButton.sizeHint().height();

	// Accounts

	lv_accts_			=	new QListView(w_account_, "lv_accts");
	CHECK_PTR(lv_accts_);

	lv_accts_->addColumn(i18n("Account Name"));
	lv_accts_->addColumn(i18n("Type"));
	lv_accts_->addColumn(i18n("Timer"));
	lv_accts_->setAllColumnsShowFocus(true);
	lv_accts_->setFrameStyle(QFrame::Box | QFrame::Sunken);

	pb_newAccount_		=	new QPushButton(i18n("&New"), w_account_,
			"pb_newAccount");
	CHECK_PTR(pb_newAccount_);

	pb_newAccount_->setMaximumHeight(h);

	QObject::connect(pb_newAccount_, SIGNAL(clicked()),
			this, SLOT(s_newAccount()));

	pb_editAccount_		=	new QPushButton(i18n("&Edit"), w_account_,
			"pb_newAccount");
	CHECK_PTR(pb_editAccount_);

	pb_editAccount_->setMaximumHeight(h);

	QObject::connect(pb_editAccount_, SIGNAL(clicked()),
			this, SLOT(s_editAccount()));

	pb_removeAccount_	=	new QPushButton(i18n("&Remove"), w_account_,
			"pb_removeAccount");
	CHECK_PTR(pb_removeAccount_);

	pb_removeAccount_->setMaximumHeight(h);

	QObject::connect(pb_removeAccount_, SIGNAL(clicked()),
			this, SLOT(s_removeAccount()));

	// Layouts

	topLevelLayout_			= new QGridLayout(this, 1, 1, 10, 10);
	CHECK_PTR(topLevelLayout_);

	accountGroupLayout_		= new QGridLayout(w_account_,	4, 2, 0, 10);
	CHECK_PTR(accountGroupLayout_);

	accountGroupLayout_->setColStretch(0, 3);
	accountGroupLayout_->setColStretch(1, 1);

	topLevelLayout_->addWidget(rgb_account_,	0, 0);

	accountGroupLayout_->addMultiCellWidget(lv_accts_,	0, 3, 0, 0);
	accountGroupLayout_->addWidget(pb_newAccount_,		0, 1);
	accountGroupLayout_->addWidget(pb_editAccount_,		1, 1);
	accountGroupLayout_->addWidget(pb_removeAccount_,	2, 1);

	accountGroupLayout_->activate();

	topLevelLayout_->activate();

	updateMailboxList();
}


// Slots

/**
 * According to the account type chosen by serverTypeDialog,
 * make a new EmpathMailbox of that type, and tell the corresponding
 * config*Dialog to work on that mailbox. Then exec() the dialog and
 * if OK was pressed, keep the mailbox (append to the MailboxList)
 */
	void
EmpathAccountsSettingsDialog::s_newAccount()
{
	EmpathMailbox * tempMailbox = 0;

	empathDebug("s_newAccount called");
	EmpathServerTypeDialog serverTypeDialog(this, "serverTypeDialog");
	if (serverTypeDialog.exec() == Cancel) return;

	switch (serverTypeDialog.accountType()) {

		case Maildir:
			{
				empathDebug("creating new Maildir mailbox");
				tempMailbox = new EmpathMailboxMaildir("Unnamed Maildir");
				CHECK_PTR(tempMailbox);

				KLineEditDlg led(
					i18n("Mailbox name"), tempMailbox->name(), this);

				if (!led.exec() || led.text().isEmpty()) return;

				tempMailbox->setName(led.text());

				EmpathConfigMaildirDialog configDialog(
						(EmpathMailboxMaildir *)tempMailbox,
						this, "configDialog");

				if (configDialog.exec() == Cancel) {
					empathDebug("Deleting unwanted mailbox");
					delete tempMailbox;
					return;
				}

				empath->mailboxList().append(tempMailbox);
			}
			break;


		case Mbox:
			{
				empathDebug("creating new Mbox mailbox");
				tempMailbox = new EmpathMailboxMbox("Unnamed Mbox");
				CHECK_PTR(tempMailbox);

				KLineEditDlg led(
					i18n("Mailbox name"), tempMailbox->name(), this);

				if (!led.exec() || led.text().isEmpty()) return;

				tempMailbox->setName(led.text());

				EmpathConfigMboxDialog configDialog(
						(EmpathMailboxMbox *)tempMailbox, this, "configDialog");

				if (configDialog.exec() == Cancel) {
					empathDebug("Deleting unwanted mailbox");
					delete tempMailbox;
					return;
				}

				empath->mailboxList().append(tempMailbox);
			}
			break;

		case MMDF:
			{
				empathDebug("creating new MMDF mailbox");
				tempMailbox = new EmpathMailboxMbox("Unnamed MMDF");
				CHECK_PTR(tempMailbox);

				KLineEditDlg led(
					i18n("Mailbox name"), tempMailbox->name(), this);

				if (!led.exec() || led.text().isEmpty()) return;

				tempMailbox->setName(led.text());

				EmpathConfigMMDFDialog configDialog(
						(EmpathMailboxMMDF *)tempMailbox, this, "configDialog");

				if (configDialog.exec() == Cancel) {
					empathDebug("Deleting unwanted mailbox");
					delete tempMailbox;
					return;
				}

				empath->mailboxList().append(tempMailbox);
			}
			break;

		case POP3:
			{
				empathDebug("creating new POP3 mailbox");
				tempMailbox = new EmpathMailboxPOP3("Unnamed POP3");
				CHECK_PTR(tempMailbox);

				KLineEditDlg led(
					i18n("Mailbox name"), tempMailbox->name(), this);

				if (!led.exec() || led.text().isEmpty()) return;

				tempMailbox->setName(led.text());

				EmpathConfigPOP3Dialog configDialog(
						(EmpathMailboxPOP3 *)tempMailbox,
						false, this, "configDialog");

				if (configDialog.exec() == Cancel) {
					empathDebug("Deleting unwanted mailbox");
					delete tempMailbox;
					return;
				}

				empath->mailboxList().append(tempMailbox);
			}
			break;

		case IMAP4:
			{
				empathDebug("creating new IMAP4 mailbox");
				tempMailbox = new EmpathMailboxIMAP4("Unnamed IMAP4");
				CHECK_PTR(tempMailbox);

				KLineEditDlg led(
					i18n("Mailbox name"), tempMailbox->name(), this);

				if (!led.exec() || led.text().isEmpty()) return;

				tempMailbox->setName(led.text());

				EmpathConfigIMAP4Dialog configDialog(
						(EmpathMailboxIMAP4 *)tempMailbox, this, "configDialog");

				if (configDialog.exec() == Cancel) {
					empathDebug("Deleting unwanted mailbox");
					delete tempMailbox;
					return;
				}

				empath->mailboxList().append(tempMailbox);
			}
			break;

		default:
			break;

	}
	updateMailboxList();
}

	void
EmpathAccountsSettingsDialog::s_editAccount()
{
	empathDebug("s_editAccount called");

	EmpathMailbox * m =
		empath->mailboxList().find(lv_accts_->currentItem()->text(0));

	if (m == 0) {
		empathDebug("Ouch ! couldn't find mailbox !");
		return;
	}

	KLineEditDlg led(i18n("Mailbox name"), m->name(), this);

	if (!led.exec() || led.text().isEmpty()) return;

	m->setName(led.text());

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

	empathDebug("configDialog for mailbox '" + m->name() + "' returned" +
			QString(dlg_retval == OK ? "OK" : "Cancel"));

	if (dlg_retval == OK) updateMailboxList();
}

	void
EmpathAccountsSettingsDialog::updateMailboxList()
{
	empathDebug("Updating mailbox list");

	lv_accts_->setUpdatesEnabled(false);

	lv_accts_->clear();

	EmpathMailboxListIterator it(empath->mailboxList());

	for (; it.current(); ++it) {

		EmpathMailbox * m = it.current();

		QString timerInterval;

		empathDebug("Reading timer value from mailbox");

		if (m->usesTimer())
			timerInterval = QString().setNum(m->timerInterval());

		if (m->checkMail())
			timerInterval = QString().setNum(m->checkMailInterval());
		QString accType;

		switch (m->type()) {

			case Maildir:	accType = "Maildir";	break;
			case MMDF:		accType = "MMDF";		break;
			case Mbox:		accType = "Mbox";		break;
			case POP3:		accType = "POP3";		break;
			case IMAP4:		accType = "IMAP4";		break;

			default:								break;
		}

		new QListViewItem(
				lv_accts_,
				m->name(),
				accType,
				timerInterval);
	}
}

	void
EmpathAccountsSettingsDialog::s_removeAccount()
{
	empathDebug("s_removeAccount called");

	EmpathMailboxListIterator it(empath->mailboxList());

	for (; it.current(); ++it)
		if (it.current()->name() == QString(lv_accts_->currentItem()->text(0)))
			empath->mailboxList().remove(it.current());

	updateMailboxList();

}

	void
EmpathAccountsSettingsDialog::saveData()
{
	// Nothing to save ?
}
