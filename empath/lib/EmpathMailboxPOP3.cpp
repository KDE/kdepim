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

#ifdef __GNUG__
# pragma implementation "EmpathMailboxPOP3.h"
#endif

// Qt includes
#include <qregexp.h>
#include <qdir.h>

// KDE includes
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kio_job.h>

// Local includes
#include "EmpathMailboxPOP3.h"
#include "EmpathMessageList.h"
#include "EmpathFolderList.h"
#include "Empath.h"
#include "EmpathConfig.h"
#include "EmpathUtilities.h"

	int
EmpathPOPIndex::compareItems(EmpathPOPIndexEntry * i1, EmpathPOPIndexEntry * i2)
{
	return i1->number() - i2->number();
}

EmpathPOPCommand::EmpathPOPCommand(EmpathPOPCommand::Type t, int n)
	:	type_(t),
		msgNo_(n)
{
	empathDebug("ctor");

	switch (t) {
		
		case Stat:		command_ = "stat";		break;
		case List:		command_ = "list";		break;
		case UIDL:		command_ = "uidl";		break;
		case Get:		command_ = "download";	break;
		case Remove:	command_ = "remove";	break;
		default:								break;
	}

	if (n != -1)
		command_ += '/' + QString().setNum(n);
}

EmpathPOPCommand::~EmpathPOPCommand()
{
	empathDebug("dtor");
}

	QString
EmpathPOPCommand::command()
{
	return command_;
}

	EmpathPOPCommand::Type
EmpathPOPCommand::type()
{
	return type_;
}

EmpathPOPIndexEntry::EmpathPOPIndexEntry(int i, const QString & s)
	:	number_(i),
		uidl_(s)
{
	empathDebug("ctor");
}

EmpathPOPIndexEntry::~EmpathPOPIndexEntry()
{
	empathDebug("dtor");
}

	int
EmpathPOPIndexEntry::number()
{
	return number_;
}

	QString
EmpathPOPIndexEntry::uidl()
{
	return uidl_;
}

EmpathMailboxPOP3::EmpathMailboxPOP3(const QString & name)
	:	EmpathMailbox		(name),
		serverPort_			(110),
		logging_			(false),
		numMessages_		(0),
		mailboxSize_		(0),
		logFileOpen_		(false),
		authenticationTries_(8)
{
	empathDebug("ctor");

	type_	= POP3;
	
	job		= new KIOJob();
	CHECK_PTR(job);
	
	commandQueue_.setAutoDelete(true);
}

EmpathMailboxPOP3::~EmpathMailboxPOP3()
{
	empathDebug("dtor");
	delete job;
}

	void
EmpathMailboxPOP3::init()
{
	empathDebug("init() called");
	
	QObject::connect(
		job, SIGNAL(sigFinished(int)),
		this, SLOT(s_jobFinished(int)));
	
	QObject::connect(
		job, SIGNAL(sigData(int, const char *, int)),
		this, SLOT(s_data(int, const char *, int)));
	
//	QString folderPixmapName = "mini-folder-inbox.png";

	EmpathURL url(url_);
	url.setFolderPath(i18n("Inbox"));
	
	EmpathFolder * folder_inbox = new EmpathFolder(url);
	folderList_.append(folder_inbox);
}

	bool
EmpathMailboxPOP3::alreadyHave()
{
	return false;
}

	void
EmpathMailboxPOP3::_enqueue(EmpathPOPCommand::Type t, int i)
{
	EmpathPOPCommand * p = new EmpathPOPCommand(t, i);
	commandQueue_.enqueue(p);
}

	void
EmpathMailboxPOP3::_nextCommand()
{
	if (commandQueue_.isEmpty())
		return;

	QString prefix =
		"pop://" + username_ + ":" + password_ + "@" + serverAddress_ + "/";

	QString command = prefix + commandQueue_.head()->command();
}

	bool
EmpathMailboxPOP3::checkForNewMail()
{
	empathDebug("_checkForNewMail() called");
	_enqueue(EmpathPOPCommand::Stat);
	
	// XXX

	EmpathPOPIndexIterator it(index_);

	for (; it.current(); ++it)
		_enqueue(EmpathPOPCommand::Get, it.current()->number());

	// XXX

	return false;
}

	bool
EmpathMailboxPOP3::getMail()
{
	return false;
}

	void
EmpathMailboxPOP3::s_checkNewMail()
{
	empathDebug("checkNewMail()");
	checkForNewMail();
}

	QString
EmpathMailboxPOP3::writeMessage(const EmpathURL &, RMM::RMessage &)
{
	empathDebug("writeMessage() called");
	empathDebug("This mailbox is READ ONLY !");
	return QString::null; 
}
	
	bool
EmpathMailboxPOP3::newMail() const
{
	return false;
}

	void
EmpathMailboxPOP3::syncIndex(const EmpathURL &)
{
	if (!job->get("uidl"));
		job->get("index");
}

	Q_UINT32
EmpathMailboxPOP3::sizeOfMessage(const EmpathURL &)
{
	return 0;
}

	QString
