/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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

#include <kdebug.h>

#include "calendarsyncee.h"

using namespace KSync;
using namespace KCal;

CalendarSyncEntry::CalendarSyncEntry( Incidence *incidence )
  : mIncidence(incidence)
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

bool CalendarSyncEntry::equals( SyncEntry *entry )
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

CalendarSyncEntry *CalendarSyncEntry::clone()
{
  return new CalendarSyncEntry( mIncidence );
}


CalendarSyncee::CalendarSyncee()
{
  mCalendar = new CalendarLocal;

  mEntries.setAutoDelete(true);
}

CalendarSyncee::CalendarSyncee( CalendarLocal *calendar )
{
  mCalendar = calendar;

  mEntries.setAutoDelete(true);
}

CalendarSyncee::~CalendarSyncee()
{
  delete mCalendar;
}

CalendarSyncEntry *CalendarSyncee::firstEntry()
{
  mEvents = mCalendar->events();
  mCurrentEvent = mEvents.begin();
  if( mCurrentEvent == mEvents.end() ) return 0;
  return createEntry( *mCurrentEvent );
}

CalendarSyncEntry *CalendarSyncee::nextEntry()
{
  ++mCurrentEvent;
  if ( mCurrentEvent == mEvents.end() ) return 0;
  return createEntry( *mCurrentEvent );
}

#if 0
CalendarSyncEntry *CalendarSyncee::findEntry(const QString &id)
{
  Event *event = mCalendar->getEvent(id);
  return createEntry(event);
}
#endif

void CalendarSyncee::addEntry( SyncEntry *entry )
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

void CalendarSyncee::removeEntry( SyncEntry *entry )
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
