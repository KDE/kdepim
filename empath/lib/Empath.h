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
 * Empath is the main class for the app
 *
 * @short App's main class
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
        Q_UINT32    startTime()    const { return startupSeconds_; }
        pid_t        processID()    const { return processID_; }
        QString        hostName()    const { return hostName_; }
        
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
        
        /**
         * Call this when you change the type of server for outgoing
         * messages.
         */
        void updateOutgoingServer();
    
        /**
         * Gets the message specified. The message is placed in the cache, so
         * you can forget about it as it will be deleted later.
         * If the message can't be retrieved, returns 0.
         */
        RMM::RMessage     * message(const EmpathURL &);
        
        /**
         * Gets a pointer to the folder specified in the url, or 0.
         */
        EmpathFolder    * folder(const EmpathURL &);
        
        /**
         * Gets a pointer to the mailbox specified in the url, or 0.
         */
        EmpathMailbox    * mailbox(const EmpathURL &);
        
        /**
         * Create a new task and pass back the pointer.
         * Don't delete the task, just call done();
         */
        EmpathTask        * addTask(const QString & name);
        
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
        
        static Empath * EMPATH;
        
        /**
         * This must be called after the constructor. You can initialise
         * the ui first, but be careful ;)
         */
        void init();
        
    public slots:
        
        void s_infoMessage(const QString &);
        
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
        void s_compose();
        
        /**
         * @short Reply to the given message.
         */
        void s_reply(const EmpathURL & url);
        
        /**
         * @short Reply to the given message.
         */
        void s_replyAll(const EmpathURL & url);
        
        /**
         * @short Forward given message.
         */
        void s_forward(const EmpathURL & url);
        
        /**
         * @short Remove given message.
         */
        bool remove(const EmpathURL &);
        bool remove(const EmpathURL &, const QStringList &);
        
        /**
         * Bounce a message.
         */
        void s_bounce(const EmpathURL &);

        bool mark(const EmpathURL &, RMM::MessageStatus);
        bool mark(const EmpathURL &, const QStringList &, RMM::MessageStatus);
        
        void s_setupDisplay();
        void s_setupIdentity();
        void s_setupSending();
        void s_setupComposing();
        void s_setupAccounts();
        void s_setupFilters();
        
        void s_about();
        
        void s_bugReport();
    
    signals:
    
        /**
         * Connected to on-screen folder lists to enable them to be updated when
         * necessary.
         */
        void updateFolderLists();
        void newMailArrived();
        void newComposer(Empath::ComposeType, const EmpathURL &);
        void newComposer(const QString &);
        
        void setupDisplay();
        void setupIdentity();
        void setupSending();
        void setupComposing();
        void setupAccounts();
        void setupFilters();
        
        void bugReport();

        void about();
        
        void newTask(EmpathTask *);
        void infoMessage(const QString &);
        
    private:
    
        // General

        void _saveHostName();
        void _setStartTime();
        
        EmpathMailboxList        mailboxList_;
        EmpathFilterList        filterList_;
        
        EmpathMailSender        * mailSender_;
        EmpathMessageDataCache    cache_;
        QString                    hostName_;
        pid_t                    processID_;
        Q_UINT32                startupSeconds_;
};

inline void Empath::send(RMM::RMessage & m)        { mailSender_->send(m);        }
inline void Empath::queue(RMM::RMessage & m)    { mailSender_->queue(m);    }
inline void Empath::sendQueued()                { mailSender_->sendQueued();}
inline void Empath::s_setupDisplay()            { emit(setupDisplay());        }
inline void Empath::s_setupIdentity()            { emit(setupIdentity());    }
inline void Empath::s_setupSending()            { emit(setupSending());        }
inline void Empath::s_setupComposing()            { emit(setupComposing());    }
inline void Empath::s_setupAccounts()            { emit(setupAccounts());    }
inline void Empath::s_setupFilters()            { emit(setupFilters());        }
inline void Empath::s_newMailArrived()             { emit(newMailArrived());    }
inline void Empath::s_newTask(EmpathTask * t)    { emit(newTask(t));            }
inline void Empath::s_about()                    { emit(about());            }
inline void Empath::s_bugReport()                { emit(bugReport());        }
inline void Empath::filter(const EmpathURL & m)    { filterList_.filter(m);    }

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
