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
# pragma implementation "EmpathComposeSettingsDialog.h"
#endif

#include <qwhatsthis.h>

// KDE includes
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kapp.h>

// Local includes
#include "EmpathComposeSettingsDialog.h"
#include "EmpathSeparatorWidget.h"
#include "EmpathMailSender.h"
#include "EmpathConfig.h"
#include "Empath.h"
#include "EmpathUIUtils.h"
#include "EmpathUtilities.h"

EmpathComposeSettingsDialog::EmpathComposeSettingsDialog(QWidget * parent)
    :   KDialog(parent, "ComposeSettings", true),
        applied_(false)
{
    setCaption(i18n("Compose Settings"));
    
    QLabel * l_extra =
        new QLabel(i18n("Extra headers to edit:"), this, "l_extra");

    QLabel * l_reply =
        new QLabel(i18n("Reply to sender:"), this, "l_reply");
   
    QLabel * l_replyAll =
        new QLabel(i18n("Reply to All:"), this, "l_replyAll");

    QLabel * l_forward =
        new QLabel(i18n("Forward:"), this, "l_forward");

    le_reply_       = new QLineEdit(this, "le_reply");
    le_replyAll_    = new QLineEdit(this, "le_replyAll");
    le_forward_     = new QLineEdit(this, "le_forward");
    le_extra_       = new QLineEdit(this, "le_extra");
    
    cb_quote_ =
        new QCheckBox(i18n("Automatically &quote message when replying"),
                this, "cb_quote");
   
    cb_addSig_ =
        new QCheckBox(i18n("Add &signature"), this, "l_addSig");
   
    cb_digSign_ =
        new QCheckBox(i18n("Add &digital Signature"), this, "cb_digSign");

    cb_wrap_ =
        new QCheckBox(i18n("Wrap &long lines at column number"), this,
                "cb_wrap");
    sb_wrap_ =
        new QSpinBox(40, 240, 1, this, "cb_wrap");
    
    sb_wrap_->setValue(76);
    sb_wrap_->setEnabled(false);
    
   
    buttonGroup_ = new QButtonGroup(this, "buttonGroup");
    buttonGroup_->hide();

    rb_sendNow_        =
        new QRadioButton(i18n("Send messages &immediately"),
            this, "rb_sendNow");

    rb_sendNow_->setChecked(true);
    
    rb_sendLater_    =
        new QRadioButton(i18n("Send messages &later"), this, "rb_sendLater");

    buttonGroup_->insert(rb_sendNow_,   EmpathMailSender::SendNow);
    buttonGroup_->insert(rb_sendLater_, EmpathMailSender::SendLater);
    
    cb_externalEditor_ =
        new QCheckBox(i18n("Use external editor"),
            this, "cb_useExternalEditor");
        
    le_externalEditor_ = new QLineEdit(this, "le_externalEditor");

#include "EmpathDialogButtonBox.cpp"

    // Layouts
    
    QVBoxLayout * layout = new QVBoxLayout(this, dialogSpace);

    QGridLayout * layout0 = new QGridLayout(layout);

    layout0->addWidget(l_extra,     0, 0);
    layout0->addWidget(l_reply,     1, 0);
    layout0->addWidget(l_replyAll,  2, 0);
    layout0->addWidget(l_forward,   3, 0);

    layout0->addWidget(le_extra_,   0, 1);
    layout0->addWidget(le_reply_,   1, 1);
    layout0->addWidget(le_replyAll_,2, 1);
    layout0->addWidget(le_forward_, 3, 1);

    layout->addWidget(new EmpathSeparatorWidget(this));
 
    layout->addWidget(cb_quote_);
    layout->addWidget(cb_addSig_);
    layout->addWidget(cb_digSign_);

    QHBoxLayout * layout1 = new QHBoxLayout(layout);
    layout1->addWidget(cb_wrap_);
    layout1->addWidget(sb_wrap_);

    QHBoxLayout * layout2 = new QHBoxLayout(layout);
    layout2->addWidget(cb_externalEditor_);
    layout2->addWidget(le_externalEditor_);

    layout->addWidget(new EmpathSeparatorWidget(this));

    layout->addWidget(rb_sendNow_);
    layout->addWidget(rb_sendLater_);

    layout->addStretch(10);

    layout->addWidget(new EmpathSeparatorWidget(this));
 
    layout->addWidget(buttonBox_);
 
    QWhatsThis::add(le_reply_, i18n(
        "Choose the phrase that will be added\n"
        "before a message when you reply.\n"
        "<i>%1</i> will be replaced by the date that\n"
        "the message was sent to you.\n"
        "<i>%2</i> will be replaced by the name of the\n"
        "person who sent you the message.\n"));

    QWhatsThis::add(le_replyAll_, i18n(
        "Choose the phrase that will be added\n"
        "before a message when you reply.\n"
        "<i>%1</i> will be replaced by the date that\n"
        "the message was sent to you.\n"
        "<i>%2</i> will be replaced by the name of the\n"
        "person who sent you the message.\n"));

    QWhatsThis::add(le_forward_, i18n(
        "Choose the phrase that will be added\n"
        "before a message when you forward it.\n"
        "<i>%1</i> will be replaced by the date that\n"
        "the message was sent to you.\n"
        "<i>%2</i> will be replaced by the name of the\n"
        "person who sent you the message.\n"));
    
    QWhatsThis::add(cb_quote_, i18n(
        "With this selected, when you reply to a\n"
        "message, you'll get the text of the message\n"
        "you're replying to in the compose window.\n"
        "This can make it easier to provide context.\n"
        "The text is quoted with '> '\n"));

    QWhatsThis::add(cb_addSig_, i18n(
        "With this selected, your signature will be\n"
        "automatically added to the end of all messages\n"
        "you send. If you don't have a signature, use\n"
        "Options->Sending to set one up.\n"
        "The text '--' will be prepended to your signature.\n"
        "This is standard behaviour and will allow other\n"
        "people's mail programs to recognise where your\n"
        "message ends, and your signature starts."));
    
    QWhatsThis::add(cb_digSign_, i18n(
        "Adding a digital signature to a message is\n"
        "a bit like adding your real signature, a\n"
        "photograph, your fingerprint, and some hair\n"
        "(for DNA analysis). In other words, you're\n"
        "saying that this message definitely came from\n"
        "you. This helps stops people masquerading as you."));

    QWhatsThis::add(cb_wrap_, i18n(
        "When you type a reply, you probably don't\n"
        "press <b>Return</b> at the end of each line.\n"
        "Internet mail messages are supposed to be read\n"
        "in a fixed font, and any lines longer than 80\n"
        "characters may not be 'wrapped around'.\n"
        "This option will add carriage returns in lines\n"
        "longer than the value set in the spin box.\n"
        "This will ensure that messages look correct when\n"
        "received by your recipient.\n"));

    QWhatsThis::add(rb_sendNow_, i18n(
        "If you choose this option, Empath will\n"
        "try to send your message immediately when\n"
        "you press send. Note that you won't have to\n"
        "stop using Empath when you press send. This\n"
        "is useful when you are permanently connected\n"
        "to a mail server, as there's no delay between\n"
        "pressing send and the message being despatched."));

    QWhatsThis::add(rb_sendLater_, i18n(
        "By choosing this option, when you press\n"
        "'send' while composing a message, the\n"
        "message will be placed in a queue, and\n"
        "will only be despatched later on, when\n"
        "you say so. This is useful when you don't\n"
        "have a permanent connection to a mail server.\n"));

    QWhatsThis::add(cb_externalEditor_, i18n(
        "By selecting this option, you allow the use\n"
        "of your favourite text editor to type messages.\n"
        "The internal editor, while useful, is not incredibly\n"
        "powerful. Some people prefer their usual editor as\n"
        "they need the extra features, or are simply used to it."));

    QWhatsThis::add(cb_externalEditor_, i18n(
        "Type here the command line to run your editor.\n"
        "This may be, for example, <b>gvim</b> or <b>emacs</b>"));

    QObject::connect(
        cb_wrap_, SIGNAL(toggled(bool)),
        sb_wrap_, SLOT(setEnabled(bool)));
}

