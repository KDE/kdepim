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
# pragma implementation "EmpathSendingSettingsDialog.h"
#endif

#include <qpixmap.h>

// KDE includes
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kapp.h>
#include <kquickhelp.h>

// Local includes
#include "EmpathSendingSettingsDialog.h"
#include "EmpathAddressSelectionWidget.h"
#include "EmpathFolderChooserWidget.h"
#include "EmpathConfig.h"
#include "EmpathUtilities.h"
#include "EmpathUIUtils.h"
#include "Empath.h"
#include "RikGroupBox.h"
	
bool EmpathSendingSettingsDialog::exists_ = false;

	void
EmpathSendingSettingsDialog::create()
{
	if (exists_) return;
	exists_ = true;
	EmpathSendingSettingsDialog * d = new EmpathSendingSettingsDialog(0, 0);
	CHECK_PTR(d);
	d->show();
	d->loadData();
}

	
EmpathSendingSettingsDialog::EmpathSendingSettingsDialog(
		QWidget * parent,
		const char * name)
	:	QDialog(parent, name, false),
		applied_(false)
{
	empathDebug("ctor");
	setCaption(i18n("Sending Settings - ") + kapp->getCaption());
	
	QLineEdit	tempLineEdit((QWidget *)0);
	Q_UINT32 h	= tempLineEdit.sizeHint().height();

	rgb_queuing_	= new RikGroupBox(i18n("Queue and Store"), 8, this, "rgb_queuing");
	CHECK_PTR(rgb_queuing_);
	
	rgb_server_	= new RikGroupBox(i18n("Server"), 8, this, "rgb_server");
	CHECK_PTR(rgb_server_);
	
	rgb_copies_		= new RikGroupBox(i18n("Copies of outgoing messages"), 8, this, "rgb_copies");
	CHECK_PTR(rgb_copies_);

	w_server_	= new QWidget(rgb_server_,	"w_server");
	CHECK_PTR(w_server_);
	
	w_copies_		= new QWidget(rgb_copies_,	"w_copies");
	CHECK_PTR(w_copies_);
	
	w_queuing_		= new QWidget(rgb_queuing_,	"w_queuing");
	CHECK_PTR(w_queuing_);
	
	rgb_queuing_->setWidget(w_queuing_);
	rgb_server_->setWidget(w_server_);
	rgb_copies_->setWidget(w_copies_);
	
	// Queuing
	
	l_queueFolder_ =
		new QLabel(i18n("Message queue folder"), w_queuing_, "l_queuing");
	CHECK_PTR(l_queueFolder_);
	
	l_queueFolder_->setFixedHeight(h);
	
	fcw_queueFolder_ =
		new EmpathFolderChooserWidget(w_queuing_, "fcw_queueFolder");
	CHECK_PTR(fcw_queueFolder_);
	
	// Sent mail
	
	l_sentFolder_ =
		new QLabel(i18n("Sent mail folder"), w_queuing_, "l_sentFolder");
	CHECK_PTR(l_sentFolder_);
	
	l_sentFolder_->setFixedHeight(h);
	
	fcw_sentFolder_ =
		new EmpathFolderChooserWidget(w_queuing_, "fcw_sentFolder");
	CHECK_PTR(fcw_sentFolder_);
	
	// Server

	serverButtonGroup_	=
		new QButtonGroup(this, "serverButtonGroup");
	CHECK_PTR(serverButtonGroup_);

	serverButtonGroup_->hide();
	serverButtonGroup_->setExclusive(true);

	rb_sendmail_		=
		new QRadioButton("Sendmail", w_server_,
				"rb_sendmail");
	CHECK_PTR(rb_sendmail_);

	KQuickHelp::add(rb_sendmail_, i18n(
			"Here you may elect to use sendmail to send\n"
			"all outgoing mail. You must fill in the full\n"
			"path to the sendmail program.\n"
			"If you don't know the full path, you can try\n"
			"typing 'which sendmail' (without the quotes)\n"
			"at a command prompt."));

	rb_sendmail_->setFixedHeight(h);

	le_sendmail_		=
		new QLineEdit(w_server_, "le_sendmail");
	CHECK_PTR(le_sendmail_);
	
	KQuickHelp::add(le_sendmail_, i18n(
			"Here you may elect to use qmail to send\n"
			"all outgoing mail. You must fill in the full\n"
			"path to the qmail program.\n"));


	le_sendmail_->setFixedHeight(h);

	pb_sendmailBrowse_	=
		new QPushButton(w_server_, "pb_sendmailBrowse");
	CHECK_PTR(pb_sendmailBrowse_);
	pb_sendmailBrowse_->setPixmap(empathIcon("browse.png"));
	
	pb_sendmailBrowse_->setFixedSize(h, h);

	QObject::connect(rb_sendmail_, SIGNAL(toggled(bool)),
			le_sendmail_, SLOT(setEnabled(bool)));
	
	QObject::connect(rb_sendmail_, SIGNAL(toggled(bool)),
			pb_sendmailBrowse_, SLOT(setEnabled(bool)));
	
	rb_qmail_		=
		new QRadioButton("Qmail", w_server_,
				"rb_qmail");
	CHECK_PTR(rb_qmail_);
	
	KQuickHelp::add(rb_qmail_, i18n(
			"Here you may elect to use sendmail to send\n"
			"all outgoing mail. You must fill in the full\n"
			"path to the sendmail program.\n"
			"If you don't know the full path, you can try\n"
			"typing 'which sendmail' (without the quotes)\n"
			"at a command prompt."));



	rb_qmail_->setMaximumHeight(h);
	rb_qmail_->setMinimumHeight(h);

	le_qmail_		=
		new QLineEdit(w_server_, "le_qmail");
	CHECK_PTR(le_qmail_);
	
	KQuickHelp::add(le_qmail_, i18n(
			"Here you may elect to use sendmail to send\n"
			"all outgoing mail. You must fill in the full\n"
			"path to the sendmail program.\n"
			"If you don't know the full path, you can try\n"
			"typing 'which sendmail' (without the quotes)\n"
			"at a command prompt."));



	le_qmail_->setMaximumHeight(h);
	le_qmail_->setMinimumHeight(h);

	pb_qmailBrowse_	=
		new QPushButton(w_server_, "pb_qmailBrowse");
	CHECK_PTR(pb_qmailBrowse_);
	pb_qmailBrowse_->setPixmap(empathIcon("browse.png"));
	
	pb_qmailBrowse_->setFixedSize(h, h);

	QObject::connect(rb_qmail_, SIGNAL(toggled(bool)),
			le_qmail_, SLOT(setEnabled(bool)));
	
	QObject::connect(rb_qmail_, SIGNAL(toggled(bool)),
			pb_qmailBrowse_, SLOT(setEnabled(bool)));
	
	rb_smtp_		=
		new QRadioButton("SMTP", w_server_,
				"rb_smtp");
	CHECK_PTR(rb_smtp_);
	
	rb_smtp_->setFixedHeight(h);

	KQuickHelp::add(rb_smtp_, i18n(
			"Here you may elect to use SMTP to send\n"
			"all outgoing mail. This generally means\n"
			"that mail is sent over a network (or via\n"
			"the internet. However, you may also use an\n"
			"SMTP server on your local machine, for\n"
			"instance 'sendmail'. If you have sendmail\n"
			"on your machine, you can just use 'sendmail'\n"
			"above, of course.\n\n"
			"Note that you must fill in the address of the\n"
			"sendmail server (e.g. <b>localhost</b> for the\n"
			"local machine, and the port that the server\n"
			"accepts connections on. The port is usually 25\n"
			"so you can probably leave it alone."));


	le_smtpServer_		=
		new QLineEdit(w_server_, "le_smtp");
	CHECK_PTR(le_smtpServer_);
	
	KQuickHelp::add(le_smtpServer_, i18n(
			"Here you may elect to use SMTP to send\n"
			"all outgoing mail. This generally means\n"
			"that mail is sent over a network (or via\n"
			"the internet. However, you may also use an\n"
			"SMTP server on your local machine, for\n"
			"instance 'sendmail'. If you have sendmail\n"
			"on your machine, you can just use 'sendmail'\n"
			"above, of course.\n\n"
			"Note that you must fill in the address of the\n"
			"sendmail server (e.g. <b>localhost</b> for the\n"
			"local machine, and the port that the server\n"
			"accepts connections on. The port is usually 25\n"
			"so you can probably leave it alone."));


	le_smtpServer_->setFixedHeight(h);

	l_smtpServerPort_	=
		new QLabel(i18n("Port:"), w_server_, "l_smtpServerPort");
	CHECK_PTR(l_smtpServerPort_);
	
	KQuickHelp::add(l_smtpServerPort_, i18n(
			"Here you may elect to use SMTP to send\n"
			"all outgoing mail. This generally means\n"
			"that mail is sent over a network (or via\n"
			"the internet. However, you may also use an\n"
			"SMTP server on your local machine, for\n"
			"instance 'sendmail'. If you have sendmail\n"
			"on your machine, you can just use 'sendmail'\n"
			"above, of course.\n\n"
			"Note that you must fill in the address of the\n"
			"sendmail server (e.g. <b>localhost</b> for the\n"
			"local machine, and the port that the server\n"
			"accepts connections on. The port is usually 25\n"
			"so you can probably leave it alone."));



	l_smtpServerPort_->setFixedWidth(l_smtpServerPort_->sizeHint().width());
	l_smtpServerPort_->setFixedHeight(h);

	sb_smtpPort_	=
		new QSpinBox(1, 99999999, 1, w_server_, "sb_smtpPort");
	CHECK_PTR(sb_smtpPort_);

	sb_smtpPort_->setValue(25);

	sb_smtpPort_->setFixedHeight(h);

	QObject::connect(rb_smtp_, SIGNAL(toggled(bool)),
			le_smtpServer_, SLOT(setEnabled(bool)));
	
	QObject::connect(rb_smtp_, SIGNAL(toggled(bool)),
			l_smtpServerPort_, SLOT(setEnabled(bool)));
	
	QObject::connect(rb_smtp_, SIGNAL(toggled(bool)),
			sb_smtpPort_, SLOT(setEnabled(bool)));
	
	serverButtonGroup_->insert(rb_sendmail_,	0);
	serverButtonGroup_->insert(rb_qmail_,		1);
	serverButtonGroup_->insert(rb_smtp_,		2);
	
	rb_sendmail_->setChecked(true);	
	le_qmail_->setEnabled(false);
	pb_qmailBrowse_->setEnabled(false);
	le_smtpServer_->setEnabled(false);
	l_smtpServerPort_->setEnabled(false);
	sb_smtpPort_->setEnabled(false);

	// Copies
	
	cb_copyOther_	=
		new QCheckBox(i18n("Send a copy &to:"), w_copies_,
				"cb_copyOther");
	CHECK_PTR(cb_copyOther_);
	
	KQuickHelp::add(cb_copyOther_, i18n(
			"If you choose this option, all outgoing messages\n"
			"are also sent to the address you type."));
	
	cb_copyOther_->setFixedHeight(h);
	cb_copyOther_->setMinimumWidth(cb_copyOther_->sizeHint().width());

	asw_copyOther_	=
		new EmpathAddressSelectionWidget(w_copies_, "asw_copyOther");
	CHECK_PTR(asw_copyOther_);
	
	KQuickHelp::add(asw_copyOther_, i18n(
			"If you choose this option, all outgoing messages\n"
			"are also sent to the address you type."));
	
	asw_copyOther_->setFixedHeight(h);
	
	QObject::connect(cb_copyOther_, SIGNAL(toggled(bool)),
			asw_copyOther_, SLOT(setEnabled(bool)));
	
	asw_copyOther_->setEnabled(false);
	
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
	
	empathDebug("Doing layout");
	topLevelLayout_				= new QGridLayout(this, 4, 1, 10, 10);
	CHECK_PTR(topLevelLayout_);

	topLevelLayout_->setRowStretch(0, 2);
	topLevelLayout_->setRowStretch(1, 3);
	topLevelLayout_->setRowStretch(2, 2);

	queuingGroupLayout_		= new QGridLayout(w_queuing_,	2, 2, 0, 10);
	CHECK_PTR(queuingGroupLayout_);
	
	queuingGroupLayout_->addWidget(l_queueFolder_,		0, 0);
	queuingGroupLayout_->addWidget(fcw_queueFolder_,	0, 1);
	queuingGroupLayout_->addWidget(l_sentFolder_,		1, 0);
	queuingGroupLayout_->addWidget(fcw_sentFolder_,		1, 1);
	
	queuingGroupLayout_->activate();

	serverGroupLayout_		= new QGridLayout(w_server_,	3, 5, 0, 10);
	CHECK_PTR(serverGroupLayout_);
	
	serverGroupLayout_->setColStretch(0, 2);
	serverGroupLayout_->setColStretch(1, 5);
	serverGroupLayout_->setColStretch(2, 2);
	serverGroupLayout_->setColStretch(3, 1);
	serverGroupLayout_->setColStretch(4, 1);
	
	copiesGroupLayout_		= new QGridLayout(w_copies_,	1, 2, 0, 10);
	CHECK_PTR(copiesGroupLayout_);

	topLevelLayout_->addWidget(rgb_queuing_,	0, 0);
	topLevelLayout_->addWidget(rgb_server_,		1, 0);
	topLevelLayout_->addWidget(rgb_copies_,		2, 0);
	topLevelLayout_->addWidget(buttonBox_,		3, 0);

	serverGroupLayout_->addWidget(rb_sendmail_,		0, 0);
	serverGroupLayout_->addWidget(rb_qmail_,		1, 0);
	serverGroupLayout_->addWidget(rb_smtp_,			2, 0);

	serverGroupLayout_->addMultiCellWidget(le_sendmail_,	0, 0, 1, 3);
	serverGroupLayout_->addMultiCellWidget(le_qmail_,		1, 1, 1, 3);
	serverGroupLayout_->addWidget(le_smtpServer_,			2, 1);
	
	serverGroupLayout_->addWidget(pb_sendmailBrowse_,		0, 4);
	serverGroupLayout_->addWidget(pb_qmailBrowse_,			1, 4);

	serverGroupLayout_->addWidget(l_smtpServerPort_,		2, 2);
	
	serverGroupLayout_->addMultiCellWidget(sb_smtpPort_,	2, 2, 3, 4);

	serverGroupLayout_->activate();
	
	copiesGroupLayout_->addWidget(cb_copyOther_,			1, 0);
	copiesGroupLayout_->addWidget(asw_copyOther_,			1, 1);
	
	copiesGroupLayout_->activate();
	
	topLevelLayout_->activate();
	
	setMinimumSize(minimumSizeHint());
	resize(minimumSizeHint());
	empathDebug("Done layout");
}

	void
