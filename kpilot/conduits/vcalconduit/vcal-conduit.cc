/* vcal-conduit.cc		VCalendar Conduit
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


// I have noticed that this is full of memory leaks, but since it is
// short lived, it shouldn't matter so much. -- PGB


#ifndef _KPILOT_OPTIONS_H
#include "options.h"
#endif

#include <sys/types.h>
#include <signal.h>
#include <iostream.h>
#include <stdlib.h>
#include <time.h>

#ifndef QBITARRAY_H
#include <qbitarray.h>
#endif

#ifndef QDIR_H
#include <qdir.h>
#endif

#ifndef QDATETM_H
#include <qdatetm.h>
#endif

#ifndef QSTRING_H
#include <qstring.h>
#endif

#ifndef QMSGBOX_H
#include <qmsgbox.h>
#endif


#ifndef _KCONFIG_H
#include <kconfig.h>
#endif

#ifndef _KDEBUG_H
#include <kdebug.h>
#endif


#ifndef _KPILOT_KPILOTCONFIG_H
#include "kpilotConfig.h"
#endif

#ifndef _KPILOT_PILOTDATABASE_H
#include "pilotDatabase.h"
#endif

#ifndef _KPILOT_PILOTRECORD_H
#include "pilotRecord.h"
#endif

#ifndef _KPILOT_PILOTDATEENTRY_H
#include "pilotDateEntry.h"
#endif


#ifndef _KPILOT_VCAL_CONDUIT_H
#include "vcal-conduit.h"
#endif

#ifndef _KPILOT_VCAL_SETUP_H
#include "vcal-setup.h"
#endif

#ifndef _KPILOT_CONDUITAPP_H
#include "conduitApp.h"
#endif

#ifndef __VCC_H__
#include "vcc.h"
#endif

static const char *id=
	"$Id$";


int main(int argc, char* argv[])
{
  ConduitApp a(argc, argv, "vcal_conduit",
  	"Calendar / Organizer conduit",
	KPILOT_VERSION);
  a.addAuthor("Preston Brown",I18N_NOOP("Organizer author"));
	a.addAuthor("Adriaan de Groot",
		I18N_NOOP("Maintainer"),
		"adridg@cs.kun.nl");
	a.addAuthor("Philipp Hullmann",
		I18N_NOOP("Bugfixer"));
  VCalConduit conduit(a.getMode());
  a.setConduit(&conduit);
  return a.exec(false,true);

	/* NOTREACHED */
	(void) id;
}


/*****************************************************************************/

VCalConduit::VCalConduit(BaseConduit::eConduitMode mode) :
	VCalBaseConduit(mode)
{
	FUNCTIONSETUP;
}


VCalConduit::~VCalConduit()
{
}


//////////////////////////////////////////////////////////////////////////


void VCalConduit::doBackup()
{
  FUNCTIONSETUP;
  PilotRecord* rec;
  int index = 0;

	if (!getCalendar(VCalSetup::VCalGroup))
	{
		noCalendarError(i18n("VCal Conduit"));
		exit(ConduitMisconfigured);
	}
		
	DEBUGCONDUIT << __FUNCTION__
		     << ": Performing full backup"
		     << endl;

	// Get ALL entries from Pilot
	//
	//
	while ((rec=readRecordByIndex(index++)))
	{
		if (rec->isDeleted())
		{
			deleteVObject(rec, VCEventProp);
		}
		else
		{
			updateVObject(rec);
		}
		delete rec;
	}

	saveVCal();
} // void VCalConduit::doBackup()


//////////////////////////////////////////////////////////////////////////


void VCalConduit::doSync()
{
	FUNCTIONSETUP;

   PilotRecord* rec;
	int recordcount=0;

	if (!getCalendar(VCalSetup::VCalGroup)) {
	  noCalendarError(i18n("VCal Conduit"));
	  exit(ConduitMisconfigured);
	}

	// get only MODIFIED entries from Pilot, compared with 
	// the above (doBackup), which gets ALL entries
	//
	//
	while ((rec=readNextModifiedRecord())) 
	{
		recordcount++;
		if (rec->isDeleted())
		{
			deleteVObject(rec, VCEventProp);
		} 
		else 
		{
			bool pilotRecModified = 
				(rec->getAttrib() & dlpRecAttrDirty);
			if (pilotRecModified) 
			{
				updateVObject(rec);
			} 
			else 
			{
			  kdWarning(CONDUIT_AREA) << __FUNCTION__
					<< "weird! we asked for a modified "
					   "record and got one that wasn't"
					<< endl;
			}
		}
		delete rec;
	}
   
	DEBUGCONDUIT << __FUNCTION__
			      << ": Read a total of "
			      << recordcount
			      << " modified records from the pilot."
			      << endl;

   // now, all the stuff that was modified/new on the pilot should be
   // added to the vCalendar.  We now need to add stuff to the pilot
   // that is modified/new in the vCalendar (the opposite).	  
   doLocalSync();
   
   // now we save the vCalendar.
   saveVCal();
} // void VCalConduit::doSync()


//////////////////////////////////////////////////////////////////////////


void VCalConduit::repeatForever(
	PilotDateEntry *dateEntry,
	int rFreq,
	VObject *vevent)
{
	FUNCTIONSETUP;

	const char *s = "<no description>";

	if (vevent)
	{
		VObject *vo = isAPropertyOf(vevent, VCSummaryProp);
		if (vo != 0L)
		{
			s=fakeCString(vObjectUStringZValue(vo));
		}
	}


	dateEntry->setRepeatFrequency(rFreq);
	dateEntry->setRepeatForever();
	kdWarning(CONDUIT_AREA) << __FUNCTION__
		<< ": repeat duration is forever for "
		<< s
		<< endl;
} 
// void VCalConduit::repeatForever(PilotDateEntry *dateEntry,	int
// rFreq, VObject *vevent)



