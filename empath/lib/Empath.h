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

#ifndef EMPATH_H
#define EMPATH_H

// Qt includes
#include <qstring.h>

// Local includes
#include "EmpathDefines.h"
#include "EmpathURL.h"
#include "EmpathMailboxList.h"
#include "EmpathFilterList.h"
#include "EmpathConfig.h"
#include "EmpathMessageDataCache.h"

#include "RMM_Enum.h"
#include "RMM_MessageID.h"
#include "RMM_Message.h"
#include "RMM_Envelope.h"

#define empath Empath::getEmpath()

class EmpathFolder;
class EmpathMailSender;
class EmpathMessageDataCache;
class EmpathIndexRecord;
class RMessage;

/**
 * Empath is the main class for the app
 *
 * @short App's main class
 * @author Rikkus
 */
class Empath : public QObject
{
	Q_OBJECT

	public:
		
		/**
		 * ctor
		 */
		Empath();
		
		/**
		 * dtor
		 */
		~Empath();
		/**
		 * In the style of KApplication and QApplication, this
		 * saves me having to pass a pointer to the (single) object of
		 * this controller class to every object in the system
		 *
		 * i.e. if I want to read the mailbox list, I can do something
		 * like Empath::getEmpath()->mailboxList()
		 * actually It'll be just empath->mailboxList() for brevity
		 *
		 * @short quick ref to controller class
		 * @return pointer to the controller class' single object
		 */
		static Empath * getEmpath();
		
		/**
		 * This are used by EmpathMailboxMaildir and should go into that class.
		 */
		Q_UINT32 startTime() const;
		/**
		 * This are used by EmpathMailboxMaildir and should go into that class.
		 */
		pid_t processID() const;
		/**
		 * This are used by EmpathMailboxMaildir and should go into that class.
		 */
		QString hostName() const;
		
		/**
		 * Apply filters to message given as messageDesc.
		 * The standard filters will look at sourceMailbox to see if it's
		 * not the internal mailbox (~/Maildir). If so, they'll assume it's
		 * a new mail and whack it into mailbox://internal/inbox.
		 * Any messages without a home will go into mailbox://internal/orphaned,
		 * for now.
		 * @param source The mailbox that's sending the message.
		 * @param messageDesc The message that needs filtering.
		 */
		void filter(const EmpathURL &);
		
		/**
		 * Pointer to the system-wide mailbox list
		 *
		 * @short shortcut to mailbox list
		 * @return pointer to mailbox list
		 */
		EmpathMailboxList & mailboxList();
		
		/**
		 * Gets a message description given id - deprecated ?
		 */
		const EmpathIndexRecord *
			messageDescription(const RMessageID & id) const;

		/**
		 * Search for a message. Necessary ?
		 */
		const RMessage * findMessage(int id);
		
		/**
		 * Call this to create a new composer
		 */
		void newComposer() const;

		/**
		 * Pointer to the system-wide sender - be it Sendmail, SMTP or whatever
		 */
		EmpathMailSender & mailSender() const;

		/**
		 * The filters
		 */
		EmpathFilterList & filterList();
		
		void statusMessage(const QString & message) const;
		void updateOutgoingServer();
		
		/**
		 * Reply to the given message. Brings up the appropriate window and sets
		 * the bits to what they should be.
		 */
		void reply(RMessage * message);
		/**
		 * Reply to the given message. Brings up the appropriate window and sets
		 * the bits to what they should be.
		 */
		void replyAll(RMessage * message);
		/**
		 * Forward given message. Brings up the appropriate window and sets
		 * the bits to what they should be.
		 */
		void forward(RMessage * m);
		/**
		 * Compose a new message.
		 */
		void compose();
		
		/**
		 * Get the size in octets of the message
		 */
		Q_UINT32 sizeOfMessage(const EmpathURL &);

		/**
		 * Get the first plain body part of the message, if there is one. Should
		 * really return preamble if there isn't.
		 */
		QString plainBodyOfMessage(const EmpathURL &);

		/**
		 * Gets the envelope of the message.
		 */
		REnvelope * envelopeOfMessage(const EmpathURL &);
		
		/**
		 * Gets the type of the message, be it plain or mime.
		 */
		RMessage::MessageType typeOfMessage(const EmpathURL &);
		
		/**
		 * Gets the message specified. Allocated with new, so delete it.
		 */
		RMessage * message(const EmpathURL &);
		
		/**
		 * Removes the message specified from the containing mailbox.
		 */
		bool removeMessage(const EmpathURL &);

		/**
		 * Gets a pointer to the folder specified in the url, or 0.
		 */
		EmpathFolder * folder(const EmpathURL &);
		
		/**
		 * Gets a pointer to the mailbox specified in the url, or 0.
		 */
		EmpathMailbox * mailbox(const EmpathURL &);
		
		static Empath * EMPATH;
		
	public slots:

		void s_newMailArrived();
	
		/**
		 * Used when folders have changed and any displayed lists need updating.
		 */
		void s_updateFolderLists();
		
		void s_saveConfig();
	
	signals:
	
		/**
		 * Connected to on-screen folder lists to enable them to be updated when
		 * necessary.
		 */
		void updateFolderLists();
		void newMailArrived();
		void newComposer(ComposeType, RMessage *);
		
		
	private:
	
		// General

		void _initFilters();
		void _saveHostName();
		void _setStartTime();
		
		// These objects will be contructed before the code in our contructor
		// is run. That means they must NOT access anything 'global', including
		// KApplication stuff when they are constructed. They will be told when to
		// 'init' themselves and will do what would normally be in their
		// respective constructors at that point instead.
		
		EmpathMailboxList		mailboxList_;
		EmpathFilterList		filterList_;
		
		EmpathMailSender		* mailSender_;
		EmpathMessageDataCache	messageDataCache_;
		QString					hostName_;
		pid_t					processID_;
		Q_UINT32				startupSeconds_;
};

#endif

