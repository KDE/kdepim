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
#include <kglobal.h>
#include <kconfig.h>
#include <kapp.h>
#include <kquickhelp.h>

// Local includes
#include "EmpathComposeSettingsDialog.h"
#include "EmpathConfig.h"
#include "Empath.h"
#include "EmpathUtilities.h"
#include "RikGroupBox.h"

bool EmpathComposeSettingsDialog::exists_ = false;

	void
EmpathComposeSettingsDialog::create()
{
	if (exists_) return;
	exists_ = true;
	EmpathComposeSettingsDialog * d = new EmpathComposeSettingsDialog(0, 0);
	CHECK_PTR(d);
	d->show();
	kapp->processEvents();
	d->loadData();
}

EmpathComposeSettingsDialog::EmpathComposeSettingsDialog(
		QWidget * parent,
		const char * name)
	:	QDialog(parent, name, false),
		applied_(false)
{
	empathDebug("ctor");
	setCaption(i18n("Compose Settings - ") + kapp->getCaption());

	QLineEdit	tempLineEdit((QWidget *)0);
	Q_UINT32 h	= tempLineEdit.sizeHint().height();
	
	rgb_phrases_	= new RikGroupBox(i18n("Phrases"), 8, this, "rgb_phrases");
	CHECK_PTR(rgb_phrases_);
	
	rgb_msg_		= new RikGroupBox(i18n("Message"), 8, this, "rgb_msg");
	CHECK_PTR(rgb_msg_);
	
	rgb_when_		= new RikGroupBox(i18n("When to send"), 8, this, "rgb_when");
	CHECK_PTR(rgb_when_);
	
	w_phrases_	= new QWidget(rgb_phrases_,	"w_phrases");
	CHECK_PTR(w_phrases_);
	
	w_msg_		= new QWidget(rgb_msg_,		"w_msg");
	CHECK_PTR(w_msg_);
	
	w_when_		= new QWidget(rgb_when_,	"w_when");
	CHECK_PTR(w_when_);
	
	rgb_phrases_->setWidget(w_phrases_);
	rgb_msg_->setWidget(w_msg_);
	rgb_when_->setWidget(w_when_);

	l_extra_	= new QLabel(i18n("Extra headers to edit:"), this, "l_extra");
	CHECK_PTR(l_extra_);
	
	le_extra_	= new QLineEdit(this, "le_extra");
	CHECK_PTR(le_extra_);
	
	// Phrases
	
	l_reply_	=
		new QLabel(i18n("Reply to sender:"), w_phrases_, "l_reply");
	CHECK_PTR(l_reply_);
	
	le_reply_	=
		new QLineEdit(w_phrases_, "le_reply");
	CHECK_PTR(le_reply_);
	
	KQuickHelp::add(le_reply_, i18n(
			"Choose the phrase that will be added\n"
			"before a message when you reply.\n"
			"%s will be replaced by the name of the\n"
			"person who sent you the message.\n"
			"%d will be replaced by the date that\n"
			"the message was sent to you."));
	
	le_reply_->setFixedHeight(h);
	
	l_replyAll_	=
		new QLabel(i18n("Reply to All:"), w_phrases_, "l_replyAll");
	CHECK_PTR(l_replyAll_);
	
	le_replyAll_	=
		new QLineEdit(w_phrases_, "le_replyAll");
	CHECK_PTR(le_replyAll_);
	
	KQuickHelp::add(le_replyAll_, i18n(
			"Choose the phrase that will be added\n"
			"before a message when you reply.\n"
			"%s will be replaced by the name of the\n"
			"person who sent you the message.\n"
			"%d will be replaced by the date that\n"
			"the message was sent to you."));
	
	le_replyAll_->setFixedHeight(h);
	
	l_forward_	=
		new QLabel(i18n("Forward:"), w_phrases_, "l_forward");
	CHECK_PTR(l_forward_);
	
	le_forward_	=
		new QLineEdit(w_phrases_, "le_forward");
	CHECK_PTR(le_forward_);
	
	KQuickHelp::add(le_forward_, i18n(
			"Choose the phrase that will be added\n"
			"before a message when you forward the\n"
			"message elsewhere.\n"
			"%s will be replaced by the name of the\n"
			"person who sent you the message.\n"
			"%d will be replaced by the date that\n"
			"the message was sent to you."));
	
	le_forward_->setFixedHeight(h);

	// Quoting
		
	cb_quote_	=
		new QCheckBox(i18n("Automatically &quote message when replying"),
				w_msg_, "cb_quote");
	CHECK_PTR(cb_quote_);
	
	KQuickHelp::add(cb_quote_, i18n(
			"With this selected, when you reply to a\n"
			"message, you'll get the text of the message\n"
			"you're replying to in the compose window.\n"
			"This can make it easier to provide context.\n"
			"The text is quoted with '> '\n"));
	
	cb_quote_->setFixedHeight(h);

	cb_addSig_	=
		new QCheckBox(i18n("Add &signature"), w_msg_, "l_addSig");
	CHECK_PTR(cb_addSig_);
	
	KQuickHelp::add(cb_addSig_, i18n(
			"With this selected, your signature will be\n"
			"automatically added to the end of all messages\n"
			"you send. If you don't have a signature, use\n"
			"Options->Sending to set one up.\n"
			"The text '--' will be prepended to your signature.\n"
			"This is standard behaviour and will allow other\n"
			"people's mail programs to recognise where your\n"
			"message ends, and your signature starts."));
	
	cb_addSig_->setFixedHeight(h);
	
	cb_digSign_	=
		new QCheckBox(i18n("Add &digital Signature"), w_msg_, "cb_digSign");
	CHECK_PTR(cb_digSign_);
	
	KQuickHelp::add(cb_digSign_, i18n(
			"Adding a digital signature to a message is\n"
			"a bit like adding your real signature, a\n"
			"photograph, your fingerprint, and some hair\n"
			"(for DNA analysis). In other words, you're\n"
			"saying that this message definitely came from\n"
			"you. This helps stops people masquerading as you."));

	cb_digSign_->setFixedHeight(h);
	
	cb_wrap_	=
		new QCheckBox(i18n("Wrap &long lines at column number"), w_msg_,
				"cb_wrap");
	CHECK_PTR(cb_wrap_);
	
	KQuickHelp::add(cb_wrap_, i18n(
			"When you type a reply, you probably don't\n"
			"press <b>Return</b> at the end of each line.\n"
			"Internet mail messages are supposed to be read\n"
			"in a fixed font, and any lines longer than 80\n"
			"characters may not be 'wrapped around'.\n"
			"This option will add carriage returns in lines\n"
			"longer than the value set in the spin box.\n"
			"This will ensure that messages look correct when\n"
			"received by your recipient.\n"));
	
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
		new QRadioButton(i18n("Send messages &immediately"), w_when_,
				"rb_sendNow");
	CHECK_PTR(rb_sendNow_);
	
	KQuickHelp::add(rb_sendNow_, i18n(
			"If you choose this option, Empath will\n"
			"try to send your message immediately when\n"
			"you press send. Note that you won't have to\n"
			"stop using Empath when you press send. This\n"
			"is useful when you are permanently connected\n"
			"to a mail server, as there's no delay between\n"
			"pressing send and the message being despatched."));

	rb_sendNow_->setFixedHeight(h);

	rb_sendNow_->setChecked(true);
	
	rb_sendLater_	=
		new QRadioButton(i18n("Send messages &later"), w_when_, "rb_sendLater");
	CHECK_PTR(rb_sendLater_);
	
	KQuickHelp::add(rb_sendLater_, i18n(
			"By choosing this option, when you press\n"
			"'send' while composing a message, the\n"
			"message will be placed in a queue, and\n"
			"will only be despatched later on, when\n"
			"you say so. This is useful when you don't\n"
			"have a permanent connection to a mail server.\n"));
	
	rb_sendLater_->setFixedHeight(h);
	
	buttonGroup_->insert(rb_sendNow_,	SendNow);
	buttonGroup_->insert(rb_sendLater_,	SendLater);
	
	cb_externalEditor_ = new QCheckBox(i18n("Use external editor"),
		this, "cb_useExternalEditor");
	
	KQuickHelp::add(cb_externalEditor_, i18n(
			"By selecting this option, you allow the use\n"
			"of your favourite text editor to type messages.\n"
			"The internal editor, while useful, is not incredibly\n"
			"powerful. Some people prefer their usual editor as\n"
			"they need the extra features, or are simply used to it."));

	CHECK_PTR(cb_externalEditor_);
	
	cb_externalEditor_->setFixedHeight(h);
	cb_externalEditor_->setFixedWidth(cb_externalEditor_->sizeHint().width());
	
	le_externalEditor_ = new QLineEdit(this, "le_externalEditor");
	le_externalEditor_->setFixedHeight(h);

	KQuickHelp::add(cb_externalEditor_, i18n(
			"Type here the command line to run your editor.\n"
			"This may be, for example, <b>gvim</b> or <b>emacs</b>"));


///////////////////////////////////////////////////////////////////////////////
// Button box

	buttonBox_	= new KButtonBox(this);
	CHECK_PTR(buttonBox_);

	buttonBox_->setFixedHeight(h);
	
	pb_help_	= buttonBox_->addButton(i18n("&Help"));	
	CHECK_PTR(pb_help_);
	
	pb_default_	= buttonBox_->addButton(i18n("&Default"));	
	CHECK_PTR(pb_default_);
	
	buttonBox_->addStretch();
	
	pb_OK_		= buttonBox_->addButton(i18n("&OK"));
	CHECK_PTR(pb_OK_);
	
	pb_OK_->setDefault(true);
	
	pb_apply_	= buttonBox_->addButton(i18n("&Apply"));
	CHECK_PTR(pb_apply_);
	
	pb_cancel_	= buttonBox_->addButton(i18n("&Cancel"));
	CHECK_PTR(pb_cancel_);
	
	buttonBox_->layout();
	
	QObject::connect(pb_OK_,		SIGNAL(clicked()),	SLOT(s_OK()));
	QObject::connect(pb_default_,	SIGNAL(clicked()),	SLOT(s_default()));
	QObject::connect(pb_apply_,		SIGNAL(clicked()),	SLOT(s_apply()));
	QObject::connect(pb_cancel_,	SIGNAL(clicked()),	SLOT(s_cancel()));
	QObject::connect(pb_help_,		SIGNAL(clicked()),	SLOT(s_help()));
/////////////////////////////////////////////////////////////////////////////


	// Layouts
	
	topLevelLayout_				= new QGridLayout(this, 5, 2, 10, 10);
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
	
	topLevelLayout_->addWidget(l_extra_,				0, 0);
	topLevelLayout_->addWidget(le_extra_,				0, 1);
	topLevelLayout_->addMultiCellWidget(rgb_phrases_,	1, 1, 0, 1);
	topLevelLayout_->addMultiCellWidget(rgb_msg_,		2, 2, 0, 1);
	topLevelLayout_->addMultiCellWidget(rgb_when_,		3, 3, 0, 1);
	topLevelLayout_->addWidget(cb_externalEditor_,		4, 0);
	topLevelLayout_->addWidget(le_externalEditor_,		4, 1);
	topLevelLayout_->addMultiCellWidget(buttonBox_,		5, 5, 0, 1);

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
	
	topLevelLayout_->activate();
	
	setMinimumSize(minimumSizeHint());
	resize(minimumSizeHint());
}

	void
