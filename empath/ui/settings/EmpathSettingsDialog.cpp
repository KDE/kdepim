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

#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

// Qt includes
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qvbox.h>

// KDE includes
#include <klocale.h>
#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kglobal.h>
#include <kcolorbutton.h>

// Local includes
#include "EmpathMailSender.h"
#include "EmpathSettingsDialog.h"
#include "IdentitySettingsForm.h"
#include "ComposeSettingsForm.h"
#include "DisplaySettingsForm.h"
#include "SendingSettingsForm.h"

EmpathSettingsDialog * EmpathSettingsDialog::instance_ = 0L;

EmpathSettingsDialog::EmpathSettingsDialog()
    :   KDialogBase(
            IconList,
            i18n("Settings"),
            Help | Default | Apply | Ok | Cancel,
            Cancel,
            (QWidget *)0L,
            "EmpathSettingsDialog",
            false,
            true
       )
{
    identitySettingsForm =
        new IdentitySettingsForm(addVBoxPage(i18n("Identity")));

    displaySettingsForm  =
        new DisplaySettingsForm(addVBoxPage(i18n("Display")));

    connect(displaySettingsForm->kcb_depth1, SIGNAL(changed(const QColor &)),
            this, SLOT(slotPreviewDepth1(const QColor &)));

    connect(displaySettingsForm->kcb_depth2, SIGNAL(changed(const QColor &)),
            this, SLOT(slotPreviewDepth2(const QColor &)));

    displaySettingsForm->l_depth1
        ->setBackgroundColor(colorGroup().base());

    displaySettingsForm->l_depth2
        ->setBackgroundColor(colorGroup().base());

    composeSettingsForm  =
        new ComposeSettingsForm(addVBoxPage(i18n("Composing")));

    sendingSettingsForm  =
        new SendingSettingsForm(addVBoxPage(i18n("Sending")));

    _load();
}

EmpathSettingsDialog::~EmpathSettingsDialog()
{
}

    void
EmpathSettingsDialog::_load()
{
    KSimpleConfig * ec = new KSimpleConfig("emaildefaults", true);

    ec->setGroup("UserInfo");

    identitySettingsForm->le_name->setText(ec->readEntry("FullName"));
    identitySettingsForm->le_email->setText(ec->readEntry("EmailAddress"));
    identitySettingsForm->le_org->setText(ec->readEntry("Organization"));
    identitySettingsForm->le_reply->setText(ec->readEntry("ReplyAddr"));

    ec->setGroup("ServerInfo");

    sendingSettingsForm->le_smtpserver->setText(ec->readEntry("Outgoing"));

    delete ec;
    ec = 0;

    KConfig * c(KGlobal::config());

    c->setGroup("Display");

    QColor depth1(c->readColorEntry("QuoteColorOne", &Qt::darkBlue));
    QColor depth2(c->readColorEntry("QuoteColorTwo", &Qt::darkCyan));

    displaySettingsForm->kcb_depth1->setColor(depth1);
    displaySettingsForm->kcb_depth2->setColor(depth2);

    slotPreviewDepth1(depth1);
    slotPreviewDepth2(depth2);

    c->setGroup("Sending");

    int t = c->readNumEntry("Type");

    if (t == EmpathMailSender::SMTP)
        sendingSettingsForm->rb_smtp->setChecked(true);
    else if (t == EmpathMailSender::Qmail)
        sendingSettingsForm->rb_qmail->setChecked(true);
    else
        sendingSettingsForm->rb_sendmail->setChecked(true);

    sendingSettingsForm->le_sendmail->setText(c->readEntry("Sendmail"));

    sendingSettingsForm->sb_smtpport->setValue(c->readNumEntry("SMTPPort"));

    sendingSettingsForm->le_qmail->setText(c->readEntry("Qmail"));

    sendingSettingsForm->cb_copyto->setChecked(c->readBoolEntry("CcOther"));

    sendingSettingsForm->le_copyto->setText(c->readEntry("CcOtherAddress"));

    c->setGroup("Composing");

    composeSettingsForm->le_reply->setText(c->readEntry("PhraseReplySender"));
    composeSettingsForm->le_replyAll->setText(c->readEntry("PhraseReplyAll"));
    composeSettingsForm->le_forward->setText(c->readEntry("PhraseForward"));

    composeSettingsForm->cb_wrap->setChecked(c->readBoolEntry("WrapLines"));
    composeSettingsForm->sb_wrapColumn->setValue(c->readNumEntry("WrapColumn"));
}

    void
