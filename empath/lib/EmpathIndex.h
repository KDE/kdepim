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

#ifndef EMPATHMESSAGEDESCRIPTIONLIST
#define EMPATHMESSAGEDESCRIPTIONLIST

// System includes
#include <gdbm.h>

// Qt includes
#include <qdict.h>
#include <qdatetime.h>

// Local includes
#include "RMM_MessageID.h"
#include "EmpathIndexRecord.h"
#include "EmpathURL.h"

class EmpathFolder;
class EmpathMessageList;

/**
 * Dictionary of index records
 * @author Rikkus
 */
class EmpathIndex
{
    public:
        
        EmpathIndex();
        EmpathIndex(const EmpathURL & folder);
								
        ~EmpathIndex();

        /**
         * Get the index record using the given RMM::RMessageID.
         */
        EmpathIndexRecord * record(const QCString & key);
        
        /**
         * Set the index to talk to the given folder.
         */
        void setFolder(const EmpathURL & folder);
 
        /**
         * Set the index to use the given filename.
         */
        void setFilename(const QString &);
        
        /**
         * Clear out.
         */
        void clear();

        /**
         * Count the number of messages stored.
         */
        Q_UINT32 count();

        /**
         * Count the number of unread messages stored.
         */
        Q_UINT32 countUnread();

        /**
         * Sync up the message list with the mailbox.
         */
        void sync();
        
        /**
         * Insert entry. Will overwrite any existing.
         */
        bool insert(const QCString &, EmpathIndexRecord &);
		
        /**
         * Remove entry.
         */
        bool remove(const QCString &);

        /**
         * @return URL of the related folder.
         */
        const EmpathURL & folder() { return folder_; }
		
        QString indexFileName() { return filename_; }
		
        QDateTime lastModified() const;
		
        QStrList allKeys();

        void setStatus(const QString & id, RMM::MessageStatus status);
        
        void recalculateCount();
        
        const char * className() const { return "EmpathIndex"; }

    protected:
        
        void _open();
        void _close();
        
        int blockSize_;
        EmpathURL folder_;
        QDateTime mtime_;
        int mode_;
        QString filename_;
        GDBM_FILE dbf_;
        
        bool touched_;
        Q_UINT32 count_;
        Q_UINT32 unreadCount_;
};

#endif

// vim:ts=4:sw=4:tw=78
