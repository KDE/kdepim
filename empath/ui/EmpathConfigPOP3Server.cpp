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
# pragma implementation "EmpathConfigPOP3Server.h"
#endif

// Qt includes
#include <qlabel.h>
#include <qlayout.h>

// KDE includes
#include <klocale.h>

// Local includes
#include "Empath.h"
#include "EmpathDefines.h"
#include "EmpathConfigPOP3Server.h"
#include "EmpathMailboxPOP3.h"
#include "EmpathPasswordEditWidget.h"
#include "EmpathUIUtils.h"

EmpathConfigPOP3Server::EmpathConfigPOP3Server
    (const EmpathURL & url, QWidget * parent)
    :   QWidget(parent, "ConfigPOP3Server"),
        url_(url)
{
    // Server username and address
    QLabel * l_inServer     = new QLabel(i18n("Server address"), this);
    QLabel * l_inServerPort = new QLabel(i18n("Server POP3 port"), this);
    QLabel * l_uname        = new QLabel(i18n("Mail server user name"), this);
    QLabel * l_pass         = new QLabel(i18n("Mail server password"), this);

    le_inServer_        = new QLineEdit(this);
    sb_inServerPort_    = new QSpinBox(1, 100000, 1, this);
    le_uname_           = new QLineEdit(this);
    epew_pass_          = new EmpathPasswordEditWidget(QString::null, this);

    // Layout
    
    QGridLayout * layout =
        new QGridLayout(this, 4, 2, dialogSpace, dialogSpace);

    layout->addWidget(l_inServer,       0, 0);
    layout->addWidget(l_inServerPort,   1, 0);
    layout->addWidget(l_uname,          2, 0);
    layout->addWidget(l_pass,           3, 0);
    
    layout->addWidget(le_inServer_,     0, 1);
    layout->addWidget(sb_inServerPort_, 1, 1);
    layout->addWidget(le_uname_,        2, 1);
    layout->addWidget(epew_pass_,       3, 1);
}

EmpathConfigPOP3Server::~EmpathConfigPOP3Server()
{
    // Empty.
}

    void
EmpathConfigPOP3Server::saveData()
{
    EmpathMailbox * mailbox = empath->mailbox(url_);

    if (mailbox == 0)
        return;

    if (mailbox->type() != EmpathMailbox::POP3) {
        empathDebug("Incorrect mailbox type");
        return;
    }

    EmpathMailboxPOP3 * m = (EmpathMailboxPOP3 *)mailbox;

    m->setServerAddress (le_inServer_->text());
    m->setServerPort    (sb_inServerPort_->value());
    m->setUsername      (le_uname_->text());
    m->setPassword      (epew_pass_->text());
}

    void
EmpathConfigPOP3Server::loadData()
{
    EmpathMailbox * mailbox = empath->mailbox(url_);

    if (mailbox == 0)
        return;

    if (mailbox->type() != EmpathMailbox::POP3) {
        empathDebug("Incorrect mailbox type");
        return;
    }

    EmpathMailboxPOP3 * m = (EmpathMailboxPOP3 *)mailbox;

    le_inServer_    ->setText   (m->serverAddress());
    sb_inServerPort_->setValue  (m->serverPort());
    le_uname_       ->setText   (m->username());
    epew_pass_      ->setText   (m->password());
}

// vim:ts=4:sw=4:tw=78
