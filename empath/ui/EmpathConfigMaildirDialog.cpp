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
# pragma implementation "EmpathConfigMaildirDialog.h"
#endif

// KDE includes
#include <kfiledialog.h>
#include <klocale.h>
#include <kapp.h>
#include <kglobal.h>
#include <kconfig.h>

// Local includes
#include "EmpathConfigMaildirDialog.h"
#include "EmpathMailboxMaildir.h"
#include "RikGroupBox.h"
#include "EmpathUIUtils.h"
		
EmpathConfigMaildirDialog::EmpathConfigMaildirDialog(
		EmpathMailboxMaildir * mailbox,
		QWidget * parent,
		const char * name)
	:	QDialog(parent, name, true),
		mailbox_(mailbox),
		applied_(false)
{
	empathDebug("ctor");
	
	setCaption(i18n("Settings for mailbox") + " \"" + mailbox->name() + "\" - "
		+ kapp->getCaption());

	rgb_server_				= new RikGroupBox(i18n("Server"), 8, this,
			"rgb_server");
	CHECK_PTR(rgb_server_);
	
	w_server_				= new QWidget(rgb_server_, "w_server");
	CHECK_PTR(w_server_);
	
	rgb_server_->setWidget(w_server_);

	QLineEdit	tempLineEdit((QWidget *)0);
	Q_UINT32 h	= tempLineEdit.sizeHint().height();

	// Main group
	
		
	l_mailboxPath_	=
		new QLabel(i18n("Mailbox location"), w_server_, "l_mailboxPath");
	CHECK_PTR(l_mailboxPath_);
	
	le_mailboxPath_ = new QLineEdit(w_server_, "le_mailboxPath");
	CHECK_PTR(le_mailboxPath_);
	le_mailboxPath_->setFixedHeight(h);
	
	pb_browseMailboxPath_ =
		new QPushButton(w_server_, "pb_browseMailboxPath");
	CHECK_PTR(pb_browseMailboxPath_);
	pb_browseMailboxPath_->setPixmap(empathIcon("browse.png"));
	
	pb_browseMailboxPath_->setFixedWidth(pb_browseMailboxPath_->sizeHint().height());
	pb_browseMailboxPath_->setFixedHeight(pb_browseMailboxPath_->sizeHint().height());

	QObject::connect(pb_browseMailboxPath_, SIGNAL(clicked()),
			this, SLOT(s_browseMailboxPath()));

	cb_mailCheckInterval_	=
		new QCheckBox(i18n("Check for new mail at interval (in minutes):"),
				w_server_, "cb_mailCheckInterval");
	CHECK_PTR(cb_mailCheckInterval_);

	cb_mailCheckInterval_->setFixedHeight(h);
	
	sb_mailCheckInterval_	=
		new QSpinBox(1, 240, 1, w_server_, "sb_mailCheckInterval");
	CHECK_PTR(sb_mailCheckInterval_);
	
	sb_mailCheckInterval_->setFixedHeight(h);

	QObject::connect(cb_mailCheckInterval_, SIGNAL(toggled(bool)),
			sb_mailCheckInterval_, SLOT(setEnabled(bool)));

///////////////////////////////////////////////////////////////////////////////
// Button box

	buttonBox_	= new KButtonBox(this);
	CHECK_PTR(buttonBox_);

	buttonBox_->setFixedHeight(h);
	
	pb_help_	= buttonBox_->addButton(i18n("&Help"));	
	CHECK_PTR(pb_help_);
	
	pb_default_	= buttonBox_->addButton(i18n("&Default"));	
	CHECK_PTR(pb_default_);
	
	buttonBox_->addStretch();
	
	pb_OK_		= buttonBox_->addButton(i18n("&OK"));
	CHECK_PTR(pb_OK_);
	
	pb_OK_->setDefault(true);
	
	pb_apply_	= buttonBox_->addButton(i18n("&Apply"));
	CHECK_PTR(pb_apply_);
	
	pb_cancel_	= buttonBox_->addButton(i18n("&Cancel"));
	CHECK_PTR(pb_cancel_);
	
	buttonBox_->layout();
	
	QObject::connect(pb_OK_,		SIGNAL(clicked()),	SLOT(s_OK()));
	QObject::connect(pb_default_,	SIGNAL(clicked()),	SLOT(s_default()));
	QObject::connect(pb_apply_,		SIGNAL(clicked()),	SLOT(s_apply()));
	QObject::connect(pb_cancel_,	SIGNAL(clicked()),	SLOT(s_cancel()));
	QObject::connect(pb_help_,		SIGNAL(clicked()),	SLOT(s_help()));
/////////////////////////////////////////////////////////////////////////////


	// Layouts
	
	topLevelLayout_			= new QGridLayout(this, 2, 1, 10, 10);
	CHECK_PTR(topLevelLayout_);
	
	// Main layout of widget's main groupbox
	serverGroupLayout_		= new QGridLayout(w_server_,	2, 3, 0, 10);
	CHECK_PTR(serverGroupLayout_);
	
	serverGroupLayout_->setRowStretch(0, 0);
	serverGroupLayout_->setRowStretch(1, 0);

	serverGroupLayout_->addWidget(l_mailboxPath_,					0, 0);
	serverGroupLayout_->addWidget(le_mailboxPath_,					0, 1);
	serverGroupLayout_->addWidget(pb_browseMailboxPath_,			0, 2);
	serverGroupLayout_->addMultiCellWidget(cb_mailCheckInterval_,	1, 1, 0, 1);
	serverGroupLayout_->addWidget(sb_mailCheckInterval_,			1, 2);

	topLevelLayout_->addWidget(rgb_server_,	0, 0);
	topLevelLayout_->addWidget(buttonBox_,	1, 0);

	serverGroupLayout_->setColStretch(0, 6);
	serverGroupLayout_->setColStretch(1, 6);
	serverGroupLayout_->setColStretch(2, 2);
	
	serverGroupLayout_->setRowStretch(0, 7);
	serverGroupLayout_->setRowStretch(1, 1);
	
	serverGroupLayout_->activate();
	topLevelLayout_->activate();
	
	rgb_server_->setMinimumSize(rgb_server_->minimumSizeHint());
	setMinimumSize(minimumSizeHint());
	resize(minimumSizeHint());
	
	loadData();
}

	void
