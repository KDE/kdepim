/* vcal-conduit.cc		VCalendar Conduit
**
** Copyright (C) 1998-2000 by Dan Pilone, Preston Brown, and
**	Herwin Jan Steehouwer
** Copyright (C) 2001 by Cornelius Schumacher
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

#ifndef _KPILOT_OPTIONS_H
#include "options.h"
#endif

#include <sys/types.h>
#include <signal.h>
#include <iostream.h>
#include <stdlib.h>
#include <time.h>

#include <qbitarray.h>
#include <qdir.h>
#include <qdatetm.h>
#include <qstring.h>
#include <qmsgbox.h>

#include <kconfig.h>
#include <kdebug.h>

#include "kpilotConfig.h"
#include "pilotDatabase.h"
#include "pilotDateEntry.h"

#include "vcal-conduit.h"
#include "vcal-setup.h"
#include "conduitApp.h"

static const char *id=
	"$Id$";


int main(int argc, char* argv[])
{
  ConduitApp a(argc,argv,"vcal_conduit","Calendar / Organizer conduit",
               KPILOT_VERSION);
  a.addAuthor("Preston Brown",I18N_NOOP("Organizer author"));
  a.addAuthor("Adriaan de Groot",I18N_NOOP("Maintainer"),"adridg@cs.kun.nl");
  a.addAuthor("Philipp Hullmann",I18N_NOOP("Bugfixer"));
  a.addAuthor("Cornelius Schumacher",I18N_NOOP("iCalendar port"));

  VCalConduit conduit(a.getMode(),a.getDBSource());
  a.setConduit(&conduit);
  return a.exec(false,true);

  /* NOTREACHED */
  (void) id;
}


VCalConduit::VCalConduit(eConduitMode mode,DatabaseSource source) :
  VCalBaseConduit(mode,source)
{
}

VCalConduit::~VCalConduit()
{
}