EmpathMailboxPOP3::plainBodyOfMessage(const EmpathURL &)
{
	return "";
}

	RMM::REnvelope *
EmpathMailboxPOP3::envelopeOfMessage(const EmpathURL & _id)
{
	empathDebug("getEnvelopeOfMessage(" + _id.asString() + ") called");
	return 0;
}

	RMM::RBodyPart::PartType
EmpathMailboxPOP3::typeOfMessage(const EmpathURL & _id)
{
	empathDebug("getTypeOfMessage(" + _id.asString() + ") called");
	return RMM::RBodyPart::Basic;
}

	EmpathURL
EmpathMailboxPOP3::path()
{
	return url_;
}

	RMM::RMessage *
EmpathMailboxPOP3::message(const EmpathURL & url)
{
//	_enqueue(EmpathPOPCommand::Get, url.messageID());
	// XXX
	return 0;
}

	bool
EmpathMailboxPOP3::mark(
	const EmpathURL &, const QStringList &, RMM::MessageStatus)
{
	return false;
}

	bool
EmpathMailboxPOP3::removeMessage(const EmpathURL &, const QStringList & l)
{
//	QStringList::ConstIterator it(l.begin());
	
//	for (; it != l.end(); ++it)
//		_enqueue(EmpathPOPCommand::Remove, *it);
	
	// XXX
	return false;
}

	bool
EmpathMailboxPOP3::removeMessage(const EmpathURL & url)
{
//	_enqueue(EmpathPOPCommand::Remove, url.messageID());
	// XXX
	return false;
}

	void
EmpathMailboxPOP3::s_jobFinished(int)
{
}

	void
EmpathMailboxPOP3::s_data(int, const char *, int)
{
	if (commandQueue_.isEmpty()) {
		// Whoah ! We should be waiting for completion.
		empathDebug("Command queue empty in s_data");
		return;
	}
	
	switch (commandQueue_.head()->type()) {
			
		case EmpathPOPCommand::Stat:
			break;
		
		case EmpathPOPCommand::List:
			break;
			
		case EmpathPOPCommand::UIDL:
			break;
		
		case EmpathPOPCommand::Get:
			break;
		
		case EmpathPOPCommand::Remove:
			break;
		
		default:
			break;
	}
}

// Illegal operations follow

	bool
EmpathMailboxPOP3::addFolder(const EmpathURL &)
{
	return false;
}

	bool
EmpathMailboxPOP3::removeFolder(const EmpathURL &)
{
	return false;
}
	bool
EmpathMailboxPOP3::mark(const EmpathURL &, RMM::MessageStatus)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////////
///////////////////////// CONFIG STUFF FOLLOWS TO END ////////////////////////
//////////////////////////////////////////////////////////////////////////////

	void
EmpathMailboxPOP3::s_getNewMail()
{
	empathDebug("getNewMail()");
}

	void
EmpathMailboxPOP3::saveConfig()
{
	empathDebug("Saving config");
	KConfig * config_ = KGlobal::config();
	config_->setGroup(EmpathConfig::GROUP_MAILBOX + url_.mailboxName());
#define CWE config_->writeEntry
	CWE(EmpathConfig::KEY_MAILBOX_TYPE,					(int)type_);
	CWE(EmpathConfig::KEY_POP3_SERVER_ADDRESS,			serverAddress_);
	CWE(EmpathConfig::KEY_POP3_SERVER_PORT,				serverPort_);
	CWE(EmpathConfig::KEY_POP3_USERNAME,				username_);
	CWE(EmpathConfig::KEY_POP3_PASSWORD,				password_);
	CWE(EmpathConfig::KEY_POP3_APOP,					useAPOP_);
	CWE(EmpathConfig::KEY_POP3_SAVE_POLICY,				(int)passwordSavePolicy_);
	CWE(EmpathConfig::KEY_POP3_LOGGING_POLICY,			logging_);
	CWE(EmpathConfig::KEY_POP3_LOG_FILE_PATH,			logFilePath_);
	CWE(EmpathConfig::KEY_POP3_LOG_FILE_DISPOSAL_POLICY,logFileDisposalPolicy_);
	CWE(EmpathConfig::KEY_POP3_MAX_LOG_FILE_SIZE,		maxLogFileSize_);
	CWE(EmpathConfig::KEY_POP3_CHECK_FOR_NEW_MAIL,		checkMail_);
	CWE(EmpathConfig::KEY_POP3_MAIL_CHECK_INTERVAL,		checkMailInterval_);
	CWE(EmpathConfig::KEY_POP3_RETRIEVE_IF_HAVE,		retrieveIfHave_);
#undef CWE
}

	void
