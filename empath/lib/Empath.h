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
class EmpathIndexRecord;
class RMessage;
class EmpathTask;

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
		static Empath * getEmpath() { return EMPATH; }
		
		/**
		 * These are used by EmpathMailboxMaildir and should go into that class.
		 */
		Q_UINT32	startTime()	const { return startupSeconds_; }
		pid_t		processID()	const { return processID_; }
		QString		hostName()	const { return hostName_; }
		
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
		EmpathMailboxList & mailboxList() { return mailboxList_; }
		
		/**
		 * Pointer to the system-wide sender - be it Sendmail, SMTP or whatever
		 */
		EmpathMailSender & mailSender() const
		{ ASSERT(mailSender_); return *mailSender_; }

		/**
		 * The filters
		 */
		EmpathFilterList & filterList() { return filterList_; }
		
		void statusMessage(const QString & message) const;
		void updateOutgoingServer();
	
		/**
		 * Gets the message specified. The message is placed in the cache, so
		 * you can forget about it as it will be deleted later.
		 * If the message can't be retrieved, returns 0.
		 */
		RMessage * message(const EmpathURL &);
		
		/**
		 * Gets a pointer to the folder specified in the url, or 0.
		 */
		EmpathFolder * folder(const EmpathURL &);
		
		/**
		 * Gets a pointer to the mailbox specified in the url, or 0.
		 */
		EmpathMailbox * mailbox(const EmpathURL &);
		
		EmpathTask * addTask(const QString & name);
		
		void addPendingMessage(RMessage &);
		void send(RMessage &);
		void sendQueued();
		
		static Empath * EMPATH;
		
		void init();
		
	public slots:
		
		void s_newTask(EmpathTask *);

		void s_newMailArrived();
	
		/**
		 * Used when folders have changed and any displayed lists need updating.
		 */
		void s_updateFolderLists() { emit(updateFolderLists()); }
		
		void s_saveConfig();
			
		/**
		 * Compose a new message.
		 */
		void s_compose()
		{ emit(newComposer(ComposeNormal, EmpathURL())); }
		
		/**
		 * @short Reply to the given message.
		 */
		void s_reply(const EmpathURL & url)
		{ emit(newComposer(ComposeReply, url)); }
		
		/**
		 * @short Reply to the given message.
		 */
		void s_replyAll(const EmpathURL & url)
		{ emit(newComposer(ComposeReplyAll, url)); }
		
		/**
		 * @short Forward given message.
		 */
		void s_forward(const EmpathURL & url)
		{ emit(newComposer(ComposeForward, url)); }
		
		/**
		 * @short Remove given message.
		 */
		bool remove(const EmpathURL &);
		
		/**
		 * Bounce a message.
		 */
		void s_bounce(const EmpathURL &) {}
		
		/**
		 * Mark a message with a given status.
		 */
		bool mark(const EmpathURL &, RMM::MessageStatus);
		
		void s_setupDisplay();
		void s_setupIdentity();
		void s_setupSending();
		void s_setupComposing();
		void s_setupAccounts();
		void s_setupFilters();
	
	signals:
	
		/**
		 * Connected to on-screen folder lists to enable them to be updated when
		 * necessary.
		 */
		void updateFolderLists();
		void newMailArrived();
		void newComposer(ComposeType, const EmpathURL &);
		
		void setupDisplay();
		void setupIdentity();
		void setupSending();
		void setupComposing();
		void setupAccounts();
		void setupFilters();
		
		void newTask(EmpathTask *);
		
	private:
	
		// General

		void _saveHostName();
		void _setStartTime();
		
		// These objects will be contructed before the code in our contructor
		// is run. That means they must NOT access anything 'global', including
		// KApplication stuff when they are constructed. They will be told when
		// to init themselves and will do what would normally be in their
		// respective constructors at that point instead.
		
		EmpathMailboxList		mailboxList_;
		EmpathFilterList		filterList_;
		
		EmpathMailSender		* mailSender_;
		EmpathMessageDataCache	cache_;
		QString					hostName_;
		pid_t					processID_;
		Q_UINT32				startupSeconds_;
};

#endif

