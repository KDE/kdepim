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
	
		/**
		 * @short Initialise this mailbox. Must be called after ctor and before
		 * any other methods.
		 */
		virtual void
			init() = 0;
		
		/**
		 * @short Trigger a config save for this box.
		 */
		virtual void saveConfig() = 0;
		
		/**
		 * @short Trigger a config read for this box.
		 */
		virtual void readConfig() = 0;
		
		/**
		 * @short Write a new message to the specified folder.
		 */
		virtual bool
			writeMessage(const EmpathURL & folder, RMessage & msg) = 0;
		
		/**
		 * @short Get the size of the message specified in the url.
		 */
		virtual Q_UINT32
			sizeOfMessage		(const EmpathURL &) = 0;
		
		/**
		 * @short Get the first plain body of the message specified in the url.
		 */
		virtual QString
			plainBodyOfMessage	(const EmpathURL &) = 0;
		
		/**
		 * @short Get the envelope of the message specified in the url.
		 */
		virtual REnvelope *
			envelopeOfMessage	(const EmpathURL &) = 0;
		
		/**
		 * @short Get the message specified in the url.
		 */
		virtual RMessage *
			message				(const EmpathURL &) = 0;
		
		/**
		 * @short Attempt to remove the message specified in the url.
		 */
		virtual bool
			removeMessage		(const EmpathURL &) = 0;
		
		/**
		 * @short Get the type of the message specified in the url.
		 */
		virtual RMessage::MessageType
			typeOfMessage		(const EmpathURL &) = 0;
		
		/**
		 * @short Attempt to create a new folder as specified in the url.
		 */
		virtual bool
			addFolder			(const EmpathURL &) = 0;
		
		/**
		 * @short Attempt to remove the folder specified in the url.
		 */
		virtual bool
			removeFolder		(const EmpathURL &) = 0;
		
		/**
		 * @short Synchronise the index for the folder specified in the url.
		 */
		virtual void
			syncIndex			(const EmpathURL &) = 0;
	
	public slots:

		virtual void s_checkNewMail()	= 0;
		virtual void s_getNewMail()		= 0;
		
		// End pure virtual methods
	
		// Now follows non-virtual methods.
		
	public:

		void		setID(Q_UINT32 id) { id_ = id; }
		Q_UINT32	id() const { return id_; }
		
		/**
		 * @short Check if the folder with the given path exists.
		 */
		bool	folderExists(const EmpathURL & folderPath);
		
		/**
		 * @short Get a pointer to the folder referenced by the given url.
		 */
		EmpathFolder *	folder(const EmpathURL & url);
		
		/**
		 * @short Get the list of folders contained by this mailbox.
		 */
		const EmpathFolderList & folderList() const { return folderList_; }

		/**
		 * @short Set whether this mailbox uses a timer.
		 */
		void		setCheckMail(bool yn);
		
		/**
		 * @short Set the timer interval for this box.
		 */
		void		setCheckMailInterval(Q_UINT32 checkMailInterval);

		/**
		 * @short Find out whether this mailbox uses a timer.
		 */
		bool		checkMail() const { return checkMail_; }
		
		/**
		 * @short Report the timer interval for this box.
		 */
		Q_UINT32	checkMailInterval() const { return checkMailInterval_; }

		/**
		 * @short Get the name of this box.
		 */
		const QString &		name()			const { return url_.mailboxName(); }
		
		/**
		 * @short Change the name of this box.
		 */
		void				setName(const QString & name);
		
		/**
		 * @short Get the full url to this box.
		 */
		const EmpathURL &	url()			const { return url_; }
		
		/**
		 * @short Get the count of messages contained within all folders
		 * owned by this box.
		 */
		Q_UINT32			messageCount()			const;
		
		/**
		 * @short Get the count of unread messages contained within all folders
		 * owned by this box.
		 */
		Q_UINT32			unreadMessageCount()	const;
		
		/**
		 * @short Report the type of this mailbox.
		 */
		AccountType	type() const { return type_; }
		
		/**
		 * @short Name of the desired pixmap to represent this box.
		 */
		const QString &	pixmapName() const { return pixmapName_; }
		
		bool 		newMailReady()	const { return (newMessagesCount_ != 0); }
		Q_UINT32 	newMails()		const { return newMessagesCount_; }

	signals:

		void updateFolderLists();
		void newMailArrived();
		void mailboxChangedByExternal();
		void countUpdated(int, int);
		
	public slots:

		void s_countUpdated(int, int);
		
	protected:
		
		EmpathFolderList	folderList_;
		EmpathIndex			index_;

		EmpathURL			url_;
		AccountType			type_;

		Q_UINT32			newMessagesCount_;

		bool				checkMail_;
		Q_UINT32			checkMailInterval_;
		
		QTimer				timer_;
		QString				pixmapName_;
		Q_UINT32 			id_;
		Q_UINT32			seq_;
};

#endif

