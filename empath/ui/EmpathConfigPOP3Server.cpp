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
# pragma implementation "EmpathConfigPOP3Server.h"
#endif

// KDE includes
#include <klocale.h>

// Local includes
#include "EmpathDefines.h"
#include "EmpathConfigPOP3Server.h"
#include "EmpathMailboxPOP3.h"

EmpathConfigPOP3Server::EmpathConfigPOP3Server(QWidget * parent, const char * name)
	: QWidget(parent, name)
{
	empathDebug("ctor");
	mailbox_ = 0;

	QLineEdit	tempLineEdit((QWidget *)0);
	Q_UINT32 h	= tempLineEdit.sizeHint().height();
	
	// Server username and address
	l_inServer_	=
		new QLabel(i18n("Server address:"), this, "l_inServer");
	CHECK_PTR(l_inServer_);
	
	l_inServer_->setFixedHeight(h);
	
	le_inServer_	=
		new QLineEdit(this, "le_inServer");
	CHECK_PTR(le_inServer_);
	
	le_inServer_->setFixedHeight(h);
	
	l_inServerPort_	=
		new QLabel(i18n("Server port:"), this, "l_inServerPort");
	CHECK_PTR(l_inServerPort_);
	
	l_inServerPort_->setFixedHeight(h);
	
	le_inServerPort_	=
		new QLineEdit(this, "le_inServerPort");
	CHECK_PTR(le_inServerPort_);
	
	le_inServerPort_->setFixedHeight(h);
	
	l_uname_	=
		new QLabel(i18n("Mail server user name:"), this, "l_uname");
	CHECK_PTR(l_uname_);
	
	l_uname_->setFixedHeight(h);
	
	le_uname_	=
		new QLineEdit(this, "le_uname");
	CHECK_PTR(le_uname_);
	
	le_uname_->setFixedHeight(h);
	
	l_pass_	=
		new QLabel(i18n("Mail server password:"), this, "l_pass");
	CHECK_PTR(l_pass_);
	
	l_pass_->setFixedHeight(h);
	
	le_pass_	=
		new QLineEdit(this, "le_pass");
	CHECK_PTR(le_pass_);

	le_pass_->setEchoMode(QLineEdit::NoEcho);
	
	le_pass_->setFixedHeight(h);

	pb_starPassword_	=
		new QPushButton(this, "pb_starPassword");
	CHECK_PTR(pb_starPassword_);
	
	pb_starPassword_->setText("*");
	pb_starPassword_->setToggleButton(true);
	
	pb_starPassword_->setFixedHeight(h);

	QObject::connect(pb_starPassword_, SIGNAL(toggled(bool)),
			this, SLOT(s_starPassword(bool)));

	// Layout
	
	topLevelLayout_		= new QGridLayout(this, 	4, 3, 10, 10);
	CHECK_PTR(topLevelLayout_);

	topLevelLayout_->setColStretch(0, 5);
	topLevelLayout_->setColStretch(1, 5);
	topLevelLayout_->setColStretch(2, 1);

	topLevelLayout_->addWidget(l_inServer_,					0, 0);
	topLevelLayout_->addMultiCellWidget(le_inServer_,		0, 0, 1, 2);
	topLevelLayout_->addWidget(l_inServerPort_,				1, 0);
	topLevelLayout_->addMultiCellWidget(le_inServerPort_,	1, 1, 1, 2);
	topLevelLayout_->addWidget(l_uname_,					2, 0);
	topLevelLayout_->addWidget(le_uname_,					2, 1);
	topLevelLayout_->addWidget(l_pass_,						3, 0);
	topLevelLayout_->addWidget(le_pass_,					3, 1);
	topLevelLayout_->addWidget(pb_starPassword_,			3, 2);

	topLevelLayout_->activate();
	
	le_pass_->setEchoMode(QLineEdit::Password);
}

	void
EmpathConfigPOP3Server::saveData()
{
	empathDebug("saveData() called");
	
	if (mailbox_ == 0) {
		empathDebug("Mailbox is 0. Can't really save that.");
		return;
	}
	
	mailbox_->setServerAddress(le_inServer_->text());
	mailbox_->setServerPort(QString(le_inServerPort_->text()).toInt());
	mailbox_->setUsername(le_uname_->text());
	mailbox_->setPassword(le_pass_->text());
}

	void
EmpathConfigPOP3Server::loadData()
{
	empathDebug("Filling in saved data");

	if (mailbox_ == 0) {
		empathDebug("Mailbox is 0. Can't really load that.");
		return;
	}

	le_inServer_->setText(mailbox_->serverAddress());
	le_inServerPort_->setText(QString().setNum(mailbox_->serverPort()));
	le_uname_->setText(mailbox_->username());
	le_pass_->setText(mailbox_->password());

}

EmpathConfigPOP3Server::~EmpathConfigPOP3Server()
{
	empathDebug("dtor");
}
		
	void
EmpathConfigPOP3Server::s_starPassword(bool yn)
{
	le_pass_->setEchoMode(yn ? QLineEdit::Password : QLineEdit::NoEcho);
}

	void
EmpathConfigPOP3Server::setMailbox(EmpathMailboxPOP3 * mailbox)
{
	empathDebug("Set mailbox " + mailbox->name());
	mailbox_ = mailbox;
	loadData();
}


