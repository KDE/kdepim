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
#include <kconfig.h>
#include <kapp.h>

// Local includes
#include "EmpathComposeSettingsDialog.h"
#include "EmpathConfig.h"
#include "Empath.h"
#include "RikGroupBox.h"
		
EmpathComposeSettingsDialog::EmpathComposeSettingsDialog(
		QWidget * parent,
		const char * name)
	:	QWidget(parent, name)
{
	empathDebug("ctor");

	QLineEdit	tempLineEdit((QWidget *)0);
	Q_UINT32 h	= tempLineEdit.sizeHint().height();
	
	rgb_phrases_	= new RikGroupBox(i18n("Phrases"), 8, this, "rgb_phrases");
	CHECK_PTR(rgb_phrases_);
	
	rgb_msg_		= new RikGroupBox(i18n("Message"), 8, this, "rgb_msg");
	CHECK_PTR(rgb_msg_);
	
	rgb_when_		= new RikGroupBox(i18n("When to send"), 8, this, "rgb_when");
	CHECK_PTR(rgb_when_);
	
	rgb_phrases_->setMinimumHeight(h * 3 + (6 * 10));
	rgb_msg_->setMinimumHeight(h * 4 + (7 * 10));
	rgb_when_->setMinimumHeight(h * 2 + (7 * 10));

	w_phrases_	= new QWidget(rgb_phrases_,	"w_phrases");
	CHECK_PTR(w_phrases_);
	
	w_msg_		= new QWidget(rgb_msg_,		"w_msg");
	CHECK_PTR(w_msg_);
	
	w_when_		= new QWidget(rgb_when_,	"w_when");
	CHECK_PTR(w_when_);
	
	rgb_phrases_->setWidget(w_phrases_);
	rgb_msg_->setWidget(w_msg_);
	rgb_when_->setWidget(w_when_);

	// Phrases
	
	l_reply_	=
		new QLabel(i18n("Reply to sender:"), w_phrases_, "l_reply");
	CHECK_PTR(l_reply_);

	le_reply_	=
		new QLineEdit(w_phrases_, "le_reply");
	CHECK_PTR(le_reply_);
	
	le_reply_->setFixedHeight(h);
	
	l_replyAll_	=
		new QLabel(i18n("Reply to All:"), w_phrases_, "l_replyAll");
	CHECK_PTR(l_replyAll_);
	
	le_replyAll_	=
		new QLineEdit(w_phrases_, "le_replyAll");
	CHECK_PTR(le_replyAll_);
	
	le_replyAll_->setFixedHeight(h);
	
	l_forward_	=
		new QLabel(i18n("Forward:"), w_phrases_, "l_forward");
	CHECK_PTR(l_forward_);
	
	le_forward_	=
		new QLineEdit(w_phrases_, "le_forward");
	CHECK_PTR(le_forward_);
	
	le_forward_->setFixedHeight(h);

	// Quoting
		
	cb_quote_	=
		new QCheckBox(i18n("Automatically &quote message when replying"),
				w_msg_, "cb_quote");
	CHECK_PTR(cb_quote_);
	
	cb_quote_->setFixedHeight(h);

	cb_addSig_	=
		new QCheckBox(i18n("Add &signature"), w_msg_, "l_addSig");
	CHECK_PTR(cb_addSig_);
	
	cb_addSig_->setFixedHeight(h);
	
	cb_digSign_	=
		new QCheckBox(i18n("Add &digital Signature"), w_msg_, "cb_digSign");
	CHECK_PTR(cb_digSign_);
	
	cb_digSign_->setFixedHeight(h);
	
	cb_wrap_	=
		new QCheckBox(i18n("Wrap &long lines at column number"), w_msg_,
				"cb_wrap");
	CHECK_PTR(cb_wrap_);
	
	cb_wrap_->setFixedHeight(h);

	sb_wrap_	=
		new QSpinBox(40, 240, 1, w_msg_, "cb_wrap");
	CHECK_PTR(sb_wrap_);

	sb_wrap_->setFixedHeight(h);
	sb_wrap_->setValue(76);
	sb_wrap_->setEnabled(false);
	
	QObject::connect(cb_wrap_, SIGNAL(toggled(bool)),
			sb_wrap_, SLOT(setEnabled(bool)));
	
	w_msg_->setMinimumWidth(
			cb_wrap_->sizeHint().width() +
			sb_wrap_->sizeHint().width());
	
	// When to send
	
	buttonGroup_	=
		new QButtonGroup(this, "buttonGroup");
	CHECK_PTR(buttonGroup_);

	buttonGroup_->hide();
	buttonGroup_->setExclusive(true);

	rb_sendNow_		=
		new QRadioButton(i18n("Send messages &straight away"), w_when_,
				"rb_sendNow");
	CHECK_PTR(rb_sendNow_);
	
	rb_sendNow_->setFixedHeight(h);

	rb_sendNow_->setChecked(true);
	
	rb_sendLater_	=
		new QRadioButton(i18n("Send messages &later"), w_when_, "rb_sendLater");
	CHECK_PTR(rb_sendLater_);
	
	rb_sendLater_->setFixedHeight(h);
	
	buttonGroup_->insert(rb_sendNow_,	SendNow);
	buttonGroup_->insert(rb_sendLater_,	SendLater);
	
	cb_externalEditor_ = new QCheckBox(i18n("Use external editor"),
		this, "cb_useExternalEditor");

	CHECK_PTR(cb_externalEditor_);
	
	cb_externalEditor_->setFixedHeight(h);
	cb_externalEditor_->setFixedWidth(cb_externalEditor_->sizeHint().width());
	
	le_externalEditor_ = new QLineEdit(this, "le_externalEditor");
	le_externalEditor_->setFixedHeight(h);
	
	// Layouts
	
	topLevelLayout_				= new QGridLayout(this, 4, 1, 10, 10);
	CHECK_PTR(topLevelLayout_);

	topLevelLayout_->setRowStretch(0, 3);
	topLevelLayout_->setRowStretch(1, 4);
	topLevelLayout_->setRowStretch(2, 2);

	phrasesGroupLayout_		= new QGridLayout(w_phrases_,	3, 2, 0, 10);
	CHECK_PTR(phrasesGroupLayout_);
	
	phrasesGroupLayout_->setColStretch(0, 2);
	phrasesGroupLayout_->setColStretch(1, 4);
	
	messageGroupLayout_		= new QGridLayout(w_msg_,		4, 2, 0, 10);
	CHECK_PTR(messageGroupLayout_);

	messageGroupLayout_->setColStretch(0, 6);
	messageGroupLayout_->setColStretch(1, 1);

	whenGroupLayout_		= new QGridLayout(w_when_,		2, 1, 0, 10);
	CHECK_PTR(whenGroupLayout_);
	
	externalLayout_ = new QGridLayout(1, 2);
	
	topLevelLayout_->addWidget(rgb_phrases_,	0, 0);
	topLevelLayout_->addWidget(rgb_msg_,		1, 0);
	topLevelLayout_->addWidget(rgb_when_,		2, 0);
	topLevelLayout_->addLayout(externalLayout_,	3, 0);

	phrasesGroupLayout_->addWidget(l_reply_,	0, 0);
	phrasesGroupLayout_->addWidget(l_replyAll_,	1, 0);
	phrasesGroupLayout_->addWidget(l_forward_,	2, 0);
	
	phrasesGroupLayout_->addWidget(le_reply_,		0, 1);
	phrasesGroupLayout_->addWidget(le_replyAll_,	1, 1);
	phrasesGroupLayout_->addWidget(le_forward_,		2, 1);

	phrasesGroupLayout_->activate();
	
	messageGroupLayout_->addMultiCellWidget(cb_quote_,		0, 0, 0, 1);
	messageGroupLayout_->addMultiCellWidget(cb_addSig_,		1, 1, 0, 1);
	messageGroupLayout_->addMultiCellWidget(cb_digSign_,	2, 2, 0, 1);
	messageGroupLayout_->addWidget(cb_wrap_,				3, 0);
	messageGroupLayout_->addWidget(sb_wrap_,				3, 1);
	
	messageGroupLayout_->activate();
	
	whenGroupLayout_->addWidget(rb_sendNow_,	0, 0);
	whenGroupLayout_->addWidget(rb_sendLater_,	1, 0);
	
	whenGroupLayout_->activate();
	
	externalLayout_->addWidget(cb_externalEditor_, 0, 0);
	externalLayout_->addWidget(le_externalEditor_, 0, 1);
	externalLayout_->activate();
	
	topLevelLayout_->activate();
}

	void
