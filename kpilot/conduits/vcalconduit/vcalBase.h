/* vcalBase.h			Base class for KOrganizer conduits
**
** Copyright (C) 2001 by Adriaan de Groot, Cornelius Schumacher
**
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/
#ifndef _VCAL_VCALBASE_H
#define _VCAL_VCALBASE_H

#include "calendar.h"

#include "baseConduit.h"

class PilotRecord;
class QDateTime;

using namespace KCal;

class VCalBaseConduit : public BaseConduit {
  public:
    VCalBaseConduit(BaseConduit::eConduitMode mode,
                    DatabaseSource source=ConduitSocket);
    virtual ~VCalBaseConduit();

    virtual void doSync() = 0;
    virtual void doBackup() = 0;
    virtual QWidget *aboutAndSetup() = 0;
    virtual const char *dbInfo() = 0;

    /**
     * There are a whole bunch of methods that set particular
     * properties on Incidences. Probably they don't belong here
     * but in versit.
     */
    static void setSummary(Incidence *vevent,const char *note);
    static void setNote(Incidence *vevent,const char *note);
    static void setSecret(Incidence *vevent,bool secret);

    /** 
     * Find the summary string of vcalendar event @arg
     * vevent. Returns an empty string if none found.
     */
    static QString getSummary(Incidence *vevent);

    /** Find the description string of vcalendar event @arg
        vevent. Returns an empty string if none found. */
    static QString getDescription(Incidence *vevent);

    /** Check @arg *vevent for KPilotStatus property. Returns 0 or
        1 if the property exists, 2 otherwise (i.e. 0 if the record
        is unmodified, non-zero if it is modified or new).  */
    static int getStatus(Incidence *vevent);

    QDateTime readTm(const struct tm &);
    struct tm writeTm(const QDateTime &);

  protected:
    void saveVCal();

    // Deletes a record from the desktop calendar
    void deleteRecord(PilotRecord *rec);
	
    bool getCalendar(const QString& group);
    void noCalendarError(const QString& conduitName);
	
    bool firstTime() const { return fFirstTime; } ;

    /** Time zone offset to GMT in minutes. Set by
        getCalendar(). */
    int fTimeZone;
    
    QString calName;

    Calendar *calendar() { return fCalendar; }

    enum { TypeTodo, TypeEvent };
    /** Delete all records from the pilot that are not in the
        vcalendar. Meant to be run at the end of a hot-sync, after 
        all new records from both sides have been inserted on the
        other. @arg entryType should be TypeTodo or TypeEvent */
    void deleteFromPilot(int);

    Todo *findTodo(recordid_t id);
    Event *findEvent(recordid_t id);

  private:
    Calendar *fCalendar;
    bool fFirstTime;
};

#endif
