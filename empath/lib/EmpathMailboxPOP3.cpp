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
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

// Qt includes
#include <qregexp.h>
#include <qdir.h>

// KDE includes
#include <klocale.h>
#include <kconfig.h>

// Local includes
#include "EmpathMailboxPOP3.h"
#include "EmpathMessageList.h"
#include "EmpathFolderList.h"
#include "Empath.h"
#include "EmpathConfig.h"
#include "EmpathUtilities.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

QString EmpathMailboxPOP3::COMMAND_APOP	=	"APOP";
QString EmpathMailboxPOP3::COMMAND_LIST	=	"LIST";
QString EmpathMailboxPOP3::COMMAND_UIDL	=	"UIDL";
QString EmpathMailboxPOP3::COMMAND_USER	=	"USER";
QString EmpathMailboxPOP3::COMMAND_PASS	=	"PASS";
QString EmpathMailboxPOP3::COMMAND_STAT	=	"STAT";
QString EmpathMailboxPOP3::COMMAND_RETR	=	"RETR";
QString EmpathMailboxPOP3::COMMAND_DELE	=	"DELE";
QString EmpathMailboxPOP3::COMMAND_RSET	=	"RSET";
QString EmpathMailboxPOP3::COMMAND_QUIT	=	"QUIT";
QString EmpathMailboxPOP3::COMMAND_NOOP	=	"NOOP";
QString EmpathMailboxPOP3::COMMAND_TOP		=	"TOP";


EmpathMailboxPOP3::EmpathMailboxPOP3(const QString & name)
	:	EmpathMailbox(name),
		serverAddress_(""),
		serverPort_(110),
		username_(""),
		password_(""),
		logging_(false),
		numMessages_(0),
		mailboxSize_(0),
		logFileOpen_(false),
		state_(Disconnected),
		authenticationTries_(8)
{
	/*
	empathDebug("ctor");
	type_ = POP3;
	
	errorStr	= "Unknown Error!";
	
	QString folderPixmapName = "mini-folder-inbox.xpm";
	QString inboxName = i18n("Inbox");

	EmpathURL url("empath://" + name_ + inboxName);
	
	EmpathFolder * folder_inbox = new EmpathFolder(url);
	
	folderList_.append(*folder_inbox);
	*/
}

EmpathMailboxPOP3::~EmpathMailboxPOP3()
{
	/*
	empathDebug("dtor");
	_changeState(Disconnected);
	*/
}

	bool
EmpathMailboxPOP3::_supportsAPOP()
{
	/*
	empathDebug("_supportsAPOP() called");

	empathDebug("greeting string was: \"" + greeting_ + "\"");

	if (!state_ == Authorisation) return false;
	
	QRegExp rfc822ish("<.*@.*>");
	
	return (greeting_.find(rfc822ish) != -1);
	*/
}

	int
EmpathMailboxPOP3::_write(const QString & str)
{
	/*
	if (state_ == Disconnected) return false;
	_log("To POP    <- \"" + str + "\"");
	return write(sock_fd, str + "\r\n", str.length() + 2);
	*/
}

	QCString
EmpathMailboxPOP3::_getLine()
{
	/*
	QCString s;
	char ch;
	
	while (::read(sock_fd, &ch, 1) > 0) {
		s += ch;
		if (ch == '\n') {
			s[s.length()] = '\0';
			return s;
		}
	}
	return "";
	*/
}

	bool
EmpathMailboxPOP3::_connectToServer()
{
	/*
	empathDebug("_connectToServer() called");
	if (!state_ == Disconnected) return true;
	struct sockaddr_in sin;
	struct hostent * he;
	u_int32_t n;

	sock_fd = socket (AF_INET, SOCK_STREAM, IPPROTO_IP);

	memset ((char *) &sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons (serverPort_);

	if ((n = inet_addr(serverAddress_)) == (Q_UINT32)-1) {
		if ((he = gethostbyname (serverAddress_)) == 0) {
			errorStr = "Sorry I couldn't find the address for " + serverAddress_;
			return false;
    }
		memcpy ((void *)&sin.sin_addr, *(he->h_addr_list), he->h_length);
	} else
		memcpy ((void *)&sin.sin_addr, (void *)&n, sizeof(n));

	empathDebug(QString("Connecting to ") + inet_ntoa (sin.sin_addr));
	
	if (::connect(sock_fd, (struct sockaddr *)&sin, ksize_t(sizeof(struct sockaddr_in)))
			== -1) {
		
		errorStr = "Sorry I couldn't connect to your POP3 server";
		return false;
	}
	
	empathDebug("Connected");
	state_ = Authorisation;
	greeting_ = _getLine();
	return true;
	*/
}

	bool