EmpathSendingSettingsDialog::saveData()
{
	KConfig * c	= KGlobal::config();
	c->setGroup(EmpathConfig::GROUP_SENDING);
	
#define CWE c->writeEntry

	// Get server type
	int servType = (int)(
		(rb_sendmail_->isChecked()	? EmpathMailSender::Sendmail	: 0) |
		(rb_qmail_->isChecked()		? EmpathMailSender::Qmail		: 0) |
		(rb_smtp_->isChecked()		? EmpathMailSender::SMTP		: 0));
	
	CWE( EmpathConfig::KEY_OUTGOING_SERVER_TYPE,servType); 
	
	CWE( EmpathConfig::KEY_SENDMAIL_LOCATION,	le_sendmail_->text());
	
	CWE( EmpathConfig::KEY_QMAIL_LOCATION,		le_qmail_->text());
	
	CWE( EmpathConfig::KEY_SMTP_SERVER_LOCATION,le_smtpServer_->text());

	CWE( EmpathConfig::KEY_SMTP_SERVER_PORT,	sb_smtpPort_->value());
	
	CWE( EmpathConfig::KEY_CC_OTHER,			cb_copyOther_->isChecked());
	
	CWE( EmpathConfig::KEY_CC_OTHER_ADDRESS,asw_copyOther_->selectedAddress());
	
	CWE( EmpathConfig::KEY_QUEUE_FOLDER,
		fcw_queueFolder_->selectedURL().asString());
	
	CWE( EmpathConfig::KEY_SENT_FOLDER,
		fcw_sentFolder_->selectedURL().asString());
	
	empath->updateOutgoingServer();
	
#undef CWE
}

	void
