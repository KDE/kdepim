/* vcal-conduit.c		VCalendar Conduit 
**
**
** Copyright (C) 1998-2000 by Dan Pilone, Preston Brown, and
**	Herwin Jan Steehouwer
**
** A program to synchronize KOrganizer's date book with the Palm
** Pilot / KPilot. This program is part of KPilot.
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

/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/

#ifndef _VCALCONDUIT_H
#define _VCALCONDUIT_H

#include "baseConduit.h"
#include "vcc.h"
#include <pi-datebook.h>

class PilotRecord;
class PilotDateEntry;

class VCalConduit : public BaseConduit
{
public:
  VCalConduit(BaseConduit::eConduitMode mode);
  virtual ~VCalConduit();
  
  virtual void doSync();
  virtual void doBackup();
  virtual QWidget* aboutAndSetup();
  virtual void doTest();

  virtual const char* dbInfo() { return "DatebookDB"; }
  
public:
	/**
	* There are a whole bunch of methods that set particular
	* properties on VObjects. Probably they don't belong here
	* but in versit.
	*/
	void setSummary(VObject *vevent,const char *note);
	void setNote(VObject *vevent,const char *note);
	void setSecret(VObject *vevent,bool secret);
	void setStatus(VObject *vevent,int status);                                                    

protected:
  void doLocalSync();
  PilotRecord *findEntryInDB(unsigned int id);
  VObject *findEntryInCalendar(unsigned int id);
  void deleteVObject(PilotRecord *rec);
  void updateVObject(PilotRecord *rec);
  void saveVCal();
  QString TmToISO(struct tm tm);
  struct tm ISOToTm(const QString &tStr);
  int numFromDay(const QString &day);
  int timeZone;
  VObject *fCalendar;

private:
	void getCalendar();
	/**
	* Retrieve the time zone set in the vcal file.
	* Returns number of minutes relative to UTC.
	*/
	int getTimeZone() const;

	/**
	* Set the event to repeat forever, with repeat
	* frequency @arg rFreq. This function also
	* warns the user that this is probably not
	* *quite* the behavior intented but there's
	* no fix for that.
	*/
	void repeatForever(PilotDateEntry *p,int rFreq,VObject *v=0L);

	/**
	* The following enums distinguish various repeat-by
	* possiblities. Sometimes the specific value of the
	* enum (like DailyPeriod) encodes something special,
	* so these shouldn't be changed at whim without
	* changing @ref repeatUntil as well.
	*/
	typedef enum { DailyPeriod=60*60*24, 	/* seconds per day */
		WeeklyPeriod=60*60*24*7,	/* seconds per week */
		MonthlyByPosPeriod=1,		/* just a constant */
		MonthlyByDayPeriod=2,
		YearlyByDayPeriod=3
		} PeriodConstants;

	/**
	* Set the date entry to repeat every rFreq periods,
	* rDuration times, starting at start. 
	*
	* This function contains code by Dag Nygren.
	*/
	void repeatUntil(PilotDateEntry *dateEntry,
		struct tm *start,
		int rFreq,
		int rDuration,
		PeriodConstants period);

	QString calName;
};

#endif


// $Log:$