//////////////////////////////////////////////////////////////////////////


void VCalConduit::repeatUntil(
	PilotDateEntry *dateEntry,
	const struct tm *start,
	int rFreq,
	int rDuration,
	PeriodConstants period)
{
	FUNCTIONSETUP;
	struct tm rStart = *start;
	time_t end_time = mktime(&rStart);
	struct tm rEnd = *start;

	switch(period)
	{
	case DailyPeriod:
	case WeeklyPeriod:
		// Calculate the end time by adding the right number of
		// repeat periods.
		//
		//
		end_time += rFreq * (rDuration-1) * (int) period;

		dateEntry->setRepeatFrequency(rFreq);
		dateEntry->setRepeatEnd(*localtime(&end_time));
		break;
	case MonthlyByDayPeriod:
	case MonthlyByPosPeriod:
		dateEntry->setRepeatFrequency(rFreq);
		rEnd.tm_mon += rFreq * (rDuration - 1);
		rEnd.tm_year += rEnd.tm_mon / 12;
		rEnd.tm_mon %= 12;
		dateEntry->setRepeatEnd(rEnd);
		break;
	case YearlyByDayPeriod:
		dateEntry->setRepeatFrequency(rFreq);
		rEnd.tm_year += rFreq * (rDuration - 1);
		dateEntry->setRepeatEnd(rEnd);
		break;
	default:
		kdWarning(CONDUIT_AREA) << __FUNCTION__
			<< ": Unknown repeat period "
			<< (int) period
			<< endl;
	}
}
// void VCalConduit::repeatUntil(PilotDateEntry *dateEntry, struct tm
// start, int rFreq, int rDuration, PeriodConstants period)

/*****************************************************************************/

/*
 * Given a pilot record, check to see what needs to be done to the
 * analogous vobject to bring things into sync.
 */
void VCalConduit::updateVObject(PilotRecord *rec) {
  FUNCTIONSETUP;

  QDateTime todaysDate = QDateTime::currentDateTime();
  //QString tmpStr;
  //QString numStr;
  PilotDateEntry dateEntry(rec);
  
  VObject *vevent = findEntryInCalendar(rec->getID(), VCEventProp);
  if (!vevent) {
    // no event was found, so we need to add one with some initial info
    DEBUGCONDUIT << __FUNCTION__ << ": creating new vCalendar event"
		 << endl;
    vevent = addProp(calendar(), VCEventProp);
    
    addDateProperty(vevent, VCDCreatedProp, todaysDate);

    QString numStr;
    numStr.sprintf("KPilot - %ld", rec->getID());
    addPropValue(vevent, VCUniqueStringProp, numStr.latin1());
    addPropValue(vevent, VCSequenceProp, "1");
    addDateProperty(vevent,VCLastModifiedProp,todaysDate);
    
    addPropValue(vevent, VCPriorityProp, "0");
    addPropValue(vevent, VCTranspProp, "0");
    addPropValue(vevent, VCRelatedToProp, "0");
    addPropValue(vevent, KPilotIdProp,
		 numStr.setNum(dateEntry.getID()).latin1());
    addPropValue(vevent, KPilotStatusProp, "0");
  }
  
  // determine whether the vobject has been modified since the last sync
  bool vcalRecModified = (getStatus(vevent) != 0);
  
  if (vcalRecModified) {
    // we don't want to modify the vobject with pilot info, because it has
    // already been  modified on the desktop.  The VObject's modified state
    // overrides the PilotRec's modified state.
    return;
  }
  // otherwise, the vObject hasn't been touched.  Updated it with the
  // info from the PilotRec.
  
  setVcalStartEndTimes(vevent, dateEntry);
  setVcalAlarms(vevent, dateEntry);
  setVcalRecurrence(vevent, dateEntry);
  setVcalExceptions(vevent, dateEntry);

  setSummary(vevent, dateEntry.getDescription());
  setNote(vevent, dateEntry.getNote());
  setSecret(vevent, (rec->getAttrib() & dlpRecAttrSecret));
  setStatus(vevent, 0);
  
} // void VCalConduit::updateVObject(PilotRecord *rec)


//////////////////////////////////////////////////////////////////////////


