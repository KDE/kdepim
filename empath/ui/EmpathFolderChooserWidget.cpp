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
#include "Empath.h"
#include "EmpathDefines.h"
#include "EmpathFolder.h"
#include "EmpathFolderChooserWidget.h"
#include "EmpathFolderChooserDialog.h"
#include "EmpathMailboxList.h"
#include "EmpathMailbox.h"

EmpathFolderChooserWidget::EmpathFolderChooserWidget(
		QWidget * parent, const char * name)
	:	QFrame(parent, name)
{
	empathDebug("ctor");
	folder_ = 0;
	setFrameStyle(QFrame::Box | QFrame::Raised);
	setLineWidth(1);
	l_folderName_		= new QLabel(this, "l_folderName_");
	pb_selectFolder_	= new QPushButton("...", this, "pb_selectFolder_");
	layout_				= new QGridLayout(this, 1, 2, 2, 10);
	pb_selectFolder_->setFixedWidth(pb_selectFolder_->sizeHint().height());
	layout_->addWidget(l_folderName_,		0, 0);
	layout_->addWidget(pb_selectFolder_,	0, 1);
	layout_->activate();
	QObject::connect(pb_selectFolder_, SIGNAL(clicked()),
			this, SLOT(s_browse()));
	l_folderName_->setText("<" + i18n("no folder selected") + ">");
	this->setFixedHeight(pb_selectFolder_->sizeHint().height()+4);
}

EmpathFolderChooserWidget::~EmpathFolderChooserWidget()
{
	empathDebug("dtor");
}

	EmpathURL
EmpathFolderChooserWidget::selectedURL() const
{
	empathDebug("selectedURL() called");
	if (folder_ != 0) {
		empathDebug("Selected folder is " +
			folder_->url().asString());
		return folder_->url();
	}
	else return EmpathURL("");
}

	void
EmpathFolderChooserWidget::setURL(const EmpathURL & url)
{
	empathDebug("setURL(" + url.asString() + ") called");
	
	folder_ = empath->folder(url);
	
	if (folder_ == 0) {
		empathDebug("Folder is 0");
		return;
	}
	
	l_folderName_->setText(url.asString());
}

	void
EmpathFolderChooserWidget::s_browse()
{
	empathDebug("s_browse() called");
	EmpathFolderChooserDialog fcd(this, "folderChooserDialog");
	
	if (fcd.exec() == Cancel) return;

	folder_ = fcd.selectedFolder();
	if (folder_ != 0)
		l_folderName_->setText(folder_->url().asString());
}

