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
#include "EmpathConfigPOP3Password.h"
#include "EmpathConfigPOP3General.h"

EmpathConfigPOP3Widget::EmpathConfigPOP3Widget(
        EmpathMailboxPOP3 * mailbox,
        bool loadData,
        QWidget * parent,
        const char * name)
    :    KTabCtl(parent, name)
{
    empathDebug("ctor");
    
    serverWidget_ =
        new EmpathConfigPOP3Server(this, "serverWidget");
    CHECK_PTR(serverWidget_);

    loggingWidget_ =
        new EmpathConfigPOP3Logging(this, "loggingWidget");
    CHECK_PTR(loggingWidget_);
    
    passwordWidget_ =
        new EmpathConfigPOP3Password(this, "passwordWidget");
    CHECK_PTR(passwordWidget_);
    
    generalWidget_ =
        new EmpathConfigPOP3General(this, "generalWidget");
    CHECK_PTR(generalWidget_);
    
    addTab(serverWidget_,            i18n("Server"));
    addTab(generalWidget_,            i18n("General"));
    addTab(passwordWidget_,            i18n("Password"));
    addTab(loggingWidget_,            i18n("Logging"));
    
    setMailbox(mailbox, loadData);

    serverWidget_->loadData();
    generalWidget_->loadData();
    passwordWidget_->loadData();
    loggingWidget_->loadData();

    setMinimumSize(minimumSizeHint());
    resize(minimumSizeHint());
};

    void
EmpathConfigPOP3Widget::saveData()
{
    empathDebug("saveData() called");
    serverWidget_->saveData();
    generalWidget_->saveData();
    passwordWidget_->saveData();
    loggingWidget_->saveData();
    mailbox_->saveConfig();
}

    void
EmpathConfigPOP3Widget::setMailbox(EmpathMailboxPOP3 * mailbox, bool loadData)
{
    mailbox_ = mailbox;

    ASSERT(mailbox_);
    
    if (loadData)
        mailbox_->readConfig();

    empathDebug("Set mailbox " + mailbox->name());
    serverWidget_->setMailbox(mailbox);
    generalWidget_->setMailbox(mailbox);
    passwordWidget_->setMailbox(mailbox);
    loggingWidget_->setMailbox(mailbox);
}


// vim:ts=4:sw=4:tw=78