void VCalConduit::setVcalStartEndTimes(VObject *vevent,
				       const PilotDateEntry &dateEntry) {
  FUNCTIONSETUP;

  // START TIME //
  VObject *vo = isAPropertyOf(vevent, VCDTstartProp);
  
  // check whether the event is an event or an appointment.  See dateEntry
  // structure for more info.
  if (!dateEntry.getEvent()) {
    // the event doesn't "float"
    if (vo)
      setDateProperty(vo,dateEntry.getEventStart_p());
    else 
      addDateProperty(vevent, VCDTstartProp, 
		      dateEntry.getEventStart_p());
  } else {
    // the event floats
    if (vo)
      setDateProperty(vo,dateEntry.getEventStart_p(),true);
    else
      addDateProperty(vevent, VCDTstartProp,
		      dateEntry.getEventStart_p(),true);
  }

  // END TIME //
  vo = isAPropertyOf(vevent, VCDTendProp);
  
  // Patch by Heiko
  bool multiDay = ( (dateEntry.getRepeatType() == repeatDaily) &&
		    dateEntry.getEvent() );
  
  if (multiDay) {
    // I honestly don't know what was supposed to go here
  }

  QDateTime endDT;
  // handle the case of a "repeating event on a daily basis" which is the
  // pilot's way of indicating a multi-day event.
  if (!multiDay) {
    endDT.setDate(QDate(1900+dateEntry.getEventEnd().tm_year,
			dateEntry.getEventEnd().tm_mon + 1,
			dateEntry.getEventEnd().tm_mday));
    endDT.setTime(QTime(dateEntry.getEventEnd().tm_hour,
			dateEntry.getEventEnd().tm_min,
			dateEntry.getEventEnd().tm_sec));
  } else {
    endDT.setDate(QDate(1900+dateEntry.getRepeatEnd().tm_year,
			dateEntry.getRepeatEnd().tm_mon + 1,
			dateEntry.getRepeatEnd().tm_mday));
    endDT.setTime(QTime(dateEntry.getRepeatEnd().tm_hour,
			dateEntry.getRepeatEnd().tm_min,
			dateEntry.getRepeatEnd().tm_sec));
  }
      

  if (vo)
    setDateProperty(vo, endDT, dateEntry.getEvent());
  else 
    /* We don't want to add it if it isn't there already, or if the
       event isn't multiday/floating.  It is deprecated to have both
       DTSTART and DTEND set to 000000 for their times. */
    if (!dateEntry.getEvent() || multiDay)
      addDateProperty(vevent, VCDTendProp, endDT, dateEntry.getEvent());
} 
// void VCalConduit::setVcalStartEndTimes(VObject *vevent,
// PilotDateEntry *dateEntry)


//////////////////////////////////////////////////////////////////////////


void VCalConduit::setVcalAlarms(VObject *vevent, 
				const PilotDateEntry &dateEntry) {
  FUNCTIONSETUP;

  VObject *vo = isAPropertyOf(vevent, VCDAlarmProp);

  if (dateEntry.getAlarm()) {
    QDateTime alarmDT;
    alarmDT.setDate(QDate(1900+dateEntry.getEventStart().tm_year,
			  dateEntry.getEventStart().tm_mon + 1,
			  dateEntry.getEventStart().tm_mday));
    alarmDT.setTime(QTime(dateEntry.getEventStart().tm_hour,
			  dateEntry.getEventStart().tm_min,
			  dateEntry.getEventStart().tm_sec));
    
    int advanceUnits = dateEntry.getAdvanceUnits();
    switch(advanceUnits) {
    case advMinutes: advanceUnits = 1; break;
    case advHours: advanceUnits = 60; break;
    case advDays: advanceUnits = 60*24; break;
    default:
      DEBUGCONDUIT << __FUNCTION__ 
		   << ": Unknown advance units "
		   << advanceUnits
		   << endl;
      advanceUnits=1;
    }

    alarmDT = alarmDT.addSecs(60*advanceUnits*-(dateEntry.getAdvance()));

    if (vo) {
      vo = isAPropertyOf(vo, VCRunTimeProp);
      setDateProperty(vo, alarmDT);
    } else {
      vo = addProp(vevent, VCDAlarmProp);
      addDateProperty(vo, VCRunTimeProp, alarmDT);
      addPropValue(vo, VCRepeatCountProp, "1");
      addPropValue(vo, VCDisplayStringProp, "beep!");
    }
  } else if (vo)
    addProp(vo, KPilotSkipProp);
}
// void VCalConduit::setVcalAlarms(VObject *vevent, PilotDateEntry
// *dateEntry)


//////////////////////////////////////////////////////////////////////////


/* Pilot entries that repeat daily are not what we consider daily
   repeating events in vCalendar/KOrganizer.  It is actually a multi-day
   appointment and handled by setVcalStartEndTimes(). */
void VCalConduit::setVcalRecurrence(VObject *vevent, 
				    const PilotDateEntry &dateEntry) {
  FUNCTIONSETUP;

  const char *dayname[] = {"SU ", "MO ", "TU ", "WE ", "TH ", "FR ",
			   "SA "};

  VObject *vo = isAPropertyOf(vevent, VCRRuleProp);

  if (dateEntry.getRepeatType() != repeatNone &&
      !((dateEntry.getRepeatType() == repeatDaily) &&
	dateEntry.getEvent())) {
    QString tmpStr("");
    switch(dateEntry.getRepeatType()) {
    case repeatDaily:
      tmpStr.sprintf("D%i ", dateEntry.getRepeatFrequency());
      break;
    case repeatWeekly:
      {
	const int *days = dateEntry.getRepeatDays();

	DEBUGCONDUIT << __FUNCTION__
		     << ": Got repeat-weekly entry, by-days="
		     << days[0] << " "
		     << days[1] << " "
		     << days[2] << " "
		     << days[4] << " "
		     << days[5] << " "
		     << days[6] << " "
		     << endl;

	tmpStr.sprintf("W%i ", dateEntry.getRepeatFrequency());
	for (int i = 0; i < 7; i++)
	  if (days[i]) tmpStr.append(dayname[i]);
      }
      break;
    case repeatMonthlyByDay: 
      tmpStr.sprintf("MP%i %d+ ", dateEntry.getRepeatFrequency(),
		     (dateEntry.getRepeatDay() / 7) + 1);
      tmpStr.append(dayname[dateEntry.getRepeatDay() % 7]);
      break;
    case repeatMonthlyByDate:
      tmpStr.sprintf("MD%i ", dateEntry.getRepeatFrequency());
      break;
    case repeatYearly:
      tmpStr.sprintf("YD%i ", dateEntry.getRepeatFrequency());
      break;
    case repeatNone:
      DEBUGCONDUIT << __FUNCTION__
		   << ": argh! we think it repeats, "
		   << "but dateEntry has repeatNone!\n";
      break;
    default:
      break;
    }

    if (dateEntry.getRepeatForever())
      tmpStr += "#0";
    else 
      tmpStr += TmToISO(dateEntry.getRepeatEnd());

    vo = isAPropertyOf(vevent, VCRRuleProp);
    if (vo)
      setVObjectUStringZValue_(vo, fakeUnicode(tmpStr.latin1(), 0));
    else
      addPropValue(vevent, VCRRuleProp,tmpStr.latin1());
  } else if (vo)
    addProp(vo, KPilotSkipProp);
}
// void VCalConduit::setVcalRecurrence(VObject *vevent, PilotDateEntry
// *dateEntry)


