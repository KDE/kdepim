/*
	Empath - Mailer for KDE
	
	Copyright (C) 1998 Rik Hemsley rik@kde.org
	
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

// System includes
#include <iostream.h>

// Qt includes
#include <qstring.h>

// KDE includes
#include <kconfig.h>
#include <klocale.h>
#include <kapp.h>

// Local includes
#include "EmpathMailbox.h"
#include "EmpathMailboxList.h"
#include "EmpathMailboxPOP3.h"
#include "EmpathMailboxIMAP4.h"
#include "EmpathMailboxMaildir.h"
#include "EmpathMailboxMMDF.h"
#include "EmpathMailboxMbox.h"
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
	KConfig * config_ = kapp->getConfig();
	
	// Save the config group.
	KConfigGroupSaver cgs(config_, GROUP_GENERAL);
	
	// Save how many mailboxes we have.
	config_->writeEntry(KEY_NUM_MAILBOXES, count());

	QObject::QObject::connect(mailbox, SIGNAL(newMailArrived()),
		empath, SLOT(s_newMailArrived()));
	
	emit(updateFolderLists());
}

	bool
EmpathMailboxList::remove(EmpathMailbox * mailbox)
{
	empathDebug("remove");

	QListIterator<EmpathMailbox> it(*this);

	for (; it.current() ; ++it) {
		
		if (it.current()->name() == mailbox->name()) {
			
			QList::remove(it.current());
			
			empathDebug("Saving mailbox list count = " +
					QString().setNum(count()));

			// Save the number of mailboxes into the config.
			KConfig * config_ = kapp->getConfig();
			
			// Save the config group.
			KConfigGroupSaver cgs(config_, GROUP_GENERAL);
			
			// Save how many mailboxes we have.
			config_->writeEntry(KEY_NUM_MAILBOXES, count());
		}

		emit(updateFolderLists());
		return true;
	}

	return false;
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
		it.current()->getNewMail();
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
	KConfig * c = kapp->getConfig();
	c->setGroup(GROUP_GENERAL);
	
	QStrList l;
	c->readListEntry(KEY_MAILBOX_LIST, l);
	
	QObject::connect(this, SIGNAL(updateFolderLists()),
		empath, SLOT(s_updateFolderLists()));

	// Load each mailbox ( make new, tell it to read config )
	empathDebug("Reading " + QString().setNum(l.count()) + " mailboxes into list");

	AccountType mailboxType = Maildir;
	
	QStrListIterator it(l);
	
	for (; it.current() ; ++it) {
		
		c->setGroup(it.current());
		
		mailboxType = (AccountType)c->readUnsignedNumEntry(KEY_MAILBOX_TYPE);

		EmpathMailbox * m = 0;

		switch (mailboxType) {
			
			case Maildir:
				empathDebug("Adding new Maildir mailbox with name \"" +
					QString(it.current()) + "\"");
				m = new EmpathMailboxMaildir(it.current());
				CHECK_PTR(m);
				break;
			
			case Mbox:
				empathDebug("Adding new Mbox mailbox with name \"" +
					QString(it.current()) + "\"");
				m = new EmpathMailboxMbox(it.current());
				CHECK_PTR(m);
				break;
	
			case MMDF:
				empathDebug("Adding new MMDF mailbox with name \"" +
					QString(it.current()) + "\"");
				m = new EmpathMailboxMMDF(it.current());
				CHECK_PTR(m);
				break;
		
			case POP3:
				empathDebug("Adding new POP3 mailbox with name \"" +
					QString(it.current()) + "\"");
				m = new EmpathMailboxPOP3(it.current());
				CHECK_PTR(m);
				break;

			case IMAP4:
				empathDebug("Adding new IMAP4 mailbox with name \"" +
					QString(it.current()) + "\"");
				m = new EmpathMailboxIMAP4(it.current());
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
		l.append(it.current()->name());
		it.current()->saveConfig();
	}
	
	KConfig * c = kapp->getConfig();
	
	c->setGroup(GROUP_GENERAL);
	c->writeEntry(KEY_MAILBOX_LIST, l);
}