EmpathComposeSettingsDialog::~EmpathComposeSettingsDialog()
{
    // Empty.
}

    void
EmpathComposeSettingsDialog::saveData()
{
    EmpathMailSender::SendPolicy sendPolicy =
        rb_sendNow_->isChecked() ?
        EmpathMailSender::SendNow : EmpathMailSender::SendLater;
    
    KConfig * c = KGlobal::config();

    using namespace EmpathConfig;

    c->setGroup(GROUP_COMPOSE);

    c->writeEntry(C_EXTRA_HEADERS,          le_extra_->text());
    c->writeEntry(C_PHRASE_REPLY_SENDER,    le_reply_->text());
    c->writeEntry(C_PHRASE_REPLY_ALL,       le_replyAll_->text());
    c->writeEntry(C_PHRASE_FORWARD,         le_forward_->text());
    c->writeEntry(C_AUTO_QUOTE,             cb_quote_->isChecked());
    c->writeEntry(C_ADD_SIG,                cb_addSig_->isChecked());
    c->writeEntry(C_ADD_DIG_SIG,            cb_digSign_->isChecked());
    c->writeEntry(C_WRAP_LINES,             cb_wrap_->isChecked());
    c->writeEntry(C_WRAP_COLUMN,            sb_wrap_->value());
    c->writeEntry(C_SEND_POLICY,            (unsigned long)sendPolicy);
    c->writeEntry(C_USE_EXT_EDIT,           cb_externalEditor_->isChecked());
    c->writeEntry(C_EXT_EDIT,               le_externalEditor_->text());
}

    void