void VCalConduit::doBackup()
{
  FUNCTIONSETUP;

  if (!getCalendar(VCalSetup::VCalGroup))
  {
    noCalendarError(i18n("VCal Conduit"));
    exit(ConduitMisconfigured);
  }
		
  DEBUGCONDUIT << __FUNCTION__
               << ": Performing full backup"
               << endl;

  // Get ALL entries from Pilot
  int index = 0;
  PilotRecord* rec;
  while ((rec=readRecordByIndex(index++))) {
    if (rec->isDeleted()) deleteRecord(rec);
    else updateEvent(rec);
  }

  saveVCal();
}


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
  while ((rec=readNextModifiedRecord())) {
    recordcount++;
    if (rec->isDeleted()) deleteRecord(rec);
    else {
      bool pilotRecModified = rec->getAttrib() & dlpRecAttrDirty;
      if (pilotRecModified) {
        updateEvent(rec);
      } else {
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
}

/*
 * Given a pilot record, check to see what needs to be done to the
 * analogous vobject to bring things into sync.
 */
void VCalConduit::updateEvent(PilotRecord *rec)
{
  FUNCTIONSETUP;

  PilotDateEntry dateEntry(rec);

  kdDebug() << "VCalConduit::updateEvent() " << dateEntry.getDescription()
            << endl;
  
  Event *vevent = findEvent(rec->getID());
  if (!vevent) {
    // no event was found, so we need to add one with some initial info
    DEBUGCONDUIT << __FUNCTION__ << ": creating new vCalendar event"
		 << endl;
    vevent = new Event;
    vevent->setOrganizer(calendar()->getEmail());
    
    calendar()->addEvent(vevent);

    vevent->setPilotId(dateEntry.getID());
    vevent->setSyncStatus(Incidence::SYNCNONE);
  }
  
  // we don't want to modify the vobject with pilot info, because it has
  // already been  modified on the desktop.  The VObject's modified state
  // overrides the PilotRec's modified state.

  if (vevent->syncStatus() != Incidence::SYNCNONE) return;

  // otherwise, the vObject hasn't been touched.  Updated it with the
  // info from the PilotRec.
  
  setVcalStartEndTimes(vevent, dateEntry);
  setVcalAlarms(vevent, dateEntry);
  setVcalRecurrence(vevent, dateEntry);
  setVcalExceptions(vevent, dateEntry);

  setSummary(vevent, dateEntry.getDescription());
  setNote(vevent, dateEntry.getNote());
  setSecret(vevent, (rec->getAttrib() & dlpRecAttrSecret));
  
  vevent->setSyncStatus(Incidence::SYNCNONE);  

  kdDebug() << "VCalConduit::updateEvent() done" << endl;
}


void VCalConduit::setVcalStartEndTimes(Event *vevent,
				       const PilotDateEntry &dateEntry)
{
  // START TIME //
  vevent->setDtStart(readTm(dateEntry.getEventStart()));
  
  // check whether the event is an event or an appointment.  See dateEntry
  // structure for more info.
  if (dateEntry.getEvent()) {
    vevent->setFloats(true);
  } else {
    vevent->setFloats(false);
  }

  // END TIME //  
  // handle the case of a "repeating event on a daily basis" which is the
  // pilot's way of indicating a multi-day event.
  if (dateEntry.isMultiDay()) {
    vevent->setDtEnd(readTm(dateEntry.getRepeatEnd()));
  } else {
    vevent->setDtEnd(readTm(dateEntry.getEventEnd()));
  }
}


void VCalConduit::setVcalAlarms(Incidence *vevent, 
				const PilotDateEntry &dateEntry)
{
  FUNCTIONSETUP;

  if (!dateEntry.getAlarm()) return;
  
  QDateTime alarmDT = readTm(dateEntry.getEventStart());
  
  int advanceUnits = dateEntry.getAdvanceUnits();
  switch (advanceUnits) {
    case advMinutes:
      advanceUnits = 1;
      break;
    case advHours:
      advanceUnits = 60;
      break;
    case advDays:
      advanceUnits = 60*24;
      break;
    default:
      DEBUGCONDUIT << __FUNCTION__ 
                   << ": Unknown advance units "
                   << advanceUnits
                   << endl;
      advanceUnits=1;
  }

  alarmDT = alarmDT.addSecs(60*advanceUnits*-(dateEntry.getAdvance()));

  vevent->alarm()->setTime(alarmDT);
  vevent->alarm()->setRepeatCount(1);  // Enable alarm
}


void VCalConduit::setVcalRecurrence(Incidence *vevent, 
				    const PilotDateEntry &dateEntry)
{
  FUNCTIONSETUP;

  // Pilot entries that repeat daily are not what we consider daily
  // repeating events in vCalendar/KOrganizer.  It is actually a multi-day
  // appointment and handled by setVcalStartEndTimes().
  if ((dateEntry.getRepeatType() == repeatNone) ||
      ((dateEntry.getRepeatType() == repeatDaily) && dateEntry.getEvent())) {
    return;
  }

  KORecurrence *recur = vevent->recurrence();
  int freq = dateEntry.getRepeatFrequency();
  bool repeatsForever = dateEntry.getRepeatForever();
  QDate endDate;
  if (!repeatsForever) {
    endDate = readTm(dateEntry.getRepeatEnd()).date();
    kdDebug() << "-- end " << endDate.toString() << endl;
  } else {
    kdDebug() << "-- noend" << endl;
  }
  QBitArray dayArray(7);

  switch(dateEntry.getRepeatType()) {
    case repeatDaily:
      if (repeatsForever) recur->setDaily(freq,0);
      else recur->setDaily(freq,endDate);
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

        if (days[0]) dayArray.setBit(6);
	for (int i = 1; i < 7; i++) {
          if (days[i]) dayArray.setBit(i-1);
        }

        if (repeatsForever) recur->setWeekly(freq,dayArray,0);
        else recur->setWeekly(freq,dayArray,endDate);
      }
      break;
    case repeatMonthlyByDay:
      if (repeatsForever) recur->setMonthly(KORecurrence::rMonthlyPos,freq,0);
      else recur->setMonthly(KORecurrence::rMonthlyPos,freq,endDate);

      dayArray.setBit(dateEntry.getRepeatDay() % 7);
      recur->addMonthlyPos((dateEntry.getRepeatDay() / 7) + 1,dayArray);
#if 0
      tmpStr.sprintf("MP%i %d+ ", dateEntry.getRepeatFrequency(),
		     (dateEntry.getRepeatDay() / 7) + 1);
      tmpStr.append(dayname[dateEntry.getRepeatDay() % 7]);
#endif
      break;
    case repeatMonthlyByDate:
      if (repeatsForever) recur->setMonthly(KORecurrence::rMonthlyDay,freq,0);
      else recur->setMonthly(KORecurrence::rMonthlyDay,freq,endDate);
#if 0      
      tmpStr.sprintf("MD%i ", dateEntry.getRepeatFrequency());
#endif
      break;
    case repeatYearly:
      if (repeatsForever) recur->setYearly(KORecurrence::rYearlyDay,freq,0);
      else recur->setYearly(KORecurrence::rYearlyDay,freq,endDate);
#if 0
      tmpStr.sprintf("YD%i ", dateEntry.getRepeatFrequency());
#endif
      break;
    case repeatNone:
      DEBUGCONDUIT << __FUNCTION__
		   << ": argh! we think it repeats, "
		   << "but dateEntry has repeatNone!\n";
      break;
    default:
      break;
  }
}


void VCalConduit::setVcalExceptions(Incidence *vevent, 
				    const PilotDateEntry &dateEntry)
{
  FUNCTIONSETUP;

  if (((dateEntry.getRepeatType() == repeatDaily) &&
       dateEntry.getEvent()) && dateEntry.getExceptionCount()) {
    DEBUGCONDUIT << __FUNCTION__
		 << ": WARNING Exceptions ignored for multi-day event "
		 << dateEntry.getDescription()
		 << endl ;
    return;
  }

  for (int i = 0; i < dateEntry.getExceptionCount(); i++) {
    vevent->addExDate(readTm(dateEntry.getExceptions()[i]).date());
  }
}


void VCalConduit::doLocalSync()
{
  FUNCTIONSETUP;

  DEBUGCONDUIT << __FUNCTION__ 
	       << ": Performing local sync."
	       << endl;

  int recordcount = 0;

  QList<Event> events = calendar()->getAllEvents();

  // go through the whole vCalendar.  If the event has the dirty
  // (modified) flag set, make a new pilot record and add it.  we only
  // take events that have KPilotStatusProp as a property.  If this
  // property isn't present, ignore the event.

  for(Event *event = events.first();event;event = events.next()) {
    recordcount++;

    if (event->syncStatus() == Incidence::SYNCMOD) {
      // the event has been modified, need to write it to the pilot

      // we read the pilotID.
	
      int id = event->pilotId();
	
      // if id != 0, this is a modified event, otherwise it is new.

      PilotDateEntry *dateEntry = 0;
	
      if (id != 0) {
        PilotRecord *pRec = readRecordById(id);
        // if this fails, somehow the record got deleted from the pilot
        // but we were never informed! bad pilot. naughty pilot.

        if (pRec) {
          // If the record was deleted on the pilot, recreate it.
          dateEntry = new PilotDateEntry(pRec);
        } else {
          dateEntry = new PilotDateEntry();
          id = 0;
        }
      } else
        dateEntry = new PilotDateEntry();

      setStartEndTimes(dateEntry,event);
      setAlarms(dateEntry,event);

      // RECURRENCE(S) //

      // first we have a 'fake type of recurrence' when a multi-day
      // even it passed to the pilot, it is converted to an event
      // which recurs daily a number of times.
      if (event->isMultiDay() && event->doesFloat()) {
        // multi day event
        DEBUGCONDUIT << __FUNCTION__
                     << ": multi-day event from "
                     << (event->dtStart().toString()) << " to " 
                     << (event->dtEnd().toString()) << endl;
        dateEntry->setRepeatType(repeatDaily);
        dateEntry->setRepeatFrequency(1);
        struct tm end = writeTm(event->dtEnd());
        dateEntry->setRepeatEnd(end);

        if (event->exDates().count() > 0) {
          DEBUGCONDUIT << __FUNCTION__
                       << ": WARNING: exceptions ignored "
                       << "for multi-day event "
                       << event->summary()
                       << endl ;
        }
      }

      // and now the real recurring events
      setRepetition(dateEntry,event);
	
      // EXCEPTION(S) //
      int count;
      struct tm *exceptionList = getExceptionDates(event,&count);
      if (exceptionList) {
        dateEntry->setExceptionCount(count);
        dateEntry->setExceptions(exceptionList);
      } else {
        dateEntry->setExceptionCount(0);
      }

      // SUMMARY //
      dateEntry->setDescription(event->summary());

      // DESCRIPTION //
      dateEntry->setNote(event->description());

      // put the pilotRec in the database...
      PilotRecord *pRec = dateEntry->pack();
      pRec->setAttrib(dateEntry->getAttrib() & ~dlpRecAttrDirty);
      id = writeRecord(pRec);
      ::free(pRec);

      delete dateEntry;

      if (id > 0) {
        // Writing succeeded. Write the id we got from writeRecord
        // back to the vObject.
        event->setPilotId(id);
      }
      // Clear the 'modified' flag.
      event->setSyncStatus(Incidence::SYNCNONE);
    }
  }

  DEBUGCONDUIT << __FUNCTION__ << ": Read " << recordcount
               << " records total." << endl;
  
  KConfig& config = KPilotConfig::getConfig(VCalSetup::VCalGroup);
  bool deleteOnPilot = config.readBoolEntry("DeleteOnPilot", true);

  if (firstTime()) firstSyncCopy(deleteOnPilot);

  if (deleteOnPilot) deleteFromPilot(VCalBaseConduit::TypeEvent);

  setFirstTime(config,false);
  config->sync();
}


struct tm *VCalConduit::getExceptionDates(Event *vevent, int *n)
{
  FUNCTIONSETUP;

  struct tm *tmList = 0;
  int count = 0;

  QDateList dates = vevent->exDates();
  QDate *date = dates.first();
  while(date) {
    struct tm extm = writeTm(*date);
    ++count;
    tmList = (struct tm *) realloc(tmList, sizeof(struct tm)*count);
    if (!tmList)
      kdFatal(CONDUIT_AREA) << __FUNCTION__
			    << ": realloc() failed!" << endl;
    tmList[count-1] = extm;
  
    date = dates.next();
  }

  if (n) *n = count;
  return tmList;
}


void VCalConduit::firstSyncCopy(bool DeleteOnPilot)
{
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

    if (findEvent(rec->getID())) {
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
            updateEvent(rec);
            break;
          case 1:
            // Skip this item, deletion is handled by deleteFromPilot().
            break;
          case 2:
            insertall = true;
            skipall = false;
            updateEvent(rec);
            break;
        }
      } else if (insertall) {
        // all records are to be inserted.
        updateEvent(rec);
      }
    }
    delete rec;
  }
}


