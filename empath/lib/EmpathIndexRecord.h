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

#ifndef EMPATH_INDEX_RECORD_H
#define EMPATH_INDEX_RECORD_H

// Qt includes
#include <qstring.h>
#include <qdatetime.h>
#include <qdatastream.h>

/**
 * @short An index record
 * 
 * @author Rikkus
 */
class EmpathIndexRecord
{
    public:

        enum Status {
            Read        = 1 << 0,
            Marked      = 1 << 1,
            Trashed     = 1 << 2,
            Replied     = 1 << 3,
            New         = 1 << 4,
            Old         = 1 << 5,
            Forwarded   = 1 << 6,
            Queued      = 1 << 7,
            Sent        = 1 << 8
        };

        /**
         * @internal
         */
        EmpathIndexRecord()
            :   id_             (QString::null),
                subject_        (""),
                senderName_     (""),
                senderAddress_  (""),
                timeZone_       (0),
                status_         (Status(0)),
                size_           (0),
                messageID_      (""),
                parentID_       (""),
                hasAttachments_ (false),
                tagged_         (false)
        {
            date_.setTime_t(0);
        }

        /**
         * Copy ctor.
         */
        EmpathIndexRecord(const EmpathIndexRecord & i)
            :   id_             (i.id_),
                subject_        (i.subject_),
                senderName_     (i.senderName_),
                senderAddress_  (i.senderAddress_),
                date_           (i.date_),
                timeZone_       (i.timeZone_),
                status_         (i.status_),
                size_           (i.size_),
                messageID_      (i.messageID_),
                parentID_       (i.parentID_),
                hasAttachments_ (i.hasAttachments_),
                tagged_         (i.tagged_)
        {
            // Empty.
        }

        /**
         * The big mega-ctor.
         */
        EmpathIndexRecord(
                const QString &     id,
                const QString &     subject,
                const QString &     senderName,
                const QString &     senderAddress,
                const QDateTime &   date,
                int                 timeZone,
                Status              status,
                unsigned int        size,
                const QString &     messageID,
                const QString &     parentID,
                bool                hasAttachments)
            :
                id_             (id),
                subject_        (subject),
                senderName_     (senderName),
                senderAddress_  (senderAddress),
                date_           (date),
                timeZone_       (timeZone),
                status_         (status),
                size_           (size),
                messageID_      (messageID),
                parentID_       (parentID),
                hasAttachments_ (hasAttachments),
                tagged_         (false)
        {
            if (!subject_)
                subject_ = QString::fromUtf8("");
        }


        EmpathIndexRecord & operator = (const EmpathIndexRecord & i)
        {
            if (this == &i) // Avoid a = a.
                return *this;

            id_             = i.id_;
            subject_        = i.subject_;
            senderName_     = i.senderName_;
            senderAddress_  = i.senderAddress_;
            date_           = i.date_;
            timeZone_       = i.timeZone_,
            status_         = i.status_;
            size_           = i.size_;
            messageID_      = i.messageID_;
            parentID_       = i.parentID_;
            hasAttachments_ = i.hasAttachments_;
            tagged_         = i.tagged_;

            return *this;
        }


        ~EmpathIndexRecord()
        {
            // Empty.
        }


        /**
         * Stream the index record out to a QDataStream.
         */
        friend QDataStream & operator << (
            QDataStream & s,
            EmpathIndexRecord & rec
        )
        {
            s   << rec.id_
                << static_cast<Q_INT32>(rec.tagged_)
                << rec.subject_
                << rec.senderName_
                << rec.senderAddress_
                << rec.date_
                << static_cast<Q_INT32>(rec.timeZone_)
                << static_cast<Q_INT32>(rec.status_)
                << static_cast<Q_INT32>(rec.size_)
                << rec.messageID_
                << rec.parentID_
                << static_cast<Q_INT32>(rec.hasAttachments_);

            return s;
        }

        /**
         * Stream the index record in from a QDataStream.
         */
        friend QDataStream & operator >> (
             QDataStream & s,
             EmpathIndexRecord & rec
        )
        {
            Q_INT32 statusAsInt;
            Q_INT32 taggedAsInt;
            Q_INT32 hasAttachmentsAsInt;
            Q_INT32 sizeAsInt;
            Q_INT32 timeZoneAsInt;

            s   >> rec.id_
                >> taggedAsInt
                >> rec.subject_
                >> rec.senderName_
                >> rec.senderAddress_
                >> rec.date_
                >> timeZoneAsInt
                >> statusAsInt
                >> sizeAsInt
                >> rec.messageID_
                >> rec.parentID_
                >> hasAttachmentsAsInt;

            rec.status_ = static_cast<EmpathIndexRecord::Status>(statusAsInt);
            rec.tagged_         = static_cast<bool>(taggedAsInt);
            rec.hasAttachments_ = static_cast<bool>(hasAttachmentsAsInt);
            rec.timeZone_       = static_cast<int>(timeZoneAsInt);
            rec.size_           = static_cast<unsigned int>(sizeAsInt);

            return s;
        }

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
        Status status() const
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
        { return !parentID_.isEmpty(); }

        /**
         * Does this message have more than one part ?
         */
        bool hasAttachments() const
        { return hasAttachments_; }

        /**
         * Change the status of this record.
         */
        void setStatus(Status s)
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
        Status          status_;
        unsigned int    size_;
        QString         messageID_;
        QString         parentID_;
        bool            hasAttachments_;

        bool            tagged_;
};

#endif

// vim:ts=4:sw=4:tw=78
