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
# pragma implementation "EmpathConfigIMAP4Dialog.h"
#endif

// KDE includes
#include <klocale.h>

// Local includes
#include "EmpathConfigIMAP4Dialog.h"
#include "EmpathMailboxIMAP4.h"
		
EmpathConfigIMAP4Dialog::EmpathConfigIMAP4Dialog(
		EmpathMailboxIMAP4 * mailbox,
		QWidget * parent,
		const char * name)
	:	QDialog(parent, name, true),
		mailbox_(mailbox)
{
	empathDebug("ctor");

	mailbox_	= 0;
	
	buttonBox_				= new KButtonBox(this);
	CHECK_PTR(buttonBox_);
	
	rgb_server_				= new RikGroupBox(i18n("Server"), 8, this,
			"rgb_server");
	CHECK_PTR(rgb_server_);
	
	w_server_				= new QWidget(rgb_server_,		"w_server");
	CHECK_PTR(w_server_);
	
	rgb_server_->setWidget(w_server_);
	
	QLineEdit	tempLineEdit((QWidget *)0);
	Q_UINT32 h	= tempLineEdit.sizeHint().height();

	buttonBox_->setFixedHeight(h);
	
	// Widgets

	// Bottom button group
	pb_OK_		= buttonBox_->addButton(i18n("&OK"));
	pb_Cancel_	= buttonBox_->addButton(i18n("&Cancel"));
	pb_Help_	= buttonBox_->addButton(i18n("&Help"));	

	// Server username and address
	l_notImp_	=
		new QLabel(i18n("Sorry not implemented yet"), w_server_, "l_notImp");
	CHECK_PTR(l_notImp_);

	// Layouts
	
	topLevelLayout_			= new QGridLayout(this, 1, 1, 10, 10);
	CHECK_PTR(topLevelLayout_);
	
	// Main layout of widget's main groupbox
	serverGroupLayout_		= new QGridLayout(w_server_,	1, 1, 0, 10);
	CHECK_PTR(serverGroupLayout_);
	
	serverGroupLayout_->addWidget(l_notImp_,			0, 0);
	
	topLevelLayout_->addWidget(rgb_server_,	0, 0);
	topLevelLayout_->addWidget(buttonBox_,	1, 0);

	serverGroupLayout_->activate();
	topLevelLayout_->activate();
	
	fillInSavedData();

	setMinimumSize(320,200);

	resize(320, 200);
}

	void
EmpathConfigIMAP4Dialog::s_OK()
{
	// That's it
	accept();
}

	void
EmpathConfigIMAP4Dialog::fillInSavedData()
{

}

	void
EmpathConfigIMAP4Dialog::s_Cancel()
{
	reject();
}

	void
EmpathConfigIMAP4Dialog::s_Help()
{
}