//////////////////////////////////////////////////////////////////////////


void VCalConduit::setVcalExceptions(VObject *vevent, 
				    const PilotDateEntry &dateEntry) {
  FUNCTIONSETUP;

  if (((dateEntry.getRepeatType() == repeatDaily) &&
       dateEntry.getEvent()) && dateEntry.getExceptionCount())
    DEBUGCONDUIT << __FUNCTION__
		 << ": WARNING Exceptions ignored for multi-day event "
		 << dateEntry.getDescription()
		 << endl ;

  VObject *vo = isAPropertyOf(vevent, VCExDateProp);
  QString tmpStr("");
  if (dateEntry.getExceptionCount()) {
    for (int i = 0; i < dateEntry.getExceptionCount(); i++) {
      tmpStr += TmToISO(dateEntry.getExceptions()[i]);
      tmpStr += ";";
    }
    tmpStr.truncate(tmpStr.length() - 1);
    if (vo)
      setVObjectUStringZValue_(vo, fakeUnicode(tmpStr.latin1(), 0));
    else
      addPropValue(vevent, VCExDateProp, tmpStr.latin1());
  } else if (vo)
    addProp(vo, KPilotSkipProp);
}


//////////////////////////////////////////////////////////////////////////


void VCalConduit::doLocalSync()
{
  FUNCTIONSETUP;

  DEBUGCONDUIT << __FUNCTION__ 
	       << ": Performing local sync."
	       << endl;

  VObjectIterator i;
  initPropIterator(&i, calendar());

  int recordcount = 0;

  // go through the whole vCalendar.  If the event has the dirty
  // (modified) flag set, make a new pilot record and add it.  we only
  // take events that have KPilotStatusProp as a property.  If this
  // property isn't present, ignore the event.

  /* Since the calendar is a singly linked list, we need a pointer to
     the previous item. */
  VObject *previousEvent = 0;

  while (moreIteration(&i)) {
    recordcount++;
    VObject *vevent = nextVObject(&i);

    if (getStatus(vevent) != 2 &&
	(strcmp(vObjectName(vevent), VCEventProp) == 0)) {
      // the calendar entry is an event and has a KPilotStatus field

      recordid_t id;
      if (getStatus(vevent) == 1) {
	// the event has been modified, need to write it to the pilot
	// we read the pilotID.
	
	id = getRecordID(vevent);
	
	// if id != 0, this is a modified event, otherwise it is new.

	PilotDateEntry *dateEntry = 0L;
	
	if (id != 0) {
	  PilotRecord *pRec = readRecordById(id);
	  // if this fails, somehow the record got deleted from the pilot
	  // but we were never informed! bad pilot. naughty pilot.

	  if (pRec)
	    // If the record was deleted on the pilot, recreate it.
	    dateEntry = new PilotDateEntry(pRec);
	  else {
	    dateEntry = new PilotDateEntry();
	    id = 0;
	  }
	} else
	  dateEntry = new PilotDateEntry();

	setStartEndTimes(dateEntry, vevent);
	setAlarms(dateEntry, vevent);

	// RECURRENCE(S) //
	{
	  // first we have a 'fake type of recurrence' when a multi-day
	  // even it passed to the pilot, it is converted to an event
	  // which recurs daily a number of times.

	  struct tm start = getStartTime(vevent);
	  QDateTime tmpDtStart = tmToQDateTime(start);
	  struct tm end = getEndTime(vevent);
	  QDateTime tmpDtEnd = tmToQDateTime(end);

	  if (tmpDtEnd.isValid())
	    if (tmpDtStart.daysTo(tmpDtEnd) > 0 &&
		start.tm_hour == 0 && start.tm_min == 0 && 
		start.tm_sec == 0) {
	      // multi day event
	      DEBUGCONDUIT << __FUNCTION__
			   << ": multi-day event from "
			   << tmpDtStart.toString() << " to " 
			   << tmpDtEnd.toString() << endl;
	      dateEntry->setRepeatType(repeatDaily);
	      dateEntry->setRepeatFrequency(1);
	      dateEntry->setRepeatEnd(end);
	      if (isAPropertyOf(vevent, VCExDateProp) != 0L)
		DEBUGCONDUIT << __FUNCTION__
			     << ": WARNING: exceptions ignored "
			     << "for multi-day event "
			     << getSummary(vevent)
			     << endl ;
	    }
	}

	// and now the real recurring events
	setRepetition(dateEntry, getRepetition(vevent));
	
	// EXCEPTION(S) //
	{
	  int count;
	  struct tm *exceptionList = getExceptionDates(vevent, &count);
	  if (exceptionList) {
	    dateEntry->setExceptionCount(count);
	    dateEntry->setExceptions(exceptionList);
	  } else
	    dateEntry->setExceptionCount(0);
	}

	// SUMMARY //
	dateEntry->setDescription(getSummary(vevent).latin1());

	// DESCRIPTION //
	dateEntry->setNote(getDescription(vevent).latin1());

	// put the pilotRec in the database...
	{
	  PilotRecord *pRec = dateEntry->pack();
	  pRec->setAttrib(dateEntry->getAttrib() & ~dlpRecAttrDirty);
	  id = writeRecord(pRec);
	  ::free(pRec);
	}

	delete dateEntry;

	if (id > 0) {
	  // Writing succeeded. Write the id we got from writeRecord
	  // back to the vObject.
	  setNumProperty(vevent, KPilotIdProp, id);
	} 
      }
      // Clear the 'modified' flag.
      setNumProperty(vevent, KPilotStatusProp, 0);
    }
    previousEvent = vevent;
  }

  DEBUGCONDUIT << __FUNCTION__ << ": Read " << recordcount
			<< " records total." << endl;
  
  KConfig& config = KPilotConfig::getConfig(VCalSetup::VCalGroup);
  bool DeleteOnPilot = config.readBoolEntry("DeleteOnPilot", true);

  if (firstTime())
    firstSyncCopy(DeleteOnPilot);

  if (DeleteOnPilot)
    deleteFromPilot(VCEventProp);

  setFirstTime(config, false);
} // void VCalConduit::doLocalSync()


