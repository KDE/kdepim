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
#ifndef ICALFORMATIMPL_H
#define ICALFORMATIMPL_H

#include <qstring.h>

#include "scheduler.h"
#include "freebusy.h"

extern "C" {
  #include <ical.h>
  #include <icalss.h>
}

namespace KCal {

class Compat;

/**
  This class provides the libical dependent functions for ICalFormat.
*/
class ICalFormatImpl {
  public:
    /** Create new iCal format for calendar object */
    ICalFormatImpl( ICalFormat *parent );
    virtual ~ICalFormatImpl();

    bool populate( Calendar *, icalcomponent *fs);

    icalcomponent *writeIncidence(Incidence *incidence);
    icalcomponent *writeTodo(Todo *todo);
    icalcomponent *writeEvent(Event *event);
    icalcomponent *writeFreeBusy(FreeBusy *freebusy,
                                 Scheduler::Method method);
    icalcomponent *writeJournal(Journal *journal);
    void writeIncidence(icalcomponent *parent,Incidence *incidence);
    icalproperty *writeAttendee(Attendee *attendee);
    icalproperty *writeAttachment(Attachment *attach);
    icalproperty *writeRecurrenceRule(Recurrence *);
    icalproperty *writeAlarm(Alarm *alarm);

    QString extractErrorProperty(icalcomponent *);
    Todo *readTodo(icalcomponent *vtodo);
    Event *readEvent(icalcomponent *vevent);
    FreeBusy *readFreeBusy(icalcomponent *vfreebusy);
    Journal *readJournal(icalcomponent *vjournal);
    Attendee *readAttendee(icalproperty *attendee);
    Attachment *readAttachment(icalproperty *attach);
    void readIncidence(icalcomponent *parent,Incidence *incidence);
    void readRecurrenceRule(icalproperty *rrule,Incidence *event);
    void readRecurrence( const struct icalrecurrencetype &r, Recurrence* recur );
    void readAlarm(icalcomponent *alarm,Incidence *incidence);
    /** Return the PRODID string loaded from calendar file */
    const QString &loadedProductId()  { return mLoadedProductId; }

    icaltimetype writeICalDate(const QDate &);
    QDate readICalDate(icaltimetype);
    icaltimetype writeICalDateTime(const QDateTime &);
    QDateTime readICalDateTime(icaltimetype);
    icaldurationtype writeICalDuration(int seconds);
    int readICalDuration(icaldurationtype);
    icalcomponent *createCalendarComponent(Calendar * = 0);
    icalcomponent *createScheduleComponent(IncidenceBase *,Scheduler::Method);

  private:
    void writeIncidenceBase(icalcomponent *parent,IncidenceBase *);
    void readIncidenceBase(icalcomponent *parent,IncidenceBase *);
    void writeCustomProperties(icalcomponent *parent,CustomProperties *);
    void readCustomProperties(icalcomponent *parent,CustomProperties *);
    void dumpIcalRecurrence(icalrecurrencetype);

    ICalFormat *mParent;
    Calendar *mCalendar;

    QString mLoadedProductId;         // PRODID string loaded from calendar file
    int mCalendarVersion;             // determines backward compatibility mode on read

    Event::List mEventsRelate;           // events with relations
    Todo::List mTodosRelate;             // todos with relations

    static const int mSecondsPerWeek;
    static const int mSecondsPerDay;
    static const int mSecondsPerHour;
    static const int mSecondsPerMinute;

    Compat *mCompat;
};

}

#endif
