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
#include <qlineedit.h>

// KDE includes
#include <klocale.h>
#include <kapp.h>

// Local includes
#include "EmpathDefines.h"
#include "EmpathFolderChooserDialog.h"
#include "EmpathFolder.h"
#include "EmpathFolderWidget.h"
#include "EmpathFolderChooserWidget.h"

EmpathFolderChooserDialog::EmpathFolderChooserDialog(
		QWidget * parent,
		const char * name)
	:	QDialog(parent, name, true)
{
	empathDebug("ctor");
	setCaption(i18n("Folder Chooser - ") + kapp->getCaption());
	folderWidget_	=
		new EmpathFolderWidget(this, "folderWidget");
	CHECK_PTR(folderWidget_);

	QLineEdit	tempLineEdit((QWidget *)0);
	Q_UINT32 h	= tempLineEdit.sizeHint().height();
	
	buttonBox_	= new KButtonBox(this);
	CHECK_PTR(buttonBox_);

	buttonBox_->setFixedHeight(h);
	
	// Bottom button group
	pb_help_	= buttonBox_->addButton(i18n("&Help"));	
	buttonBox_->addStretch();
	pb_OK_		= buttonBox_->addButton(i18n("&OK"));
	pb_cancel_	= buttonBox_->addButton(i18n("&Cancel"));
	
	buttonBox_->layout();

	QObject::connect(pb_OK_, SIGNAL(clicked()),
			this, SLOT(s_OK()));
	
	QObject::connect(pb_cancel_, SIGNAL(clicked()),
			this, SLOT(s_cancel()));
	
	QObject::connect(pb_help_, SIGNAL(clicked()),
			this, SLOT(s_help()));

	mainLayout_ = new QGridLayout(this, 2, 1, 10, 10);
	CHECK_PTR(mainLayout_);

	mainLayout_->setRowStretch(0, 7);
	mainLayout_->setRowStretch(1, 1);
	
	mainLayout_->addWidget(folderWidget_,	0, 0);
	mainLayout_->addWidget(buttonBox_,		1, 0);
	
	mainLayout_->activate();
}

EmpathFolderChooserDialog::~EmpathFolderChooserDialog()
{
	empathDebug("dtor");
}

	void
EmpathFolderChooserDialog::s_OK()
{
	empathDebug("s_OK called");
	accept();
}

	void
EmpathFolderChooserDialog::s_cancel()
{
	empathDebug("s_cancel called");
	reject();
}

	void
EmpathFolderChooserDialog::s_help()
{
	empathDebug("s_help called");
}

	EmpathURL
EmpathFolderChooserDialog::selected() const
{
	empathDebug("selected() called");
	return folderWidget_->selected();
}

