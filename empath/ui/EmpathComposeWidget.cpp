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

// Qt includes
#include <qregexp.h>

// KDE includes
#include <kprocess.h>
#include <klocale.h>
#include <kconfig.h>
#include <kapp.h>

// Local includes
#include "EmpathComposeWidget.h"
#include "EmpathHeaderEditWidget.h"
#include "EmpathSubjectSpecWidget.h"
#include "EmpathConfig.h"
#include "Empath.h"
#include "EmpathMailSender.h"

EmpathComposeWidget::EmpathComposeWidget(
	ComposeType			t,
	const EmpathURL &	m,
	QWidget *			parent,
	const char *		name)
	:	QWidget(parent, name),
		composeType_(t),
		url_(m)
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
	
	KConfig * c(kapp->getConfig());
	c->setGroup(EmpathConfig::GROUP_DISPLAY);
	
	if ((FontStyle)(c->readNumEntry(EmpathConfig::KEY_FONT_STYLE)) == Fixed)
		editorWidget_->setFont(c->readFontEntry(EmpathConfig::KEY_FIXED_FONT));
	else
		editorWidget_->setFont(c->readFontEntry(EmpathConfig::KEY_VARIABLE_FONT));

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

	_init();
}

	void
EmpathComposeWidget::_init()
{
	switch (composeType_) {

	// If we're just composing a new message, we drop out here.
		case ComposeNormal:
			empathDebug("Just a normal compose");
			break;

		case ComposeReply:
			{
				empathDebug("Replying");
				
				RMessage * m(empath->message(url_));
				if (m == 0) return;
				
				RMessage message(*m);
				
				QString s;
			   
				// Find out who sent the message, and fill in 'To:'
				s = message.envelope().firstSender().asString();

				empathDebug("Replying to \"" + s + "\""); 
				headerEditWidget_->setToText(s);
				if (!s.isEmpty())
					subjectSpecWidget_->setFocus();
				
				// Fill in the subject.
				s = message.envelope().subject().asString();
				empathDebug("Subject was \"" + s + "\""); 

				if (s.isEmpty())
					subjectSpecWidget_->setSubject(
						i18n("Re: (no subject given)"));
				else
					if (s.find(QRegExp("^[Rr][Ee]:")) != -1)
						subjectSpecWidget_->setSubject(s);
					else
						subjectSpecWidget_->setSubject("Re: " + s);
				
				// Now quote original message if we need to.
				
				KConfig * c(kapp->getConfig());
				c->setGroup(EmpathConfig::GROUP_COMPOSE);
				
				empathDebug("Quoting original if necessary");

				if (c->readBoolEntry(EmpathConfig::KEY_AUTO_QUOTE)) {
					empathDebug("It is necessary");
				
					QCString s(message.data());
					empathDebug("original:");
					empathDebug(s);
					
					s.replace(QRegExp("\\n"), "\n> ");
					empathDebug("quoted:");
					empathDebug(s);
					
					editorWidget_->setText(s);

				}
				
				editorWidget_->setFocus();
			}
			break;

		case ComposeReplyAll:
			empathDebug("Replying to all");
			break;

		case ComposeForward:
			{
				empathDebug("Forwarding");
				
				RMessage * m(empath->message(url_));
				if (m == 0) return;
				
				RMessage message(*m);
				
				QString s;
			   
				// Fill in the subject.
				s = message.envelope().subject().asString();
				empathDebug("Subject was \"" + s + "\""); 

				if (s.isEmpty())
					subjectSpecWidget_->setSubject(
						i18n("Fwd: (no subject given)"));
				else
					if (s.find(QRegExp("^[Ff][Ww][Dd]:")) != -1)
						subjectSpecWidget_->setSubject(s);
					else
						subjectSpecWidget_->setSubject("Fwd: " + s);
			}
			break;

		default:
			empathDebug("Uh ? don't know if I'm replying, forwarding or what");
			break;
	}
}

EmpathComposeWidget::~EmpathComposeWidget()
{
	empathDebug("dtor");
}

	RMessage
EmpathComposeWidget::message()
{
	empathDebug("message called");

	RMessage msg;
	msg.envelope() = headerEditWidget_->envelope();

	if (composeType_ == ComposeReply || composeType_ == ComposeReplyAll) {
		
		RMessage * m(empath->message(url_));
		
		RMessage message(*m);
		
		// If there is a references header in the message we're replying to,
		// we add the message id of that message to the references list.
		if (message.envelope().has(RMM::HeaderReferences)) {
			
			QCString references = message.envelope().references().asString();
			references += ": ";
			references += message.envelope().references().asString();
			references += " ";
			references += message.envelope().messageID().asString();
			msg.envelope().set(RMM::HeaderReferences, references);
			
		} else {
			
			// No references field. In that case, we just make an In-Reply-To
			// header.
			QCString inReplyTo = message.envelope().inReplyTo().asString();
			inReplyTo += RMM::headerNames[RMM::HeaderInReplyTo];
			inReplyTo += ": ";
			inReplyTo += message.envelope().messageID().asString();
			msg.envelope().set(RMM::HeaderInReplyTo, inReplyTo);
		}
	}

	// Put the user's added headers at the end, as they're more likely to
	// be important, and it's easier to see them when they're near to the
	// message body text.
	
	// Header / body separator
//	msg.body() = editorWidget_->text().ascii();

	return msg;
}

	bool
EmpathComposeWidget::messageHasAttachments()
{
	empathDebug("messageHasAttachments() called");
//	return attachmentListWidget_->hasAttachments();
	return false;
}

	void
EmpathComposeWidget::spawnExternalEditor(const QCString & text)
{
	empathDebug("spawnExternalEditor() called");
	
//	EmpathEditorProcess * p = new EmpathExternalEditorProcess;
//	CHECK_PTR(p);
}