EmpathConfigMaildirDialog::s_OK()
{
	hide();
	if (!applied_)
		s_apply();
	KGlobal::config()->sync();

	// That's it
	accept();
}

	void
EmpathConfigMaildirDialog::saveData()
{
	ASSERT(mailbox_ != 0);
	mailbox_->setPath(le_mailboxPath_->text());
	mailbox_->setCheckMail(cb_mailCheckInterval_->isChecked());
	mailbox_->setCheckMailInterval(sb_mailCheckInterval_->value());
}

	void
EmpathConfigMaildirDialog::loadData()
{
	ASSERT(mailbox_ != 0);
	le_mailboxPath_->setText(mailbox_->path());
	cb_mailCheckInterval_->setChecked(mailbox_->checkMail());
	sb_mailCheckInterval_->setValue(mailbox_->checkMailInterval());
	sb_mailCheckInterval_->setEnabled(mailbox_->checkMail());
}

	void
EmpathConfigMaildirDialog::s_cancel()
{
	if (!applied_)
		KGlobal::config()->rollback(true);
	reject();
}

	void
EmpathConfigMaildirDialog::s_apply()
{
	if (applied_) {
		pb_apply_->setText(i18n("&Apply"));
		KGlobal::config()->rollback(true);
		KGlobal::config()->reparseConfiguration();
		loadData();
		applied_ = false;
	} else {
		pb_apply_->setText(i18n("&Revert"));
		pb_cancel_->setText(i18n("&Close"));
		applied_ = true;
	}
	saveData();
}

	void
EmpathConfigMaildirDialog::s_default()
{
	le_mailboxPath_->setText("");
	cb_mailCheckInterval_->setChecked(true);
	sb_mailCheckInterval_->setValue(1);
}

	void
EmpathConfigMaildirDialog::s_help()
{
	empathInvokeHelp(QString::null, QString::null);
}

	void
EmpathConfigMaildirDialog::s_browseMailboxPath()
{
	QString temp_path =
		KFileDialog::getOpenFileName(QString::null, "*", this, "getPath");

	if (temp_path.length() != 0) le_mailboxPath_->setText(temp_path);
}