EmpathComposeSettingsDialog::saveData()
{
	KConfig * c	= kapp->getConfig();
	c->setGroup(EmpathConfig::GROUP_COMPOSE);
#define CWE c->writeEntry
	CWE( EmpathConfig::KEY_PHRASE_REPLY_SENDER,		le_reply_->text()					);
	CWE( EmpathConfig::KEY_PHRASE_REPLY_ALL,			le_replyAll_->text()				);
	CWE( EmpathConfig::KEY_PHRASE_FORWARD,			le_forward_->text()					);
	CWE( EmpathConfig::KEY_AUTO_QUOTE,				cb_quote_->isChecked()				);
	CWE( EmpathConfig::KEY_ADD_SIG,					cb_addSig_->isChecked()				);
	CWE( EmpathConfig::KEY_ADD_DIG_SIG,				cb_digSign_->isChecked()			);
	CWE( EmpathConfig::KEY_WRAP_LINES,				cb_wrap_->isChecked()				);
	CWE( EmpathConfig::KEY_WRAP_COLUMN,				QString(sb_wrap_->text()).toInt()	);
	CWE( EmpathConfig::KEY_SEND_POLICY,
		(unsigned long)(rb_sendNow_->isChecked() ? SendNow : SendLater));
	
	CWE( EmpathConfig::KEY_USE_EXTERNAL_EDITOR,		cb_externalEditor_->isChecked()		);
	CWE( EmpathConfig::KEY_EXTERNAL_EDITOR,			le_externalEditor_->text()			);
#undef CWE
}

	void
