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

EmpathMailboxPOP3::EmpathMailboxPOP3(const QString & name)
	:	EmpathMailbox		(name),
		serverAddress_		(QString::null),
		serverPort_			(110),
		username_			(QString::null),
		password_			(QString::null),
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
	
	QObject::connect(
		job, SIGNAL(sigFinished(int)),
		this, SLOT(s_jobFinished(int)));
	
	QObject::connect(
		job, SIGNAL(sigData(int, const char *, int)),
		this, SLOT(s_data(int, const char *, int)));
	
	QString folderPixmapName = "mini-folder-inbox.png";
	QString inboxName = i18n("Inbox");

	EmpathURL url(url_);
	url.setFolderPath(inboxName);
	
	EmpathFolder * folder_inbox = new EmpathFolder(url);
	
	folderList_.append(folder_inbox);
}

EmpathMailboxPOP3::~EmpathMailboxPOP3()
{
	empathDebug("dtor");
//	delete job;
}

	bool
EmpathMailboxPOP3::alreadyHave()
{
	return false;
}

	bool
EmpathMailboxPOP3::checkForNewMail()
{
	empathDebug("_checkForNewMail() called");

	return false;
}

	bool
EmpathMailboxPOP3::getMail()
{
	empathDebug("_getMail() called");
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
	empathDebug("newMail() called");
	return false;
}

	void
EmpathMailboxPOP3::syncIndex(const EmpathURL &)
{
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
EmpathMailboxPOP3::message(const EmpathURL &)
{
	return 0;
}

	void
EmpathMailboxPOP3::init()
{
	empathDebug("init() called");
}

	bool
EmpathMailboxPOP3::removeMessage(const EmpathURL &)
{
	return false;
}

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

	void
EmpathMailboxPOP3::s_data(int, const char *, int)
{
#if 0
	switch (state_) {
		
		case WaitForList:
			int i = 0;
			int x = 0;
			while ((x = s.find(QRegExp("\n"), i)) != -1) {
				dict[].setSize(
			}
			break;
			
		case WaitForUIDL:
			break;
		
		case WaitForData:
			break;
		
		case NoWait:
		default:
			break;
	}
#endif
}

	void
EmpathMailboxPOP3::s_jobFinished(int)
{
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

	bool
EmpathMailboxPOP3::mark(
	const EmpathURL &, const QStringList &, RMM::MessageStatus)
{
	return false;
}

	bool
EmpathMailboxPOP3::removeMessage(const EmpathURL &, const QStringList &)
{
	return false;
}

