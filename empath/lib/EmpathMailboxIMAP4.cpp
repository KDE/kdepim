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
# pragma implementation "EmpathMailboxIMAP4.h"
#endif

// Local includes
#include "EmpathMailboxIMAP4.h"

EmpathMailboxIMAP4::EmpathMailboxIMAP4(const QString & name)
	:	EmpathMailbox	(name),
		serverAddress_	(QString::null),
		serverPort_		(110),
		username_		(QString::null),
		password_		(QString::null)
{
	empathDebug("ctor");
	type_	= IMAP4;
	setName(name);
}


	bool	
EmpathMailboxIMAP4::getMail()
{
	return false;
}

	void
EmpathMailboxIMAP4::s_getNewMail()
{
}
	void
EmpathMailboxIMAP4::s_checkNewMail()
{
}

	void
EmpathMailboxIMAP4::saveConfig()
{
}

	void
EmpathMailboxIMAP4::readConfig()
{
}

// Set methods
		
	void
EmpathMailboxIMAP4::setServerAddress(const QString & serverAddress)
{
	serverAddress_	= serverAddress;
}

	void
EmpathMailboxIMAP4::setServerPort(Q_UINT32 serverPort)
{
	serverPort_ = serverPort;
}

	void
EmpathMailboxIMAP4::setUsername(const QString & username)
{
	username_ = username;
}

	void
EmpathMailboxIMAP4::setPassword(const QString & password)
{
	password_ = password;
}

	void
EmpathMailboxIMAP4::setPasswordSavePolicy(SavePolicy policy)
{
	passwordSavePolicy_ = policy;
}

	void
EmpathMailboxIMAP4::setLoggingPolicy(bool policy)
{
	loggingPolicy_ = policy;
}

	void
EmpathMailboxIMAP4::setLogFilePath(const QString & logPath)
{
	logFilePath_ = logPath;
}

	void
EmpathMailboxIMAP4::setLogFileDisposalPolicy(bool policy)
{
	logFileDisposalPolicy_ = policy;
}

	void
EmpathMailboxIMAP4::setMaxLogFileSize(Q_UINT32 maxSize)
{
	maxLogFileSize_ = maxSize;
}

// Get methods
		
	QString
EmpathMailboxIMAP4::serverAddress()
{
	return serverAddress_;
}

	Q_UINT32
EmpathMailboxIMAP4::serverPort()
{
	return serverPort_;
}

	QString
EmpathMailboxIMAP4::username()
{
	return username_;
}

	QString
EmpathMailboxIMAP4::password()
{
	return password_;
}

	EmpathMailbox::SavePolicy
EmpathMailboxIMAP4::passwordSavePolicy()
{
	return passwordSavePolicy_;
}

	bool
EmpathMailboxIMAP4::loggingPolicy()
{
	return loggingPolicy_;
}

	QString
EmpathMailboxIMAP4::logFilePath()
{
	return logFilePath_;
}

	bool
EmpathMailboxIMAP4::logFileDisposalPolicy()
{
	return logFileDisposalPolicy_;
}

	Q_UINT32
EmpathMailboxIMAP4::maxLogFileSize()
{
	return maxLogFileSize_;
}

	QString
EmpathMailboxIMAP4::writeMessage(const EmpathURL &, RMessage &)
{
	return QString::null;
}

	bool
EmpathMailboxIMAP4::newMail() const
{
	return false;
}
	void
EmpathMailboxIMAP4::syncIndex(const EmpathURL &)
{
}

	RMessage *
EmpathMailboxIMAP4::message(const EmpathURL &)
{
	return 0;
}

	Q_UINT32
EmpathMailboxIMAP4::sizeOfMessage(const EmpathURL &)
{
	return 0;
}

	QString
EmpathMailboxIMAP4::plainBodyOfMessage(const EmpathURL &)
{
	return QString::null;
}

	REnvelope *
EmpathMailboxIMAP4::envelopeOfMessage(const EmpathURL &)
{
	return 0;
}

	RBodyPart::PartType
EmpathMailboxIMAP4::typeOfMessage(const EmpathURL &)
{
	return RBodyPart::Basic;
}

	void
EmpathMailboxIMAP4::init()
{
	empathDebug("init() called");
}

	bool
EmpathMailboxIMAP4::removeMessage(const EmpathURL &)
{
	return false;
}

	bool
EmpathMailboxIMAP4::addFolder(const EmpathURL &)
{
	return false;
}

	bool
EmpathMailboxIMAP4::removeFolder(const EmpathURL &)
{
	return false;
}

	bool
EmpathMailboxIMAP4::mark(const EmpathURL &, RMM::MessageStatus)
{
	return false;
}

