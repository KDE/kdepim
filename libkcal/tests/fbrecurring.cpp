
#include "icalformat.h"
#include "event.h"
#include "calendarlocal.h"

#include <libkcal/freebusy.h>
#include <iostream>

using namespace KCal;
using namespace std;

int main()
{
  ICalFormat f;

  CalendarLocal cal( TQString::fromLatin1("UTC") );

  Event *event1 = new Event;
  event1->setSummary("A");
  event1->setDtStart( TQDateTime(TQDate(2006,1,1), TQTime(12,0,0)) );
  //event1->setDuration(60*60);
  event1->setDtEnd( TQDateTime(TQDate(2006,1,1), TQTime(13,0,0)) );
  event1->setFloats(FALSE);
  event1->recurrence()->setDaily( 1 );
  //event1->recurrence()->setDuration( 2 );
  event1->recurrence()->setEndDateTime( TQDateTime(TQDate(2006,1,3), TQTime(13,0,0)) );
  cout << f.toICalString(event1).latin1() << endl;
  cal.addEvent(event1);

  Event *event2 = new Event;
  event2->setSummary("B");
  event2->setDtStart( TQDateTime(TQDate(2006,1,1), TQTime(13,0,0)) );
  //event2->setDuration(60*60);
  event2->setDtEnd( TQDateTime(TQDate(2006,1,1), TQTime(14,0,0)) );
  event2->setFloats(FALSE);
  event2->recurrence()->setDaily( 1 );
  //event2->recurrence()->setDuration( 3 );
  event2->recurrence()->setEndDateTime( TQDateTime(TQDate(2006,1,4), TQTime(13,0,0)) );
  cout << f.toICalString(event2).latin1() << endl;
  cal.addEvent(event2);

  Calendar *c = &cal;

  TQDateTime start = TQDateTime(TQDate(2006,1,2), TQTime(0,0,0));
  TQDateTime end = TQDateTime(TQDate(2006,1,3), TQTime(0,0,0));

  FreeBusy *freebusy = new FreeBusy( c, start, end );
  TQString result = f.createScheduleMessage( freebusy, Scheduler::Publish );
  cout << result.latin1() << endl;

  return 0;
}