EmpathMailboxPOP3::_loginAPOP()
{
	/*
	empathDebug("_loginAPOP() called");
	if (!state_ == Authorisation) return false;
	QCString tempStr;
	
	for (	Q_UINT32 authTries = 0 ;
			authTries < authenticationTries_ ;
			authTries++) {
	
		empathDebug("Authentication attempt #" + QString().setNum(authTries));
	
		timeStamp_ = tempStr.right(tempStr.length() - tempStr.findRev("<"));
		timeStamp_ = timeStamp_.left(timeStamp_.find(">") + 1);
	
		MD5_CTX md5context;
	
		unsigned char msgdigest[16];
		QString inp = timeStamp_ + password_;
		unsigned char * md5input_TimeStamp = (unsigned char *)timeStamp_.ascii();
		unsigned char * md5input_Password = (unsigned char *)password_.ascii();
	
		MD5Init(&md5context);
		
		MD5Update(
				&md5context,
				md5input_TimeStamp,
				strlen((char *)md5input_TimeStamp));
		
		MD5Update(
				&md5context,
				md5input_Password,
				strlen((char *)md5input_Password));
		
		MD5Final(msgdigest, &md5context);
		
		char t[2];
	
		QString digest;
		
		for (int i = 0 ; i < 16; i++) {
			sprintf(t, "%0x", msgdigest[i]);
			digest += t;
		}
	
		digest += "\0";
	
		_write (COMMAND_APOP + " " + username_ + " " + digest);
	
		if (_positiveResponse())  {
			empathDebug("APOP authorisation successful");
			state_ = Transaction;
			return true;
		}

		else empathDebug("APOP authorisation UNSUCCESSFUL");
	}

	// Drop connection and reconnect, so we can get back to auth state.
	_changeState(Disconnected);
	_changeState(Authorisation);
	errorStr = "Ran out of authentication attempts. Giving up.";	
	return false;
	*/
}

	bool
EmpathMailboxPOP3::_getIndex()
{
	/*
	empathDebug("_getIndex() called");
	index_.clear();
	
	if (!_getSizeList()) return false;

	QCString tempStr;
	
	if (!_changeState(Transaction)) return false;
	
	empathDebug("Doing UIDL");
	_write (COMMAND_UIDL);

	if (!_positiveResponse()) return false;

	int i = 0;
	while (true) {
		
		tempStr = _getLine();
		empathDebug("tempStr: " + tempStr);

		if (tempStr.isEmpty()) {
			
			errorStr = "Sorry server threw a wobbler while doing 'uidl'";
			return false;
		}
	
		if (tempStr.at(0) == '.') {
			// end of listing
			empathDebug("end of UIDL list");
			break;
		}
	
		int gap = tempStr.find(' ');
		if (gap == -1) return false;
		
		QCString s = tempStr.right(tempStr.length() - ++gap);
		
		index__.append(s);
		
		++i;
	}
	return true;
	*/
}


	bool
EmpathMailboxPOP3::_getSizeList()
{
	/*
	empathDebug("Getting size list");
	QCString tempStr;
	
	if (!_changeState(Transaction)) return false;
	
	_write (COMMAND_LIST);

	if (!_positiveResponse()) return false;

	while (true) {
		
		tempStr = _getLine();
		
		if (tempStr.isEmpty()) {
			
			errorStr = "Sorry server threw a wobbler while doing 'list'";
			return false;
		}
	
		if (tempStr.at(0) == '.') {
			// end of listing
			break;
		}
	
		uID msgno;
		uID * msgsize = new uID;
		int firstSpacePos = tempStr.find(' ');
	
		msgno = tempStr.left(firstSpacePos).toULong();
		*msgsize = tempStr.right(tempStr.length() - firstSpacePos).toULong();
	
		empathDebug("Size: " + QString().setNum(*msgsize));
		
	}
	return true;
	*/
}

	bool
EmpathMailboxPOP3::alreadyHave()
{
	return false;
}

	bool
