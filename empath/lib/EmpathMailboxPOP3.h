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

#ifndef EMPATHMAILBOXPOP3_H
#define EMPATHMAILBOXPOP3_H

// Qt includes
#include <qstring.h>
#include <qcstring.h>
#include <qfile.h>
#include <qdict.h>
#include <qstrlist.h>

// Local includes
#include "EmpathMailbox.h"
#include "md5.h"

class EmpathConfig;
class Empath;
class KIOJob;

class EmpathMailboxPOP3 : public EmpathMailbox
{
	Q_OBJECT

	public:

		EmpathMailboxPOP3(const QString & name);
		
		~EmpathMailboxPOP3();

		bool checkForNewMail();
		bool logging();
		void setLogging(bool policy);
		bool alreadyHave();
		EmpathURL path();

#include "EmpathMailboxAbstract.h"

	protected slots:

		void s_data(int, const char *, int);
		void s_jobFinished(int);
	
	public:

		// Set methods
		
		void setServerAddress			(const QString &);
		void setServerPort				(Q_UINT32);
		void setUsername				(const QString &);
		void setPassword				(const QString &);
		void setUseAPOP					(bool);
		void setPasswordSavePolicy		(SavePolicy);
		void setLoggingPolicy			(bool);
		void setLogFilePath				(const QString &);
		void setLogFileDisposalPolicy	(bool);
		void setMaxLogFileSize			(Q_UINT32);
		void setMessageSizeThreshold	(Q_UINT32);
		void setLargeMessagePolicy		(LargeMessagePolicy);
		void setSaveAllAddresses		(bool);
		void setRetrieveIfHave			(bool);

		// Get methods
		
		QString				serverAddress();
		Q_UINT32			serverPort();
		QString				username();
		QString				password();
		bool				useAPOP();
		SavePolicy			passwordSavePolicy();
		bool				loggingPolicy();
		QString				logFilePath();
		bool				logFileDisposalPolicy();
		Q_UINT32			maxLogFileSize();
		Q_UINT32			messageSizeThreshold();
		LargeMessagePolicy	largeMessagePolicy();
		bool				autoGetNewMail();
		bool				saveAllAddresses();
		bool				retrieveIfHave();

	private:

		// The pop3 server is a state machine, sort of.
		// It can either be in the Transaction or Authorisation state.
		// We, on the other hand, have another state - Not connected to the server.
	
		// Order dependency
		QString				serverAddress_;
		Q_UINT32			serverPort_;
		QString				username_;
		QString				password_;
		bool				logging_;
		Q_UINT32			numMessages_;
		Q_UINT32			mailboxSize_;
		bool				logFileOpen_;
		Q_UINT32			authenticationTries_;
		// End order dependency
		
		bool				useAPOP_;
		SavePolicy			passwordSavePolicy_;
		bool				loggingPolicy_;
		QString				logFilePath_;
		bool				logFileDisposalPolicy_;
		Q_UINT32			maxLogFileSize_;
		Q_UINT32			messageSizeThreshold_;
		LargeMessagePolicy	largeMessagePolicy_;
		bool				saveAllAddresses_;
		bool				retrieveIfHave_;
		int					sock_fd; // socket fd
		QCString			errorStr;
		bool				connected_;
		bool				loggedIn_;
		QString				timeStamp_;
		QFile				logFile_;
		QCString			greeting_;
		
		KIOJob * job;
		
		enum State {
			NoWait,
			WaitForList,
			WaitForUIDL,
			WaitForData };
};


#endif