EmpathComposeSettingsDialog::saveData()
{
	KConfig * c	= KGlobal::config();
	c->setGroup(EmpathConfig::GROUP_COMPOSE);
#define CWE c->writeEntry
	CWE( EmpathConfig::KEY_EXTRA_HEADERS,			le_extra_->text());
	CWE( EmpathConfig::KEY_PHRASE_REPLY_SENDER,		le_reply_->text());
	CWE( EmpathConfig::KEY_PHRASE_REPLY_ALL,		le_replyAll_->text());
	CWE( EmpathConfig::KEY_PHRASE_FORWARD,			le_forward_->text());
	CWE( EmpathConfig::KEY_AUTO_QUOTE,				cb_quote_->isChecked());
	CWE( EmpathConfig::KEY_ADD_SIG,					cb_addSig_->isChecked());
	CWE( EmpathConfig::KEY_ADD_DIG_SIG,				cb_digSign_->isChecked());
	CWE( EmpathConfig::KEY_WRAP_LINES,				cb_wrap_->isChecked());
	CWE( EmpathConfig::KEY_WRAP_COLUMN,				sb_wrap_->value());
	CWE( EmpathConfig::KEY_SEND_POLICY,
		(unsigned long)(rb_sendNow_->isChecked() ? SendNow : SendLater));
	
	CWE( EmpathConfig::KEY_USE_EXTERNAL_EDITOR,cb_externalEditor_->isChecked());
	CWE( EmpathConfig::KEY_EXTERNAL_EDITOR,		le_externalEditor_->text());
#undef CWE
}

	void
