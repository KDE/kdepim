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

// System includes
#include <unistd.h> // Linux man pages lie - mkstemp is in stdlib.h
#include <stdlib.h>
#include <sys/stat.h>

// KDE includes
#include <kprocess.h>
#include <klocale.h>
#include <kconfig.h>
#include <kapp.h>

// Local includes
#include "EmpathComposeWidget.h"
#include "EmpathHeaderEditWidget.h"
#include "EmpathAttachmentListWidget.h"
#include "EmpathSubjectSpecWidget.h"
#include "EmpathConfig.h"
#include "Empath.h"
#include "EmpathMailSender.h"

EmpathComposeWidget::EmpathComposeWidget(
	ComposeType t,
	const EmpathURL & m,
	QWidget * parent,
	const char * name)
	: QWidget(parent, name)
{
	empathDebug("ctor");

	headerEditWidget_		=
		new EmpathHeaderEditWidget(this, "headerEditWidget");
	CHECK_PTR(headerEditWidget_);

	subjectSpecWidget_		=
		new EmpathSubjectSpecWidget(this, "subjectSpecWidget");
	CHECK_PTR(subjectSpecWidget_);

	l_priority_			=
		new QLabel(i18n("Priority:"), this, "l_priority_");
	CHECK_PTR(l_priority_);

	l_priority_->setFixedWidth(l_priority_->sizeHint().width());

	cmb_priority_		=
		new QComboBox(this, "cmb_priority_");
	CHECK_PTR(cmb_priority_);

	cmb_priority_->insertItem("Highest");
	cmb_priority_->insertItem("High");
	cmb_priority_->insertItem("Normal");
	cmb_priority_->insertItem("Low");
	cmb_priority_->insertItem("Lowest");

	cmb_priority_->setFixedWidth(cmb_priority_->sizeHint().width());
	cmb_priority_->setCurrentItem(2);

	editorWidget_			=
		new QMultiLineEdit(this, "editorWidget");
	CHECK_PTR(editorWidget_);

	layout_	= new QGridLayout(this, 3, 1, 2, 0, "layout_");
	CHECK_PTR(layout_);

	layout_->setColStretch(0, 7);

	layout_->setRowStretch(0, 0);
	layout_->setRowStretch(1, 0);
	layout_->setRowStretch(2, 10);


	midLayout_ = new QGridLayout(1, 3, 10);
	CHECK_PTR(midLayout_);
	layout_->addLayout(midLayout_, 1, 0);

	midLayout_->setColStretch(0, 1);
	midLayout_->addWidget(subjectSpecWidget_,	0, 0);
	midLayout_->addWidget(l_priority_,			0, 1);
	midLayout_->addWidget(cmb_priority_,		0, 2);
	midLayout_->activate();

	empathDebug("Adding header edit");
	layout_->addWidget(headerEditWidget_,	0, 0);
	empathDebug("Adding editor");
	layout_->addWidget(editorWidget_,		2, 0);

	layout_->activate();

	headerEditWidget_->setFocus();

	switch (t) {

	// If we're just composing a new message, we drop out here.
		case ComposeNormal:
			break;

		case ComposeReply:
			{
				KConfig * config = kapp->getConfig();
				config->setGroup(EmpathConfig::GROUP_COMPOSE);
				if (!config->readBoolEntry(EmpathConfig::KEY_USE_EXTERNAL_EDITOR)) {
					//	editorWidget_->setText(message->firstPlainBody());
					return;
				}
			}
			break;

		case ComposeReplyAll:
			break;

		case ComposeForward:
			{
				RMessage * message(empath->message(m));
				if (message == 0) return;
				QCString s = message->envelope().to().asString();
				headerEditWidget_->setToText(s);
				if (!s.isEmpty())
					subjectSpecWidget_->setFocus();
			}
			break;

		default:
			break;
	}
}

EmpathComposeWidget::~EmpathComposeWidget()
{
	empathDebug("dtor");
}

	QCString
EmpathComposeWidget::messageAsString()
{
	QCString msgData;

	msgData =	headerEditWidget_->headersAsText();
	msgData +=	"Subject: " + subjectSpecWidget_->getSubject();

	// Header / body separator
	msgData +=	'\n';
	msgData +=	editorWidget_->text();

	return msgData;
}

	bool
EmpathComposeWidget::messageHasAttachments()
{
	empathDebug("messageHasAttachments() called");
//	return attachmentListWidget_->hasAttachments();
	return false;
}

	QList<EmpathAttachmentSpec>
EmpathComposeWidget::messageAttachments()
{
	empathDebug("messageAttachments() called");
//	return attachmentListWidget_->attachmentList();
}

	void
EmpathComposeWidget::spawnExternalEditor(const QCString & text)
{
	empathDebug("spawnExternalEditor() called");
	
//	EmpathEditorProcess * p = new EmpathExternalEditorProcess;
//	CHECK_PTR(p);
}

