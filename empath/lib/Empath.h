/*
    Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
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
# pragma interface "Empath.h"
#endif

#ifndef EMPATH_H
#define EMPATH_H

#include <sys/types.h>

// Qt includes
#include <qstring.h>
#include <qobject.h>
#include <qcache.h>

// Local includes
#include "EmpathDefines.h"
#include "EmpathURL.h"
#include "EmpathMailboxList.h"
#include "EmpathFilterList.h"
#include "EmpathCachedMessage.h"
#include "EmpathComposeForm.h"
#include "EmpathViewFactory.h"

#include "RMM_Enum.h"
#include "RMM_Message.h"

#define empath Empath::getEmpath()

class EmpathMailSender;
class EmpathFolder;
class EmpathTask;
class EmpathComposer;
class EmpathJobScheduler;

/**
 * Empath is the controller class for Empath's kernel.
 *
 * You can't construct this. Use the static method Empath::create() instead.
*
 * @short Controller class
 * @author Rikkus
 */
class Empath : public QObject
{
    Q_OBJECT

    public:

        enum SetupType {
            SetupDisplay,
            SetupIdentity,
            SetupComposing,
            SetupSending,
            SetupAccounts,
            SetupFilters,
            SetupWizard
        };
    
        /** 
         * Creates an Empath object.
         */
        static void start();
        
        /**
         * Use this to kill off Empath. You should delete the UI first. This
         * allows you to bring down the UI quickly, before Empath dies.
         * Once this method returns, everything should have been cleaned
         * up, synced, and destructed. You may delete your KApplication.
         */
        void shutdown();
        
        /**
         * This must be called after the constructor. You can construct
         * a ui first, but don't try to access any mailboxes or filters
         * as they're not initialised until you call this. @see start
         */
        void init();

        /**
         * In the style of KApplication and QApplication, this
         * saves you having to pass a pointer to the (single) object of
         * this controller class to every object in the system.
         *
         * There is a macro 'empath' defined that makes this even easier.
         *
         * @short Pointer to controller class.
         * @return Pointer to controller class.
         */
        static Empath * getEmpath() { return EMPATH; }

        /**
         * @return The folder being used as the inbox.
         */
        EmpathURL inbox()   const;

        /**
         * @return The folder being used for queued messages. 
         */
        EmpathURL outbox()  const;
        
        /**
         * @return The folder being used for sent messages. 
         */
        EmpathURL sent()    const;
        
        /**
         * @return The folder being used for draft messages. 
         */
        EmpathURL drafts()  const;
        
        /**
         * @return The folder being used for 'deleted' messages. 
         */
        EmpathURL trash()   const;
        
        /**
         * @internal
         * This is used by generateUnique.
         */
        Q_UINT32    startTime()    const { return startupSeconds_; }
        /**
         * @internal
         * This is used by generateUnique.
         */
        pid_t        processID()    const { return processID_; }
        /**
         * @internal
         * This is used by generateUnique.
         */
        QString        hostName()    const { return hostName_; }
        
        /**
         * Pass the message referred to in the URL through the filtering
         * mechanism.
         */
        void filter(const EmpathURL &);
        
        /**
         * The system-wide mailbox list.
         *
         * @short A shortcut to the mailbox list
         * @return A reference to the mailbox list
         */
        EmpathMailboxList & mailboxList() { return mailboxList_; }

        /**
         * Pointer to the job scheduler
         */
        EmpathJobScheduler * jobScheduler() { return jobScheduler_; }
        
        /**
         * @internal
         * Reference to the system-wide sender. Don't worry about the type,
         * just use it. Actually, you should be using send(), queue() and
         * sendQueued() instead, so this is being marked internal.
         */
        EmpathMailSender * mailSender() const { return mailSender_; }

        /**
         * The filter list.
         */
        EmpathFilterList & filterList() { return filterList_; }
        
        /**
         * Call this when you change the type of server for outgoing
         * messages. The old server will be deleted and the new one
         * will be used from then on.
         */
        void updateOutgoingServer();
    