EmpathMailboxPOP3::_login()
{
	/*
	empathDebug("_login() called");
	if (state_ != Authorisation) return false;

	// Try to login with APOP if it is supported. If not, try the normal method.
//	if (_supportsAPOP()) {
//		return _loginAPOP();
//	} else empathDebug("Server does not support APOP");

	QCString tempStr;
	
	_write (COMMAND_USER + " " + username_);
	
	if (!_positiveResponse()) return false;

	_write(COMMAND_PASS + " " + password_);
 
	if (!_positiveResponse()) return false;
	
	state_ = Transaction;
	return true;
	*/
}

	Q_UINT32
EmpathMailboxPOP3::_countMessages()
{
	/*
	empathDebug("_countMessages() called");
	
	if (!_changeState(Transaction)) return false;
	
	// Find out how many messages are in the mailbox.
	_write(COMMAND_STAT);
	
	QCString tempStr = _getLine();
	if (tempStr.left(3) != "+OK") return 0;
  
	Q_UINT32 msgs, bytes;
	msgs = bytes = 0;

	empathDebug("parsing stat output");
	int firstSpacePos = tempStr.find(' ');
	int secondSpacePos = tempStr.find(' ', ++firstSpacePos);
	
	if (firstSpacePos == -1 || secondSpacePos == -1) return 0;
	
	msgs = tempStr.mid(firstSpacePos, secondSpacePos - firstSpacePos).toULong();
	bytes = tempStr.mid(++secondSpacePos, tempStr.length() - secondSpacePos).toULong();

	empathDebug("done parse stat output");

	numMessages_ = msgs;
	mailboxSize_ = bytes;

	empathDebug("There are " +
			QString().setNum(msgs) + " messages on the server, totalling " +
			QString().setNum(bytes) + " bytes");

	return msgs;
	*/
}

	REnvelope *
EmpathMailboxPOP3::_getEnvelope(const QString & _id)
{
	/*
	empathDebug("_getEnvelope(" + _id + ") called");

	if (!_changeState(Transaction)) return 0;
	
	int msgNum = index__[_id]->index;
	_write(COMMAND_TOP + " " + QCString().setNum(msgNum) + " 0");

	if (!_positiveResponse()) return 0;
	
	// Read envelope
	
	QCString envelopeAsString;
	QCString tempStr;

	while (true) {
		
		tempStr = _getLine();
		
		if (tempStr.isEmpty()) {
			
			errorStr = "Sorry server threw a wobbler while getting message" +
				QCString().setNum(msgNum);
			return 0;
		}
		
		if (tempStr == ".\r\n") {
			// end of envelope
			break;
		}
			
		// CRLF -> LF
			
		tempStr[tempStr.length() - 2] = '\n';
		tempStr[tempStr.length() - 1] = 0;
			
		// Check for byte-stuffing
			
		if (tempStr[0] == '.')
			tempStr = tempStr.right(tempStr.length() - 1);

		envelopeAsString += tempStr;
	}

	empathDebug("Creating a new envelope");
	REnvelope * tempEnvelope = new REnvelope;
	tempEnvelope->set(envelopeAsString);

	return tempEnvelope;
	*/
}

	RMessage *
EmpathMailboxPOP3::_getMessage(const QString & _id)
{
	/*
	empathDebug("getMessage (" + _id + ") called");
	
	QCString tempStr;

	if (!_changeState(Transaction)) return 0;
	
	int msgNum = index_[id]->index;
	_write(COMMAND_RETR + " " + QCString().setNum(msgNum));

	if (!_positiveResponse()) return 0;

	// Read message
	
	QCString messageAsString;

	while (true) {
		
		tempStr = _getLine();
		
		if (tempStr.isEmpty()) {
			
			errorStr = "Sorry server threw a wobbler while getting message" +
				QCString().setNum(msgNum);
			return 0;
		}
		
		if (tempStr == ".\r\n") {
			// end of message
			break;
		}
			
		// CRLF -> LF
			
		tempStr[tempStr.length() - 2] = '\n';
		tempStr[tempStr.length() - 1] = 0;
			
		// Check for byte-stuffing
			
		if (tempStr[0] == '.')
			tempStr = tempStr.right(tempStr.length() - 1);

		messageAsString += tempStr;
	}

	empathDebug("Creating a new message");
	RMessage * tempMessage = new RMessage;
	tempMessage->set(messageAsString);

	return tempMessage;
	*/
}

	bool
