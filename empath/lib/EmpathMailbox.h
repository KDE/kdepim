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
# pragma interface "EmpathMailbox.h"
#endif

#ifndef EMPATHMAILBOX_H
#define EMPATHMAILBOX_H

// Qt includes
#include <qstring.h>
#include <qtimer.h>

// Local includes
#include "EmpathJobInfo.h"
#include "EmpathDefines.h"
#include "EmpathEnum.h"
#include "EmpathURL.h"
#include "EmpathFolderList.h"
#include <RMM_Message.h>
#include <RMM_Envelope.h>
#include <RMM_Message.h>
#include <RMM_MessageID.h>

class EmpathFolder;

/**
 * @short Mailbox base class
 * @author Rikkus
 */
class EmpathMailbox : public QObject
{
    Q_OBJECT
    
    public:
        
        enum Type { Maildir, POP3, IMAP4 };
        
        EmpathMailbox();
    
        /**
         * Create a mailbox with the specified name.
         */
        EmpathMailbox(const QString & name);

        /**
         * Copy ctor.
         */
        EmpathMailbox(const EmpathMailbox &);

        EmpathMailbox & operator = (const EmpathMailbox &);

        virtual ~EmpathMailbox();
        
        bool operator == (const EmpathMailbox & other) const
        { return id_ == other.id_; }

        // Async methods

        void queueJob(EmpathJobInfo &);

        // Pure virtuals.

        /**
         * Initialise.
         */
        virtual void init() = 0;
        /**
         * Trigger a config save for this box.
         */
        virtual void saveConfig() = 0;
        
        /**
         * Trigger a config read for this box.
         */
        virtual void loadConfig() = 0;
 
        /**
         * Synchronise the index for the folder specified in the url.
         */
        virtual void sync(const EmpathURL &) = 0;
       
    protected:

        virtual void _runJob(EmpathJobInfo &) = 0;

    public slots:

        virtual void s_checkMail()   = 0;
        
        // End pure virtual methods
    
    public:

        void        setID(Q_UINT32 id)  { id_ = id; }
        Q_UINT32    id() const          { return id_; }
        /**
         * Check if the folder with the given path exists.
         */
        bool    folderExists(const EmpathURL & folderPath);
        /**
         * Get a pointer to the folder referenced by the given url.
         */
        EmpathFolder *    folder(const EmpathURL & url);
        /**
         * Get the list of folders contained by this mailbox.
         */
        const EmpathFolderList & folderList() const { return folderList_; }
        /**
         * Set whether this mailbox uses a timer.
         */
        void setAutoCheck(bool yn);
        /**
         * Set the timer interval for this box.
         */
        void setAutoCheckInterval(Q_UINT32);
        /**
         * Find out whether this mailbox uses a timer.
         */
        bool autoCheck() const { return autoCheck_; }
        /**
         * Report the timer interval for this box.
         */
        Q_UINT32 autoCheckInterval() const { return autoCheckInterval_; }
        /**
         * Get the name of this box.
         */
        QString name()    const { return url_.mailboxName(); }
        /**
         * Change the name of this box.
         */
        void setName(const QString & name);
        /**
         * Get the full url to this box.
         */
        const EmpathURL & url() const { return url_; }
        /**
         * Get the count of messages contained within all folders
         * owned by this box.
         */
        Q_UINT32 messageCount() const;
        /**
         * Get the count of unread messages contained within all folders
         * owned by this box.
         */
        Q_UINT32 unreadMessageCount() const;
        /**
         * Report the type of this mailbox.
         */
        Type type() const { return type_; }
        /**
         * Report the type of this mailbox as a string.
         */
        QString typeAsString() const { return typeString_; }
        /**
         * Name of the desired pixmap to represent this box.
         */
        QString     pixmapName()    const { return pixmapName_; }
        /**
         * Find out if there's any new mail ready.
         */
        bool        newMailReady()  const { return (newMessagesCount_ != 0);}
        /**
         * Count the number of new mails ready.
         */
        Q_UINT32    newMails()      const { return newMessagesCount_; }

    signals:

        void rename(EmpathMailbox *, const QString &);
        void updateFolderLists();
        void syncFolderLists();
        void newMailArrived();
        void mailboxChangedByExternal();
        void countUpdated(unsigned int, unsigned int);
        
    public slots:

        void s_countUpdated(unsigned int, unsigned int);
        
    protected:
        
        EmpathFolderList    folderList_;

        EmpathIndex * index_;

        EmpathURL   url_;

        Type type_;
        QString typeString_;

        Q_UINT32    newMessagesCount_;

        bool        autoCheck_;
        Q_UINT32    autoCheckInterval_;
        
        QTimer      timer_;
        QString     pixmapName_;
        Q_UINT32    id_;
        Q_UINT32    seq_;

    private:

        void _connectUp();
        void _enqueue(EmpathJobInfo &);

        EmpathJobQueue queue_;
        
        void _runQueue();
};

#endif

// vim:ts=4:sw=4:tw=78
