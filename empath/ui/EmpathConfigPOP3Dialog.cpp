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
# pragma implementation "EmpathConfigPOP3Dialog.h"
#endif

// Qt includes
#include <qlayout.h>

// KDE includes
#include <klocale.h>
#include <kbuttonbox.h>

// Local includes
#include "EmpathConfigPOP3Dialog.h"
#include "EmpathPasswordEditWidget.h"
#include "EmpathMailboxPOP3.h"
#include "EmpathUIUtils.h"
#include "Empath.h"
 
EmpathConfigPOP3Dialog::EmpathConfigPOP3Dialog
    (const EmpathURL & url, QWidget * parent)
    :   QDialog(parent, "ConfigPOP3Dialog", true),
        url_(url)
{
    QString caption = i18n("Configuring mailbox %1");
    setCaption(caption.arg(url_.mailboxName()));
    
    // Server username and address
    QLabel * l_inServer     = new QLabel(i18n("Server address"), this);
    QLabel * l_inServerPort = new QLabel(i18n("Server POP3 port"), this);
    QLabel * l_uname        = new QLabel(i18n("Mail server user name"), this);
    QLabel * l_pass         = new QLabel(i18n("Mail server password"), this);

    le_inServer_        = new QLineEdit(this);
    sb_inServerPort_    = new QSpinBox(1, 100000, 1, this);
    le_uname_           = new QLineEdit(this);
    epew_pass_          = new EmpathPasswordEditWidget(QString::null, this);

    KButtonBox * buttonBox    = new KButtonBox(this);

    // Bottom button group
    pb_Help_    = buttonBox->addButton(i18n("&Help"));    
    buttonBox->addStretch();
    pb_OK_      = buttonBox->addButton(i18n("&OK"));
    pb_Cancel_  = buttonBox->addButton(i18n("&Cancel"));
    
    buttonBox->layout();

    QVBoxLayout * layout = new QVBoxLayout(this, dialogSpace);

    QGridLayout * layout1 = new QGridLayout(layout, 4, 2, dialogSpace);

    layout1->addWidget(l_inServer,       0, 0);
    layout1->addWidget(l_inServerPort,   1, 0);
    layout1->addWidget(l_uname,          2, 0);
    layout1->addWidget(l_pass,           3, 0);
    
    layout1->addWidget(le_inServer_,     0, 1);
    layout1->addWidget(sb_inServerPort_, 1, 1);
    layout1->addWidget(le_uname_,        2, 1);
    layout1->addWidget(epew_pass_,       3, 1);

    layout->addStretch(10);
    layout->addWidget(buttonBox);
    
    QObject::connect(pb_OK_, SIGNAL(clicked()),
            this, SLOT(s_OK()));
    
    QObject::connect(pb_Cancel_, SIGNAL(clicked()),
            this, SLOT(s_Cancel()));
    
    QObject::connect(pb_Help_, SIGNAL(clicked()),
            this, SLOT(s_Help()));

    loadData();
}

EmpathConfigPOP3Dialog::~EmpathConfigPOP3Dialog()
{
}

    void
EmpathConfigPOP3Dialog::s_OK()
{
    saveData();
    accept();
}

    void
EmpathConfigPOP3Dialog::s_Cancel()
{
    reject();
}

    void
EmpathConfigPOP3Dialog::s_Help()
{
    // STUB
}

    void
EmpathConfigPOP3Dialog::loadData()
{
    EmpathMailbox * mailbox = empath->mailbox(url_);

    if (mailbox == 0)
        return;

    if (mailbox->type() != EmpathMailbox::POP3) {
        empathDebug("Incorrect mailbox type");
        return;
    }

    EmpathMailboxPOP3 * m = static_cast<EmpathMailboxPOP3 *>(mailbox);

    le_inServer_    ->setText   (m->serverAddress());
    sb_inServerPort_->setValue  (m->serverPort());
    le_uname_       ->setText   (m->username());
    epew_pass_      ->setText   (m->password());
}

    void
EmpathConfigPOP3Dialog::saveData()
{
    EmpathMailbox * mailbox = empath->mailbox(url_);

    if (mailbox == 0)
        return;

    if (mailbox->type() != EmpathMailbox::POP3) {
        empathDebug("Incorrect mailbox type");
        return;
    }

    EmpathMailboxPOP3 * m = static_cast<EmpathMailboxPOP3 *>(mailbox);

    m->setServerAddress (le_inServer_->text());
    m->setServerPort    (sb_inServerPort_->value());
    m->setUsername      (le_uname_->text());
    m->setPassword      (epew_pass_->text());
}

// vim:ts=4:sw=4:tw=78