EmpathMailboxPOP3::_deleteMessage(const QString & _id)
{
	/*
	empathDebug("_deleteMessage(" + _id.asString() + ") called");
	QCString tempStr;
	
	if (!_changeState(Transaction)) return false;
	
	int msgNum = index_[id]->index;
	// delete the message on the server
	_write(COMMAND_DELE + " " + QCString().setNum(msgNum));
	
	tempStr = _getLine();
	
	if (tempStr.left(3) != "+OK") {
		errorStr =
			"Sorry no response from POP3 server on request to delete message " +
			QString().setNum(msgNum);
			return false;
	}
	empathDebug("Message " + QString().setNum(msgNum) + " deleted from server");
	return true;
	*/
}
	
	bool
EmpathMailboxPOP3::_closeConnection(bool delMessages)
{
	/*
	empathDebug("_closeConnection() called");
	if (state_ == Disconnected) return false;

	if (state_ == Transaction && !delMessages) {
		_write(COMMAND_RSET);
		// Eat response
		_getLine();
	}
	
	_write(COMMAND_QUIT);
	// Eat response
	_getLine();
	close (sock_fd);
	empathDebug("Connection closed");
	return true;
	*/
}

	bool
EmpathMailboxPOP3::checkForNewMail()
{
	/*
	empathDebug("_checkForNewMail() called");

	if (!_changeState(Transaction)) return false;
	
	if (index_.count() == 0) _getIndex();
	
	// Now comes the clever bit.
	// I just tell the Empath object that I want this message filtering.
	// All decision making is done by the filters.
	// I don't decide to download the message, or delete it.
	// Those decisions are deferred to the filters.
	// If a filter wants to look at the size of the message, it'll ask for its
	// size. Then and only then will I bother to look at its size.
	// Note: This could slow down the retrieval of messages, as I could have
	// done a 'stat' query and got the sizes of all the messages at once.
	// Perhaps I'll just do the stat query anyway the first time I'm asked for a
	// message's size. That way when I'm asked again, I can have all the sizes
	// cached. Same goes for UIDL.
	
	EmpathURL inboxPath("empath://" + name_ + "/" + folderList_.at(0)->name());
	
	empathDebug(
		"Index contains references to " +
		QString().setNum(index_.count()) +
		" messages");
		
	QDictIterator<sizeAndIndex> it(index_);
	
	for (; it.current(); ++it) {
		empathDebug("Filtering message " + QString().setNum(it.current()->index));
		QString id(QCString(it.currentKey()));
		empath->filterMessage(EmpathURL(name, inboxPath, id));
	}

	return true;
	*/
}

	bool
EmpathMailboxPOP3::getMail()
{
	empathDebug("_getMail() called");
	return false;
}

	void
EmpathMailboxPOP3::checkNewMail()
{
	/*
	empathDebug("checkNewMail()");
	checkForNewMail();
	_changeState(Disconnected);
	*/
}

	void
EmpathMailboxPOP3::getNewMail()
{
	/*
	empathDebug("getNewMail()");

	if (logging_) {
		_openLog(empathDir() + "empath_pop.log");
	}
	*/
	
}

	void
EmpathMailboxPOP3::saveConfig()
{
	/*
	empathDebug("Saving config");
	canonName_ = GROUP_MAILBOX + QString().setNum(id_);
	KConfig * config_ = kapp->getConfig();
	config_->setGroup(canonName_);
#define CWE config_->writeEntry
	CWE(KEY_MAILBOX_TYPE,					(int)type_);
	CWE(KEY_MAILBOX_NAME,					name_);
	CWE(KEY_POP3_SERVER_ADDRESS,			serverAddress_);
	CWE(KEY_POP3_SERVER_PORT,				serverPort_);
	CWE(KEY_POP3_USERNAME,					username_);
	CWE(KEY_POP3_PASSWORD,					password_);
	CWE(KEY_POP3_APOP,						useAPOP_);
	CWE(KEY_POP3_SAVE_POLICY,				(int)passwordSavePolicy_);
	CWE(KEY_POP3_LOGGING_POLICY,			logging_);
	CWE(KEY_POP3_LOG_FILE_PATH,				logFilePath_);
	CWE(KEY_POP3_LOG_FILE_DISPOSAL_POLICY,	logFileDisposalPolicy_);
	CWE(KEY_POP3_MAX_LOG_FILE_SIZE,			maxLogFileSize_);
	CWE(KEY_POP3_MESSAGE_SIZE_THRESHOLD,	messageSizeThreshold_);
	CWE(KEY_POP3_LARGE_MESSAGE_POLICY,		(int)largeMessagePolicy_);
	CWE(KEY_POP3_CHECK_FOR_NEW_MAIL,		checkMail_);
	CWE(KEY_POP3_MAIL_CHECK_INTERVAL,		checkMailInterval_);
	CWE(KEY_POP3_SAVE_ALL_ADDRESSES,		saveAllAddresses_);
	CWE(KEY_POP3_RETRIEVE_IF_HAVE,			retrieveIfHave_);
#undef CWE
	*/
}

	void
