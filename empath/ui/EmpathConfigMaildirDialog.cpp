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
#include <kfiledialog.h>
#include <klocale.h>

// Local includes
#include "EmpathConfigMaildirDialog.h"
#include "EmpathMailboxMaildir.h"
		
EmpathConfigMaildirDialog::EmpathConfigMaildirDialog(
		EmpathMailboxMaildir * mailbox,
		QWidget * parent,
		const char * name)
	:	QDialog(parent, name, true),
		mailbox_(mailbox)
{
	empathDebug("ctor");

	buttonBox_				= new KButtonBox(this);
	CHECK_PTR(buttonBox_);
	
	rgb_server_				= new RikGroupBox(i18n("Server"), 8, this,
			"rgb_server");
	CHECK_PTR(rgb_server_);
	
	w_server_				= new QWidget(rgb_server_, "w_server");
	CHECK_PTR(w_server_);
	
	rgb_server_->setWidget(w_server_);

	QLineEdit	tempLineEdit((QWidget *)0);
	Q_UINT32 h	= tempLineEdit.sizeHint().height();

	// Widgets

	// Bottom button group
	pb_Help_	= buttonBox_->addButton(i18n("&Help"));	
	buttonBox_->addStretch();
	pb_OK_		= buttonBox_->addButton(i18n("&OK"));
	pb_Cancel_	= buttonBox_->addButton(i18n("&Cancel"));
	
	buttonBox_->setFixedHeight(buttonBox_->sizeHint().height());

	QObject::connect(pb_OK_, SIGNAL(clicked()),
			this, SLOT(s_OK()));
	
	QObject::connect(pb_Cancel_, SIGNAL(clicked()),
			this, SLOT(s_Cancel()));
	
	QObject::connect(pb_Help_, SIGNAL(clicked()),
			this, SLOT(s_Help()));

	// Main group
	
		
	l_mailboxPath_	=
		new QLabel(i18n("Mailbox location"), w_server_, "l_mailboxPath");
	CHECK_PTR(l_mailboxPath_);
	
	le_mailboxPath_ = new QLineEdit(w_server_, "le_mailboxPath");
	CHECK_PTR(le_mailboxPath_);
	le_mailboxPath_->setFixedHeight(h);
	
	pb_browseMailboxPath_ =
		new QPushButton("...", w_server_, "pb_browseMailboxPath");
	CHECK_PTR(pb_browseMailboxPath_);
	
	pb_browseMailboxPath_->setFixedSize(pb_browseMailboxPath_->sizeHint());

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
	
	fillInSavedData();

	setMinimumSize(460, 200);

	resize(460, 200);
}

	void
EmpathConfigMaildirDialog::s_OK()
{
	ASSERT(mailbox_ != 0);
	mailbox_->setPath(le_mailboxPath_->text());
	mailbox_->setCheckMail(cb_mailCheckInterval_->isChecked());
	mailbox_->setCheckMailInterval(QString(sb_mailCheckInterval_->text()).toInt());
	mailbox_->saveConfig();
	
	// That's it
	done(OK);
}

	void
EmpathConfigMaildirDialog::fillInSavedData()
{
	ASSERT(mailbox_ != 0);
	mailbox_->readConfig();
	le_mailboxPath_->setText(mailbox_->path());
	cb_mailCheckInterval_->setChecked(mailbox_->checkMail());
	sb_mailCheckInterval_->setValue(mailbox_->checkMailInterval());
}

	void
EmpathConfigMaildirDialog::s_Cancel()
{
	done(Cancel);
}

	void
EmpathConfigMaildirDialog::s_Help()
{
}

	void
EmpathConfigMaildirDialog::s_browseMailboxPath()
{
	QString temp_path =
		KFileDialog::getOpenFileName(QString::null, "*", this, "getPath");

	if (temp_path.length() != 0) le_mailboxPath_->setText(temp_path);
}