EmpathComposeSettingsDialog::loadData()
{
	KConfig * c	= KGlobal::config();
	c->setGroup(EmpathConfig::GROUP_COMPOSE);
	
	le_extra_->setText(
		c->readEntry(EmpathConfig::KEY_EXTRA_HEADERS));
	
	le_reply_->setText(
		c->readEntry(EmpathConfig::KEY_PHRASE_REPLY_SENDER, i18n("%s wrote:")));
	
	le_replyAll_->setText(
		c->readEntry(EmpathConfig::KEY_PHRASE_REPLY_ALL, i18n("%s wrote:")));
	
	le_forward_->setText(c->readEntry(EmpathConfig::KEY_PHRASE_FORWARD,
		i18n("Forwarded message from %s")));

	cb_quote_->setChecked(
		c->readBoolEntry(EmpathConfig::KEY_AUTO_QUOTE, true));
	
	cb_addSig_->setChecked(
		c->readBoolEntry(EmpathConfig::KEY_ADD_SIG, true));

	cb_digSign_->setChecked(
		c->readBoolEntry(EmpathConfig::KEY_ADD_DIG_SIG, false));

	cb_wrap_->setChecked(
		c->readBoolEntry(EmpathConfig::KEY_WRAP_LINES, true));
	
	sb_wrap_->setValue(
		c->readNumEntry(EmpathConfig::KEY_WRAP_COLUMN, 76));
	
	rb_sendNow_->setChecked((
		(SendPolicy)
		 c->readNumEntry(EmpathConfig::KEY_SEND_POLICY, SendLater)) == SendNow);

	rb_sendLater_->setChecked(
		!rb_sendNow_->isChecked());
	
	cb_externalEditor_->setChecked(
		c->readBoolEntry(EmpathConfig::KEY_USE_EXTERNAL_EDITOR, false));
	
	le_externalEditor_->setText(
		c->readEntry(EmpathConfig::KEY_EXTERNAL_EDITOR, "gvim"));
}

	void
