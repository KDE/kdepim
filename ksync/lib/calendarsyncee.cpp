// $Id$

#include <kdebug.h>

#include "calendarsyncee.h"

CalendarSyncEntry::CalendarSyncEntry(Incidence *incidence) :
  mIncidence(incidence)
{
}

QString CalendarSyncEntry::name()
{
  return mIncidence->summary();
}

QString CalendarSyncEntry::id()
{
  return mIncidence->uid();
}

QString CalendarSyncEntry::timestamp()
{
  return mIncidence->lastModified().toString();
}

bool CalendarSyncEntry::equals(KSyncEntry *entry)
{
  CalendarSyncEntry *calEntry = dynamic_cast<CalendarSyncEntry *>(entry);
  if (!calEntry) {
    kdDebug() << "CalendarSyncee::addEntry(): Wrong type." << endl;
    return false;
  }

  if (mIncidence->uid() != calEntry->incidence()->uid()) return false;
  if (mIncidence->lastModified() != calEntry->incidence()->lastModified())
    return false;

  return true;
}

CalendarSyncee::CalendarSyncee()
{
  mCalendar = new CalendarLocal;

  mEntries.setAutoDelete(true);
}

CalendarSyncee::~CalendarSyncee()
{
  delete mCalendar;
}

bool CalendarSyncee::read()
{
  mCalendar->close();
  return mCalendar->load(filename());
}

bool CalendarSyncee::write()
{
  return mCalendar->save(filename());
}


CalendarSyncEntry *CalendarSyncee::firstEntry()
{
  mEvents = mCalendar->events();
  mCurrentEvent = mEvents.begin();
  return createEntry( *mCurrentEvent );
}

CalendarSyncEntry *CalendarSyncee::nextEntry()
{
  ++mCurrentEvent;
  return createEntry( *mCurrentEvent );
}

#if 0
CalendarSyncEntry *CalendarSyncee::findEntry(const QString &id)
{
  Event *event = mCalendar->getEvent(id);
  return createEntry(event);
}
#endif

void CalendarSyncee::addEntry(KSyncEntry *entry)
{
  CalendarSyncEntry *calEntry = dynamic_cast<CalendarSyncEntry *>(entry);
  if (!calEntry) {
    kdDebug() << "CalendarSyncee::addEntry(): SyncEntry has wrong type."
              << endl;
  } else {
    Event *sourceEvent = dynamic_cast<Event *>(calEntry->incidence());
    if (!sourceEvent) {
      kdDebug() << "CalendarSyncee::addEntry(): Incidence is not of type Event."
                << endl;
    } else {
      kdDebug() << "Cloning..." << endl;
      Event *event = dynamic_cast<Event *>(sourceEvent->clone());
      kdDebug() << "Cloning...." << endl;
      mCalendar->addEvent(event);
      kdDebug() << "Cloning....." << endl;
    }
  }
}

void CalendarSyncee::removeEntry(KSyncEntry *entry)
{
  CalendarSyncEntry *calEntry = dynamic_cast<CalendarSyncEntry *>(entry);
  if (!calEntry) {
    kdDebug() << "CalendarSyncee::removeEntry(): SyncEntry has wrong type."
              << endl;
  } else {
    Event *ev = dynamic_cast<Event *>(calEntry->incidence());
    if (ev) {
      mCalendar->deleteEvent(ev);
    } else {
      kdDebug() << "CalendarSyncee::removeEntry(): Incidence has wrong type."
                << endl;
    }
  }
}

CalendarSyncEntry *CalendarSyncee::createEntry(Incidence *incidence)
{
  if (incidence) {
    CalendarSyncEntry *entry = new CalendarSyncEntry(incidence);
    mEntries.append(entry);
    return entry;
  } else {
    return 0;
  }  
}
