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
# pragma implementation "EmpathFolderChooserWidget.h"
#endif

#include <qpixmap.h>

// KDE includes
#include <klocale.h>

// Local includes
#include "Empath.h"
#include "EmpathDefines.h"
#include "EmpathFolderChooserWidget.h"
#include "EmpathFolderChooserDialog.h"
#include "EmpathMailboxList.h"
#include "EmpathMailbox.h"
#include "EmpathUIUtils.h"

EmpathFolderChooserWidget::EmpathFolderChooserWidget(
		QWidget * parent, const char * name)
	:	QFrame(parent, name)
{
	empathDebug("ctor");
	setFrameStyle(QFrame::Box | QFrame::Raised);
	setLineWidth(1);
	l_folderName_		= new QLabel(this, "l_folderName_");
	CHECK_PTR(l_folderName_);
	
	pb_selectFolder_	= new QPushButton(this, "pb_selectFolder_");
	CHECK_PTR(pb_selectFolder_);
	pb_selectFolder_->setPixmap(empathIcon("browse.png"));
	
	layout_				= new QGridLayout(this, 1, 2, 2, 10);
	CHECK_PTR(layout_);

	pb_selectFolder_->setFixedWidth(pb_selectFolder_->sizeHint().height());
	
	layout_->addWidget(l_folderName_,		0, 0);
	layout_->addWidget(pb_selectFolder_,	0, 1);
	layout_->activate();
	
	QObject::connect(pb_selectFolder_, SIGNAL(clicked()),
			this, SLOT(s_browse()));
	
	l_folderName_->setText("<" + i18n("no folder selected") + ">");
	
	setFixedHeight(
		pb_selectFolder_->sizeHint().height() +
		frameWidth() * 2);
	
	l_folderName_->setFixedHeight(pb_selectFolder_->sizeHint().height());
}

EmpathFolderChooserWidget::~EmpathFolderChooserWidget()
{
	empathDebug("dtor");
}

	EmpathURL
EmpathFolderChooserWidget::selectedURL() const
{
	return url_;
}

	void
EmpathFolderChooserWidget::setURL(const EmpathURL & url)
{
	empathDebug("setURL(" + url.asString() + ") called");
	
	url_ = url;
	
	l_folderName_->setText(url_.mailboxName() + "/" + url_.folderPath());
}

	void
EmpathFolderChooserWidget::s_browse()
{
	empathDebug("s_browse() called");
	EmpathFolderChooserDialog fcd(this, "folderChooserDialog");
	
	if (!fcd.exec()) {
		empathDebug("Cancelled");
		return;
	}

	url_ = fcd.selected();
	l_folderName_->setText(url_.mailboxName() + "/" + url_.folderPath());
}