EmpathMailboxPOP3::readConfig()
{
	/*
	empathDebug("Reading config - my canonical name is \"" + canonName_ + "\"");
	
	canonName_ = GROUP_MAILBOX + QString().setNum(id_);
	KConfig * config_ = kapp->getConfig();
	config_->setGroup(canonName_);
	
// For some reason, this just DOES NOT WORK here ! Need to do setGroup !
//	KConfigGroupSaver(config_, canonName_);
#define CRE config_->readEntry
#define CRUNE config_->readUnsignedNumEntry
#define CRBE config_->readBoolEntry
	
	empathDebug("Config group is now \"" + QString(config_->group()) + "\"");

	name_ = CRE(KEY_MAILBOX_NAME, "Unnamed");

	serverAddress_			= CRE(	KEY_POP3_SERVER_ADDRESS,	i18n("<unknown>"));
	serverPort_				= CRUNE(KEY_POP3_SERVER_PORT,		110);
	config_->setDollarExpansion(true);
	username_				= CRE(	KEY_POP3_USERNAME,			"$USER");
	config_->setDollarExpansion(false);
	password_				= CRE(	KEY_POP3_PASSWORD,			"");
	useAPOP_				= CRBE(	KEY_POP3_APOP,				true);
	passwordSavePolicy_		= (SavePolicy)CRUNE(KEY_POP3_SAVE_POLICY, Never);
	logging_				= CRBE(	KEY_POP3_LOGGING_POLICY,	false);
	logFilePath_			= CRE(	KEY_POP3_LOG_FILE_PATH,
		QDir::homeDirPath() + "/.kde/share/apps/empath/log/");
	logFileDisposalPolicy_	= CRBE(	KEY_POP3_LOG_FILE_DISPOSAL_POLICY,	false);
	maxLogFileSize_			= CRUNE(KEY_POP3_MAX_LOG_FILE_SIZE,	10);
	messageSizeThreshold_	= CRUNE(KEY_POP3_MESSAGE_SIZE_THRESHOLD,	1024);
	largeMessagePolicy_		= 
		(LargeMessagePolicy)CRUNE(KEY_POP3_LARGE_MESSAGE_POLICY, RetrieveMessage);
	checkMail_				= CRBE(	KEY_POP3_CHECK_FOR_NEW_MAIL,		true);
	checkMailInterval_		= CRUNE(KEY_POP3_MAIL_CHECK_INTERVAL,		5);
	saveAllAddresses_		= CRBE(	KEY_POP3_LOG_FILE_DISPOSAL_POLICY,	true);
	retrieveIfHave_			= CRBE(	KEY_POP3_RETRIEVE_IF_HAVE,			false);
	
#undef CRE
#undef CRUNE
#undef CRBE
	*/
}

// Set methods
		
	void
EmpathMailboxPOP3::setServerAddress(const QString & serverAddress)
{
	empathDebug("setServerAddress(" + serverAddress + ") called");
	serverAddress_	= serverAddress;
	location_		= serverAddress_;

}

	void
EmpathMailboxPOP3::setServerPort(Q_UINT32 serverPort)
{
	empathDebug("setServerPort(" + QString().setNum(serverPort) + ") called");
	serverPort_ = serverPort;
}

	void
EmpathMailboxPOP3::setUsername(const QString & username)
{
	empathDebug("setUsername(" + username + ") called");
	username_ = username;
}

	void
