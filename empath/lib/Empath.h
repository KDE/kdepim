/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
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
#include "EmpathMailSender.h"

#include "RMM_Enum.h"
#include "RMM_Message.h"

#define empath Empath::getEmpath()

class EmpathFolder;
class EmpathIndexRecord;
class EmpathTask;

typedef QCache<RMM::RMessage> EmpathMessageDataCache;

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
    
        enum ComposeType {
            ComposeReply,
            ComposeReplyAll,
            ComposeForward,
            ComposeNormal,
            ComposeBounce
        };
       
        /** 
         * This MUST be called, and it must be after Empath::start(),
         * and before the empath object is used. Mailboxes, filters, etc
         * are initialised by this method.
         *
         * This is separate from Empath::start to allow the UI to start up
         * faster. On construction, the standard KDE UI loads its
         * configuration, connects some signals to slots, and gets some
         * windows onto the display. The empath object's initialisation may
         * take some time, so this is done once the UI is already up but
         * before the event loop begins. kapp->processEvents() is used
         * to allow events to be processed even though the event loop has
         * not yet been entered.
         *
         * Example: (see full implementation in main.cpp)
         *
         * new KApplication(argc, argv, "empath");
         * Empath::start();
         * new EmpathUI;
         * empath->init();
         * kapp->exec();
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
         * Pointer to the system-wide mailbox list.
         *
         * @short A shortcut to the mailbox list
         * @return A reference to the mailbox list
         */
        EmpathMailboxList & mailboxList() { return mailboxList_; }
        
        /**
         * @internal
         * Reference to the system-wide sender. Don't worry about the type,
         * just use it. Actually, you should be using send(), queue() and
         * sendQueued() instead, so this is being marked internal.
         */
        EmpathMailSender & mailSender() const
        { return *mailSender_; }

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
        RMM::RMessage   * message(const EmpathURL &);
        
        /**
         * Gets a pointer to the folder specified in the url, or 0.
         */
        EmpathFolder    * folder(const EmpathURL &);
        
        /**
         * Gets a pointer to the mailbox specified in the url, or 0.
         */
        EmpathMailbox   * mailbox(const EmpathURL &);
        
        /**
         * Create a new task with the given name and pass back the pointer.
         * Don't delete the task, just call done();
         * 
         * Use this whenever you're going to be doing some work for a while.
         * You simply call addTask and then call the methods of EmpathTask
         * such as setMax(), doneOne(), done(), etc. Empath will decide
         * how to let the user know what's happening. A progress meter will
         * show up on the display if your task is taking a long time.
         */
        EmpathTask      * addTask(const QString & name);
        
        /**
         * Compose a message.
         */
        void compose(const QString & recipient = QString::null);
        
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

        void cacheMessage(const EmpathURL &, RMM::RMessage *);
        
    protected:

        Empath();
        ~Empath();
       
    public slots:
        
        /**
         * Connect to this to provide a message for the user.
         * It'll probably turn up in all statusbars.
         */
        void s_infoMessage(const QString &);
   
        /**
         * Use when folders have changed and any displayed lists need updating.
         */
        void s_updateFolderLists() { emit(updateFolderLists()); }
       
        ///////////////////////////////////////////////////////////////////
        // Message composition.
        
        /**
         * Compose a new message.
         */
        void s_compose();
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
        
        void createFolder(const EmpathURL &, QString extraInfo = QString::null);

        void saveMessage(const EmpathURL &);
        
        /**
         * Ask for a message to be copied from one folder to another.
         */
        void copy(
            const EmpathURL &,
            const EmpathURL &,
            QString extraInfo = QString::null);
         
        /**
         * Ask for a message to be moved from one folder to another.
         */
        void move(
            const EmpathURL &,
            const EmpathURL &,
            QString extraInfo = QString::null);
        
        /**
         * Ask for a message to be retrieved.
         */
        void retrieve(
            const EmpathURL &,
            QString extraInfo = QString::null);
        
        /**
         * Write a new message to the specified folder.
         */
        EmpathURL write(
            const EmpathURL & folder,
            RMM::RMessage & msg,
            QString extraInfo = QString::null);
        
        /**
         * Remove given message (or folder if no message id present in URL).
         */
        void remove(
            const EmpathURL &,
            QString extraInfo = QString::null);
        
        /**
         * Remove messages. The mailbox and folder are given in the URL.
         * The QStringList is used to pass the message ids.
         */
        void remove(
             const EmpathURL &,
             const QStringList &,
             QString extraInfo = QString::null);
       
        /**
         * Mark a message with a particular status (Read, Marked, ...)
         */
        void mark(
            const EmpathURL &,
            RMM::MessageStatus,
            QString extraInfo = QString::null);
        
        /**
         * Mark many messages with a particular status.
         * The mailbox and folder to use are given in the URL. The QStringList
         * is used to pass the message ids.
         */
        void mark(
            const EmpathURL &,
            const QStringList &,
            RMM::MessageStatus,
            QString extraInfo = QString::null);

        //////////////////////////////////////////////////////////////////
        // Request user interaction to alter configuration.

        /**
         * Request that the UI bring up the settings for the display.
         */
        void s_setupDisplay();
        /**
         * Request that the UI bring up the settings for the user's identity.
         */
        void s_setupIdentity();
        /**
         * Request that the UI bring up the settings for sending messages.
         */
        void s_setupSending();
        /**
         * Request that the UI bring up the settings for composing messages.
         */
        void s_setupComposing();
        /**
         * Request that the UI bring up the settings for the mailboxes.
         */
        void s_setupAccounts();
        /**
         * Request that the UI bring up the settings for the filters.
         */
        void s_setupFilters();
        /**
         * Connect to this from anywhere to provide the about box.
         */
        void s_about();

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

    protected slots:        

        /**
         * @internal
         */
        void s_retrieveComplete(
            bool status,
            const EmpathURL & from,
            const EmpathURL & to,
            QString ixinfo,
            QString xinfo);

        /**
         * @internal
         */
        void s_retrieveComplete(
            bool status,
            const EmpathURL & url,
            QString xinfo);


        /**
         * @internal
         */
        void s_moveComplete(
            bool status,
            const EmpathURL & from,
            const EmpathURL & to,
            QString ixinfo,
            QString xinfo);
        
        /**
         * @internal
         */
        void s_copyComplete(
            bool status,
            const EmpathURL & from,
            const EmpathURL & to,
            QString ixinfo,
            QString xinfo);

        /**
         * @internal
         */
        void s_removeComplete(
            bool status,
            const EmpathURL & url,
            QString ixinfo,
            QString xinfo);

        /**
         * @internal
         */
        void s_markComplete(
            bool status,
            const EmpathURL & url,
            QString ixinfo,
            QString xinfo);

        /**
         * @internal
         */
        void s_writeComplete(
            bool status,
            const EmpathURL & url,
            QString ixinfo,
            QString xinfo);

        /**
         * @internal
         */
        void s_createFolderComplete(
            bool status,
            const EmpathURL & url,
            QString ixinfo,
            QString xinfo);

        /**
         * @internal
         */
        void s_removeFolderComplete(
            bool status,
            const EmpathURL & url,
            QString ixinfo,
            QString xinfo);
 
    signals:
        
        /**
         * Please ask the user to enter their information !
         */
        void setupWizard();
 
        /**
         * A request for a message to be retrieved from a mailbox
         * has been completed. The string xinfo can be used to check if it
         * was you who made this request.
         */
        void retrieveComplete(
            bool status,
            const EmpathURL & from,
            const EmpathURL & to,
            QString xinfo);

        /**
         * A request for a message to be retrieved from a mailbox
         * has been completed. The string xinfo can be used to check if it
         * was you who made this request.
         */
        void retrieveComplete(
            bool status,
            const EmpathURL & url,
            QString xinfo);

        /**
         * A request for a message to be moved from one folder to another
         * has been completed. The string xinfo can be used to check if it
         * was you who made this request.
         */
        void moveComplete(
            bool status,
            const EmpathURL & from,
            const EmpathURL & to,
            QString xinfo);
        
        /**
         * A request for a message to be copied from one folder to another
         * has been completed. The string xinfo can be used to check if it
         * was you who made this request.
         */
        void copyComplete(
            bool status,
            const EmpathURL & from,
            const EmpathURL & to,
            QString xinfo);

        /**
         * A request for a message to be removed from a mailbox
         * has been completed. The string xinfo can be used to check if it
         * was you who made this request.
         */
        void removeComplete(
            bool status,
            const EmpathURL & url,
            QString xinfo);

        /**
         * A request for a message to be marked with the given status
         * has been completed. The string xinfo can be used to check if it
         * was you who made this request.
         */
        void markComplete(
            bool status,
            const EmpathURL & url,
            QString xinfo);

        /**
         * A request for a message to be written to a mailbox
         * has been completed. The string xinfo can be used to check if it
         * was you who made this request.
         */
        void writeComplete(
            bool status,
            const EmpathURL & url,
            QString xinfo);

        /**
         * A request for a new folder to be created
         * has been completed. The string xinfo can be used to check if it
         * was you who made this request.
         */
        void createFolderComplete(
            bool status,
            const EmpathURL & url,
            QString xinfo);

        /**
         * A request for a folder to be removed
         * has been completed. The string xinfo can be used to check if it
         * was you who made this request.
         */
        void removeFolderComplete(
            bool status,
            const EmpathURL & url,
            QString xinfo);
        
        /**
         * Signals that the on-screen folder lists should be updated.
         * Usually connected to a slot in the UI module.
         */
        void updateFolderLists();
        /**
         * Signals that new mail has arrived somewhere.
         */
        void newMailArrived();
        /**
         * Signals that we want to compose a message.
         * The URL is provided to refer to a message which may be quoted
         * when replying.
         * Usually connected to a slot in the UI module.
         */
        void newComposer(Empath::ComposeType, const EmpathURL &);
        /**
         * Signals that we want to compose a message to the given
         * recipient.
         * Usually connected to a slot in the UI module.
         */
        void newComposer(const QString &);
        /**
         * Signals that the display settings should be provided for
         * review. In other words, bring up the display settings dialog.
         * Usually connected to a slot in the UI module.
         */
        void setupDisplay();
        /**
         * Signals that the display settings should be provided for
         * review. In other words, bring up the display settings dialog.
         * Usually connected to a slot in the UI module.
         */
        void setupIdentity();
        /**
         * Signals that the sending settings should be provided for
         * review. In other words, bring up the sending settings dialog.
         * Usually connected to a slot in the UI module.
         */
        void setupSending();
        /**
         * Signals that the composing settings should be provided for
         * review. In other words, bring up the composing settings dialog.
         * Usually connected to a slot in the UI module.
         */
        void setupComposing();
        /**
         * Signals that the accounts settings should be provided for
         * review. In other words, bring up the accounts settings dialog.
         * Usually connected to a slot in the UI module.
         */
        void setupAccounts();
        /**
         * Signals that the filter settings should be provided for
         * review. In other words, bring up the filter settings dialog.
         * Usually connected to a slot in the UI module.
         */
        void setupFilters();
        
        /**
         * Signals that we want to file a bug report.
         * Usually connected to a slot in the UI module.
         */
        void bugReport();

        /**
         * Signals that we want to see who's responsible for this stuff.
         * Usually connected to a slot in the UI module.
         */
        void about();
        
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
    
        // General
        
        void _saveHostName();
        void _setStartTime();
        
        EmpathMailboxList       mailboxList_;
        EmpathFilterList        filterList_;
        
        EmpathMailSender        * mailSender_;
        EmpathMessageDataCache  cache_;
        QString                 hostName_;
        pid_t                   processID_;
        Q_UINT32                startupSeconds_;
        
        static bool started_;
        
        unsigned long int seq_;
        
        QString startupSecondsStr_;
        QString pidStr_;
};

