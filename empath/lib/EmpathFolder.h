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


#ifndef EMPATHFOLDER_H
#define EMPATHFOLDER_H

// Qt includes
#include <qobject.h>
#include <qstring.h>

// Local includes
#include "EmpathDefines.h"
#include "EmpathIndexRecord.h"
#include "EmpathURL.h"
#include "rmm/Message.h"

class EmpathIndex;
class EmpathMailbox;

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
         */
        QString writeMessage(RMM::Message &);

        /**
         */
        RMM::Message retrieveMessage(const QString & messageID);

        /**
         */
        bool removeMessage(const QString & id);

        /**
         */
        EmpathSuccessMap removeMessage(const QStringList & idList);

        /**
         */
        bool markMessage(const QString & id, EmpathIndexRecord::Status);
        
        /**
         */
        EmpathSuccessMap markMessage(
            const QStringList &,
            EmpathIndexRecord::Status
        );

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
         * @internal
         * Unique id.
         */
        Q_UINT32 id() const { return id_; }

        EmpathIndex * index() { return index_; }

        QStringList allIndexKeys();

        /**
         * Call this when an item has disappeared from the index.
         */
        void itemGone(const QString & s) { emit(itemLeft(s));        }
        /**
         * Call this when a new item appears in the index.
         */
        void itemCome(const QString & s) { emit(itemArrived(s));    }

        void setContainer(bool);
        bool isContainer() const;
        
    protected slots:

        void s_itemGone(const QString & key) { itemGone(key); }

        void s_statusChange(const QString & key, EmpathIndexRecord::Status);

    signals:

        /**
         * The message counts have been updated.
         * @arg c Total message count.
         * @arg uc Unread message count.
         */
        void countUpdated(unsigned int c, unsigned int uc);
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
        QString     pixmapName_;
        EmpathIndex * index_;
        EmpathURL   url_;
        bool        container_;
};

#endif

// vim:ts=4:sw=4:tw=78