        /**
         * Get a previously requested message.
         * @return A pointer to an RMM::RMessage, unless the message can't
         * be found, when it returns 0.
         */
        RMM::RMessage   message(const EmpathURL &);

        /**
         * Gets a pointer to the folder specified in the url, or 0.
         */
        EmpathFolder    * folder(const EmpathURL &);
        
        /**
         * Gets a pointer to the mailbox specified in the url, or 0.
         */
        EmpathMailbox   * mailbox(const EmpathURL &);
        
        /**
         * Queue a new message for sending later.
         */
        void queue(RMM::RMessage &);
        
        /**
         * Send a message. If the user set queueing as the default,
         * it'll be queued, surprisingly.
         */
        void send(RMM::RMessage &);
        
        /**
         * Attempt to send all queued messages.
         */
        void sendQueued();
        
        /**
         * @internal
         */
        static Empath * EMPATH;
        
        /**
         * Generate an unique filename
         */
        QString generateUnique();
        void cacheMessage(const EmpathURL &, RMM::RMessage);
        
//        void jobFinished(EmpathJob);

        EmpathViewFactory & viewFactory();

    protected:

        Empath();
        ~Empath();
       
    public slots:

        /**
         * Check mail in all boxes.
         */
        void s_checkMail();
        
        /**
         * Connect to this to provide a message for the user.
         * It'll probably turn up in all statusbars.
         */
        void s_infoMessage(const QString &);
   
        /**
         * Use when folders have changed and any displayed lists need updating.
         */
        void s_updateFolderLists();
        void s_syncFolderLists();

        /**
         * Please ask the user to enter settings for the mailbox
         * specified in the URL.
         */
        void s_configureMailbox(const EmpathURL &, QWidget *);
       
        ///////////////////////////////////////////////////////////////////
        // Message composition.
        
        /**
         * Compose a new message.
         */
        void s_compose(const QString & recipient = QString::null);
        /**
         * Reply to the given message.
         */
        void s_reply(const EmpathURL & url);
        /**
         * Reply to the given message.
         */
        void s_replyAll(const EmpathURL & url);
        /**
         * Forward given message.
         */
        void s_forward(const EmpathURL & url);
        /**
         * Bounce a message.
         */
        void s_bounce(const EmpathURL &);
        /**
         * Creates a new composer using the bug report template.
         */
        void s_bugReport();

        ///////////////////////////////////////////////////////////////////
        // Async methods.
        
        EmpathJobID createFolder(const EmpathURL &);
        EmpathJobID removeFolder(const EmpathURL &);

        void saveMessage(const EmpathURL &, QWidget *);
        
        /**
         * Ask for a message to be copied from one folder to another.
         */
        EmpathJobID copy(const EmpathURL & src, const EmpathURL & dest);
         
        /**
         * Ask for a message to be moved from one folder to another.
         */
        EmpathJobID move(const EmpathURL & src, const EmpathURL & dest);
        
        /**
         * Ask for a message to be retrieved.
         */
        EmpathJobID retrieve(const EmpathURL & messageURL);
        
        /**
         * Write a new message to the specified folder.
         */
        EmpathJobID write(RMM::RMessage & msg, const EmpathURL & folder);
        
        /**
         * Remove given message.
         */
        EmpathJobID remove(const EmpathURL & messageURL);
        
        /**
         * Remove messages. The mailbox and folder are given in the URL.
         * The QStringList is used to pass the message ids.
         */
        EmpathJobID remove(const EmpathURL & folder, const QStringList & ids);
       
        /**
         * Mark a message with a particular status (Read, Marked, ...)
         */
        EmpathJobID mark(const EmpathURL & messageURL, RMM::MessageStatus);
        
        /**
         * Mark many messages with a particular status.
         * The mailbox and folder to use are given in the URL. The QStringList
         * is used to pass the message ids.
         */
        EmpathJobID mark(
            const EmpathURL & folder,
            const QStringList & ids,
            RMM::MessageStatus
        );

