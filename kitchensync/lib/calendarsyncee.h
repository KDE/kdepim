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
#ifndef CALENDARSYNCEE_H
#define CALENDARSYNCEE_H

#include <libkcal/calendarlocal.h>

#include "syncee.h"

namespace KSync {

class CalendarSyncEntry : public SyncEntry
{
  public:
    CalendarSyncEntry( KCal::Incidence * );
  
    QString type() const { return "CalendarSyncEntry"; }
    QString name();
    QString id();
    QString timestamp();
    
    bool equals( SyncEntry *entry );

    CalendarSyncEntry *clone();

    KCal::Incidence *incidence() { return mIncidence; }

  private:
    KCal::Incidence *mIncidence;
};

/**
  This class provides an implementation of the @KSyncee interface for KSync. It
  provides syncing of iCalendar files.
*/
class CalendarSyncee : public Syncee
{
  public:
    CalendarSyncee();
    CalendarSyncee( KCal::CalendarLocal * );
    ~CalendarSyncee();

    QString type() const { return "CalendarSyncee"; }
    
    CalendarSyncEntry *firstEntry();
    CalendarSyncEntry *nextEntry();
    
//    CalendarSyncEntry *findEntry( const QString &id );

    void addEntry( SyncEntry * );
    void removeEntry( SyncEntry * );

    SyncEntry::PtrList added() { return SyncEntry::PtrList(); }
    SyncEntry::PtrList modified() { return SyncEntry::PtrList(); }
    SyncEntry::PtrList removed() { return SyncEntry::PtrList(); }
    Syncee *clone() { return 0; }

    KCal::CalendarLocal *calendar() const { return mCalendar; }

    bool writeBackup( const QString & );
    bool restoreBackup( const QString & );

  private:
    CalendarSyncEntry *createEntry( KCal::Incidence * );
  
    KCal::CalendarLocal *mCalendar;
    KCal::Event::List mEvents;
    KCal::Event::List::ConstIterator mCurrentEvent;
    
    QPtrList<CalendarSyncEntry> mEntries;
};

}

#endif
