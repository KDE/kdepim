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

#ifndef _VCAL_VCALCONDUIT_H
#define _VCAL_VCALCONDUIT_H

#include <time.h>

#ifndef _PILOT_DATEBOOK_H_
#include <pi-datebook.h>
#endif

#ifndef _KPILOT_VCALBASE_H
#include "vcalBase.h"
#endif

#ifndef QBITARRAY_H
#include <qbitarray.h>
#endif

class PilotRecord;
class PilotDateEntry;
class VObject;
	
class VCalConduit : public VCalBaseConduit
{
public:
  VCalConduit(BaseConduit::eConduitMode mode);
  virtual ~VCalConduit();
  
  virtual void doSync();
  virtual void doBackup();
  virtual QWidget* aboutAndSetup();
  virtual void doTest();

  virtual const char* dbInfo() { return "DatebookDB"; }
  


protected:
	void doLocalSync();
	void updateVObject(PilotRecord *rec);

private:
	struct tm getStartTime(VObject *vevent);
	struct tm getEndTime(VObject *vevent);
	void setAlarms(PilotDateEntry *dateEntry, VObject
		       *vevent) const;
	void firstSyncCopy(bool DeleteOnPilot);

	/** Copy the start and end times from @arg *vevent to @arg
	 *dateEntry. */
	void setStartEndTimes(PilotDateEntry *dateEntry,
			      VObject *vevent);

	static void setVcalStartEndTimes(VObject *vevent, 
					 const PilotDateEntry &dateEntry);
	static void setVcalAlarms(VObject *vevent, 
				  const PilotDateEntry &dateEntry);
	static void setVcalRecurrence(VObject *vevent, 
				      const PilotDateEntry &dateEntry);
	static void setVcalExceptions(VObject *vevent, 
				      const PilotDateEntry &dateEntry);

	struct eventRepetition {
	  enum ::repeatTypes type;
	  int freq;
	  bool hasEndDate;
	  struct tm startDate, endDate;
	  int duration; // 0 means forever
	  QBitArray weekdays;
	  DayOfMonthType repeatDay; // for monthlyByPos
	  
	  eventRepetition() {
	    type = ::repeatNone;
	    weekdays = QBitArray(7);
	  }

	  eventRepetition(const eventRepetition &e) {
	    type = e.type;
	    freq = e.freq;
	    hasEndDate = e.hasEndDate;
	    startDate = e.startDate;
	    endDate = e.endDate;
	    duration = e.duration;
	    weekdays = e.weekdays;
	    repeatDay = e.repeatDay;
	  }
	};

	eventRepetition getRepetition(VObject *vevent);
	void setRepetition(PilotDateEntry *dateEntry, 
			   const eventRepetition &er);

	/** Get the list of exceptions for a repeating event. The
	    result is an array of struct tm and should be free()d
	    after use. The number of exceptions is written to @arg
	    *n. */
	struct tm *getExceptionDates(VObject *vevent, int *n);

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
			 const struct tm *start,
			 int rFreq,
			 int rDuration,
			 PeriodConstants period);
};

#endif


// $Log$
// Revision 1.15  2001/04/16 13:48:35  adridg
// --enable-final cleanup and #warning reduction
//
// Revision 1.14  2001/03/10 18:26:04  adridg
// Refactored vcal conduit and todo conduit
//
// Revision 1.13  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.12  2001/02/07 15:46:32  adridg
// Updated copyright headers for source release. Added CVS log. No code change.
//
