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


#ifndef EMPATH_H
#define EMPATH_H

// Qt includes
#include <qstring.h>
#include <qobject.h>
#include <qcache.h>

// Local includes
#include "EmpathComposeForm.h"
#include "EmpathIndexRecord.h"
#include "EmpathURL.h"

#include "rmm/Message.h"

#define empath Empath::getEmpath()

class EmpathSender;
class EmpathTask;
class EmpathJobScheduler;
class EmpathCachedMessage;
class EmpathComposer;
class EmpathFilterList;
class EmpathMailSender;
class EmpathMailboxList;
class EmpathMailbox;
class EmpathFolder;

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
        EmpathURL inbox() const { return inbox_; } 

        /**
         * @return The folder being used for queued messages. 
         */
        EmpathURL outbox() const { return outbox_; } 

        /**
         * @return The folder being used for sent messages. 
         */
        EmpathURL sent() const { return sent_; } 

        /**
         * @return The folder being used for draft messages. 
         */
        EmpathURL drafts() const { return drafts_; } 

        /**
         * @return The folder being used for 'deleted' messages. 
         */
        EmpathURL trash() const { return trash_; } 

        /**
         * @internal
         * This is used by generateUnique.
         */
        Q_UINT32    startTime() const { return startupSeconds_; }
        /**
         * @internal
         * This is used by generateUnique.
         */
        unsigned int processID() const { return processID_; }
        /**
         * @internal
         * This is used by generateUnique.
         */
        QString     hostName() const { return hostName_; }

        /**
         * Pass the message referred to in the URL through the filtering
         * mechanism.
         */
        void filter(const EmpathURL &);

        /**
         * Call this when you change the type of server for outgoing
         * messages. The old server will be deleted and the new one
         * will be used from then on.
         */
        void updateOutgoingServer();

        /**
         * Get a previously requested message.
         * @return A pointer to an RMM::Message, unless the message can't
         * be found, when it returns 0.
         */
        RMM::Message message(const EmpathURL &);

        /**
         * Gets a pointer to the folder specified in the url, or 0.
         */
        EmpathFolder * folder(const EmpathURL &);

        /**
         * Gets a pointer to the mailbox specified in the url, or 0.
         */
        EmpathMailbox * mailbox(const EmpathURL &);

        /**
         * Queue a new message for sending later.
         */
        void queue(RMM::Message &);

        /**
         * Send a message. If the user set queueing as the default,
         * it'll be queued, surprisingly.
         */
        void send(RMM::Message &);

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

        void cacheMessage(const EmpathURL &, RMM::Message);

        EmpathMailboxList * mailboxList();
        EmpathFilterList  * filterList();

    protected:

        Empath();
        ~Empath();

    public:

        EmpathJobID readIndex(
            const EmpathURL &,
            QObject * = 0,
            int = 0
        );

        EmpathJobID createFolder(
            const EmpathURL &,
            QObject * = 0,
            int = 0
        );

        EmpathJobID removeFolder(
            const EmpathURL &,
            QObject * = 0,
            int = 0
        );

        /**
         * Ask for a message to be copied from one folder to another.
         */
        EmpathJobID copy(
            const EmpathURL & src,
            const EmpathURL & dest,
            QObject * = 0,
            int = 0
        );

        /**
         * Ask for a message to be moved from one folder to another.
         */
        EmpathJobID move(
            const EmpathURL & src,
            const EmpathURL & dest,
            QObject * = 0,
            int = 0
        );

        /**
         * Ask for a message to be retrieved.
         */
        EmpathJobID retrieve(
            const EmpathURL & messageURL,
            QObject * = 0,
            int = 0
        );

        /**
         * Write a new message to the specified folder.
         */
        EmpathJobID write(
            RMM::Message & msg,
            const EmpathURL & folder,
            QObject * = 0,
            int = 0
        );

        /**
         * Remove given message.
         */
        EmpathJobID remove(
            const EmpathURL & messageURL,
            QObject * = 0,
            int = 0
        );

        /**
         * Remove messages. The mailbox and folder are given in the URL.
         * The QStringList is used to pass the message ids.
         */
        EmpathJobID remove(
            const EmpathURL & folder,
            const QStringList & ids,
            QObject * = 0,
            int = 0
        );

        /**
         * Mark a message with a particular status (Read, Marked, ...)
         */
        EmpathJobID mark(
            const EmpathURL & messageURL,
            EmpathIndexRecord::Status,
            QObject * = 0,
            int = 0
        );

        /**
         * Mark many messages with a particular status.
         * The mailbox and folder to use are given in the URL. The QStringList
         * is used to pass the message ids.
         */
        EmpathJobID mark(
            const EmpathURL & folder,
            const QStringList & ids,
            EmpathIndexRecord::Status,
            QObject * = 0,
            int = 0
        );

    public slots:

        /**
         * Check mail in all boxes.
         */
        void s_checkMail();

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
        void s_compose();
        void s_composeTo(const QString & recipient);
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

        ///////////////////////////////////////////////////////////////////
        // Async methods.

        void saveMessage(const EmpathURL &, QWidget *);


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
        void newComposer(const EmpathComposeForm &);
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

    private:

        EmpathJobScheduler  * _jobScheduler();
        EmpathMailSender    * _sender();

        void _saveHostName();
        void _setStartTime();

        EmpathURL inbox_, outbox_, sent_, drafts_, trash_;

        QString         hostName_;
        unsigned int    processID_;
        Q_UINT32        startupSeconds_;
        QString         startupSecondsStr_;
        QString         pidStr_;

        QCache<EmpathCachedMessage> cache_;

        // Order dependency
        EmpathMailboxList       * mailboxList_;
        EmpathFilterList        * filterList_;

        EmpathMailSender        * sender_;
        EmpathJobScheduler      * jobScheduler_;

        unsigned long int seq_;
        // End order dependency
};

#endif

// vim:ts=4:sw=4:tw=78
