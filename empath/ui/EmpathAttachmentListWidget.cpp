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

	attachmentList_.setAutoDelete(true);

	QGridLayout * layout = new QGridLayout(this, 2, 4, 0, 10);
	CHECK_PTR(layout);
	
	lv_attachments_	= new QListView(this, "lv_attachments");
	CHECK_PTR(lv_attachments_);

	lv_attachments_->addColumn(QString::null);
	lv_attachments_->addColumn(i18n("Filename"));
	lv_attachments_->addColumn(i18n("Type"));
	lv_attachments_->addColumn(i18n("Encoding"));

	KConfig * config = kapp->getConfig();
	config->setGroup(EmpathConfig::GROUP_COMPOSE);

	bool addSig = config->readBoolEntry(EmpathConfig::KEY_ADD_SIG);
	QString sigPath = config->readEntry(EmpathConfig::KEY_SIG_PATH);
	if (addSig && sigPath.length() != 0)
		new QListViewItem(lv_attachments_,
			QString::null, baseName(sigPath), "Signature", "7bit");
	
	setupToolbar();

	layout->addWidget(pb_add_,		0, 0);
	layout->addWidget(pb_edit_,		0, 1);
	layout->addWidget(pb_remove_,	0, 2);
	
	layout->addMultiCellWidget(lv_attachments_,	1, 1, 0, 3);

	layout->activate();
	
	this->setMinimumWidth(lv_attachments_->sizeHint().width());

}

	QSize
EmpathAttachmentListWidget::sizeHint() const
{
	return QSize(lv_attachments_->sizeHint().width(),
			pb_add_->sizeHint().height() + lv_attachments_->sizeHint().height());
}

EmpathAttachmentListWidget::~EmpathAttachmentListWidget()
{
	empathDebug("dtor");
}

	void
EmpathAttachmentListWidget::setupToolbar()
{
	pb_add_ = new QPushButton(this, "pb_add");
	CHECK_PTR(pb_add_);
	
	pb_edit_ = new QPushButton(this, "pb_add");
	CHECK_PTR(pb_edit_);
	
	pb_remove_ = new QPushButton(this, "pb_add");
	CHECK_PTR(pb_remove_);

	pb_add_->setPixmap(empathIcon("fileopen.xpm"));
	
	pb_edit_->setPixmap(empathIcon("viewmag+.xpm"));
	
	pb_remove_->setPixmap(empathIcon("filedel.xpm"));

//	pb_add_->setFixedSize(pb_add_->sizeHint());
//	pb_edit_->setFixedSize(pb_edit_->sizeHint());
//	pb_remove_->setFixedSize(pb_remove_->sizeHint());
}

	bool
EmpathAttachmentListWidget::hasAttachments() const
{
	empathDebug("hasAttachments() called");
	return (attachmentList_.count() != 0);
}

	QList<EmpathAttachmentSpec>
EmpathAttachmentListWidget::attachmentList() const
{
	empathDebug("attachmentList() called");
	return attachmentList_;
}

	void
EmpathAttachmentListWidget::addAttachment(const EmpathAttachmentSpec & att)
{
	EmpathAttachmentSpec * a = new EmpathAttachmentSpec();

	a->filename	= att.filename;
	a->type		= att.type;
	a->encoding	= att.encoding;
	a->sizeK	= att.sizeK;

	if (a->type.length()		== 0)	a->type		= "Unknown";
	if (a->encoding.length()	== 0)	a->encoding	= "8 bit";
	if (a->sizeK				== -1)	a->sizeK	= 0;
	
	attachmentList_.append(a);
}

	void
EmpathAttachmentListWidget::addAttachmentList(
		const QList<EmpathAttachmentSpec> & attList)
{
	QListIterator<EmpathAttachmentSpec> it(attList);

	for (; it.current(); ++it)
		addAttachment(*(it.current()));
}

