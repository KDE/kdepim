#ifndef CALENDARSYNCEE_H
#define CALENDARSYNCEE_H
// $Id$

#include <calendar.h>

#include "ksyncer.h"

using namespace KCal;

class CalendarSyncEntry : public KSyncEntry
{
  public:
    CalendarSyncEntry(Incidence *);
  
    QString name();
    QString id();
    QString timestamp();
    
    bool equals(KSyncEntry *entry);

    Incidence *incidence() { return mIncidence; }

  private:
    Incidence *mIncidence;
};

/**
  This class provides an implementation of the @KSyncee interface for KSync. It
  provides syncing of iCalendar files.
*/
class CalendarSyncee : public KSyncee
{
  public:
    CalendarSyncee();
    ~CalendarSyncee();
  
    CalendarSyncEntry *firstEntry();
    CalendarSyncEntry *nextEntry();
    
//    CalendarSyncEntry *findEntry(const QString &id);

    void addEntry(KSyncEntry *);
    void removeEntry(KSyncEntry *);

    bool read();
    bool write();

  private:
    CalendarSyncEntry *createEntry(Incidence *);
  
    Calendar *mCalendar;
    QList<Event> mEvents;
    Event *mCurrentEvent;
    
    QList<CalendarSyncEntry> mEntries;
};

#endif