        //////////////////////////////////////////////////////////////////
        // Request user interaction to alter configuration.

        /**
         * Request that the UI bring up the settings for the given type.
         */
        void s_setup(SetupType, QWidget *);

        /**
         * Connect to this from anywhere to provide the about box.
         */
        void s_about(QWidget *);

        //////////////////////////////////////////////////////////////////
        // Internal.
          
        /**
         * @internal
         */
        void s_newTask(EmpathTask *);

        /**
         * @internal
         */
        void s_newMailArrived();
        
        /**
         * @internal
         */
        void s_saveConfig();

        /**
         * @internal
         */
//        void s_saveNameReady(const EmpathURL & url, QString path);

        /**
         * @short We want to show a folder's contents
         * Call this when you want a folder's contents to be displayed.
         * Used only by EmpathFolderWidget, which maintains unique
         * numbers for each instance of itself so you can decide whether
         * to ignore the signal or not.
         */
        void s_showFolder(const EmpathURL & url, unsigned int idx);
       
        /**
         * @internal
         */
//        void s_messageReadyForSave(bool, const EmpathURL &, QString);
        
    signals:

        /**
         * EmpathMessageListWidget connects to this to be notified when
         * it should show the contents of a folder. It uses the id to
         * decide whether to ignore the signal.
         */
        void showFolder(const EmpathURL &, unsigned int id);

        /**
         * EmpathMailbox connects to this to be notified of
         * checkMail requests.
         */
        void checkMail();

        /**
         * Please ask the user to configure this mailbox.
         */
        void configureMailbox(const EmpathURL &, QWidget *);
        
        /**
         * Please ask the user to enter a path to save this message
         * under.
         */
        void getSaveName(const EmpathURL &, QWidget *);

        /**
         * Signals that the on-screen folder lists should be updated.
         * Usually connected to a slot in the UI module.
         * Call this when a folder has been added/removed.
         * Once you are done adding/removing folders, call syncFolderLists().
         */
        void updateFolderLists();
        /**
         * Signals that the on-screen folder lists should be synced.
         * Usually connected to a slot in the UI module.
         * Call updateFolderLists() when a folder has been added/removed.
         * Once you are done adding/removing folders, call this.
         */
        void syncFolderLists();
        /**
         * Signals that new mail has arrived somewhere.
         */
        void newMailArrived();
        /**
         * Signals that we want to compose a message.
         * All the info about the message is kept in the composeform.
         * Usually connected to a slot in the UI module.
         */
        void newComposer(EmpathComposeForm);
       /**
         * Signals that the settings should be provided for
         * review. In other words, bring up the settings dialog.
         * Usually connected to a slot in the UI module.
         */
        void setup(Empath::SetupType, QWidget *);
        /**
         * Signals that we want to see who's responsible for this stuff.
         * Usually connected to a slot in the UI module.
         */
        void about(QWidget *);
        
        /**
         * Signals that a new task has started.
         * Usually connected to a slot in the UI module.
         */
        void newTask(EmpathTask *);
        /**
         * Signals that we want to tell the user something.
         * Usually connected to a slot in the UI module.
         */
        void infoMessage(const QString &);
        
    private:

        EmpathURL inbox_, outbox_, sent_, drafts_, trash_;
    
        void _saveHostName();
        void _setStartTime();
        
        EmpathMailboxList       mailboxList_;
        EmpathFilterList        filterList_;
        
        EmpathMailSender        * mailSender_;
        EmpathComposer          * composer_;
        EmpathJobScheduler      * jobScheduler_;

        QString                 hostName_;
        pid_t                   processID_;
        Q_UINT32                startupSeconds_;
        
        static bool started_;
        
        unsigned long int seq_;
        
        QString startupSecondsStr_;
        QString pidStr_;
        
        QDict<EmpathCachedMessage> cache_;

        EmpathViewFactory viewFactory_;
};

#endif

// vim:ts=4:sw=4:tw=78
