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
#include <qregexp.h>
#include <qfile.h>
#include <qtextstream.h>

// KDE includes
#include <kprocess.h>
#include <klocale.h>
#include <kconfig.h>
#include <kapp.h>

// Local includes
#include "EmpathComposeWidget.h"
#include "EmpathHeaderEditWidget.h"
#include "EmpathSubjectSpecWidget.h"
#include "EmpathEditorProcess.h"
#include "EmpathConfig.h"
#include "Empath.h"
#include "EmpathMailSender.h"
#include "RMM_DateTime.h"
#include "RMM_MailboxList.h"
#include "RMM_Mailbox.h"

EmpathComposeWidget::EmpathComposeWidget(
		ComposeType			t,
		const EmpathURL &	m,
		QWidget *			parent,
		const char *		name)
	:
		QWidget(parent, name),
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
	
	editorWidget_->setFont(c->readFontEntry(EmpathConfig::KEY_FIXED_FONT));

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
}

	void
EmpathComposeWidget::_init()
{
	switch (composeType_) {

		case ComposeReply:		_reply();		break; 
		case ComposeReplyAll:	_reply(true);	break; 
		case ComposeForward:	_forward();		break; 
		case ComposeNormal:		default:		break;
	}
	
	if (composeType_ == ComposeForward) {
		// Don't bother opening external editor
		return;
	}
	
	KConfig * c(kapp->getConfig());
	c->setGroup(EmpathConfig::GROUP_COMPOSE);

	if (c->readBoolEntry(EmpathConfig::KEY_USE_EXTERNAL_EDITOR, false)) {

		editorWidget_->setEnabled(false);
		spawnExternalEditor(editorWidget_->text().ascii());
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

	QCString s;
	s += headerEditWidget_->envelope();

	if (composeType_ == ComposeReply || composeType_ == ComposeReplyAll) {
		
		RMessage * m(empath->message(url_));
		if (m == 0) {
			empathDebug("No message to reply to");
			RMessage x;
			return x;
		}
		
		RMessage message(*m);
		
		// If there is a references header in the message we're replying to,
		// we add the message id of that message to the references list.
		if (message.envelope().has(RMM::HeaderReferences)) {
			
			QCString references = "References: ";
			references += message.envelope().references().asString();
			references += ": ";
			references += message.envelope().references().asString();
			references += " ";
			references += message.envelope().messageID().asString();
			s += references;
			s += "\n";
			
		} else {
			
			// No references field. In that case, we just make an In-Reply-To
			// header.
			s += "In-Reply-To: ";
		   	s += message.envelope().messageID().asString();
			s += "\n";
		}
	}

	// Put the user's added headers at the end, as they're more likely to
	// be important, and it's easier to see them when they're near to the
	// message body text.
	
	
	KConfig * c(kapp->getConfig());
	c->setGroup(EmpathConfig::GROUP_IDENTITY);
	
	s += QCString("From: ");
	s += QCString(c->readEntry(EmpathConfig::KEY_NAME).ascii());
	s += QCString(" <");
	s += QCString(c->readEntry(EmpathConfig::KEY_EMAIL).ascii());
	s += QCString(">");
	s += "\n";
	
	s +=
		QCString("Subject: ") +
		QCString(subjectSpecWidget_->getSubject().ascii());
	s += "\n";
	
	RDateTime dt;
	dt.createDefault();
	s += "Date: " + dt.asString();
	s += "\n";
	
	RMessageID id;
	id.createDefault();
	
	s += "Message-Id: " + id.asString();
    s += "\n\n";
	
	// Body
	s += editorWidget_->text().ascii();
	
	c->setGroup(EmpathConfig::GROUP_COMPOSE);
	
	if (c->readBoolEntry(EmpathConfig::KEY_ADD_SIG)) {
		
		QFile f(c->readEntry(EmpathConfig::KEY_SIG_PATH));
		
		if (f.open(IO_ReadOnly)) {	

			QTextStream t(&f);
		
			QCString sig;
		
			while (!t.eof())
				sig += t.readLine().ascii() + QCString("\n");
		
			s += "\n" + sig;
		}
	}

	
	empathDebug("MESSAGE DATA: ");
	empathDebug(s);
	RMessage msg(s);
	empathDebug("MESSAGE DATA: ");
	empathDebug(msg.asString());

	return msg;
}

	bool
EmpathComposeWidget::messageHasAttachments()
{
	empathDebug("messageHasAttachments() called");
	return false;
}

	void
EmpathComposeWidget::spawnExternalEditor(const QCString & text)
{
	empathDebug("spawnExternalEditor() called");
	
	EmpathEditorProcess * p = new EmpathEditorProcess(text);
	CHECK_PTR(p);
	
	QObject::connect(
		p, 		SIGNAL(done(bool, QCString)),
		this,	SLOT(s_editorDone(bool, QCString)));
	
	p->go();
}

	void
EmpathComposeWidget::_reply(bool toAll)
{
	empathDebug("Replying");
	
	RMessage * m(empath->message(url_));
	if (m == 0) return;
	
	RMessage message(*m);
	
	QCString to, cc;
	
	// First fill in the primary return address. This will be the Reply-To
	// address if there's one given, otherwise it will be the first in
	// the sender list.
	if (!toAll) {

		if (message.envelope().has(RMM::HeaderReplyTo))
			to = message.envelope().replyTo().at(0)->asString();
		else
			to = message.envelope().to().at(0)->asString();
		
		headerEditWidget_->setToText(to);
	}
	
	if (toAll) {
		
		if (message.envelope().has(RMM::HeaderReplyTo)) {
		
			to = message.envelope().replyTo().asString();
			headerEditWidget_->setToText(to);
		
		} else if (message.envelope().has(RMM::HeaderFrom)) {
		
			to = message.envelope().from().at(0).asString();
			headerEditWidget_->setToText(to);
		}
		
		if (message.envelope().has(RMM::HeaderCc)) {

			if (message.envelope().cc().count() != 0) {
			
				bool firstTime = false;
				
				for (int i = 0; i < message.envelope().cc().count(); i++) {
			
					if (!firstTime)
						cc += ", ";
					cc += message.envelope().cc().at(i).asString();
				}

				headerEditWidget_->setCcText(cc);
			}
		}
	}

	
	subjectSpecWidget_->setFocus();
	
	// Fill in the subject.
	QString s = message.envelope().subject().asString();
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

	// Add the 'On (date) (name) wrote' bit
		
	if (!c->readBoolEntry(EmpathConfig::KEY_AUTO_QUOTE)) {
		editorWidget_->setFocus();
		return;
	}

	s = message.data();

	s.replace(QRegExp("\\n"), "\n> ");

	QString thingyWrote =
		c->readEntry(
			EmpathConfig::KEY_PHRASE_REPLY_SENDER, "") + '\n';
	
	// Be careful here. We don't want to reveal people's
	// email addresses.
	if (message.envelope().has(RMM::HeaderFrom) &&
		!message.envelope().from().at(0).phrase().isEmpty()) {
		
		thingyWrote.replace(QRegExp("\\%s"),
			message.envelope().from().at(0).phrase());

		if (message.envelope().has(RMM::HeaderDate))
			thingyWrote.replace(QRegExp("\\%d"),
				message.envelope().date().qdt().date().toString());
		else
			thingyWrote.replace(QRegExp("\\%d"),
				i18n("An unknown date and time"));
	
		editorWidget_->setText('\n' + thingyWrote + s);
	}

	editorWidget_->setFocus();
}

	void
EmpathComposeWidget::_forward()
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

	void
EmpathComposeWidget::s_editorDone(bool ok, QCString text)
{
	if (!ok) {
//		statusBar()->message(i18n("Message not modified by external editor"));
		return;
	}
	
	editorWidget_->setText(text);
	editorWidget_->setEnabled(true);
}