EmpathComposeSettingsDialog::s_OK()
{
	hide();
	if (!applied_)
		s_apply();
	KGlobal::config()->sync();
	delete this;
}

	void
EmpathComposeSettingsDialog::s_help()
{
	empathInvokeHelp(QString::null, QString::null);
}

	void
EmpathComposeSettingsDialog::s_apply()
{
	if (applied_) {
		pb_apply_->setText(i18n("&Apply"));
		KGlobal::config()->rollback(true);
		KGlobal::config()->reparseConfiguration();
		loadData();
		applied_ = false;
	} else {
		pb_apply_->setText(i18n("&Revert"));
		pb_cancel_->setText(i18n("&Close"));
		applied_ = true;
	}
	saveData();
}

	void
EmpathComposeSettingsDialog::s_default()
{
	le_extra_			->	setText		(QString::null);
	le_reply_			->	setText		(i18n("%s wrote:"));
	le_replyAll_		->	setText		(i18n("%s wrote:"));
	le_forward_			->	setText		(i18n("Forwarded message from %s"));
	cb_quote_			->	setChecked	(true);
	cb_addSig_			->	setChecked	(true);
	cb_digSign_			->	setChecked	(false);
	cb_wrap_			->	setChecked	(true);
	sb_wrap_			->	setValue	(76);
	rb_sendNow_			->	setChecked	(false);
	rb_sendLater_		->	setChecked	(true);
	cb_externalEditor_	->	setChecked	(false);
	le_externalEditor_	->	setText		("gvim");
}
	
	void
EmpathComposeSettingsDialog::s_cancel()
{
	if (!applied_)
		KGlobal::config()->rollback(true);
	delete this;
}

	void
EmpathComposeSettingsDialog::closeEvent(QCloseEvent * e)
{
	e->accept();
	delete this;
}