void VCalConduit::setRepetition(PilotDateEntry *dateEntry,Incidence *incidence)
{
  FUNCTIONSETUP;

  KORecurrence *recur = incidence->recurrence();

  // Default to repeat daily, since there is no "None" element of
  // PeriodConstants.
  PeriodConstants period = DailyPeriod;

  switch (recur->doesRecur()) {
    case KORecurrence::rNone:
      dateEntry->setRepeatType(repeatNone);
      break;
    case KORecurrence::rDaily:
      dateEntry->setRepeatType(repeatDaily);
      period = DailyPeriod;
      break;
    case KORecurrence::rWeekly:
      dateEntry->setRepeatType(repeatWeekly);
      period = WeeklyPeriod;
      dateEntry->setRepeatDays(recur->days());
      break;
    case KORecurrence::rMonthlyPos:
      dateEntry->setRepeatType(repeatMonthlyByDay);
      period = MonthlyByPosPeriod;
      {
        QList<KORecurrence::rMonthPos> rl = recur->monthPositions();
        KORecurrence::rMonthPos *r = rl.first();
        if (!r) {
          kdDebug() << "Recurrence monthlyPos, but no rMonthPos" << endl;
          dateEntry->setRepeatType(repeatNone);
        } else {
          int pos = (r->rPos - 1) * 7;
          for(int i=0;i<7;++i) {
            if (r->rDays.testBit(i)) {
              pos += i;
              break;
            }
          }
          dateEntry->setRepeatDay((DayOfMonthType) pos);
        }
      }
      break;
    case KORecurrence::rMonthlyDay:
      dateEntry->setRepeatType(repeatMonthlyByDate);
      period = MonthlyByDayPeriod;
      break;
    case KORecurrence::rYearlyDay:
      dateEntry->setRepeatType(repeatYearly);
      period = YearlyByDayPeriod;
      break;
    default:
      kdDebug() << "This recurrence type is not supported." << endl;
      break;
  }

  dateEntry->setRepeatFrequency(recur->frequency());

  if (recur->duration() == 0)  {
    struct tm end = writeTm(recur->endDate());
    dateEntry->setRepeatEnd(end);
  } else if (recur->duration() < 0) {
    dateEntry->setRepeatForever();
  } else {
    dateEntry->setRepeatEnd(repeatUntil(incidence->dtStart(),recur->frequency(),
                                        recur->duration(),period));
  }
}

