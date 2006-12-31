/*
    This file is part of libkcal.

    Copyright (c) 2001-2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KCAL_ICALFORMATIMPL_H
#define KCAL_ICALFORMATIMPL_H

#include <qstring.h>
#include <qdict.h>

#include "scheduler.h"
#include "freebusy.h"


extern "C" {
  #include <ical.h>
}

namespace KCal {

class Compat;

/**
  @internal

  This class provides the libical dependent functions for ICalFormat.
*/
class ICalFormatImpl
{
  public:
    /** Create new iCal format for calendar object */
    ICalFormatImpl( ICalFormat *parent );
    virtual ~ICalFormatImpl();

    bool populate( Calendar *, icalcomponent *fs);

    icalcomponent *writeIncidence(IncidenceBase *incidence, Scheduler::Method method = Scheduler::Request );
    icalcomponent *writeTodo(Todo *todo);
    icalcomponent *writeEvent(Event *event);
    icalcomponent *writeFreeBusy(FreeBusy *freebusy,
                                 Scheduler::Method method = Scheduler::Publish );
    icalcomponent *writeJournal(Journal *journal);
    void writeIncidence(icalcomponent *parent,Incidence *incidence);
    icalproperty *writeAttendee(Attendee *attendee);
    icalproperty *writeOrganizer( const Person &organizer );
    icalproperty *writeAttachment(Attachment *attach);
    icalproperty *writeRecurrenceRule(Recurrence *);
    icalrecurrencetype writeRecurrenceRule( RecurrenceRule *recur );
    icalcomponent *writeAlarm(Alarm *alarm);

    QString extractErrorProperty(icalcomponent *);
    Todo *readTodo(icalcomponent *vtodo);
    Event *readEvent(icalcomponent *vevent, icalcomponent *vtimezone);
    FreeBusy *readFreeBusy(icalcomponent *vfreebusy);
    Journal *readJournal(icalcomponent *vjournal);
    Attendee *readAttendee(icalproperty *attendee);
    Person readOrganizer( icalproperty *organizer );
    Attachment *readAttachment(icalproperty *attach);
    void readIncidence(icalcomponent *parent, icaltimezone *timezone, Incidence *incidence);
    void readRecurrenceRule(icalproperty *rrule,Incidence *event );
    void readExceptionRule( icalproperty *rrule, Incidence *incidence );
    void readRecurrence( const struct icalrecurrencetype &r, RecurrenceRule* recur );
    void readAlarm(icalcomponent *alarm,Incidence *incidence);
    /** Return the PRODID string loaded from calendar file */
    const QString &loadedProductId()  { return mLoadedProductId; }

    static icaltimetype writeICalDate(const QDate &);
    static QDate readICalDate(icaltimetype);
    icaltimetype writeICalDateTime(const QDateTime &);
    QDateTime readICalDateTime( icaltimetype&, icaltimezone* tz = 0 );
    static icaldurationtype writeICalDuration(int seconds);
    static int readICalDuration(icaldurationtype);
    static icaldatetimeperiodtype writeICalDatePeriod( const QDate &date );
    icaldatetimeperiodtype writeICalDateTimePeriod( const QDateTime &date );

    icalcomponent *createCalendarComponent(Calendar * = 0);
    icalcomponent *createScheduleComponent(IncidenceBase *,Scheduler::Method);

  protected:
    void dumpIcalRecurrence(icalrecurrencetype);
  private:
    void writeIncidenceBase(icalcomponent *parent,IncidenceBase *);
    void readIncidenceBase(icalcomponent *parent, IncidenceBase *);
    void writeCustomProperties(icalcomponent *parent,CustomProperties *);
    void readCustomProperties(icalcomponent *parent,CustomProperties *);
    void readTimezone(icalcomponent *vtimezone);

    ICalFormat *mParent;
    Calendar *mCalendar;

    QString mLoadedProductId;         // PRODID string loaded from calendar file

    Event::List mEventsRelate;           // events with relations
    Todo::List mTodosRelate;             // todos with relations

    static const int mSecondsPerWeek;
    static const int mSecondsPerDay;
    static const int mSecondsPerHour;
    static const int mSecondsPerMinute;

    Compat *mCompat;

    class ToComponentVisitor;
    class Private;
    Private *d;
};

}

#endif