EmpathMailboxPOP3::readConfig()
{
	empathDebug("Reading config");
	KConfig * config_ = KGlobal::config();
	config_->setGroup(EmpathConfig::GROUP_MAILBOX + url_.mailboxName());
	
// For some reason, this just DOES NOT WORK here ! Need to do setGroup !
//	KConfigGroupSaver(config_, canonName_);
#define CRE config_->readEntry
#define CRUNE config_->readUnsignedNumEntry
#define CRBE config_->readBoolEntry
	
	empathDebug("Config group is now \"" + QString(config_->group()) + "\"");

	url_.setMailboxName(CRE(EmpathConfig::KEY_MAILBOX_NAME, "Unnamed"));

	serverAddress_ =
		CRE(EmpathConfig::KEY_POP3_SERVER_ADDRESS, i18n("<unknown>"));
	
	serverPort_ =
		CRUNE(EmpathConfig::KEY_POP3_SERVER_PORT, 110);
	
	config_->setDollarExpansion(true);
	username_ =
		CRE(EmpathConfig::KEY_POP3_USERNAME, "$USER");
	config_->setDollarExpansion(false);
	
	password_ =
		CRE(EmpathConfig::KEY_POP3_PASSWORD, "");
	
	useAPOP_ =
		CRBE(EmpathConfig::KEY_POP3_APOP, true);
	
	passwordSavePolicy_ =
		(SavePolicy)
		CRUNE(EmpathConfig::KEY_POP3_SAVE_POLICY, Never);
	
	logging_ =
		CRBE(EmpathConfig::KEY_POP3_LOGGING_POLICY,	false);
	
	logFilePath_ =
		CRE(EmpathConfig::KEY_POP3_LOG_FILE_PATH,
		QDir::homeDirPath() + "/.kde/share/apps/empath/log/");
	
	logFileDisposalPolicy_	=
		CRBE(EmpathConfig::KEY_POP3_LOG_FILE_DISPOSAL_POLICY, false);
	
	maxLogFileSize_ =
		CRUNE(EmpathConfig::KEY_POP3_MAX_LOG_FILE_SIZE,	10);
	
	checkMail_ =
		CRBE(EmpathConfig::KEY_POP3_CHECK_FOR_NEW_MAIL, true);
	
	checkMailInterval_ =
		CRUNE(EmpathConfig::KEY_POP3_MAIL_CHECK_INTERVAL, 5);
	
	retrieveIfHave_ =
		CRBE(EmpathConfig::KEY_POP3_RETRIEVE_IF_HAVE, false);
	
#undef CRE
#undef CRUNE
#undef CRBE
}

// Set methods
		
	void
EmpathMailboxPOP3::setServerAddress(const QString & serverAddress)
{
	serverAddress_	= serverAddress;
}

	void
EmpathMailboxPOP3::setServerPort(Q_UINT32 serverPort)
{
	serverPort_ = serverPort;
}

	void
EmpathMailboxPOP3::setUsername(const QString & username)
{
	username_ = username;
}

	void
EmpathMailboxPOP3::setUseAPOP(bool yn)
{
	useAPOP_ = yn;
}

	void
EmpathMailboxPOP3::setPassword(const QString & password)
{
	password_ = password;
}

	void
EmpathMailboxPOP3::setPasswordSavePolicy(SavePolicy policy)
{
	passwordSavePolicy_ = policy;
}

	void
EmpathMailboxPOP3::setLoggingPolicy(bool policy)
{
	loggingPolicy_ = policy;
}

	void
EmpathMailboxPOP3::setLogFilePath(const QString & logPath)
{
	logFilePath_ = logPath;
}

	void
EmpathMailboxPOP3::setLogFileDisposalPolicy(bool policy)
{
	logFileDisposalPolicy_ = policy;
}

	void
EmpathMailboxPOP3::setMaxLogFileSize(Q_UINT32 maxSize)
{
	maxLogFileSize_ = maxSize; 
}

	void
EmpathMailboxPOP3::setRetrieveIfHave(bool yn)
{
	retrieveIfHave_ = yn;
}

// Get methods
		
	QString
EmpathMailboxPOP3::serverAddress()
{
	return serverAddress_;
}

	Q_UINT32
EmpathMailboxPOP3::serverPort()
{
	return serverPort_;
}

	QString
EmpathMailboxPOP3::username()
{
	return username_;
}

	QString
EmpathMailboxPOP3::password()
{
	return password_;
}

	bool
EmpathMailboxPOP3::useAPOP()
{
	return useAPOP_;
}

	EmpathMailbox::SavePolicy
EmpathMailboxPOP3::passwordSavePolicy()
{ 
	return passwordSavePolicy_;
}

	bool
EmpathMailboxPOP3::loggingPolicy()
{ 
	return loggingPolicy_;
}

	QString
EmpathMailboxPOP3::logFilePath()
{
	return logFilePath_;
}

	bool
EmpathMailboxPOP3::logFileDisposalPolicy()
{
	return logFileDisposalPolicy_;
}

	Q_UINT32
EmpathMailboxPOP3::maxLogFileSize()
{
	return maxLogFileSize_;
}

	bool
EmpathMailboxPOP3::retrieveIfHave()
{
	return retrieveIfHave_;
}

	bool
EmpathMailboxPOP3::logging()
{
	return logging_;
}

	void
EmpathMailboxPOP3::setLogging(bool policy)
{
	logging_ = policy;
}