EmpathMailboxPOP3::setUseAPOP(bool yn)
{
	empathDebug("setUseAPOP" + QString(yn ? "true" : "false") + ") called");
	useAPOP_ = yn;
}

	void
EmpathMailboxPOP3::setPassword(const QString & password)
{
	empathDebug("setPassword(" + password + ") called");
	password_ = password;
}

	void
EmpathMailboxPOP3::setPasswordSavePolicy(SavePolicy policy)
{
	empathDebug("setPasswordSavePolicy(" + QString().setNum((int)policy) + ") called");
	passwordSavePolicy_ = policy;
}

	void
EmpathMailboxPOP3::setLoggingPolicy(bool policy)
{
	empathDebug("setLoggingPolicy(" + QString(policy ? "true" : "false") + ") called");
	loggingPolicy_ = policy;
}

	void
EmpathMailboxPOP3::setLogFilePath(const QString & logPath)
{
	empathDebug("setLogFilePath(" + logPath + ") called");
	logFilePath_ = logPath;
}

	void
EmpathMailboxPOP3::setLogFileDisposalPolicy(bool policy)
{
	empathDebug("setLogFileDisposalPolicy(" + QString(policy ? "true" : "false") + ") called");
	logFileDisposalPolicy_ = policy;
}

	void
EmpathMailboxPOP3::setMaxLogFileSize(Q_UINT32 maxSize)
{
	empathDebug("setMaxLogFileSize(" + QString().setNum(maxSize) + ") called");
	maxLogFileSize_ = maxSize; 
}

	void
EmpathMailboxPOP3::setMessageSizeThreshold(Q_UINT32 threshold)
{
	empathDebug("setMessageSizeThreshold(" + QString().setNum(threshold) + ") called");
	messageSizeThreshold_ = threshold;
}

	void
EmpathMailboxPOP3::setLargeMessagePolicy(LargeMessagePolicy policy)
{
	empathDebug("setLargeMessagePolicy(" + QString().setNum((int)policy) + ") called");
	largeMessagePolicy_ = policy;
}

	void
EmpathMailboxPOP3::setRetrieveIfHave(bool yn)
{
	empathDebug("setDeleteFromServer(" + QString(yn ? "true" : "false") + ") called");
	retrieveIfHave_ = yn;
}

	void