EmpathComposeSettingsDialog::loadData()
{
	KConfig * c	= kapp->getConfig();
	c->setGroup(EmpathConfig::GROUP_COMPOSE);
	
	le_reply_->setText(c->readEntry(EmpathConfig::KEY_PHRASE_REPLY_SENDER, i18n("%s wrote:")));
	le_replyAll_->setText(c->readEntry(EmpathConfig::KEY_PHRASE_REPLY_ALL, i18n("%s wrote:")));
	le_forward_->setText(c->readEntry(EmpathConfig::KEY_PHRASE_FORWARD,
		i18n("Forwarded message from %s")));

	cb_quote_->setChecked(c->readBoolEntry(EmpathConfig::KEY_AUTO_QUOTE, true));
	cb_addSig_->setChecked(c->readBoolEntry(EmpathConfig::KEY_ADD_SIG, true));
	cb_digSign_->setChecked(c->readBoolEntry(EmpathConfig::KEY_ADD_DIG_SIG, false));
	cb_wrap_->setChecked(c->readBoolEntry(EmpathConfig::KEY_WRAP_LINES, true));
	sb_wrap_->setValue(c->readNumEntry(EmpathConfig::KEY_WRAP_COLUMN, 76));
	
	rb_sendNow_->setChecked(
		((SendPolicy)c->readNumEntry(EmpathConfig::KEY_SEND_POLICY, SendNow)) == SendNow);

	rb_sendLater_->setChecked(!rb_sendNow_->isChecked());
	
	cb_externalEditor_->setChecked(
		c->readBoolEntry(EmpathConfig::KEY_USE_EXTERNAL_EDITOR, false));
	
	le_externalEditor_->setText(c->readEntry(EmpathConfig::KEY_EXTERNAL_EDITOR, "gvim"));
}