EmpathComposeSettingsDialog::loadData()
{
    KConfig * c = KGlobal::config();
    
    using namespace EmpathConfig;
    
    c->setGroup(GROUP_COMPOSE);
    
    EmpathMailSender::SendPolicy sendPolicy =
        (EmpathMailSender::SendPolicy)
        c->readNumEntry(C_SEND_POLICY, EmpathMailSender::SendLater);

    le_extra_->setText(c->readEntry(C_EXTRA_HEADERS));
    le_reply_->setText(c->readEntry(C_PHRASE_REPLY_SENDER, i18n(DFLT_REPLY.ascii())));
    le_replyAll_->setText(c->readEntry(C_PHRASE_REPLY_ALL, i18n(DFLT_REPLY_ALL.ascii())));
    
    le_forward_->setText(c->readEntry(C_PHRASE_FORWARD, i18n(DFLT_FORWARD.ascii())));

    cb_quote_->setChecked   (c->readBoolEntry(C_AUTO_QUOTE,   true));
    cb_addSig_->setChecked  (c->readBoolEntry(C_ADD_SIG,      true));
    cb_digSign_->setChecked (c->readBoolEntry(C_ADD_DIG_SIG,  false));
    
    cb_wrap_->setChecked(c->readBoolEntry(C_WRAP_LINES,    true));
    sb_wrap_->setValue(c->readNumEntry(C_WRAP_COLUMN,   76));

    rb_sendNow_     ->setChecked(sendPolicy == EmpathMailSender::SendLater);
    rb_sendLater_   ->setChecked(!rb_sendNow_->isChecked());
    
    cb_externalEditor_->setChecked  (c->readBoolEntry(C_USE_EXT_EDIT, false));
    le_externalEditor_->setText     (c->readEntry(C_EXT_EDIT, "gvim"));
}

    void
EmpathComposeSettingsDialog::s_OK()
{
    hide();
    if (!applied_)
        s_apply();
    KGlobal::config()->sync();
    accept();
}

    void
EmpathComposeSettingsDialog::s_help()
{
    //empathInvokeHelp(QString::null, QString::null);
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
    le_extra_           ->  setText     (QString::null);
    le_reply_           ->  setText     (i18n("%s wrote:"));
    le_replyAll_        ->  setText     (i18n("%s wrote:"));
    le_forward_         ->  setText     (i18n("Forwarded message from %s"));
    cb_quote_           ->  setChecked  (true);
    cb_addSig_          ->  setChecked  (true);
    cb_digSign_         ->  setChecked  (false);
    cb_wrap_            ->  setChecked  (true);
    sb_wrap_            ->  setValue    (76);
    rb_sendNow_         ->  setChecked  (false);
    rb_sendLater_       ->  setChecked  (true);
    cb_externalEditor_  ->  setChecked  (false);
    le_externalEditor_  ->  setText     ("gvim");
}
    
    void
EmpathComposeSettingsDialog::s_cancel()
{
    if (!applied_)
        KGlobal::config()->rollback(true);
    reject();
}

// vim:ts=4:sw=4:tw=78
