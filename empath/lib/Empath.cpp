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
#include "EmpathMailSenderQMTP.h"
#include "EmpathMessageDataCache.h"
#include "EmpathFilterList.h"

Empath * Empath::EMPATH = 0;

Empath::Empath()
	: QObject()
{
	empathDebug("ctor");
	// Save a pointer to ourself
	EMPATH = this;
	mailSender_ = 0;
	
	empathDebug("Saving pid, hostname and start time");
	processID_ = getpid();
	
	_saveHostName();
	_setStartTime();
	
	empathDebug("Initialising mailbox list and filters");

	mailboxList_.init();
	filterList_.load();
	
	empathDebug("Initialising outgoing server");
	updateOutgoingServer();
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
Empath::reply(RMessage * m)
{
	empathDebug("reply() called");
	emit(newComposer(ComposeReply, m));
}

	void
Empath::replyAll(RMessage * m)
{
	empathDebug("replyAll() called");
	emit(newComposer(ComposeReplyAll, m));
}

	void
Empath::forward(RMessage * m)
{
	empathDebug("forward() called");
	emit(newComposer(ComposeForward, m));
}

	void
Empath::compose()
{
	empathDebug("compose() called");
	emit(newComposer(ComposeNormal, (RMessage *)0));
}

	void
Empath::updateOutgoingServer()
{
	empathDebug("Updating outgoing server type");

	if (mailSender_ != 0) {
		delete mailSender_;
		mailSender_ = 0;
	}
	else
		empathDebug("There was no outgoing server. Not attempting to delete");
	
	KConfig * c = kapp->getConfig();
	c->setGroup(GROUP_GENERAL);
	
	switch (
		(OutgoingServerType)(c->readUnsignedNumEntry(KEY_OUTGOING_SERVER_TYPE))) {
	
		case Sendmail:
			
			empathDebug("Setting up a sender using Sendmail");
			
			mailSender_ = new EmpathMailSenderSendmail;
			
			CHECK_PTR(mailSender_);
			mailSender_->readConfig();
			
			break;
		
		case Qmail:

			empathDebug("Setting up a sender using qmail");
			
			mailSender_ = new EmpathMailSenderQmail;
			
			CHECK_PTR(mailSender_);
			mailSender_->readConfig();
			
			break;
		
		case SMTP:
			
			empathDebug("Setting up a sender using SMTP");
			
			mailSender_ = new EmpathMailSenderSMTP;
			
			CHECK_PTR(mailSender_);
			mailSender_->readConfig();
			
			break;
		
		case QMTP:
			
			empathDebug("Setting up a sender using QMTP");
			
			mailSender_ = new EmpathMailSenderQMTP();

			CHECK_PTR(mailSender_);
			mailSender_->readConfig();
			
			break;

		default:
			mailSender_ = 0;
			break;
	}	
}

	void
Empath::_setStartTime()
{
	struct timeval timeVal;
	struct timezone timeZone;
	
	gettimeofday(&timeVal, &timeZone);
	startupSeconds_ = timeVal.tv_sec;

}
/*
	void
Empath::_showTipOfTheDay() const
{
	KConfig * c = kapp->getConfig();

	c->setGroup(GROUP_GENERAL);
	
	if (!c->readBoolEntry(KEY_TIP_OF_THE_DAY_AT_STARTUP, false)) return;

	EmpathTipOfTheDay totd(
			(QWidget *)0L,
			"tipWidget",
			kapp->kde_datadir() + "/empath/tips/EmpathTips",
			"",
			0);
	totd.exec();
}
*/
	void
Empath::statusMessage(const QString & messageText) const
{
	//mainWindow_->statusMessage(messageText, seconds * 1000);
}

	Q_UINT32
Empath::startTime() const
{
	return startupSeconds_; 
}
	
	pid_t
Empath::processID() const
{
	return processID_;
}
	
	QString
Empath::hostName() const
{
	return hostName_;
}
	
	EmpathMailboxList &
Empath::mailboxList()
{
	return mailboxList_;
}

	EmpathMailSender &
Empath::mailSender() const
{
	return *mailSender_;
}

	Empath *
Empath::getEmpath()
{
	return EMPATH;
}

	EmpathMessageDataCache &
Empath::messageDataCache() const
{
	return messageDataCache_;
}

	void
Empath::filter(const EmpathURL & source)
{
	empathDebug("filter \"" + source.asString() + "\" called");
	filterList_.filter(source);
	kapp->processEvents();
}

	const EmpathIndexRecord *
Empath::messageDescription(const RMessageID & id) const
{
	EmpathMailboxListIterator mit(mailboxList_);
	
	for (; mit.current(); ++mit) {
		
		EmpathFolderListIterator fit(mit.current()->folderList());
		
		for (; fit.current(); ++fit) {
			
			const EmpathIndexRecord * desc = fit.current()->messageDescription(id);

			if (desc->messageID() == id)
				return desc;
		}
	}

	return 0;
}

	void
Empath::_saveHostName()
{
	struct utsname utsName;
	if (uname(&utsName) == 0)
		hostName_ = utsName.nodename;
}

	void
Empath::_initFilters()
{
}

	EmpathFilterList &
Empath::filterList()
{
	return filterList_;
}

	Q_UINT32
Empath::sizeOfMessage(const EmpathURL & source)
{
	EmpathMailbox * m = mailbox(source.mailboxName());
	if (m == 0) return 0;
	return m->sizeOfMessage(source);
}

	QString
Empath::plainBodyOfMessage(const EmpathURL & source)
{
	EmpathMailbox * m = mailbox(source.mailboxName());
	if (m == 0) return "";
	return m->plainBodyOfMessage(source);
}

	REnvelope *
Empath::envelopeOfMessage(const EmpathURL & source)
{
	EmpathMailbox * m = mailbox(source.mailboxName());
	if (m == 0) return 0;
	return m->envelopeOfMessage(source);
}

	RMessage::MessageType
Empath::typeOfMessage(const EmpathURL & source)
{
	EmpathMailbox * m = mailbox(source.mailboxName());
	if (m == 0) return RMessage::BasicMessage;
	return m->typeOfMessage(source);
}

	RMessage *
Empath::message(const EmpathURL & source)
{
	EmpathMailbox * m = mailbox(source.mailboxName());
	if (m == 0) return 0;
	return m->message(source);
}

	EmpathMailbox *
Empath::mailbox(const EmpathURL & url)
{
	empathDebug("mailbox() called, looking for " + url.mailboxName());
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
	
	EmpathMailbox * m = mailbox(url.mailboxName());
	
	if (m == 0) return 0;

	return m->folder(url.folderPath());
}

	bool
Empath::removeMessage(const EmpathURL & ref)
{
	EmpathMailbox * m = mailbox(ref.mailboxName());
	if (m == 0) return false;
	return m->removeMessage(ref);
}

	void
Empath::s_updateFolderLists()
{
	emit (updateFolderLists());
}

