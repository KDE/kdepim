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
// $Id$

#include <qstring.h>

#include "scheduler.h"

extern "C" {
  #include <ical.h>
  #include <icalss.h>
}

namespace KCal {

/**
  This class provides the libical dependent functions for ICalFormat.
*/
class ICalFormatImpl {
  public:
    /** Create new iCal format for calendar object */
    ICalFormatImpl(ICalFormat *parent, Calendar *);
    virtual ~ICalFormatImpl();

    bool populate(icalfileset *fs);

    icalcomponent *writeTodo(Todo *todo);
    icalcomponent *writeEvent(Event *event);
    icalcomponent *writeJournal(Journal *journal);
    void writeIncidence(icalcomponent *parent,Incidence *incidence);
    icalproperty *writeAttendee(Attendee *attendee);
    icalproperty *writeRecurrenceRule(Recurrence *);
    icalproperty *writeAlarm(Alarm *alarm);

    QString extractErrorProperty(icalcomponent *);    
    Todo *readTodo(icalcomponent *vtodo);
    Event *readEvent(icalcomponent *vevent);
    Journal *readJournal(icalcomponent *vjournal);
    Attendee *readAttendee(icalproperty *attendee);
    void readIncidence(icalcomponent *parent,Incidence *incidence);
    void readRecurrenceRule(icalproperty *rrule,Incidence *event);
    void readAlarm(icalcomponent *alarm,Incidence *incidence);

    icaltimetype writeICalDate(const QDate &);
    QDate readICalDate(icaltimetype);
    icaltimetype writeICalDateTime(const QDateTime &,bool utc=true);
    QDateTime readICalDateTime(icaltimetype);
    icaldurationtype writeICalDuration(int seconds);
    int readICalDuration(icaldurationtype);
    QString readUtf8Text(const char *);
    icalcomponent *createCalendarComponent();
    icalcomponent *createScheduleComponent(Incidence *,Scheduler::Method);

  private:
    void dumpIcalRecurrence(icalrecurrencetype);
  
    ICalFormat *mParent;
    Calendar *mCalendar;
  
    QPtrList<Event> mEventsRelate;           // events with relations
    QPtrList<Todo> mTodosRelate;             // todos with relations
    
    static const int mSecondsPerWeek;
    static const int mSecondsPerDay;
    static const int mSecondsPerHour;
    static const int mSecondsPerMinute;
};

}

#endif