//////////////////////////////////////////////////////////////////////////


struct tm *VCalConduit::getExceptionDates(VObject *vevent, int *n) {
  FUNCTIONSETUP;
  struct tm *tmList = 0;
  int count = 0;
  VObject *vo = isAPropertyOf(vevent, VCExDateProp);
  if (vo) {
    char *s = fakeCString(vObjectUStringZValue(vo));
    QString tmpStr(s);
    deleteStr(s);
    int index = 0, index2 = 0;
    struct tm extm;
    while ((index2 = tmpStr.find(',', index)) != -1) {
      ++count;
      tmList = (struct tm *) realloc(tmList, sizeof(struct
						    tm)*count);
      if (tmList == 0L)
	kdFatal(CONDUIT_AREA) << __FUNCTION__
			      << ": realloc() failed!"
			      << endl;
      extm = ISOToTm(tmpStr.mid(index, (index2-index)));
      tmList[count-1] = extm;
      index = index2 + 1;
    }
    ++count;
    tmList = (struct tm *) realloc(tmList, sizeof(struct tm)*count);
    if (tmList == 0L)
      kdFatal(CONDUIT_AREA) << __FUNCTION__
			    << ": realloc() failed!" << endl;
    extm = ISOToTm(tmpStr.mid(index, (tmpStr.length()-index)));
    tmList[count-1] = extm;
  }
  if (n) *n = count;
  return tmList;
} // struct tm *VCalConduit::getExceptionDates(const VObject *vevent, int *n)


//////////////////////////////////////////////////////////////////////////

void VCalConduit::firstSyncCopy(bool DeleteOnPilot) {
  FUNCTIONSETUP;

  bool insertall = false, skipall = false;

  /* Build a list of records in the pilot calendar that are not
     found in the vcal and thus probably have been deleted. */
  
  // Get all entries from Pilot
  PilotRecord *rec;
  int index = 0;
  while ((rec = readRecordByIndex(index++)) != 0) {
    PilotDateEntry *dateEntry = new PilotDateEntry(rec);
    
    if (!dateEntry) {
      kdError(CONDUIT_AREA) << __FUNCTION__
			    << ": Conversion to PilotDateEntry failed"
			    << endl;
      continue;
    }
    
    VObject *vevent = findEntryInCalendar(rec->getID(),
					  VCEventProp);
    if (vevent == 0L) {
      DEBUGCONDUIT << __FUNCTION__
		   << ": Entry found on pilot but not in vcalendar."
		   << endl;
      
      // First hot-sync, ask user how to treat this event.
      if (!insertall && !skipall) {
	DEBUGCONDUIT << __FUNCTION__
		     << ": Questioning event disposition."
		     << endl;
	
	QString text = i18n("This is the first time that "
			    "you have done a HotSync\n"
			    "with the vCalendar conduit. "
			    "There is an appointment\n"
			    "in the PalmPilot which is not "
			    "in the vCalendar (KOrganizer).\n\n");
	text += i18n("Appointment: %1.\n\n"
		     "What must be done with this appointment?")
	  .arg(dateEntry->getDescription());
	
	int response =
	  QMessageBox::information(0, 
				   i18n("KPilot vCalendar Conduit"), 
				   text, 
				   i18n("&Insert"), 
				   DeleteOnPilot ? i18n("&Delete") : 
				   i18n("&Skip"),
				   i18n("Insert &All"),
				   0);
	
	DEBUGCONDUIT << __FUNCTION__ 
		     << ": Event disposition "
		     << response
		     << endl;
	
	switch (response) {
	case 0:
	default: 
	  /* Default is to insert this single entry and ask again
	     later. */
	  updateVObject(rec);
	  break;
	case 1:
	  // Skip this item, deletion is handled by deleteFromPilot().
	  break;
	case 2:
	  insertall = true;
	  skipall = false;
	  updateVObject(rec);
	  break;
	} // switch (response)
      } else if (insertall) {
	// all records are to be inserted.
	updateVObject(rec);
      }
    } // if (!vevent)
    delete rec;
  } // while ((rec = readRecordByIndex(index++)) != 0)
} // void VCalConduit::processDeleted()


