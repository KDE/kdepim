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

#ifndef EMPATHMAILBOX_H
#define EMPATHMAILBOX_H

// Qt includes
#include <qstring.h>
#include <qtimer.h>

// Local includes
#include "EmpathDefines.h"
#include "EmpathEnum.h"
#include "EmpathURL.h"
#include "EmpathFolderList.h"
#include "EmpathIndexRecord.h"
#include <rmm/Message.h>
#include <rmm/Envelope.h>
#include <rmm/Message.h>
#include <rmm/MessageID.h>

class EmpathFolder;
class EmpathIndex;

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

        virtual EmpathIndex * index(const EmpathURL &) = 0;

        virtual RMM::Message retrieveMessage(const EmpathURL & url) = 0;

        virtual QString writeMessage(RMM::Message &, const EmpathURL & f) = 0;

        virtual bool removeMessage(const EmpathURL & url) = 0;

        virtual EmpathSuccessMap removeMessage(
            const EmpathURL & folder,
            const QStringList & messageIDList) = 0;

        virtual bool markMessage(
            const EmpathURL & url,
            EmpathIndexRecord::Status
        ) = 0;

        virtual EmpathSuccessMap markMessage(
            const EmpathURL & folder,
            const QStringList & messageIDList,
            EmpathIndexRecord::Status) = 0;

        virtual bool createFolder(const EmpathURL & url) = 0;
        virtual bool removeFolder(const EmpathURL & url) = 0;

        /**
         * Get the count of messages contained within all folders
         * owned by this box.
         */
        virtual unsigned int messageCount() const = 0;

        /**
         * Get the count of unread messages contained within all folders
         * owned by this box.
         */
        virtual unsigned int unreadMessageCount() const = 0;

    public slots:

        virtual void s_checkMail()   = 0;

        // End pure virtual methods

    public:

        void        setID(unsigned int id)  { id_ = id; }
        unsigned int    id() const          { return id_; }
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
        void setAutoCheckInterval(unsigned int);
        /**
         * Find out whether this mailbox uses a timer.
         */
        bool autoCheck() const { return autoCheck_; }
        /**
         * Report the timer interval for this box.
         */
        unsigned int autoCheckInterval() const { return autoCheckInterval_; }
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
        unsigned int    newMails()      const { return newMessagesCount_; }

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

        EmpathURL url_;

        Type type_;
        QString typeString_;

        unsigned int newMessagesCount_;

        bool autoCheck_;
        unsigned int autoCheckInterval_;

        QTimer      timer_;
        QString     pixmapName_;

        unsigned int id_;
        unsigned int seq_;

    private:

        void _connectUp();
        void _runQueue();
};

#endif

// vim:ts=4:sw=4:tw=78