struct tm VCalConduit::repeatUntil(const QDateTime &startDt,int rFreq,
                                   int rDuration,PeriodConstants period)
{
  FUNCTIONSETUP;

  struct tm start = writeTm(startDt);
  time_t end_time = mktime(&start);
  struct tm rEnd = start;

  switch(period) {
    case DailyPeriod:
    case WeeklyPeriod:
      // Calculate the end time by adding the right number of
      // repeat periods.
      end_time += rFreq * (rDuration-1) * (int) period;
      return *localtime(&end_time);
    case MonthlyByDayPeriod:
    case MonthlyByPosPeriod:
      rEnd.tm_mon += rFreq * (rDuration - 1);
      rEnd.tm_year += rEnd.tm_mon / 12;
      rEnd.tm_mon %= 12;
      return rEnd;
    case YearlyByDayPeriod:
      rEnd.tm_year += rFreq * (rDuration - 1);
      return rEnd;
    default:
      kdWarning(CONDUIT_AREA) << __FUNCTION__
                              << ": Unknown repeat period "
                              << (int) period
                              << endl;
      return rEnd;
  }
}


void VCalConduit::setStartEndTimes(PilotDateEntry *dateEntry,Event *vevent)
{
  dateEntry->setEvent(vevent->doesFloat());

  struct tm start, end;

  start = writeTm(vevent->dtStart());
  dateEntry->setEventStart(start);
  
  end = writeTm(vevent->dtEnd());
  dateEntry->setEventEnd(end);
}

void VCalConduit::setAlarms(PilotDateEntry *dateEntry,Event *vevent)
{
  if (vevent->alarm()->repeatCount() == 0) {
    dateEntry->setAlarm(0);
  } else {
    dateEntry->setAlarm(1);

    QDateTime startDt = vevent->dtStart();
    QDateTime alarmDt = vevent->alarm()->time();
      
    int diffSecs = startDt.secsTo(alarmDt);
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
  }
}


/* put up the about / setup dialog. */
QWidget* VCalConduit::aboutAndSetup()
{
  return new VCalSetup();
}


void VCalConduit::doTest()
{
  // TODO: dump calendar
}


// $Log$
// Revision 1.43  2001/06/18 19:51:40  cschumac
// Fixed todo and datebook conduits to cope with KOrganizers iCalendar format.
// They use libkcal now.
//
// Revision 1.42  2001/06/13 21:30:24  adridg
// Avoid uninitialized variable warning
//
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