//////////////////////////////////////////////////////////////////////////

struct VCalConduit::eventRepetition
VCalConduit::getRepetition(VObject *vevent) {
  FUNCTIONSETUP;

  struct eventRepetition r;
  VObject *vo = isAPropertyOf(vevent, VCRRuleProp);
  if (vo) {
    r.startDate = getStartTime(vevent);
    char *s = fakeCString(vObjectUStringZValue(vo));
    QString tmpStr(s);
    deleteStr(s);
    tmpStr.simplifyWhiteSpace();
    tmpStr = tmpStr.upper();

    int start = 0;
    if (tmpStr.left(1) == "D") {
      DEBUGCONDUIT << __FUNCTION__ << ": repeat daily" << endl;
      r.type = ::repeatDaily;
      start = 1;
    } else if (tmpStr.left(1) == "W") {
      DEBUGCONDUIT << __FUNCTION__ << ": repeat weekly" << endl;
      r.type = ::repeatWeekly;
      start = 1;
    } else if (tmpStr.left(2) == "MP") {
      DEBUGCONDUIT << __FUNCTION__ << ": repeat monthly by day" << endl;
      r.type = ::repeatMonthlyByDay;
      start = 2;
    } else if (tmpStr.left(2) == "MD") {
      DEBUGCONDUIT << __FUNCTION__ << ": repeat monthly by date" << endl;
      r.type = ::repeatMonthlyByDate;
      start = 2;
    } else if (tmpStr.left(2) == "YD") {
      DEBUGCONDUIT << __FUNCTION__ << ": repeat yearly" << endl;
      r.type = ::repeatYearly;
      start = 2;
    } else
      r.type = ::repeatNone;

    int index = tmpStr.find(' ');
    int last = tmpStr.findRev(' ') + 1;

    r.freq = tmpStr.mid(start, (index - 1)).toInt();
    index++; // advance to beginning of stuff after freq

    r.hasEndDate = false;

    switch (r.type) {

    case ::repeatDaily:
      index = last; // advance to last field
      if (tmpStr.mid(index, 1) == "#") index++;
      if (tmpStr.find('T', index) != -1) {
	r.endDate = ISOToTm(tmpStr.mid(index, tmpStr.length() -
				       index).latin1());
	r.hasEndDate = true;
      } else
	r.duration =
	  tmpStr.mid(index, tmpStr.length() - index).toInt();
      break;

    case ::repeatWeekly:
      if (index == last) {
	QDate tmpDate(1900 + r.startDate.tm_year,
		      r.startDate.tm_mon + 1,
		      r.startDate.tm_mday);
	r.weekdays.setBit(tmpDate.dayOfWeek() - 1);
      } else {
	while (index < last) {
	  QString dayStr = tmpStr.mid(index, 3);
	  int dayNum = numFromDay(dayStr);
	  r.weekdays.setBit(dayNum);
	  index += 3; // advance to next day, or possibly "#"
	}
      }
      index = last; 
      if (tmpStr.mid(index,1) == "#") index++;
      if (tmpStr.find('T', index) != -1) {
	// repeat until a given date
	r.endDate = 
	  ISOToTm(tmpStr.mid(index, tmpStr.length() -
			     index).latin1());
	r.hasEndDate = true;
      } else
	// repeat a given number of times
	r.duration = tmpStr.mid(index, tmpStr.length() - index).toInt();
      break;

    case ::repeatMonthlyByDay:
      if (index == last) {
	QDate tmpDate(1900 + r.startDate.tm_year,
		      r.startDate.tm_mon + 1,
		      r.startDate.tm_mday);
	short tmpPos = tmpDate.day() / 7 + 1;
	if (tmpPos == 5) tmpPos = -1;
	r.repeatDay =
	  (DayOfMonthType) (7 * (tmpPos - 1) + 
			    tmpDate.dayOfWeek() - 1);
      } else {
	while (index < last) {
	  short tmpPos = tmpStr.mid(index, 1).toShort();
	  index++;
	  if (tmpStr.mid(index,1) == "-")
	    // convert tmpPos to negative
	    tmpPos = -tmpPos;
	  index += 2; // advance to day(s)
	  int dayNum = 0;
	  while (numFromDay(tmpStr.mid(index, 3)) >= 0) {
	    if (!dayNum) // pilot can only handle 1 day in month-by-pos
	      dayNum = numFromDay(tmpStr.mid(index, 3));
	    index += 3; // advance to next day, or possibly pos / "#"
	  }
	  r.repeatDay = (DayOfMonthType) (7 * (tmpPos - 1) + dayNum);
	}
      }
      index = last; 
      if (tmpStr.mid(index,1) == "#") index++;
      if (tmpStr.find('T', index) != -1) {
	r.endDate = ISOToTm(tmpStr.mid(index, tmpStr.length() -
				       index).latin1());
	r.hasEndDate = true;
      } else
	r.duration = tmpStr.mid(index, tmpStr.length() -
				index).toInt();
      break;

    case ::repeatMonthlyByDate:
    case ::repeatYearly:
      //if (index != last) // +++ ???
      //while (index < last)
      //  index = tmpStr.find(' ', index) + 1;
      index = last;
      if (tmpStr.mid(index,1) == "#") index++;
      if (tmpStr.find('T', index) != -1) {
	r.endDate = ISOToTm(tmpStr.mid(index, tmpStr.length() -
				       index).latin1());
	r.hasEndDate = true;
      } else
	r.duration = tmpStr.mid(index, tmpStr.length() -
				index).toInt();
      break;
      
    case ::repeatNone:
      break;

    default:
      kdError(CONDUIT_AREA) << __FUNCTION__
			    << ": unknown repetition type!"
			    << endl;
      break;
    }

  } else
    r.type = ::repeatNone;

  return r;
}


