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

#ifndef EMPATHMAILBOX_H
#define EMPATHMAILBOX_H

// Qt includes
#include <qstring.h>
#include <qtimer.h>
#include <qpixmap.h>

// Local includes
#include "RMM_Message.h"
#include "RMM_Envelope.h"
#include "RMM_Message.h"
#include "RMM_MessageID.h"
#include "EmpathDefines.h"
#include "EmpathURL.h"
#include "EmpathFolderList.h"

class EmpathFolder;

class EmpathMailbox : public QObject
{
	Q_OBJECT
	
	public:
		
		EmpathMailbox();
	
		EmpathMailbox(const QString & name);

		EmpathMailbox(const EmpathMailbox &);

		EmpathMailbox & operator = (const EmpathMailbox &);

		virtual ~EmpathMailbox();
		
		bool operator == (const EmpathMailbox & other) const
		{ return id_ == other.id_; }
		
		// Pure virtual methods
		virtual bool getMail() = 0;
		virtual bool newMail() const = 0;

		virtual void saveConfig() = 0;
		virtual void readConfig() = 0;
		
		virtual bool writeMessage(EmpathFolder * parentFolder, const RMessage &)=0;
		
		virtual Q_UINT32				sizeOfMessage		(const EmpathURL &) = 0;
		virtual QString					plainBodyOfMessage	(const EmpathURL &) = 0;
		virtual REnvelope *				envelopeOfMessage	(const EmpathURL &) = 0;
		virtual RMessage *				message				(const EmpathURL &) = 0;
		virtual bool 					removeMessage		(const EmpathURL &) = 0;
		virtual RMessage::MessageType	typeOfMessage		(const EmpathURL &) = 0;
		virtual bool					addFolder			(const EmpathURL &) = 0;
		virtual bool					removeFolder		(const EmpathURL &) = 0;
		
		virtual void init() = 0;
		
	public slots:

		virtual void checkNewMail() = 0;
		virtual void getNewMail() = 0;
		
		// End pure virtual methods
	
	public:

		virtual void readMailForFolder(EmpathFolder * folder) = 0;
		
		void setID(Q_UINT32 id);
		Q_UINT32 id() const;

		bool newMailReady() const;
		Q_UINT32 newMails() const;


		RMessage & firstMailReady();
		
		void setCheckMail(bool yn);
		void setCheckMailInterval(Q_UINT32 checkMailInterval);

		bool checkMail() const;
		Q_UINT32 checkMailInterval() const;

		void setName(const QString & name);
		QString name() const { return name_; }

		QString path() const;
		
		QString location() const;
		
		AccountType type() const;

		bool folderExists(const QString & folderPath);
		const EmpathFolder * folder(const QString & folderPath);
		const EmpathFolderList & folderList() const;

		bool createFolder(const QString & path);

		bool usesTimer() const;
		Q_UINT32 timerInterval() const;

		Q_UINT32 messageCount() const;
		Q_UINT32 unreadMessageCount() const;

		const QPixmap & pixmap() const;

		void update(EmpathFolder * f);
		
		void emitMailRead(int i) { emit mailRead(i); }

	signals:

		void updateFolderLists();
		void newMailArrived();
		void mailboxChangedByExternal();
		void countUpdated(int, int);
		void mailRead(int);
		
	public slots:

		void s_countUpdated(EmpathFolder *, int, int);
		
	protected:
		
		EmpathFolderList	folderList_;
		EmpathIndex			index_;

		QString			name_;
		AccountType		type_;
		QString			location_;

		QString			canonName_;
		Q_UINT32		newMessagesCount_;

		bool			checkMail_;
		Q_UINT32		checkMailInterval_;
		QTimer			* timer_;
		QPixmap			pixmap_;
		Q_UINT32 		id_;
		
		Q_UINT32		seq_;
};

#endif

