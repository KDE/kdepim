/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
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
# pragma implementation "EmpathConfigMaildirDialog.h"
#endif

// System includes
#include <stdlib.h>

// Qt includes
#include <qlayout.h>

// KDE includes
#include <klocale.h>
#include <kapp.h>
#include <kglobal.h>
#include <kconfig.h>

// Local includes
#include "EmpathConfigMaildirDialog.h"
#include "EmpathPathSelectWidget.h"
#include "EmpathMailboxMaildir.h"
#include "EmpathUIUtils.h"
#include "Empath.h"
 
bool EmpathConfigMaildirDialog::exists_ = false;

    void
EmpathConfigMaildirDialog::create(const EmpathURL & url, QWidget * parent)
{
    if (exists_) return;
    exists_ = true;
    EmpathConfigMaildirDialog * d = new EmpathConfigMaildirDialog(url, parent);
    d->loadData();
    d->exec();
}
        
EmpathConfigMaildirDialog::EmpathConfigMaildirDialog
        (const EmpathURL & url, QWidget * parent)
    :   QDialog(parent, "EmpathConfigMaildirDialog", true),
        url_(url),
        applied_(false)
{
    QString caption = i18n("Configuring mailbox %1");
    setCaption(caption.arg(url_.mailboxName()));

    EmpathMailbox * mailbox = empath->mailbox(url);

    if (mailbox == 0) {
        empathDebug("Mailbox is 0 !");
        return;
    }

    if (mailbox->type() != EmpathMailbox::Maildir) {
        empathDebug("Not EmpathMaildirMailbox !")
        return;
    }

    cb_mailCheckInterval_ =
        new QCheckBox(i18n("Check for new mail at interval (in minutes):"),
                this, "cb_mailCheckInterval");

    sb_mailCheckInterval_ =
        new QSpinBox(1, 240, 1, this, "sb_mailCheckInterval");

    QObject::connect(cb_mailCheckInterval_, SIGNAL(toggled(bool)),
            sb_mailCheckInterval_, SLOT(setEnabled(bool)));

    KButtonBox * buttonBox  = new KButtonBox(this);
    
    pb_help_    = buttonBox->addButton(i18n("&Help"));    
    pb_default_ = buttonBox->addButton(i18n("&Default"));    
    
    buttonBox->addStretch();
    
    pb_OK_      = buttonBox->addButton(i18n("&OK"));
    
    pb_OK_->setDefault(true);
    
    pb_apply_   = buttonBox->addButton(i18n("&Apply"));
    pb_cancel_  = buttonBox->addButton(i18n("&Cancel"));
    
    buttonBox->layout();
    
    QObject::connect(pb_OK_,        SIGNAL(clicked()),  SLOT(s_OK()));
    QObject::connect(pb_default_,   SIGNAL(clicked()),  SLOT(s_default()));
    QObject::connect(pb_apply_,     SIGNAL(clicked()),  SLOT(s_apply()));
    QObject::connect(pb_cancel_,    SIGNAL(clicked()),  SLOT(s_cancel()));
    QObject::connect(pb_help_,      SIGNAL(clicked()),  SLOT(s_help()));
 
    QVBoxLayout * v = new QVBoxLayout(this, dialogSpace);
    
    QHBoxLayout * b = new QHBoxLayout(v);
    b->addWidget(cb_mailCheckInterval_);
    b->addWidget(sb_mailCheckInterval_);

    v->addWidget(buttonBox);
    
    loadData();
}

EmpathConfigMaildirDialog::~EmpathConfigMaildirDialog()
{
    exists_ = false;
}

    void
EmpathConfigMaildirDialog::s_OK()
{
    hide();
    if (!applied_)
        s_apply();

    KGlobal::config()->sync();

    // That's it
    accept();
    delete this;
}

    void
EmpathConfigMaildirDialog::saveData()
{
    EmpathMailbox * mailbox = empath->mailbox(url_);

    if (mailbox == 0) {
        empathDebug("Mailbox is 0 !");
        return;
    }

    if (mailbox->type() != EmpathMailbox::Maildir) {
        empathDebug("Not EmpathMaildirMailbox !")
        return;
    }

    EmpathMailboxMaildir * m = (EmpathMailboxMaildir *)mailbox;

    m->setAutoCheck         (cb_mailCheckInterval_->isChecked());
    m->setAutoCheckInterval (sb_mailCheckInterval_->value());

    m->saveConfig();
}

    void
EmpathConfigMaildirDialog::loadData()
{
    EmpathMailbox * mailbox = empath->mailbox(url_);

    if (mailbox == 0) {
        empathDebug("Mailbox is 0 !");
        return;
    }

    if (mailbox->type() != EmpathMailbox::Maildir) {
        empathDebug("Not EmpathMaildirMailbox !")
        return;
    }

    EmpathMailboxMaildir * m = (EmpathMailboxMaildir *)mailbox;

    cb_mailCheckInterval_   ->setChecked    (m->autoCheck());
    sb_mailCheckInterval_   ->setValue      (m->autoCheckInterval());
    sb_mailCheckInterval_   ->setEnabled    (m->autoCheck());
}

    void
EmpathConfigMaildirDialog::s_cancel()
{
    if (!applied_)
        KGlobal::config()->rollback(true);
    reject();
    delete this;
}

    void
EmpathConfigMaildirDialog::s_apply()
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
EmpathConfigMaildirDialog::s_default()
{
    cb_mailCheckInterval_->setChecked(true);
    sb_mailCheckInterval_->setValue(1);
}

    void
EmpathConfigMaildirDialog::s_help()
{
    //empathInvokeHelp(QString::null, QString::null);
}

// vim:ts=4:sw=4:tw=78
