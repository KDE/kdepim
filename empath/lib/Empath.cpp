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
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <errno.h>

// Qt includes
#include <qtimer.h>
#include <qlist.h>
#include <qdatetime.h>
#include <qfile.h>
#include <qfileinfo.h>

// KDE includes
#include <kconfig.h>
#include <kapp.h>

// Local includes
#include "EmpathDefines.h"
#include "EmpathConfig.h"
#include "Empath.h"
#include "EmpathMailboxList.h"
#include "EmpathIndexRecord.h"
#include "EmpathMessageList.h"
#include "EmpathMailSenderSendmail.h"
#include "EmpathMailSenderQmail.h"
#include "EmpathMailSenderSMTP.h"
#include "EmpathFilterList.h"
#include "EmpathTask.h"
#include "EmpathTaskTimer.h"

Empath * Empath::EMPATH = 0;

Empath::Empath()
	:	QObject(),
		mailSender_(0)
{
	empathDebug("ctor");
	EMPATH = this;
	cache_.setMaxCost(1048576);		// 1Mb cache
	updateOutgoingServer();			// Must initialise the pointer.
}

	void
Empath::init()
{
	empathDebug("====================== INIT  START ========================");	
	processID_ = getpid();
	_saveHostName();
	_setStartTime();
	mailboxList_.init();
	filterList_.load();
	empathDebug("======================= INIT END =========================");	
}

Empath::~Empath()
{
	empathDebug("dtor");
	delete mailSender_;
	mailSender_ = 0;
}

	void
Empath::s_saveConfig()
{
	filterList_.save();
	mailboxList_.saveConfig();
	kapp->getConfig()->sync();
}

	void
Empath::updateOutgoingServer()
{
	delete mailSender_;
	mailSender_ = 0;
	
	KConfig * c = kapp->getConfig();
	c->setGroup(EmpathConfig::GROUP_GENERAL);
	
	OutgoingServerType st =
		(OutgoingServerType)
		(c->readUnsignedNumEntry(EmpathConfig::KEY_OUTGOING_SERVER_TYPE,
								 Sendmail));
	
	switch (st) {
		case Sendmail:	mailSender_ = new EmpathMailSenderSendmail;	break;
		case Qmail:		mailSender_ = new EmpathMailSenderQmail;	break;
		case SMTP:		mailSender_ = new EmpathMailSenderSMTP;		break;
		default:		mailSender_ = 0; return;					break;
	}

	CHECK_PTR(mailSender_);

	mailSender_->readConfig();
}

	RMessage *
Empath::message(const EmpathURL & source)
{
	empathDebug("message(" + source.asString() + ") called");
	
	// Try and get the message from the cache.
	
	RMessage * message(cache_[source.messageID()]);
	
	if (message != 0) {
		empathDebug("message \"" + source.asString() + "\" found in cache");
		return message;
	}
	
	empathDebug("message \"" + source.asString() + "\" not in cache");
	
	// It's not in the cache. Get it from the mailbox.
	EmpathMailbox * m = mailbox(source);
	
	if (m == 0) return 0;
	
	message = m->message(source);
	
	if (message == 0) return 0;
	
	// Put the message in the cache.
	if (!cache_.insert(source.messageID(), message, message->size())) {
		delete message;
		message = 0;
	}

	return message;
}

	EmpathMailbox *
Empath::mailbox(const EmpathURL & url)
{
	EmpathMailboxListIterator it(mailboxList_);
	
	for (; it.current(); ++it)
		if (it.current()->name() == url.mailboxName())
			return it.current();
	
	return 0;
}

	EmpathFolder *
Empath::folder(const EmpathURL & url)
{
	empathDebug("folder(" + url.asString() + ") called");
	EmpathMailbox * m = mailbox(url);
	if (m == 0) return 0;
	return m->folder(url);
}

	bool
Empath::remove(const EmpathURL & url)
{
	EmpathMailbox * m = mailbox(url);
	if (m == 0) return false;
	return m->removeMessage(url);
}

	bool
Empath::mark(const EmpathURL & url, RMM::MessageStatus s)
{
	EmpathFolder * f = folder(url);
	if (f == 0) return false;
	return f->mark(url, s);
}


	EmpathTask *
Empath::addTask(const QString & name)
{
	EmpathTask * t = new EmpathTask(name);
	CHECK_PTR(t);
	EmpathTaskTimer * timer = new EmpathTaskTimer(t);
	return t;
}


// Private methods follow

	void
Empath::_setStartTime()
{
	struct timeval timeVal;
	struct timezone timeZone;
	
	gettimeofday(&timeVal, &timeZone);
	startupSeconds_ = timeVal.tv_sec;
}
	void
Empath::_saveHostName()
{
	struct utsname utsName;
	if (uname(&utsName) == 0)
		hostName_ = utsName.nodename;
}

