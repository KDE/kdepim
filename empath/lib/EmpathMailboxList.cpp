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
# pragma implementation "EmpathMailboxList.h"
#endif

// Qt includes
#include <qstring.h>
#include <qstringlist.h>

// KDE includes
#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>

// Local includes
#include "EmpathMailbox.h"
#include "EmpathMailboxList.h"
#include "EmpathMailboxPOP3.h"
#include "EmpathMailboxIMAP4.h"
#include "EmpathMailboxMaildir.h"
#include "EmpathDefines.h"
#include "EmpathMessageList.h"
#include "EmpathConfig.h"
#include "Empath.h"

EmpathMailboxList::EmpathMailboxList()
{ 
    empathDebug("ctor");
    setAutoDelete(true);
}

EmpathMailboxList::~EmpathMailboxList()
{
    empathDebug("dtor");
}

    void
EmpathMailboxList::append(EmpathMailbox * mailbox)
{
    empathDebug("append(" +  mailbox->name() + ") called");
    QList::append(mailbox);

    empathDebug("Saving mailbox list count = " + QString().setNum(count()));

    // Save the number of mailboxes into the config.
    KConfig * config_ = KGlobal::config();
    
    // Save the config group.
    KConfigGroupSaver cgs(config_, EmpathConfig::GROUP_GENERAL);
    
    QObject::QObject::connect(mailbox, SIGNAL(newMailArrived()),
        empath, SLOT(s_newMailArrived()));
    
    saveConfig();
    
    emit(updateFolderLists());
}

    bool
EmpathMailboxList::remove(EmpathMailbox * mailbox)
{
    empathDebug("remove \"" + mailbox->name() + "\" called");

    if (!QList::remove(mailbox)) {
        empathDebug("Couldn't remove mailbox");
        return false;
    }
    
    saveConfig();
    
    emit(updateFolderLists());

    return true;
}

    EmpathMailbox *
EmpathMailboxList::find(const QString & name) const
{
    empathDebug("Searching for mailbox " + name);

    QListIterator<EmpathMailbox> it(*this);

    for (; it.current() ; ++it) {
        empathDebug("Testing mailbox called " + it.current()->name());
        if (it.current()->name() == name) {
            empathDebug("This is the one");
            return it.current();
        }
    }
    return 0;

}

    void
EmpathMailboxList::getNewMail()
{
    QListIterator<EmpathMailbox> it(*this);

    for (; it.current(); ++it) {
        it.current()->s_getNewMail();
    }
}

    EmpathFolder *
EmpathMailboxList::folder(const EmpathURL & folderURL) const
{
    EmpathMailbox * m = find(folderURL.mailboxName());
    return (m == 0) ? 0 : (EmpathFolder *)m->folder(folderURL.folderPath());
}

    void
EmpathMailboxList::init()
{
    empathDebug("init() called");
    readConfig();
}

    void
EmpathMailboxList::readConfig()
{
    KConfig * c(KGlobal::config());
    c->setGroup(EmpathConfig::GROUP_GENERAL);
    
    QStringList l;
    l = c->readListEntry(EmpathConfig::KEY_MAILBOX_LIST);
    
    QObject::connect(this, SIGNAL(updateFolderLists()),
        empath, SLOT(s_updateFolderLists()));

    // Load each mailbox ( make new, tell it to read config )
    empathDebug("Reading " + QString().setNum(l.count()) +
        " mailboxes into list");

    EmpathMailbox::AccountType mailboxType = EmpathMailbox::Maildir;
    
    QStringList::ConstIterator it(l.begin());
    
    for (; it != l.end() ; ++it) {
        
        c->setGroup(EmpathConfig::GROUP_MAILBOX + *it);
        
        mailboxType =
            (EmpathMailbox::AccountType)
            c->readUnsignedNumEntry(EmpathConfig::KEY_MAILBOX_TYPE);

        EmpathMailbox * m = 0;

        switch (mailboxType) {
            
            case EmpathMailbox::Maildir:
                empathDebug("Adding new Maildir mailbox with name \"" +
                    *it + "\"");
                m = new EmpathMailboxMaildir(*it);
                CHECK_PTR(m);
                break;
            
            case EmpathMailbox::POP3:
                empathDebug("Adding new POP3 mailbox with name \"" +
                    *it + "\"");
                m = new EmpathMailboxPOP3(*it);
                CHECK_PTR(m);
                break;

            case EmpathMailbox::IMAP4:
                empathDebug("Adding new IMAP4 mailbox with name \"" +
                    *it + "\"");
                m = new EmpathMailboxIMAP4(*it);
                CHECK_PTR(m);
                break;
            
            default:
                empathDebug("Unknown mailbox");
                continue;
                break;
        }

        if (m == 0) {
            empathDebug("Mailbox is 0 ! (?)");
            continue;
        }
        
        empathDebug("Adding mailbox with name = " + m->name());
        QList::append(m);
        m->init();
    }
}

    void
EmpathMailboxList::saveConfig() const
{
    empathDebug("saveConfig() called");
    EmpathMailboxListIterator it(*this);
    
    QStrList l;
    
    for (; it.current(); ++it) {
        empathDebug("Mailbox with name '" + it.current()->name() + "' saved");
        // FIXME Use QStringList when it works for KConfig
        l.append(it.current()->name().ascii());
        it.current()->saveConfig();
    }
    
    KConfig * c = KGlobal::config();
    
    c->setGroup(EmpathConfig::GROUP_GENERAL);
    c->writeEntry(EmpathConfig::KEY_MAILBOX_LIST, l);
    c->sync();
}

// vim:ts=4:sw=4:tw=78