EmpathSendingSettingsDialog::loadData()
{
	KConfig * c	= KGlobal::config();
	c->setGroup(EmpathConfig::GROUP_SENDING);
	
	EmpathMailSender::OutgoingServerType t =
		(EmpathMailSender::OutgoingServerType)
		(c->readNumEntry(EmpathConfig::KEY_OUTGOING_SERVER_TYPE));
	
	rb_sendmail_->setChecked(	t == EmpathMailSender::Sendmail);
	rb_qmail_->setChecked(		t == EmpathMailSender::Qmail);
	rb_smtp_->setChecked(		t == EmpathMailSender::SMTP);
	
	le_sendmail_->setText(
		c->readEntry(EmpathConfig::KEY_SENDMAIL_LOCATION));
	
	le_qmail_->setText(
		c->readEntry(EmpathConfig::KEY_QMAIL_LOCATION));
	
	le_smtpServer_->setText(
		c->readEntry(EmpathConfig::KEY_SMTP_SERVER_LOCATION));
	
	sb_smtpPort_->setValue(
		c->readNumEntry(EmpathConfig::KEY_SMTP_SERVER_PORT));
	
	cb_copyOther_->setChecked(
		c->readBoolEntry(EmpathConfig::KEY_CC_OTHER));
	
	asw_copyOther_->setAddress(
		c->readEntry(EmpathConfig::KEY_CC_OTHER_ADDRESS));
	
	fcw_queueFolder_->setURL(
		c->readEntry(EmpathConfig::KEY_QUEUE_FOLDER));
	
	fcw_sentFolder_->setURL(
		c->readEntry(EmpathConfig::KEY_SENT_FOLDER));
	
	empath->updateOutgoingServer();
}

	void
EmpathSendingSettingsDialog::s_OK()
{
	hide();
	if (!applied_)
		s_apply();
	KGlobal::config()->sync();
	delete this;
}

	void
EmpathSendingSettingsDialog::s_help()
{
	//empathInvokeHelp(QString::null, QString::null);
}

	void
EmpathSendingSettingsDialog::s_apply()
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
EmpathSendingSettingsDialog::s_default()
{
	le_sendmail_->setText("/usr/lib/sendmail");
	le_qmail_->setText("/var/qmail/bin/qmail-inject");
	cb_copyOther_->setChecked(false);
	rb_smtp_->setChecked(false);
	rb_qmail_->setChecked(false);
	rb_sendmail_->setChecked(true);
	le_smtpServer_->setText("localhost");
	sb_smtpPort_->setValue(25);
}
	
	void
EmpathSendingSettingsDialog::s_cancel()
{
	if (!applied_)
		KGlobal::config()->rollback(true);
	delete this;
}

	void
EmpathSendingSettingsDialog::closeEvent(QCloseEvent * e)
{
	e->accept();
	delete this;
}

