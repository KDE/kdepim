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

#ifndef EMPATHMAILBOXIMAP4_H
#define EMPATHMAILBOXIMAP4_H

// Qt includes
#include <qstring.h>

// Local includes
#include "EmpathMailbox.h"

class EmpathMailboxIMAP4 : public EmpathMailbox
{
	Q_OBJECT

	public:

		EmpathMailboxIMAP4(const QString & name);
		
		EmpathMailboxIMAP4(
				const QString & name,
				const QString & serverAddress,
				Q_UINT32 serverPort,
				const QString & username,
				const QString & password);

		EmpathMailboxIMAP4(const EmpathMailboxIMAP4 & acc);

		EmpathMailboxIMAP4 & operator = (const EmpathMailboxIMAP4 & acc);


		~EmpathMailboxIMAP4 () { empathDebug ("dtor"); } // blank
	
		void readMailForFolder(EmpathFolder * folder);
		
#include "EmpathMailboxAbstract.h"
		
		// Set methods
		
		void setServerAddress(const QString & serverAddress);
		void setServerPort(Q_UINT32 serverPort);
		void setUsername(const QString & username);
		void setPassword(const QString & password);
		void setPasswordSavePolicy(SavePolicy policy);
		void setLoggingPolicy(bool policy);
		void setLogFilePath(const QString & logPath);
		void setLogFileDisposalPolicy(bool policy);
		void setMaxLogFileSize(Q_UINT32 maxSize);
		void setMessageSizeThreshold(Q_UINT32 threshold);
		void setLargeMessagePolicy(LargeMessagePolicy policy);
		void setCheckForNewMail(bool yn);
		void setMailCheckInterval(Q_UINT32 interval);
		void setDeleteFromServer(bool yn);
		void setAutoGetNewMail(bool yn);
		void setSaveAllAddresses(bool yn);
		void setNotify(bool yn);
		void setRetrieveIfHave(bool yn);


		// Get methods
		
		QString serverAddress();
		Q_UINT32 serverPort();
		QString username();
		QString password();
		SavePolicy passwordSavePolicy();
		bool loggingPolicy();
		QString logFilePath();
		bool logFileDisposalPolicy();
		Q_UINT32 maxLogFileSize();
		Q_UINT32 messageSizeThreshold();
		LargeMessagePolicy largeMessagePolicy();
		bool checkForNewMail();
		Q_UINT32 mailCheckInterval();
		bool deleteFromServer();
		bool autoGetNewMail();
		bool saveAllAddresses();
		bool notify();
		bool retrieveIfHave();
	
	private:

		QString				serverAddress_;
		Q_UINT32		serverPort_;
		QString				username_;
		QString				password_;
		SavePolicy			passwordSavePolicy_;
		bool				loggingPolicy_;
		QString				logFilePath_;
		bool				logFileDisposalPolicy_;
		unsigned			int maxLogFileSize_;
		unsigned			int messageSizeThreshold_;
		LargeMessagePolicy	largeMessagePolicy_;
		bool				checkMail_;
		unsigned			int checkMailInterval_;
		bool				deleteFromServer_;
		bool				autoGetNewMail_;
		bool				saveAllAddresses_;
		bool				notify_;
		bool				retrieveIfHave_;
};

#endif