EmpathMailboxPOP3::setSaveAllAddresses(bool yn)
{
	empathDebug("setSaveAllAddresses(" + QString(yn ? "true" : "false") + ") called");
	saveAllAddresses_ = yn;
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

	SavePolicy
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

	Q_UINT32
EmpathMailboxPOP3::messageSizeThreshold()
{
	return messageSizeThreshold_;
}

	LargeMessagePolicy
EmpathMailboxPOP3::largeMessagePolicy()
{
	return largeMessagePolicy_;
}

	bool
EmpathMailboxPOP3::saveAllAddresses()
{
	return saveAllAddresses_;
}

	bool
EmpathMailboxPOP3::retrieveIfHave()
{
	return retrieveIfHave_;
}

	void
EmpathMailboxPOP3::s_serverRead()
{
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

	void
EmpathMailboxPOP3::_openLog(const QString & filename)
{
	/*
	empathDebug("_openLog(" + filename + ") called");
	logFile_.setName(filename);
	
	if (!logFile_.open(IO_WriteOnly)) {
		empathDebug("Couldn't open log file \"" + filename + "\"");
		logFileOpen_ = false;
		return;
	}
	
	logFileOpen_ = true;
	*/
}

	void
EmpathMailboxPOP3::_log(QCString text)
{
	/*
//	empathDebug("_log(" + text + ") called");
	if (!logFileOpen_) {
		return;
	}
	
	text += '\n';
	if (text.left(4) == COMMAND_PASS)
	
	if (logFile_.writeBlock(text, text.length()) == -1) {
		empathDebug("Error writing to log file !");
	}

	logFile_.flush();
*/
}

	bool
EmpathMailboxPOP3::writeMessage(EmpathFolder * parentFolder, const RMessage &)
{
	/*
	empathDebug("writeMessage() called");
	empathDebug("This mailbox is READ ONLY !");
	return false;
	*/
}
	
	bool
EmpathMailboxPOP3::newMail() const
{
	/*
	empathDebug("newMail() called");
	return false;
	*/
}

	void
EmpathMailboxPOP3::readMailForFolder(EmpathFolder * folder)
{
	/*
	empathDebug("readMailForFolder(" + folder->name() + ") called");
	*/
}

	bool
EmpathMailboxPOP3::_changeState(State newState)
{
	/*
	if (state_ == newState) return true;
	
	switch (newState) {
		
		case Disconnected:
			
			return _closeConnection(false);
			break;
			
		case Authorisation:
			
			// Can't get back to auth state when in transaction.
			if (state_ == Transaction) {
				if (!_changeState(Disconnected)) return false;
				return _login();
			}
			else return _login();
			break;
			
		case Transaction:
			
			if (state_ == Disconnected)
				return (_connectToServer() && _login());
			else return _login();
			break;
			
		default:

			return false;
			break;
	}
	*/
}

	bool
EmpathMailboxPOP3::_positiveResponse()
{
	/*
	if (state_ == Disconnected) return false;
	
	QString responseStr = _getLine();
	
	if (responseStr.left(4) == "-ERR")
		empathDebug("Error from POP server: " + responseStr);
	
	return (!responseStr.isEmpty() && responseStr.left(3) == "+OK");
	*/
}

	Q_UINT32
EmpathMailboxPOP3::sizeOfMessage(const EmpathURL & _id)
{
	/*
	empathDebug("getSizeOfMessage(" + _id.asString() + ") called");
	
	QString id = _id.asString();
	if (state_ == Transaction) {
		
		// We were left in the transaction state.
		// This means the list of message sizes is still valid, if we got it.
		
		if (index_.count() == 0) {
			empathDebug("Getting index");
			_getIndex();
		}

		if (index_.count() != 0) {
			empathDebug("We already have the index.");
			empathDebug("Size of message == " +
				QString().setNum(index_[id]->size));
			return index_[id]->size;
		}
	}
	
	if (!_changeState(Transaction)) return 0;
	
	_write(COMMAND_STAT + " " + QString().setNum(index_[id]->index));
	
	QString tempStr = _getLine();
	
	if (tempStr.left(3) != "+OK") return 0;
	
	uID msgno;
	uID msgsize;
	int firstSpacePos = tempStr.find(' ');
	msgno = tempStr.left(firstSpacePos).toULong();
	msgsize = tempStr.right(tempStr.length() - firstSpacePos).toULong();
	
	return 0;
	*/
}

	QString
EmpathMailboxPOP3::plainBodyOfMessage(const EmpathURL & _id)
{
	/*
	empathDebug("getPlainBodyOfMessage(" + _id.asString() + ") called");

	RMessage * tempMessage = _getMessage(_id.messageID());
	
	if (tempMessage == 0)
		return "";
	
	tempMessage->parse();
	return tempMessage->body().firstPlainBodyPart();
	*/
}

	REnvelope *
EmpathMailboxPOP3::envelopeOfMessage(const EmpathURL & _id)
{
	/*
	empathDebug("getEnvelopeOfMessage(" + _id.asString() + ") called");

	return _getEnvelope(_id.messageID());
	*/
}

	RMessage::MessageType
EmpathMailboxPOP3::typeOfMessage(const EmpathURL & _id)
{
	/*
	empathDebug("getTypeOfMessage(" + _id.asString() + ") called");

	RMessage * tempMessage = _getMessage(_id);
	
	if (tempMessage == 0)
		return RMessage::BasicMessage;
	
	tempMessage->parse();
	return tempMessage->type();
	*/
}

	EmpathURL
EmpathMailboxPOP3::path()
{
	/*
	return name_;
	*/
}

	RMessage *
EmpathMailboxPOP3::message(const EmpathURL & id)
{
	/*
	return _getMessage(id);
	*/
}

	void
EmpathMailboxPOP3::init()
{
	/*
	empathDebug("init() called");
	*/
}

	bool
EmpathMailboxPOP3::removeMessage(const EmpathURL & id)
{
	/*
	if (!_changeState(Transaction)) return false;
	
	int msgNum = index_[QString(id.asString())]->index;
	_write(COMMAND_DELE + " " + QString().setNum(msgNum));

	if (!_positiveResponse()) return 0;

	return true;
	*/
}

	bool
EmpathMailboxPOP3::addFolder(const EmpathURL & id)
{
	return false;
}

	bool
EmpathMailboxPOP3::removeFolder(const EmpathURL & id)
{
	return false;
}

