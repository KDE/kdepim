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
#include "EmpathConfigPOP3Dialog.h"
#include "EmpathConfigPOP3Widget.h"
#include "RikGroupBox.h"
#include "EmpathMailboxPOP3.h"

EmpathConfigPOP3Dialog::EmpathConfigPOP3Dialog(
		EmpathMailboxPOP3 * mailbox,
		bool loadData,
		QWidget * parent,
		const char * name)
	:	QDialog(parent, name, true),
		mailbox_(mailbox)
{
	empathDebug("ctor");
	settingsWidget_	=
		new EmpathConfigPOP3Widget(mailbox_, loadData, this, "settingsWidget");
	CHECK_PTR(settingsWidget_);

	QPushButton tempButton((QWidget *)0, "MI");
	int h = tempButton.sizeHint().height();
	
	buttonBox_	= new KButtonBox(this);
	CHECK_PTR(buttonBox_);

	buttonBox_->setFixedHeight(h);
	
	// Bottom button group
	pb_Help_	= buttonBox_->addButton(i18n("&Help"));	
	buttonBox_->addStretch();
	pb_OK_		= buttonBox_->addButton(i18n("&OK"));
	pb_Cancel_	= buttonBox_->addButton(i18n("&Cancel"));
	
	buttonBox_->layout();

	QObject::connect(pb_OK_, SIGNAL(clicked()),
			this, SLOT(s_OK()));
	
	QObject::connect(pb_Cancel_, SIGNAL(clicked()),
			this, SLOT(s_Cancel()));
	
	QObject::connect(pb_Help_, SIGNAL(clicked()),
			this, SLOT(s_Help()));

	mainLayout_ = new QGridLayout(this, 2, 1, 10, 10);
	CHECK_PTR(mainLayout_);

	mainLayout_->setRowStretch(0, 7);
	mainLayout_->setRowStretch(1, 1);
	
	mainLayout_->addWidget(settingsWidget_,	0, 0);
	mainLayout_->addWidget(buttonBox_,		1, 0);
	
	mainLayout_->activate();
	setMinimumSize(490,537);
	resize(490,537);
}

	void
EmpathConfigPOP3Dialog::s_OK()
{
	empathDebug("s_OK called");
	settingsWidget_->saveData();
	accept();
}

	void
EmpathConfigPOP3Dialog::s_Cancel()
{
	empathDebug("s_Cancel called");
	reject();
}

	void
EmpathConfigPOP3Dialog::s_Help()
{
	empathDebug("s_Help called");
}

