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
    icalproperty *writeRecurrenceRule(KORecurrence *);
    icalproperty *writeAlarm(KOAlarm *alarm);

    QString extractErrorProperty(icalcomponent *);    
    Todo *readTodo(icalcomponent *vtodo);
    Event *readEvent(icalcomponent *vevent);
    Journal *readJournal(icalcomponent *vjournal);
    Attendee *readAttendee(icalproperty *attendee);
    void readIncidence(icalcomponent *parent,Incidence *incidence);
    void readRecurrenceRule(icalproperty *rrule,Incidence *event);
    void readAlarm(icalcomponent *alarm,Incidence *incidence);

    icaltimetype writeICalDate(const QDate &);
    icaltimetype writeICalDateTime(const QDateTime &);
    QDate readICalDate(icaltimetype);
    QDateTime readICalDateTime(icaltimetype);
    QString readUtf8Text(const char *);
    icalcomponent *createCalendarComponent();
    icalcomponent *createScheduleComponent(Incidence *,Scheduler::Method);

  private:
    void dumpIcalRecurrence(icalrecurrencetype);
  
    ICalFormat *mParent;
    Calendar *mCalendar;
  
    QList<Event> mEventsRelate;           // events with relations
    QList<Todo> mTodosRelate;             // todos with relations
};

}

#endif
