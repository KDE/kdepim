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
# pragma interface "EmpathIndexRecord.h"
#endif

#ifndef EMPATHMESSAGEDESCRIPTION_H
#define EMPATHMESSAGEDESCRIPTION_H

// Qt includes
#include <qstring.h>
#include <qdatetime.h>

// Local includes
#include <RMM_Message.h>

/**
 * @short An index record
 * 
 * @author Rikkus
 */
class EmpathIndexRecord
{
    public:
        
        /**
         * @internal
         */
        EmpathIndexRecord();
            
        /**
         * Create a new index record using the given id and the given message.
         */
        EmpathIndexRecord(const QString & id, RMM::RMessage &);
        
        /**
         * Copy ctor.
         */
        EmpathIndexRecord(const EmpathIndexRecord &);

        /**
         * The big mega-ctor.
         */
        EmpathIndexRecord(
                const QString &     id,
                const QString &     subject,
                const QString &     senderName,
                const QString &     senderAddress,
                const QDateTime &   date,
                int                 timezone,
                unsigned int        status,
                unsigned int        size,
                const QString &     messageID,
                const QString &     parentMessageID,
                bool                hasAttachments);

        EmpathIndexRecord & operator = (const EmpathIndexRecord &);
        
        ~EmpathIndexRecord();
        
        /**
         * Stream the index record out to a QDataStream.
         */
        friend QDataStream & operator << (QDataStream &, EmpathIndexRecord &);
        
        /**
         * Stream the index record in from a QDataStream.
         */
        friend QDataStream & operator >> (QDataStream &, EmpathIndexRecord &);

        /**
         * The unique id of this record.
         */
        QString id() const
        { return id_; }
        
        /**
         * The subject of the related message.
         */
        QString subject() const
        { return subject_; }
 
        /**
         * The name of the sender of the related message. This is usually
         * the first sender mentioned in 'From:' but may be that referenced
         * in 'Sender:' if there's no 'From:' header.
         */
        QString senderName() const
        { return senderName_; }
 
        /**
         * The address of the sender of the related message.
         * @see senderName
         */
        QString senderAddress() const
        { return senderAddress_; }
        
        /**
         * The date of sending of the related message.
         */
        QDateTime date() const
        { return date_; }

        /**
         * Timezone, expressed in minutes.
         */
        int timeZone() const
        { return timeZone_; }
        
        /**
         * The status of the related message (Read, Marked, ...).
         */
        unsigned int status() const
        { return status_; }
        
        /**
         * The size of the related message.
         */
        unsigned int size() const
        { return size_; }
        
        /**
         * The message-id of the related message.
         */
        QString messageID() const
        { return messageID_; }
        
        /**
         * The message-id of the previous message (for threading).
         */
        QString parentID() const
        { return parentID_; }
        
        /**
         * Find out if there's a previous message (for threading).
         */
        bool hasParent() const
        { return !(parentID_ == "<@>"); }
        
        /**
         * Does this message have more than one part ?
         */
        bool hasAttachments() const
        { return hasAttachments_; }
        
        /**
         * Change the status of this record.
         */
        void setStatus(unsigned int s)
        { status_ = s; }
        
        /**
         * @internal
         */
        void tag(bool b)
        { tagged_ = b; }

        /**
         * @internal
         */
        bool isTagged() const
        { return tagged_; }

        bool isNull() const
        { return id_.isNull(); }

        bool operator ! () const
        { return isNull(); }
        
        /**
         * @internal
         */
        const char * className() const { return "EmpathIndexRecord"; }
        
    private:
        
        // Order dependency
        QString         id_;
        QString         subject_;
        QString         senderName_;
        QString         senderAddress_;
        QDateTime       date_;
        int             timeZone_;
        unsigned int    status_;
        unsigned int    size_;
        QString         messageID_;
        QString         parentID_;
        bool            hasAttachments_;
        
        bool            tagged_;
};

#endif

// vim:ts=4:sw=4:tw=78