EmpathSettingsDialog::slotApply()
{
    KConfig * ec = new KConfig("emaildefaults");

    ec->setGroup("UserInfo");

    ec->writeEntry("FullName",      identitySettingsForm->le_name->text());
    ec->writeEntry("EmailAddress",  identitySettingsForm->le_email->text());
    ec->writeEntry("Organization",  identitySettingsForm->le_org->text());
    ec->writeEntry("ReplyAddr",     identitySettingsForm->le_reply->text());

    ec->setGroup("ServerInfo");

    ec->writeEntry("Outgoing",      sendingSettingsForm->le_smtpserver->text());

    ec->sync();

    delete ec;
    ec = 0;

    KConfig * c(KGlobal::config());

    c->setGroup("Display");

    c->writeEntry("QuoteColorOne", displaySettingsForm->kcb_depth1->color());
    c->writeEntry("QuoteColorTwo", displaySettingsForm->kcb_depth2->color());

    c->setGroup("Sending");

    int t;

    if (sendingSettingsForm->rb_smtp->isChecked())
        t = EmpathMailSender::SMTP;
    else if (sendingSettingsForm->rb_qmail->isChecked())
        t = EmpathMailSender::Qmail;
    else
        t = EmpathMailSender::Sendmail;

    c->writeEntry("Type", t);

    c->writeEntry("Sendmail",       sendingSettingsForm->le_sendmail->text());

    c->writeEntry("SMTPPort",
            sendingSettingsForm->sb_smtpport->value());

    c->writeEntry("Qmail",          sendingSettingsForm->le_qmail->text());

    c->writeEntry("CcOther",
            sendingSettingsForm->cb_copyto->isChecked());

    c->writeEntry("CcOtherAddress", sendingSettingsForm->le_copyto->text());

    c->setGroup("Composing");

    c->writeEntry("PhraseReplySender",
            composeSettingsForm->le_reply->text());

    c->writeEntry("PhraseReplyAll",
            composeSettingsForm->le_replyAll->text());

    c->writeEntry("PhraseForward",
            composeSettingsForm->le_forward->text());

    c->writeEntry("WrapLines",
            composeSettingsForm->cb_wrap->isChecked());

    c->writeEntry("WrapColumn",
            composeSettingsForm->sb_wrapColumn->value());

    c->sync();
}

    void
EmpathSettingsDialog::slotDefault()
{
    _identityDefaults();
    _displayDefaults();
    _composeDefaults();
    _sendingDefaults();
}

    void
EmpathSettingsDialog::slotCancel()
{
    KGlobal::config()->rollback();
    instance_ = 0L;
    delete this;
}

    void
EmpathSettingsDialog::_identityDefaults()
{
    KSimpleConfig * c = new KSimpleConfig("emaildefaults", true);

    c->setGroup("UserInfo");

    struct passwd * p = getpwuid(getuid());

    identitySettingsForm->le_name
        ->setText(c->readEntry("FullName", p->pw_gecos));

    char domainname[128];
    gethostname(domainname, 128);

    QString s(p->pw_name + QString("@") + domainname);

    identitySettingsForm->le_email
        ->setText(c->readEntry("EmailAddress", s));

    identitySettingsForm->le_org
        ->setText(c->readEntry("Organization", "None"));

    identitySettingsForm->le_reply
        ->setText(c->readEntry("ReplyAddr"));

    delete c;
    c = 0;
}

    void
EmpathSettingsDialog::_displayDefaults()
{
    displaySettingsForm->kcb_depth1->setColor(Qt::darkBlue);
    displaySettingsForm->kcb_depth2->setColor(Qt::darkCyan);
}

    void
EmpathSettingsDialog::_composeDefaults()
{
    composeSettingsForm->le_reply->setText(i18n("%1 wrote"));
    composeSettingsForm->le_replyAll->setText(i18n("%1 wrote"));
    composeSettingsForm->le_forward->setText(i18n("Forwarded message from %1"));
    composeSettingsForm->cb_wrap->setChecked(true);
    composeSettingsForm->sb_wrapColumn->setValue(76);
}

    void
EmpathSettingsDialog::_sendingDefaults()
{
    sendingSettingsForm->rb_smtp->setChecked(true);
    sendingSettingsForm->le_qmail->setText("/var/qmail/bin/qmail-inject");
    sendingSettingsForm->le_sendmail->setText("/usr/lib/sendmail");

    KSimpleConfig * c = new KSimpleConfig("emaildefaults", true);

    c->setGroup("ServerInfo");

    sendingSettingsForm->le_smtpserver
        ->setText(c->readEntry("Outgoing", "localhost"));

    delete c;
    c = 0;

    sendingSettingsForm->sb_smtpport->setValue(25);
    sendingSettingsForm->rb_sendnow->setChecked(true);
    sendingSettingsForm->cb_copysent->setChecked(true);
    sendingSettingsForm->cb_copyto->setChecked(false);
    sendingSettingsForm->le_copyto->setText("");
}

    void
EmpathSettingsDialog::slotPreviewDepth1(const QColor & c)
{
    displaySettingsForm->l_depth1->setTextFormat(RichText);

    displaySettingsForm->l_depth1
        ->setText("<font color=\"" + c.name() + "\">&gt; " + i18n("Example") +
                "</font>");
}

    void
EmpathSettingsDialog::slotPreviewDepth2(const QColor & c)
{
    displaySettingsForm->l_depth2->setTextFormat(RichText);

    displaySettingsForm->l_depth2
        ->setText("<font color=\"" + c.name() + "\">&gt; &gt; " +
                i18n("Example") + "</font>");
}

// vim:ts=4:sw=4:tw=78
