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

#ifndef EMPATHFOLDER_H
#define EMPATHFOLDER_H

// Qt includes
#include <qobject.h>
#include <qstring.h>

// Local includes
#include "EmpathIndex.h"
#include "EmpathURL.h"
#include <RMM_Message.h>
#include <RMM_MessageID.h>

class EmpathMailbox;
class EmpathIndexRecord;
class EmpathIndex;
class EmpathIndexAllocator;

/**
 * @short Mail folder
 * 
 * An EmpathMailbox creates these on startup. It generally looks for existing
 * folders and creates a matching EmpathFolder for each. Of course, it may
 * know exactly what folders it has - e.g. an EmpathMailboxPOP3 only has one
 * folder, Inbox.
 * 
 * When indexing a real mail folder, an EmpathMailbox will update the related
 * EmpathFolder's index.
 * 
 * @author Rikkus
 */
class EmpathFolder : public QObject
{
    Q_OBJECT

    public:

        EmpathFolder();

        /**
         * Create and set the URL this maps to.
         */
        EmpathFolder(const EmpathURL & url);
        
        virtual ~EmpathFolder();

        /**
         * Compares URLs.
         */
        bool operator == (const EmpathFolder & other) const;

        /**
         * Pointer to parent folder, or 0 if this is a toplevel folder.
         */
        EmpathFolder * parent() const;

        /**
         * Set the name of a pixmap to use in the GUI.
         */
        void setPixmap(const QString &);

        /**
         * Get the name of the pixmap used in the GUI.
         */
        const QString &      pixmapName()    const { return pixmapName_;    }
        
        /**
         * The URL to the folder, e.g. empath://Mailbox1/Folder1/Folder2
         */
        const EmpathURL &    url()            const { return url_;    }

        /**
         * How many messages in total are stored here ?
         */
        Q_UINT32    messageCount()
        { return index_.count(); }

        /**
         * How many messages stored are unread ?
         */
        Q_UINT32    unreadMessageCount()
        { return index_.countUnread(); }

        /**
         * @internal
         * Unique id.
         */
        Q_UINT32 id() const { return id_; }

        /**
         * The index.
         */
        EmpathIndex & index() { return index_; }

        /**
         * Get an index record given an ID.
         * @return 0 if couldn't be found.
         */ 
        const EmpathIndexRecord * record(const QCString & key);

        /**
         * Call this when you want to sync with the mailbox and update any
         * on-screen lists.
         */
        void update();

        /**
         * Call this when an item has disappeared from the index.
         */
        void itemGone(const QString & s) { emit(itemLeft(s));        }
        /**
         * Call this when a new item appears in the index.
         */
        void itemCome(const QString & s) { emit(itemArrived(s));    }
        
        QString indexFileName() { return indexFileName_; }
        
        void setStatus(const QString & id, RMM::MessageStatus status)
        { index_.setStatus(id, status); }
        
    protected slots:
        
        /**
         * Connected to update()
         */
        void s_update() { update(); }

    signals:

        /**
         * The message counts have been updated.
         * @arg c Total message count.
         * @arg uc Unread message count.
         */
        void countUpdated(int c, int uc);
        /**
         * Signals a new message has arrived with given id.
         */
        void itemArrived(const QString &);
        
        /**
         * Signals a message has gone away (with given id).
         */
        void itemLeft    (const QString &);

    private:

        EmpathFolder(const EmpathFolder &) : QObject() {}

        Q_UINT32    id_;
        Q_UINT32    messageCount_;
        Q_UINT32    unreadMessageCount_;
        QString     pixmapName_;
        EmpathIndex index_;
        EmpathURL   url_;
        QString     indexFileName_;
};

#endif

// vim:ts=4:sw=4:tw=78
