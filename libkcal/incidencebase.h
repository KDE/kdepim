/*
    This file is part of libkcal.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KCAL_INCIDENCEBASE_H
#define KCAL_INCIDENCEBASE_H
// $Id$
//
// Incidence - base class of calendaring components
//

#include <qdatetime.h>
#include <qobject.h>
#include <qstringlist.h>
#include <qvaluelist.h>

#include "attendee.h"

namespace KCal {

typedef QValueList<QDate> DateList;

/**
  This class provides the base class common to all calendar components.
*/
class IncidenceBase : public QObject
{
    Q_OBJECT
  public:
    IncidenceBase();
    IncidenceBase(const IncidenceBase &);
    ~IncidenceBase();

    /** set the unique text string for the event */
    void setVUID(const QString &);
    /** get the unique text string for the event */
    QString VUID() const;

    /** sets the organizer for the event */
    void setOrganizer(const QString &o);
    QString organizer() const;

    /** Set readonly status. */
    virtual void setReadOnly( bool );
    /** Return if the object is read-only. */
    bool isReadOnly() const { return mReadOnly; }

    /** for setting the event's starting date/time with a QDateTime. */
    virtual void setDtStart(const QDateTime &dtStart);
    /** returns an event's starting date/time as a QDateTime. */
    QDateTime dtStart() const;
    /** returns an event's starting time as a string formatted according to the
     users locale settings */
    QString dtStartTimeStr() const;
    /** returns an event's starting date as a string formatted according to the
     users locale settings */
    QString dtStartDateStr(bool shortfmt=true) const;
    /** returns an event's starting date and time as a string formatted according
     to the users locale settings */
    QString dtStartStr() const;

    virtual void setDuration(int seconds);
    int duration() const;
    void setHasDuration(bool);
    bool hasDuration() const;

    /** returns TRUE or FALSE depending on whether the event "floats,"
     * or doesn't have a time attached to it, only a date. */
    bool doesFloat() const;
    /** sets the event's float value. */
    void setFloats(bool f);

    /** Add Attendee to this incidence. */
    void addAttendee(Attendee *a);
//    void removeAttendee(Attendee *a);
//    void removeAttendee(const char *n);
    /** Remove all Attendees. */
    void clearAttendees();
    /** Return list of attendees. */
    QPtrList<Attendee> attendees() const { return mAttendees; };
    /** Return number of attendees. */
    int attendeeCount() const { return mAttendees.count(); };
    /** Return the Attendee with this email */
    Attendee* attendeeByMail(const QString &);

  signals:
    /** Emitted by the member functions, when the Incidence has been updated. */
    void eventUpdated(IncidenceBase *);

  protected:
    bool mReadOnly;

  private:
    // base components
    QDateTime mDtStart;
    QString mOrganizer;
    QString mVUID;
    QPtrList<Attendee> mAttendees;

    bool mFloats;

    int mDuration;
    bool mHasDuration;
};

}

#endif
