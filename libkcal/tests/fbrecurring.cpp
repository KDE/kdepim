
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

  CalendarLocal cal( QString::fromLatin1("UTC") );

  Event *event1 = new Event;
  event1->setSummary("A");
  event1->setDtStart( QDateTime(QDate(2006,1,1), QTime(12,0,0)) );
  //event1->setDuration(60*60);
  event1->setDtEnd( QDateTime(QDate(2006,1,1), QTime(13,0,0)) );
  event1->setFloats(FALSE);
  event1->recurrence()->setDaily( 1 );
  //event1->recurrence()->setDuration( 2 );
  event1->recurrence()->setEndDateTime( QDateTime(QDate(2006,1,3), QTime(13,0,0)) );
  cout << f.toICalString(event1).latin1() << endl;
  cal.addEvent(event1);

  Event *event2 = new Event;
  event2->setSummary("B");
  event2->setDtStart( QDateTime(QDate(2006,1,1), QTime(13,0,0)) );
  //event2->setDuration(60*60);
  event2->setDtEnd( QDateTime(QDate(2006,1,1), QTime(14,0,0)) );
  event2->setFloats(FALSE);
  event2->recurrence()->setDaily( 1 );
  //event2->recurrence()->setDuration( 3 );
  event2->recurrence()->setEndDateTime( QDateTime(QDate(2006,1,4), QTime(13,0,0)) );
  cout << f.toICalString(event2).latin1() << endl;
  cal.addEvent(event2);

  Calendar *c = &cal;

  QDateTime start = QDateTime(QDate(2006,1,2), QTime(0,0,0));
  QDateTime end = QDateTime(QDate(2006,1,3), QTime(0,0,0));

  FreeBusy *freebusy = new FreeBusy( c, start, end );
  QString result = f.createScheduleMessage( freebusy, Scheduler::Publish );
  cout << result.latin1() << endl;

  return 0;
}