//////////////////////////////////////////////////////////////////////////


void VCalConduit::setRepetition(PilotDateEntry *dateEntry,
				const eventRepetition &er) {
  FUNCTIONSETUP;

	// Default to repeat daily, since there is
	// no "None" element of PeriodConstants.
	//
	PeriodConstants period = DailyPeriod;

  dateEntry->setRepeatType(er.type);

  if (er.type != repeatNone) {
    DEBUGCONDUIT << __FUNCTION__
		 << ": type " << er.type
		 << ", freq " << er.freq
		 << ", hasEndDate " << er.hasEndDate
		 << ", duration " << er.duration
		 << ", repeatDay" << er.repeatDay
		 << endl;

    switch (er.type) {
    case repeatDaily:
      period = DailyPeriod;
      break;
    case repeatWeekly:
      dateEntry->setRepeatDays(er.weekdays);
      period = WeeklyPeriod;
      break;
    case repeatMonthlyByDay:
      dateEntry->setRepeatDay(er.repeatDay);
      period = MonthlyByPosPeriod;
      break;
    case repeatMonthlyByDate:
      period = MonthlyByDayPeriod;
      break;
    case repeatYearly:
      period = YearlyByDayPeriod;
      break;
    default:
      kdError(CONDUIT_AREA) << __FUNCTION__
			    << ": unknown repetition type "
			    << er.type << endl;
      break;
    }

	if (er.hasEndDate) 
	{
		dateEntry->setRepeatFrequency(er.freq);
		dateEntry->setRepeatEnd(er.endDate);
	} 
	else if (er.duration == 0) 
	{
		dateEntry->setRepeatFrequency(er.freq);
		dateEntry->setRepeatForever();
	} 
	else 
	{
		repeatUntil(dateEntry, &er.startDate, er.freq, er.duration,
			period);
	}
  }
}


//////////////////////////////////////////////////////////////////////////


struct tm VCalConduit::getStartTime(VObject *vevent) {
  FUNCTIONSETUP;

  struct tm start;
  VObject *vo = isAPropertyOf(vevent, VCDTstartProp);
  if (vo) {
    char *s = fakeCString(vObjectUStringZValue(vo));
    start = ISOToTm(QString(s));
    deleteStr(s);
  } else
    memset(&start, 0, sizeof(start));
  return start;
} // struct tm VCalConduit::getStartTime(const VObject *vevent)


//////////////////////////////////////////////////////////////////////////


struct tm VCalConduit::getEndTime(VObject *vevent) {
  FUNCTIONSETUP;

  struct tm end;
  VObject *vo = isAPropertyOf(vevent, VCDTendProp);
  if (vo) {
    char *s = fakeCString(vObjectUStringZValue(vo));
    end = ISOToTm(QString(s));
    deleteStr(s);
  } else
    memset(&end, 0, sizeof(end));
  return end;
} // struct tm VCalConduit::getEndTime(const VObject *vevent)


//////////////////////////////////////////////////////////////////////////


void VCalConduit::setStartEndTimes(PilotDateEntry *dateEntry,
				   VObject *vevent) {
  FUNCTIONSETUP;

  int timeless_event = 0;
  struct tm start, end;
  
  if (getDateProperty(&start, vevent, VCDTstartProp)) {
    if (start.tm_hour == 0 &&
	start.tm_min == 0 &&
	start.tm_sec == 0)
      timeless_event = 1; // the event floats    
    dateEntry->setEventStart(start);
  }
	  
  dateEntry->setEvent(timeless_event);

  if (getDateProperty(&end, vevent, VCDTendProp)) 
    dateEntry->setEventEnd(end);
  else 
    // if the event has no DTend, get it from start time.
    dateEntry->setEventEnd(start);
} // void VCalConduit::setStartEndTimes(PilotDateEntry *dateEntry, const VObject *vevent)


//////////////////////////////////////////////////////////////////////////


