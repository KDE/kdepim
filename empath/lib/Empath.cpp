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
#include "EmpathMessageDataCache.h"
#include "EmpathFilterList.h"

Empath * Empath::EMPATH = 0;

Empath::Empath()
	:	QObject(),
		mailSender_(0)
{
	empathDebug("ctor");
	EMPATH = this;
	
	empathDebug("Saving pid, hostname and start time");
	processID_ = getpid();
	_saveHostName();
	_setStartTime();
	
	cache_.setMaxCost(1048576); // 1Mb cache

	empathDebug("===========================================================");	
	empathDebug("Initialising mailboxes");
	mailboxList_.init();
	empathDebug("===========================================================");	
	empathDebug("Initialising filters");
	filterList_.load();
	empathDebug("===========================================================");	
	empathDebug("Initialising outgoing server");
	updateOutgoingServer();
	empathDebug("===========================================================");	
}

Empath::~Empath()
{
	empathDebug("dtor");

	ASSERT(mailSender_ != 0);
	empathDebug("Deleting mail sender");
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
Empath::s_newMailArrived()
{
	empathDebug("Got new mail arrived signal");
	emit(newMailArrived());
}

	void
Empath::updateOutgoingServer()
{
	empathDebug("Updating outgoing server type");

	if (mailSender_ == 0) {
		empathDebug("There was no outgoing server. Not attempting to delete");
	} else {
		delete mailSender_;
		mailSender_ = 0;
	}
	
	KConfig * c = kapp->getConfig();
	c->setGroup(EmpathConfig::GROUP_GENERAL);
	
	OutgoingServerType st =
		(OutgoingServerType)
		(c->readUnsignedNumEntry(EmpathConfig::KEY_OUTGOING_SERVER_TYPE));
	
	switch (st) {
	
		case Sendmail:
			mailSender_ = new EmpathMailSenderSendmail;
			CHECK_PTR(mailSender_);
			break;
		
		case Qmail:
			mailSender_ = new EmpathMailSenderQmail;
			CHECK_PTR(mailSender_);
			break;
		
		case SMTP:
			mailSender_ = new EmpathMailSenderSMTP;
			CHECK_PTR(mailSender_);
			break;
		
		default:
			mailSender_ = 0;
			return;
			break;
	}

	mailSender_->readConfig();
}

	void
Empath::_setStartTime()
{
	struct timeval timeVal;
	struct timezone timeZone;
	
	gettimeofday(&timeVal, &timeZone);
	startupSeconds_ = timeVal.tv_sec;
}

	void
Empath::statusMessage(const QString & messageText) const
{
}

	void
Empath::filter(const EmpathURL & source)
{
	empathDebug("filter \"" + source.asString() + "\" called");
	filterList_.filter(source);
	kapp->processEvents();
}

	void
Empath::_saveHostName()
{
	struct utsname utsName;
	if (uname(&utsName) == 0)
		hostName_ = utsName.nodename;
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
	
	// Put the message in the cache, and return it.
	if (!cache_.insert(source.messageID(), message, message->size())) {
		delete message;
		message = 0;
		return 0;
	}

	return message;
}

	EmpathMailbox *
Empath::mailbox(const EmpathURL & url)
{
	empathDebug("mailbox() called, looking for \"" + url.mailboxName() + "\"");
	EmpathMailboxListIterator it(mailboxList_);
	for (; it.current(); ++it) {
		empathDebug("Looking at mailbox \"" + it.current()->name() + "\"");
		if (it.current()->name() == url.mailboxName()) {
			empathDebug("... found !");
			return it.current();
		}
	}
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

	void
Empath::s_remove(const EmpathURL & url)
{
	EmpathMailbox * m = mailbox(url.mailboxName());
	if (m == 0) return;
	m->removeMessage(url);
}

	void
Empath::s_mark(const EmpathURL & url, RMM::MessageStatus s)
{
	EmpathMailbox * m = mailbox(url.mailboxName());
	if (m == 0) return;
//	m->mark(url, s);
}

	void
Empath::s_setupDisplay()
{
	emit(setupDisplay());
	empathDebug("setupDisplay() called");
}
	void
Empath::s_setupIdentity()
{
	emit(setupIdentity());
}

	void
Empath::s_setupSending()
{
	emit(setupSending());
}

	void
Empath::s_setupComposing()
{
	emit(setupComposing());
}

	void
Empath::s_setupAccounts()
{
	emit(setupAccounts());
}

	void
Empath::s_setupFilters()
{
	emit(setupFilters());
}

