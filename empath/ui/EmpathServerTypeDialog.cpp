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
#include "EmpathServerTypeDialog.h"
		
EmpathServerTypeDialog::EmpathServerTypeDialog(
		QWidget * parent,
		const char * name)
	:	QDialog(parent, name, true)
{
	empathDebug("ctor");
	setCaption(i18n("Mailbox type"));

	buttonGroup_	= new QButtonGroup(this, "buttonGroup");
	CHECK_PTR(buttonGroup_);
	
	buttonGroup_->hide();
	buttonGroup_->setExclusive(true);

	rgb_type_		= new RikGroupBox(i18n("Server type"), 8, this, "rgb_type");
	CHECK_PTR(rgb_type_);
	
	w_type_			= new QWidget(rgb_type_,	"w_type");
	CHECK_PTR(w_type_);
	
	rgb_type_->setWidget(w_type_);
	
	// Account Type group box

	rb_serverTypeMaildir_	=
		new QRadioButton(i18n("Maildir"), w_type_, "rb_serverTypeMaildir");
	CHECK_PTR(rb_serverTypeMaildir_);
	
	int h = rb_serverTypeMaildir_->sizeHint().height();
	
	
	rb_serverTypeMbox_	=
		new QRadioButton(i18n("Mbox"), w_type_, "rb_serverTypeMbox");
	CHECK_PTR(rb_serverTypeMaildir_);
	
	rb_serverTypeMMDF_	=
		new QRadioButton(i18n("MMDF"), w_type_, "rb_serverTypeMMDF");
	CHECK_PTR(rb_serverTypeMMDF_);
	
	rb_serverTypePOP3_	=
		new QRadioButton(i18n("POP3"), w_type_, "rb_serverTypePOP3");
	CHECK_PTR(rb_serverTypePOP3_);
	
	rb_serverTypeIMAP4_	=
		new QRadioButton(i18n("IMAP4"), w_type_, "rb_serverTypeIMAP4");
	CHECK_PTR(rb_serverTypeIMAP4_);

	rb_serverTypeMaildir_->setFixedHeight(h);
	rb_serverTypeMbox_->setFixedHeight(h);
	rb_serverTypeMMDF_->setFixedHeight(h);
	rb_serverTypePOP3_->setFixedHeight(h);
	rb_serverTypeIMAP4_->setFixedHeight(h);
	
	rb_serverTypeIMAP4_->setEnabled(false);

	rb_serverTypeMaildir_->setChecked(true);

	rgb_type_->setMinimumSize(
			rb_serverTypeMaildir_->sizeHint().width() +
			20,
			h * 10 + 20);

	buttonGroup_->insert(rb_serverTypeMaildir_,	Maildir);
	buttonGroup_->insert(rb_serverTypeMbox_,	Mbox);
	buttonGroup_->insert(rb_serverTypeMMDF_,	MMDF);
	buttonGroup_->insert(rb_serverTypePOP3_,	POP3);
	buttonGroup_->insert(rb_serverTypeIMAP4_,	IMAP4);
	
	buttonBox_ = new KButtonBox(this);
	CHECK_PTR(buttonBox_);
	
	pb_Help_	= buttonBox_->addButton(i18n("&Help"));
	buttonBox_->addStretch();
	pb_OK_		= buttonBox_->addButton(i18n("&OK"));
	pb_Cancel_	= buttonBox_->addButton(i18n("&Cancel"));
	buttonBox_->layout();
	
	buttonBox_->setFixedHeight(buttonBox_->height());

	QObject::connect(pb_Help_, SIGNAL(clicked()),
			this, SLOT(s_Help()));
	
	QObject::connect(pb_OK_, SIGNAL(clicked()),
			this, SLOT(s_OK()));
	
	QObject::connect(pb_Cancel_, SIGNAL(clicked()),
			this, SLOT(s_Cancel()));
	
	// Layouts
	
	topLevelLayout_		= new QGridLayout(this,		2, 1, 10, 10);
	CHECK_PTR(topLevelLayout_);
	
	typeGroupLayout_	= new QGridLayout(w_type_,	5, 1, 0, 10);
	CHECK_PTR(typeGroupLayout_);
	
	topLevelLayout_->setRowStretch(0, 3);
	topLevelLayout_->setRowStretch(1, 1);

	topLevelLayout_->addWidget(rgb_type_,	0, 0);
	topLevelLayout_->addWidget(buttonBox_,	1, 0);

	typeGroupLayout_->addWidget(rb_serverTypeMaildir_,	0, 0);
	typeGroupLayout_->addWidget(rb_serverTypeMbox_,		1, 0);
	typeGroupLayout_->addWidget(rb_serverTypeMMDF_,		2, 0);
	typeGroupLayout_->addWidget(rb_serverTypePOP3_,		3, 0);
	typeGroupLayout_->addWidget(rb_serverTypeIMAP4_,	4, 0);

	typeGroupLayout_->activate();

	topLevelLayout_->activate();

	setMinimumSize(300, 150);
	resize(300, 150);
}

	void
EmpathServerTypeDialog::s_OK()
{
	done(OK);
}

	void
EmpathServerTypeDialog::s_Cancel()
{
	done(-1);
}

	void
EmpathServerTypeDialog::s_Help()
{
	// Help
}

	AccountType
EmpathServerTypeDialog::accountType()
{
	if (rb_serverTypeMaildir_->isChecked())
	return Maildir;	
	if (rb_serverTypeMbox_->isChecked())
	return Mbox;	
	if (rb_serverTypeMMDF_->isChecked())
	return MMDF;	
	if (rb_serverTypePOP3_->isChecked())
	return POP3;
	if (rb_serverTypeIMAP4_->isChecked())
	return IMAP4;
	// Get out of gaol
	return Maildir;
}

