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
# pragma implementation "EmpathIndexRecord.h"
#endif

// Qt includes
#include <qdatetime.h>
#include <qdatastream.h>

// KDE includes
#include <kglobal.h>
#include <klocale.h>

// Local includes
#include "Empath.h"
#include "EmpathIndexRecord.h"
#include <RMM_Enum.h>
#include <RMM_MessageID.h>
#include <RMM_Address.h>
#include <RMM_DateTime.h>

EmpathIndexRecord::EmpathIndexRecord()
    :   id_             (QString::null),
        subject_        (""),
        senderName_     (""),
        senderAddress_  (""),
        timeZone_       (0),
        status_         (0),
        size_           (0),
        messageID_      (""),
        parentID_       (""),
        tagged_         (false)
{
    date_.setTime_t(0);
}
        
EmpathIndexRecord::EmpathIndexRecord(const EmpathIndexRecord & i)
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
        tagged_         (i.tagged_)
{
    // Empty.
}

EmpathIndexRecord::EmpathIndexRecord(const QString & id, RMM::RMessage & m)
    :   id_             (id),
        subject_        (m.envelope().subject().asString()),
        senderName_     (m.envelope().firstSender().phrase()),
        senderAddress_  (m.envelope().firstSender().route()),
        date_           (m.envelope().date().qdt()),
        timeZone_       (0), // TODO m.envelope().date().timeZone()),
        status_         ((unsigned int)(m.status())),
        size_           (m.size()),
        messageID_      (m.envelope().messageID().asString()),
        parentID_       (m.envelope().parentMessageId().asString()),
        tagged_         (false)
{
    if (!subject_)
        subject_ = QString::fromUtf8("");
}

EmpathIndexRecord::EmpathIndexRecord(
        const QString &     id,
        const QString &     subject,
        const QString &     senderName,
        const QString &     senderAddress,
        const QDateTime &   date,
        int                 timeZone,
        unsigned int        status,
        unsigned int        size,
        const QString &     messageID,
        const QString &     parentID)
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
        tagged_         (false)
{
    if (!subject_)
        subject_ = QString::fromUtf8("");
}

    EmpathIndexRecord &
EmpathIndexRecord::operator = (const EmpathIndexRecord & i)
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
    tagged_         = i.tagged_;

    return *this;
}

EmpathIndexRecord::~EmpathIndexRecord()
{
    // Empty.
}

#if 0
// Re-enable this for locale == en_UK only ?
    QString
EmpathIndexRecord::niceDate(bool twelveHour)
{
    if (isNull_)
        return QString::null;

    QDateTime now(QDateTime::currentDateTime());
    QDateTime then(date_.qdt());
    
    // Use difference between times to work out how old a message is, and see
    // if we can represent it in a more concise fashion.
    // FIXME: This is only for locale en_GB

    QString dts;
    
    // If the dates differ, then print the day of week..
    if (then.daysTo(now) != 0) {
        dts += then.date().dayName(then.date().dayOfWeek()) + " ";
    // Print the day of month.
        dts += QString().setNum(then.date().day()) + " ";
    }

    // If the months differ, print month name.
    if (then.date().month() != now.date().month())
        dts += then.date().monthName(then.date().month()) + " ";
        
    // If the message is from a different year, add that too.
    if (then.date().year() != now.date().year())
        dts += QString().setNum(then.date().year()) + " ";

    // If the day is the same, print the time of the message. 
    if (then.date().daysTo(now.date()) == 0) 
        dts = KGlobal::locale()->formatTime(then.time());
       
    return dts;
}
#endif

    QDataStream &
operator << (QDataStream & s, EmpathIndexRecord & rec)
{
    s   << rec.id_
        << (Q_UINT8)rec.tagged_
        << rec.subject_
        << rec.senderName_
        << rec.senderAddress_
        << rec.date_
        << rec.timeZone_
        << (Q_UINT8)rec.status_
        << rec.size_
        << rec.messageID_
        << rec.parentID_;

    return s;
}

    QDataStream &
operator >> (QDataStream & s, EmpathIndexRecord & rec)
{
    Q_UINT8 statusAsInt;
    Q_UINT8 taggedAsInt;

    s   >> rec.id_
        >> taggedAsInt
        >> rec.subject_
        >> rec.senderName_
        >> rec.senderAddress_
        >> rec.date_
        >> rec.timeZone_
        >> statusAsInt
        >> rec.size_
        >> rec.messageID_
        >> rec.parentID_;

    rec.status_ = (unsigned int)statusAsInt;
    rec.tagged_ = (bool)taggedAsInt;
    
    return s;
}

// vim:ts=4:sw=4:tw=78
