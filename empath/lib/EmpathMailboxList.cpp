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
#include "Empath.h"
#include "EmpathConfig.h"
#include "EmpathMailbox.h"
#include "EmpathMailboxList.h"
#include "EmpathMailboxPOP3.h"
#include "EmpathMailboxIMAP4.h"
#include "EmpathMailboxMaildir.h"
#include "EmpathDefines.h"

EmpathMailboxList::EmpathMailboxList()
{ 
    setAutoDelete(true);
}

EmpathMailboxList::~EmpathMailboxList()
{
    // Empty.
}

    bool
EmpathMailboxList::remove(const EmpathURL & u)
{
    if (!QDict<EmpathMailbox>::remove(u.mailboxName())) {
        empathDebug("Couldn't remove mailbox");
        return false;
    }
    
    saveConfig();
    
    emit(updateFolderLists());

    return true;
}

    EmpathFolder *
EmpathMailboxList::folder(const EmpathURL & url) const
{
    EmpathMailbox * m = find(url.mailboxName());
    return (m == 0) ? 0 : m->folder(url);
}

    void
EmpathMailboxList::loadConfig()
{
    QObject::connect(
        this,   SIGNAL(updateFolderLists()),
        empath, SLOT(s_updateFolderLists()));

    QDict<EmpathMailbox>::clear();

    KConfig * c(KGlobal::config());
    
    using namespace EmpathConfig;
    c->setGroup(GROUP_GENERAL);
    
    QStringList l;
    l = c->readListEntry(GEN_MAILBOX_LIST);
    
    EmpathMailbox::Type mailboxType = EmpathMailbox::Maildir;
    
    QStringList::ConstIterator it(l.begin());
    
    for (; it != l.end() ; ++it) {
        
        c->setGroup(GROUP_MAILBOX + *it);
        
        mailboxType =
            (EmpathMailbox::Type)
            c->readUnsignedNumEntry(M_TYPE);

        EmpathMailbox * m = createNew(mailboxType);

        m->setName(*it);

        if (m == 0) {
            empathDebug("Couldn't create mailbox !");
            continue;
        }
        
        m->init();
    }
}

    void
EmpathMailboxList::saveConfig() const
{
    EmpathMailboxListIterator it(*this);
    
    QStringList l;
    
    for (; it.current(); ++it) {
        l << it.current()->name();
        it.current()->saveConfig();
    }
    
    KConfig * c = KGlobal::config();
    
    using namespace EmpathConfig;
    c->setGroup(GROUP_GENERAL);
    c->writeEntry(GEN_MAILBOX_LIST, l);
    c->sync();
}

    QString
EmpathMailboxList::uniqueName()
{
    QString s("Unnamed");

    int idx = 0;
    bool taken = true;
    
    while (taken) {
        
        taken = false;
        
        EmpathMailboxListIterator it(*this);

        for (; it.current(); ++it)
            if (it.current()->name() == s)
                taken = true;

        if (taken)
            s = "Unnamed" + QString().setNum(++idx);
    }

    return s;
}

    EmpathMailbox *
EmpathMailboxList::createNew(EmpathMailbox::Type t)
{
    EmpathMailbox * m(0);
    
    switch (t) {

        case EmpathMailbox::Maildir:
            m = new EmpathMailboxMaildir(uniqueName());
            break;

        case EmpathMailbox::POP3:
            m = new EmpathMailboxPOP3(uniqueName());
            break;

        case EmpathMailbox::IMAP4:
            m = new EmpathMailboxIMAP4(uniqueName());
            break;

        default:
            break;
    }

    if (m == 0) {
        empathDebug("Cannot create new mailbox");
        return 0;
    }

    _append(m);
    return m;
}

    void
EmpathMailboxList::s_rename(EmpathMailbox * mailbox, const QString & oldName)
{
    setAutoDelete(false);
    QDict<EmpathMailbox>::remove(oldName);
    QDict<EmpathMailbox>::insert(mailbox->name(), mailbox);
    setAutoDelete(true);
    emit(updateFolderLists());
}

    void
EmpathMailboxList::_append(EmpathMailbox * mailbox)
{
    if (find(mailbox->name()))
        return;
    
    QDict<EmpathMailbox>::insert(mailbox->name(), mailbox);

    QObject::connect(
        mailbox,    SIGNAL(rename(EmpathMailbox *, const QString &)),
        this,       SLOT(s_rename(EmpathMailbox *, const QString &)));
    
    emit(updateFolderLists());
}


// vim:ts=4:sw=4:tw=78
