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
# pragma implementation "EmpathConfigPOP3Widget.h"
#endif

// KDE includes
#include <klocale.h>

// Local includes
#include "EmpathMailboxPOP3.h"
#include "EmpathConfigPOP3Widget.h"
#include "EmpathConfigPOP3Server.h"
#include "EmpathConfigPOP3Logging.h"
#include "Empath.h"

EmpathConfigPOP3Widget::EmpathConfigPOP3Widget
    (const EmpathURL & url, QWidget * parent)
    :   KTabCtl(parent, "ConfigPOP3Widget"),
        url_(url)
{
    serverWidget_   = new EmpathConfigPOP3Server(url_, this);
    loggingWidget_  = new EmpathConfigPOP3Logging(url, this);
    
    addTab(serverWidget_,   i18n("Server"));
    addTab(loggingWidget_,  i18n("Logging"));
    
    loadData();
};

EmpathConfigPOP3Widget::~EmpathConfigPOP3Widget()
{
    // Empty.
}

    void
EmpathConfigPOP3Widget::saveData()
{
    EmpathMailbox * mailbox = empath->mailbox(url_);

    if (mailbox == 0)
        return;

    if (mailbox->type() != EmpathMailbox::POP3) {
        empathDebug("Incorrect mailbox type");
        return;
    }
    
    EmpathMailboxPOP3 * m = (EmpathMailboxPOP3 *)mailbox;

    serverWidget_->saveData();
    loggingWidget_->saveData();
    m->saveConfig();
}

    void
EmpathConfigPOP3Widget::loadData()
{
    EmpathMailbox * mailbox = empath->mailbox(url_);

    if (mailbox == 0)
        return;

    if (mailbox->type() != EmpathMailbox::POP3) {
        empathDebug("Incorrect mailbox type");
        return;
    }
    
    EmpathMailboxPOP3 * m = (EmpathMailboxPOP3 *)mailbox;

    m->loadConfig();
    serverWidget_->loadData();
    loggingWidget_->loadData();
}

// vim:ts=4:sw=4:tw=78
