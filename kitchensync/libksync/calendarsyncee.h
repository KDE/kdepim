/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Holger Hans Peter Freyther <freyther@kde.org>

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

#include <libkcal/calendar.h>

#include "syncee.h"

namespace KSync {
class CalendarMerger;
class KDE_EXPORT CalendarSyncEntry : public SyncEntry
{
  public:
    CalendarSyncEntry( Syncee* parent );
    CalendarSyncEntry( KCal::Incidence *, Syncee *parent );
    CalendarSyncEntry( const CalendarSyncEntry& );

    QString name();
    QString id();
    QString timestamp();

    bool equals( SyncEntry *entry );

    CalendarSyncEntry *clone();

    KCal::Incidence *incidence()const;
    void setId(const QString &id);

    KPIM::DiffAlgo* diffAlgo( SyncEntry*, SyncEntry* );

  private:
    KCal::Incidence *mIncidence;
};

/**
  This class provides an implementation of the @see KSyncee interface for KSync.
  It provides syncing of iCalendar files.
*/
class KDE_EXPORT CalendarSyncee : public Syncee
{
  public:
    CalendarSyncee( KCal::Calendar*, CalendarMerger* merger = 0);
    ~CalendarSyncee();

    void reset();

    CalendarSyncEntry *firstEntry();
    CalendarSyncEntry *nextEntry();

    void addEntry( SyncEntry * );
    void removeEntry( SyncEntry * );


    KCal::Calendar *calendar() const { return mCalendar; }

    bool writeBackup( const QString & );
    bool restoreBackup( const QString & );

    QString generateNewId() const;

  private:
    CalendarSyncEntry *createEntry( KCal::Incidence * );

    void clearEntries();

    KCal::Calendar *mCalendar;
    KCal::Event::List mEvents;
    KCal::Event::List::ConstIterator mCurrentEvent;
    KCal::Todo::List mTodos;
    KCal::Todo::List::ConstIterator mCurrentTodo;
    bool mIteratingEvents : 1;

    QMap<KCal::Incidence *,CalendarSyncEntry *> mEntries;
};

}

#endif