void VCalConduit::setAlarms(PilotDateEntry *dateEntry, VObject
			    *vevent) const {
  FUNCTIONSETUP;

  // ALARM(s) //
  VObject *vo;
  if ((vo = isAPropertyOf(vevent, VCDAlarmProp))) {
    VObject *a = 0L, *b = 0L;
    if ((a = isAPropertyOf(vo, VCRunTimeProp)) &&
	(b = isAPropertyOf(vevent, VCDTstartProp))) {
      dateEntry->setAlarm(1);
      QDate tmpDate;
      QTime tmpTime;
      int year, month, day, hour, minute, second;
      
      QString tmpStr = fakeCString(vObjectUStringZValue(a));
      year = tmpStr.left(4).toInt();
      month = tmpStr.mid(4,2).toInt();
      day = tmpStr.mid(6,2).toInt();
      hour = tmpStr.mid(9,2).toInt();
      minute = tmpStr.mid(11,2).toInt();
      second = tmpStr.mid(13,2).toInt();
      tmpDate.setYMD(year, month, day);
      tmpTime.setHMS(hour, minute, second);
	      
      ASSERT(tmpDate.isValid());
      ASSERT(tmpTime.isValid());
      QDateTime tmpDT(tmpDate, tmpTime);
      // correct for GMT if string is in Zulu format
      if (tmpStr.right(1) == QString("Z"))
	tmpDT = tmpDT.addSecs(60 * fTimeZone);
      
      tmpStr = fakeCString(vObjectUStringZValue(b));
      year = tmpStr.left(4).toInt();
      month = tmpStr.mid(4,2).toInt();
      day = tmpStr.mid(6,2).toInt();
      hour = tmpStr.mid(9,2).toInt();
      minute = tmpStr.mid(11,2).toInt();
      second = tmpStr.mid(13,2).toInt();
      tmpDate.setYMD(year, month, day);
      tmpTime.setHMS(hour, minute, second);
      
      ASSERT(tmpDate.isValid());
      ASSERT(tmpTime.isValid());
      QDateTime tmpDT2(tmpDate, tmpTime);
      // correct for GMT if string is in Zulu format
      if (tmpStr.right(1) == QString("Z"))
	tmpDT2 = tmpDT2.addSecs(60*fTimeZone);
      
      int diffSecs = tmpDT.secsTo(tmpDT2);
      if (diffSecs > 60*60*24) {
	dateEntry->setAdvanceUnits(advDays);
	dateEntry->setAdvance((int) diffSecs/(60*60*24));
      } else if (diffSecs > 60*60) {
	dateEntry->setAdvanceUnits(advHours);
	dateEntry->setAdvance((int) diffSecs/(60*60));
      } else {
	dateEntry->setAdvanceUnits(advMinutes);
	dateEntry->setAdvance((int) diffSecs/60);
      }
    } else
      dateEntry->setAlarm(0);
  } else 
    dateEntry->setAlarm(0);
} // void VCalConduit::setAlarms(PilotDateEntry *dateEntry, const VObject *vevent) const


//////////////////////////////////////////////////////////////////////////

/* put up the about / setup dialog. */
QWidget* VCalConduit::aboutAndSetup()
{
  return new VCalSetup();
}


//////////////////////////////////////////////////////////////////////////


void mimeError(char *s)
{
	kdWarning(CONDUIT_AREA) << __FUNCTION__
		<< ": "
		<< s
		<< endl;
}


//////////////////////////////////////////////////////////////////////////

/* virtual */ void VCalConduit::doTest()
{
	FUNCTIONSETUP;

	registerMimeErrorHandler(mimeError);
	getCalendar(VCalSetup::VCalGroup);
	printVObject(stderr,calendar());
}


// $Log$
// Revision 1.41  2001/06/05 22:58:40  adridg
// General rewrite, cleanup thx. Philipp Hullmann
//
// Revision 1.40  2001/05/25 16:06:52  adridg
// DEBUG breakage
//
// Revision 1.39  2001/04/23 21:26:02  adridg
// Some testing and i18n() fixups, 8-bit char fixes
//
// Revision 1.38  2001/04/23 06:29:30  adridg
// Patches for bug #23385 and probably #23289
//
// Revision 1.37  2001/04/18 21:20:29  adridg
// Response to bug #24291
//
// Revision 1.36  2001/04/18 07:46:37  adridg
// Fix for part of bug #23385 by Philipp Hullman
//
// Revision 1.35  2001/04/16 13:54:17  adridg
// --enable-final file inclusion fixups
//
// Revision 1.34  2001/04/03 09:55:13  adridg
// Administrative, cleanup
//
// Revision 1.33  2001/04/01 17:32:05  adridg
// Fiddling around with date properties
//
// Revision 1.32  2001/03/27 11:10:39  leitner
// ported to Tru64 unix: changed all stream.h to iostream.h, needed some
// #ifdef DEBUG because qstringExpand etc. were not defined.
//
// Revision 1.31  2001/03/24 16:11:06  adridg
// Fixup some date-to-vcs functions
//
// Revision 1.30  2001/03/15 16:53:26  adridg
// i18n from KOrganizer -> Pilot
//
// Revision 1.29  2001/03/10 18:26:04  adridg
// Refactored vcal conduit and todo conduit
//
// Revision 1.28  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.27  2001/03/07 19:49:07  adridg
// Bugfix #20318 and #21816
//
// Revision 1.26  2001/03/05 23:57:53  adridg
// Added KPILOT_VERSION
//
// Revision 1.25  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.24.2.1  2001/03/07 17:38:37  adridg
// Bugfix for #21816 and #20318. No messages changed.
//
// Revision 1.24  2001/02/05 19:16:28  adridg
// Removing calls to exit() from internal functions
//
// Revision 1.23  2001/02/02 17:31:57  adridg
// Reduced debugging message overload
//
// Revision 1.22  2001/01/15 14:50:50  bero
// Fix build
//
// Revision 1.21  2001/01/08 22:27:12  adridg
// Fixed QFile::encodeName stupidity
//
// Revision 1.20  2001/01/05 12:06:01  adridg
// Updated version number, removed VCalConduit::version()
//
// Revision 1.19  2001/01/03 00:05:13  adridg
// Administrative
//
// Revision 1.18  2000/12/31 16:44:00  adridg
// Patched up the debugging stuff again
//
// Revision 1.17  2000/12/30 20:27:42  adridg
// Dag's Patches for Repeating Events
//
// Revision 1.16  2000/12/09 12:51:07  adridg
// New patches
//
// Revision 1.15  2000/12/05 07:47:27  adridg
// New debugging stuff
//
// Revision 1.14  2000/11/26 01:41:48  adridg
// Last of Heiko's patches
//
