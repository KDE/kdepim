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
# pragma interface "EmpathIndexRecord.h"
#endif

#ifndef EMPATHMESSAGEDESCRIPTION_H
#define EMPATHMESSAGEDESCRIPTION_H

// Qt includes
#include <qstring.h>
#include <qlist.h>

// Local includes
#include "EmpathDefines.h"
#include <RMM_Enum.h>
#include <RMM_Message.h>
#include <RMM_MessageID.h>
#include <RMM_Mailbox.h>
#include <RMM_DateTime.h>


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
                RMM::RMailbox &     sender,
                RMM::RDateTime &    date,
                RMM::MessageStatus  status,
                Q_UINT32            size,
                RMM::RMessageID &   messageID,
                RMM::RMessageID &   parentMessageID);

        ~EmpathIndexRecord();
        
        /**
         * Stream the index record out to a QDataStream.
         */
        friend QDataStream &
            operator << (QDataStream &, EmpathIndexRecord &);
        
        /**
         * Stream the index record in from a QDataStream.
         */
        friend QDataStream &
            operator >> (QDataStream &, EmpathIndexRecord &);

        /**
         * The unique id of this record.
         */
        const QString &     id()        const   { return id_;               }
        /**
         * The subject of the related message.
         */
        const QString &     subject()   const   { return subject_;          }
        /**
         * The sender of the related message. This is usually the first
         * sender mentioned in 'From:' but may be that referenced in 'Sender:'
         * if there's no 'From:' header.
         */
        RMM::RMailbox &     sender()            { return sender_;           }
        /**
         * The date of sending of the related message.
         */
        RMM::RDateTime &    date()              { return date_;             }
        /**
         * The status of the related message (Read, Marked, ...).
         */
        RMM::MessageStatus  status()    const   { return status_;           }
        /**
         * The size of the related message.
         */
        Q_UINT32            size()      const   { return size_;             }
        /**
         * The message-id of the related message.
         */
        RMM::RMessageID &   messageID()         { return messageId_;        }
        /**
         * The message-id of the previous message (for threading).
         */
        RMM::RMessageID &   parentID()          { return parentMessageId_;  }
        
        /**
         * Find out if there's a previous message (for threading).
         */
        bool                hasParent();
        
        /**
         * Nice date representation. FIXME: Locale dependent.
         */
        QString             niceDate(bool twelveHour);

        /**
         * Change the status of this record.
         */
        void setStatus(RMM::MessageStatus s);
        
        /**
         * @internal
         */
        void tag(bool b)    { tagged_ = b; }
        /**
         * @internal
         */
        bool isTagged()     { return tagged_; }
        
        /**
         * @internal
         */
        const char * className() const { return "EmpathIndexRecord"; }
        
    private:
        
        // Order dependency
        QString             id_;
        QString             subject_;
        RMM::RMailbox       sender_;
        RMM::RDateTime      date_;
        RMM::MessageStatus  status_;
        Q_UINT32            size_;
        RMM::RMessageID     messageId_;
        RMM::RMessageID     parentMessageId_;
        
        bool                tagged_;
};

/**
 * @internal
 * @author Rikkus
 */
class EmpathIndexRecordList : public QList<EmpathIndexRecord>
{
    public:
        EmpathIndexRecordList() : QList<EmpathIndexRecord>() {}
        virtual ~EmpathIndexRecordList() {}
        
    protected:
        virtual int compareItems(void * i1, void * i2)
        {
            return
                ((EmpathIndexRecord *)i1)->date().qdt() >
                ((EmpathIndexRecord *)i2)->date().qdt() ? 1 : -1;
        }
};

#endif

// vim:ts=4:sw=4:tw=78
