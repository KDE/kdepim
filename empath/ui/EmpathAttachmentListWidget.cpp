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
#include <qlayout.h>

// KDE includes
#include <klocale.h>
#include <kconfig.h>
#include <kapp.h>

// Local includes
#include "EmpathAttachmentListWidget.h"
#include "EmpathConfig.h"
#include "EmpathUIUtils.h"
#include "Empath.h"

EmpathAttachmentListWidget::EmpathAttachmentListWidget(
		QWidget * parent,
		const char * name)
	:	QWidget(parent, name)
{
	empathDebug("ctor");

	QGridLayout * layout = new QGridLayout(this, 1, 1, 0, 10);
	CHECK_PTR(layout);
	
	lv_attachments_	= new QListView(this, "lv_attachments");
	CHECK_PTR(lv_attachments_);

	lv_attachments_->addColumn(QString::null);
	lv_attachments_->addColumn(i18n("Filename"));
	lv_attachments_->addColumn(i18n("Type"));
	lv_attachments_->addColumn(i18n("Encoding"));

	layout->addWidget(lv_attachments_,	0, 0);

	layout->activate();
}

EmpathAttachmentListWidget::~EmpathAttachmentListWidget()
{
	empathDebug("dtor");
}

	void
EmpathAttachmentListWidget::use(const RMessage & att)
{
}