inline void Empath::send(RMM::RMessage & m)     { mailSender_->send(m);     }
inline void Empath::queue(RMM::RMessage & m)    { mailSender_->queue(m);    }
inline void Empath::sendQueued()                { mailSender_->sendQueued();}
inline void Empath::s_setupDisplay()            { emit(setupDisplay());     }
inline void Empath::s_setupIdentity()           { emit(setupIdentity());    }
inline void Empath::s_setupSending()            { emit(setupSending());     }
inline void Empath::s_setupComposing()          { emit(setupComposing());   }
inline void Empath::s_setupAccounts()           { emit(setupAccounts());    }
inline void Empath::s_setupFilters()            { emit(setupFilters());     }
inline void Empath::s_newMailArrived()          { emit(newMailArrived());   }
inline void Empath::s_newTask(EmpathTask * t)   { emit(newTask(t));         }
inline void Empath::s_about()                   { emit(about());            }
inline void Empath::s_bugReport()               { emit(bugReport());        }
inline void Empath::filter(const EmpathURL & m) { filterList_.filter(m);    }

inline void 
Empath::compose(const QString & recipient)
{ emit(newComposer(recipient)); }

inline void 
Empath::s_compose()
{ emit(newComposer(ComposeNormal, EmpathURL())); }

inline void
Empath::s_reply(const EmpathURL & url)
{ emit(newComposer(ComposeReply, url)); }

inline void
Empath::s_replyAll(const EmpathURL & url)
{ emit(newComposer(ComposeReplyAll, url)); }

inline void
Empath::s_forward(const EmpathURL & url)
{ emit(newComposer(ComposeForward, url)); }

inline void
Empath::s_bounce(const EmpathURL & url)
{ emit(newComposer(ComposeBounce, url)); }

inline void
Empath::s_infoMessage(const QString & s)
{ emit(infoMessage(s)); }

#endif

// vim:ts=4:sw=4:tw=78
