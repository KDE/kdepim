/*
    Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
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
#include <qwhatsthis.h>

// KDE includes
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kapp.h>

// Local includes
#include "EmpathSendingSettingsDialog.h"
#include "EmpathAddressSelectionWidget.h"
#include "EmpathFolderChooserWidget.h"
#include "EmpathSeparatorWidget.h"
#include "EmpathConfig.h"
#include "EmpathUtilities.h"
#include "EmpathMailSender.h"
#include "EmpathUIUtils.h"
#include "Empath.h"
    
EmpathSendingSettingsDialog::EmpathSendingSettingsDialog(QWidget * parent)
    :   KDialog(parent, "SendingSettings", true),
        applied_(false)
{
    setCaption(i18n("Sending Settings"));
    
    // Server

    bg_server_  = new QButtonGroup(this);
    bg_server_->hide();

    rb_sendmail_    = new QRadioButton(i18n("Sendmail"), this, "rb_sendmail");
    rb_qmail_       = new QRadioButton(i18n("Qmail"), this, "rb_qmail");
    rb_smtp_        = new QRadioButton(i18n("SMTP"), this, "rb_smtp");

    efsw_sendmail_  = new EmpathFileSelectWidget(QString::null, this);
    efsw_qmail_     = new EmpathFileSelectWidget(QString::null, this);
    le_smtpServer_  = new QLineEdit(this, "le_smtp");

    QLabel * l_smtpPort =
        new QLabel(i18n("Port:"), this, "l_smtpPort");
    
    sb_smtpPort_ = new QSpinBox(1, 99999999, 1, this, "sb_smtpPort");
    sb_smtpPort_->setValue(25);
   
    bg_server_->insert(rb_sendmail_, EmpathMailSender::Sendmail);
    bg_server_->insert(rb_qmail_,    EmpathMailSender::Qmail);
    bg_server_->insert(rb_smtp_,     EmpathMailSender::SMTP);
    
    // Copies
    
    cb_copyOther_ =
        new QCheckBox(i18n("Send a copy &to:"), this, "cb_copyOther");
    
    asw_copyOther_ = new EmpathAddressSelectionWidget(this);
    
#include "EmpathDialogButtonBox.cpp"

    // Layout
    
    QVBoxLayout * layout = new QVBoxLayout(this, 10, 10);

    QGridLayout * layout0 = new QGridLayout(layout);
    QHBoxLayout * layout1 = new QHBoxLayout;

    layout0->addWidget(rb_sendmail_,    0, 0);
    layout0->addWidget(rb_qmail_,       1, 0);
    layout0->addWidget(rb_smtp_,        2, 0);

    layout0->addWidget(efsw_sendmail_,  0, 1);
    layout0->addWidget(efsw_qmail_,     1, 1);
    layout0->addLayout(layout1,         2, 1);

    layout1->addWidget(le_smtpServer_);
    layout1->addWidget(l_smtpPort);
    layout1->addWidget(sb_smtpPort_);

    layout->addWidget(new EmpathSeparatorWidget(this));

    QHBoxLayout * layout2 = new QHBoxLayout(layout);
    layout2->addWidget(cb_copyOther_);
    layout2->addWidget(asw_copyOther_);
    
    layout->addStretch(10);
    layout->addWidget(new EmpathSeparatorWidget(this));
    layout->addWidget(buttonBox_);

    // WhatsThis
    
    QString aboutSendmail = i18n(
        "Here you may elect to use sendmail to send\n"
        "all outgoing mail. You must fill in the full\n"
        "path to the sendmail program.\n"
        "If you don't know the full path, you can try\n"
        "typing `which sendmail' (without the quotes)\n"
        "at a command prompt.");

    QString aboutQmail = i18n(
        "Here you may elect to use qmail to send\n"
        "all outgoing mail. You must fill in the full\n"
        "path to the qmail-inject program.\n");

    QString aboutSMTP = i18n(
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
        "so you can probably leave it alone.");

    QString aboutCopyOther = i18n(
        "If you choose this option, all outgoing messages\n"
        "are also sent to the address you type.");

    QWhatsThis::add(rb_sendmail_,   aboutSendmail);
    QWhatsThis::add(efsw_sendmail_, aboutSendmail);
    QWhatsThis::add(rb_qmail_,      aboutQmail);
    QWhatsThis::add(efsw_qmail_,    aboutQmail);
    QWhatsThis::add(rb_smtp_,       aboutSMTP);
    QWhatsThis::add(le_smtpServer_, aboutSMTP);
    QWhatsThis::add(l_smtpPort,     aboutSMTP);
    QWhatsThis::add(sb_smtpPort_,   aboutSMTP);
    QWhatsThis::add(cb_copyOther_,  aboutCopyOther);
    QWhatsThis::add(asw_copyOther_, aboutCopyOther);
}

EmpathSendingSettingsDialog::~EmpathSendingSettingsDialog()
{
    // Empty.
}

    void
EmpathSendingSettingsDialog::saveData()
{
    KConfig * c    = KGlobal::config();

    using namespace EmpathConfig;

    c->setGroup(GROUP_SENDING);
    
    // Get server type
    c->writeEntry(S_TYPE,             bg_server_->id(bg_server_->selected())); 
    c->writeEntry(S_SENDMAIL,         efsw_sendmail_->path());
    c->writeEntry(S_QMAIL,            efsw_qmail_->path());
    c->writeEntry(S_SMTP,             le_smtpServer_->text());
    c->writeEntry(S_SMTP_PORT,        sb_smtpPort_->value());
    c->writeEntry(C_CC_OTHER,         cb_copyOther_->isChecked());
    c->writeEntry(C_CC_OTHER_ADDRESS, asw_copyOther_->text());
    
    empath->updateOutgoingServer();
}

    void
EmpathSendingSettingsDialog::loadData()
{
    KConfig * c    = KGlobal::config();

    using namespace EmpathConfig;
    c->setGroup(GROUP_SENDING);
    
    bg_server_      ->setButton     (c->readNumEntry    (S_TYPE));
    efsw_sendmail_  ->setPath       (c->readEntry       (S_SENDMAIL));
    efsw_qmail_     ->setPath       (c->readEntry       (S_QMAIL));
    le_smtpServer_  ->setText       (c->readEntry       (S_SMTP));
    sb_smtpPort_    ->setValue      (c->readNumEntry    (S_SMTP_PORT, 25));
    cb_copyOther_   ->setChecked    (c->readBoolEntry   (C_CC_OTHER));
    asw_copyOther_  ->setText       (c->readEntry       (C_CC_OTHER_ADDRESS));
    
    empath->updateOutgoingServer();
}

    void
EmpathSendingSettingsDialog::s_OK()
{
    hide();
    if (!applied_)
        s_apply();
    KGlobal::config()->sync();
    accept();
}

    void
EmpathSendingSettingsDialog::s_help()
{
    // STUB
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
    efsw_sendmail_  ->setPath("/usr/lib/sendmail");
    efsw_qmail_     ->setPath("/var/qmail/bin/qmail-inject");
    cb_copyOther_   ->setChecked(false);
    rb_smtp_        ->setChecked(false);
    rb_qmail_       ->setChecked(false);
    rb_sendmail_    ->setChecked(true);
    le_smtpServer_  ->setText("localhost");
    sb_smtpPort_    ->setValue(25);
}
    
    void
EmpathSendingSettingsDialog::s_cancel()
{
    if (!applied_)
        KGlobal::config()->rollback(true);
    reject();
}

// vim:ts=4:sw=4:tw=78
