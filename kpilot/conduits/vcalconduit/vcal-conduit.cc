/* vcal-conduit.cc                      KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the vcal-conduit plugin.
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
** Bug reports and questions can be sent to kde-pim@kde.org
*/

static const char *vcalconduit_id = "$Id$";

#include <options.h>

#include <qdatetime.h>
#include <qtimer.h>

#include <kconfig.h>

#include <calendar.h>
#include <calendarlocal.h>
#include <event.h>


/*
** KDE 2.2 uses class KORecurrence in a different header file.
*/
#ifdef KDE2
#include <korecurrence.h>
#define Recurrence_t KCal::KORecurrence
#define DateList_t QDateList
#define DateListIterator_t QDateListIterator
#else
#include <recurrence.h>
#define Recurrence_t KCal::Recurrence
#define DateList_t KCal::DateList
#define DateListIterator_t KCal::DateList::ConstIterator
#endif

#include <pilotSerialDatabase.h>
#include <pilotLocalDatabase.h>
#include <pilotDateEntry.h>

#include "vcal-factory.h"
#include "vcal-conduit.moc"


QDateTime readTm(const struct tm &t)
{
  QDateTime dt;
  dt.setDate(QDate(1900 + t.tm_year, t.tm_mon + 1, t.tm_mday));
  dt.setTime(QTime(t.tm_hour, t.tm_min, t.tm_sec));
  return dt;
}

struct tm writeTm(const QDateTime &dt)
{
  struct tm t;

  t.tm_wday = 0; // unimplemented
  t.tm_yday = 0; // unimplemented
  t.tm_isdst = 0; // unimplemented

  t.tm_year = dt.date().year() - 1900;
  t.tm_mon = dt.date().month() - 1;
  t.tm_mday = dt.date().day();
  t.tm_hour = dt.time().hour();
  t.tm_min = dt.time().minute();
  t.tm_sec = dt.time().second();

  return t;
}

class VCalConduit::VCalPrivate
{
public:
	VCalPrivate(KCal::Calendar *buddy);

#ifdef KDE2
	QList<KCal::Event> fAllEvents;
#else
	QPtrList<KCal::Event> fAllEvents;
#endif

	int updateEvents();
	void removeEvent(KCal::Event *);
	KCal::Event *findEvent(recordid_t);
	KCal::Event *getNextEvent();
	KCal::Event *getNextModifiedEvent();

protected:
	bool reading;

private:
	KCal::Calendar *fCalendar;
} ;

VCalConduit::VCalPrivate::VCalPrivate(KCal::Calendar *b) :
	fCalendar(b)
{
	fAllEvents.setAutoDelete(false);
	reading=false;
}

int VCalConduit::VCalPrivate::updateEvents()
{
	fAllEvents = fCalendar->getAllEvents();
	fAllEvents.setAutoDelete(false);
	return fAllEvents.count();
}

void VCalConduit::VCalPrivate::removeEvent(KCal::Event *e)
{
	fAllEvents.remove(e);
	fCalendar->deleteEvent(e);
}

KCal::Event *VCalConduit::VCalPrivate::findEvent(recordid_t id)
{
	KCal::Event *event = fAllEvents.first();
	while(event)
	{
		if (event->pilotId() == id) return event;
		event = fAllEvents.next();
	}

	return 0L;
}

KCal::Event *VCalConduit::VCalPrivate::getNextEvent()
{
	if (reading) return fAllEvents.next();
	reading=true;
	return fAllEvents.first();
}

KCal::Event *VCalConduit::VCalPrivate::getNextModifiedEvent()
{
	KCal::Event*e=0L;
	if (!reading) 
	{
		reading=true;
		e=fAllEvents.first();
	}
	else
	{
		e=fAllEvents.next();
	}
	while (e && e->syncStatus()==KCal::Incidence::SYNCNONE) 
	{
		e=fAllEvents.next();
	}
	return e;
}



VCalConduit::VCalConduit(KPilotDeviceLink *d,
	const char *n,
	const QStringList &a) :
	ConduitAction(d,n,a),
	fCalendar(0L),
	fCurrentDatabase(0L),
	fBackupDatabase(0L),
	fP(0L)
{
	FUNCTIONSETUP;
	(void) vcalconduit_id;
}

VCalConduit::~VCalConduit()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fP);
	KPILOT_DELETE(fCurrentDatabase);
	KPILOT_DELETE(fBackupDatabase);
	KPILOT_DELETE(fCalendar);
}

/* virtual */ void VCalConduit::exec()
{
	FUNCTIONSETUP;

	bool loadSuccesful = true;

	if (!fConfig)
	{
		kdWarning() << k_funcinfo
			<< ": No configuration set for vcal-conduit"
			<< endl;
		goto error;
	}

	if (PluginUtility::isRunning("korganizer") ||
		PluginUtility::isRunning("alarmd"))
	{
		addSyncLogEntry(i18n("KOrganizer is running, can't update datebook."));
		goto error;
	}

	fConfig->setGroup(VCalConduitFactory::group);

	fCalendarFile = fConfig->readEntry(VCalConduitFactory::calendarFile);
	fDeleteOnPilot = fConfig->readBoolEntry(VCalConduitFactory::deleteOnPilot,false);
	fFirstTime = fConfig->readBoolEntry(VCalConduitFactory::firstTime,true);

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": Using calendar file "
		<< fCalendarFile
		<< endl;
#endif

	fCurrentDatabase = new PilotSerialDatabase(pilotSocket(),
		"DatebookDB",
		this,
		"DatebookDB");
	fBackupDatabase = new PilotLocalDatabase("DatebookDB");
	fCalendar = new KCal::CalendarLocal();

	// Handle lots of error cases.
	//
	if (!fCurrentDatabase || !fBackupDatabase || !fCalendar) goto error;
	if (!fCurrentDatabase->isDBOpen() ||
		!fBackupDatabase->isDBOpen()) goto error;
	loadSuccesful = fCalendar->load(fCalendarFile);
	if (!loadSuccesful) goto error;

	fP = new VCalPrivate(fCalendar);
	fP->updateEvents();

	pilotindex=0;
	QTimer::singleShot(0,this,SLOT(syncRecord()));
	return;

error:
	if (!fCurrentDatabase)
	{
		kdWarning() << k_funcinfo
			<< ": Couldn't open database on Pilot"
			<< endl;
	}
	if (!fBackupDatabase)
	{
		kdWarning() << k_funcinfo
			<< ": Couldn't open local copy"
			<< endl;
	}
	if (!loadSuccesful)
	{
		kdWarning() << k_funcinfo
			<< ": Couldn't load <"
			<< fCalendarFile
			<< ">"
			<< endl;
	}

	emit logError(i18n("Couldn't open the calendar databases."));
	KPILOT_DELETE(fCurrentDatabase);
	KPILOT_DELETE(fBackupDatabase);
	KPILOT_DELETE(fCalendar);
	emit syncDone(this);
}

void VCalConduit::syncRecord()
{
	FUNCTIONSETUP;

	PilotRecord *r;
	if (fFirstTime) 
	{
		r = fCurrentDatabase->readRecordByIndex(pilotindex++);
	}
	else
	{
		r = fCurrentDatabase->readNextModifiedRec();
	}
	PilotRecord *s = 0L;

	if (!r)
	{
		fP->updateEvents();
		QTimer::singleShot(0 ,this,SLOT(syncEvent()));
		return;
	}

	s = fBackupDatabase->readRecordById(r->getID());
	if (!s)
	{
		addRecord(r);
	}
	else
	{
		if (r->isDeleted())
		{
			deleteRecord(r,s);
		}
		else
		{
			changeRecord(r,s);
		}
	}

	KPILOT_DELETE(r);
	KPILOT_DELETE(s);

	QTimer::singleShot(0,this,SLOT(syncRecord()));
}

void VCalConduit::syncEvent()
{
// TODO: skip PC => Palm sync for now because it corrupts the palm data...
//QTimer::singleShot(0,this,SLOT(deleteRecord()));
//return;


	FUNCTIONSETUP;
	KCal::Event*e=0L;
	if (fFirstTime)
	{
		e=fP->getNextEvent();
	}
	else
	{
		e=fP->getNextModifiedEvent();
	}
	if (!e)
	{
		QTimer::singleShot(0,this,SLOT(deleteRecord()));
		return;
	}
	// find the corresponding index on the palm and sync. If there is none, create it.
	int ix=e->pilotId();
#ifdef DEBUG
		DEBUGCONDUIT<<fname<<": found PC entry with pilotID "<<ix<<endl;
#endif
	PilotRecord *s=0L;
	PilotDateEntry*de;
	if (ix>0 && (s=fCurrentDatabase->readRecordById(ix)))
	{
		if (e->syncStatus()==KCal::Incidence::SYNCDEL)
		{
			deletePalmRecord(e, s);
		}
		else
		{
			de=new PilotDateEntry(s);
			updateEventOnPalm(e, de);
			delete de;
		}
	} else {
		de=new PilotDateEntry();
		updateEventOnPalm(e, de);
		delete de;
	}
	KPILOT_DELETE(s);
	QTimer::singleShot(0, this, SLOT(syncEvent()));
}

void VCalConduit::deleteRecord()
{
// TODO: use the backup db to findout which records have been deleted locally on the PC
	QTimer::singleShot(0, this, SLOT(cleanup()));
}

void VCalConduit::cleanup()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fCurrentDatabase);
	KPILOT_DELETE(fBackupDatabase);

	fCalendar->save(fCalendarFile);
	KPILOT_DELETE(fCalendar);

	emit syncDone(this);
}


void VCalConduit::addRecord(PilotRecord *r)
{
	FUNCTIONSETUP;

	fBackupDatabase->writeRecord(r);

	PilotDateEntry de(r);
	KCal::Event *e = new KCal::Event;
	eventFromRecord(e,de);
	// TODO: find out if there is already an entry with this data...
	fCalendar->addEvent(e);
}

void VCalConduit::deleteRecord(PilotRecord *r, PilotRecord *s)
{
	FUNCTIONSETUP;

	KCal::Event *e = fP->findEvent(r->getID());
	if (e)
	{
		// RemoveEvent also takes it out of the calendar.
		fP->removeEvent(e);
	}
	fBackupDatabase->writeRecord(r);
}

void VCalConduit::changeRecord(PilotRecord *r,PilotRecord *s)
{
	FUNCTIONSETUP;

	PilotDateEntry de(r);
	KCal::Event *e = fP->findEvent(r->getID());
	if (e)
	{
		eventFromRecord(e,de);
		fBackupDatabase->writeRecord(r);
	}
	else
	{
		kdWarning() << k_funcinfo
			<< ": While changing record -- not found in iCalendar"
			<< endl;

		addRecord(r);
	}
}

void VCalConduit::deletePalmRecord(KCal::Event*e, PilotRecord*s)
{
	FUNCTIONSETUP;
	if (s)
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": deleting record " << s->getID() << endl;
#endif
		s->setAttrib(~dlpRecAttrDeleted);
		fCurrentDatabase->writeRecord(s);
		fBackupDatabase->writeRecord(s);
	}
	else
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": could not find record to delete (" << e->pilotId() << ")" << endl;
#endif
	}
}

/* I have to use a pointer to an existing PilotDateEntry so that I can handle
   new records as well (and to prevend some crashes concerning the validity
   domain of the PilotRecord*r). In syncEvent this PilotDateEntry is created. */
void VCalConduit::updateEventOnPalm(KCal::Event*e, PilotDateEntry*de)
{
	FUNCTIONSETUP;
	if (!de) {
#ifdef DEBUG
		DEBUGCONDUIT<<fname<<": NULL event given... Skipping it"<<endl;
#endif
		return;
	}
	PilotRecord*r=entryFromEvent(de, e);

	if (r)
	{
		fBackupDatabase->writeRecord(r);
		fCurrentDatabase->writeRecord(r);
		e->setSyncStatus(KCal::Incidence::SYNCNONE);
		e->setPilotId(r->getID());
	}
}

PilotRecord*VCalConduit::entryFromEvent(PilotDateEntry*de, const KCal::Event*e)
{
	FUNCTIONSETUP;
	if (!de) {
#ifdef DEBUG
		DEBUGCONDUIT<<fname<<": NULL event given... Skipping it"<<endl;
#endif
		return NULL;
	}

	// set secrecy, start/end times, alarms, recurrence, exceptions, summary and description:
	if (e->secrecy()!=KCal::Event::SecrecyPublic) de->makeSecret();

	setStartEndTimes(de, e);
	setAlarms(de, e);
	setRecurrence(de, e);
	setExceptions(de, e);
	de->setDescription(e->summary());
	de->setNote(e->description());
	return de->pack();
}

KCal::Event *VCalConduit::eventFromRecord(KCal::Event *e, const PilotDateEntry &de)
{
	FUNCTIONSETUP;
	if (!e) {
#ifdef DEBUG
		DEBUGCONDUIT<<fname<<": NULL event given... Skipping it"<<endl;
#endif
		return NULL;
	}

	e->setOrganizer(fCalendar->getEmail());
	e->setSyncStatus(KCal::Incidence::SYNCNONE);
	e->setSecrecy(de.isSecret() ?
		KCal::Event::SecrecyPrivate :
		KCal::Event::SecrecyPublic);

	e->setPilotId(de.getID());
	e->setSyncStatus(KCal::Incidence::SYNCNONE);

	setStartEndTimes(e,de);
	setAlarms(e,de);
	setRecurrence(e,de);
	setExceptions(e,de);

	e->setSummary(de.getDescription());
	e->setDescription(de.getNote());

	return e;
}

void VCalConduit::setStartEndTimes(KCal::Event *e,const PilotDateEntry &de)
{
	FUNCTIONSETUP;
	e->setDtStart(readTm(de.getEventStart()));
	e->setFloats(de.isEvent());

	if (de.isMultiDay())
	{
		e->setDtEnd(readTm(de.getRepeatEnd()));
	}
	else
	{
		e->setDtEnd(readTm(de.getEventEnd()));
	}
}

void VCalConduit::setStartEndTimes(PilotDateEntry*de, const KCal::Event *e)
{
	FUNCTIONSETUP;
	struct tm ttm=writeTm(e->dtStart());
	de->setEventStart(ttm);
	de->setEvent(e->doesFloat());

	if (e->hasEndDate()) 
	{
		ttm=writeTm(e->dtEnd());
	}
	else
	{
		ttm=writeTm(e->dtStart());
	}
	de->setEventEnd(ttm);
}

void VCalConduit::setAlarms(KCal::Event *e, const PilotDateEntry &de)
{
	FUNCTIONSETUP;

	if (!de.getAlarm() && !e) return;

	QDateTime alarmDT = readTm(de.getEventStart());
	int advanceUnits = de.getAdvanceUnits();

	switch (advanceUnits)
	{
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
#ifdef DEBUG
		DEBUGCONDUIT << fname
			<< ": Unknown advance units "
			<< advanceUnits
			<< endl;
#endif
		advanceUnits=1;
	}

	KCal::Duration adv(60*advanceUnits*de.getAdvance());
	KCal::Alarm*alm=new KCal::Alarm(e);
	if (!alm) return;

	alm->setTime(e->dtStart());
	alm->setOffset(adv);
	e->addAlarm(alm);
	// TODO: Fix alarms
}

void VCalConduit::setAlarms(PilotDateEntry*de, const KCal::Event *e)
{
	FUNCTIONSETUP;
	
	if (!de || !e ) 
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": NULL entry given to setAlarms. "<<endl;
#endif
		return;
	}
	if (e->alarms().count()<=0)
	{
		de->setAlarm(0);
		return;
	}

	const KCal::Alarm *alm=e->alarms().first();
	if (!alm || !alm->enabled())
	{
		de->setAlarm(0);
		return;
	}
	
	int offs=alm->offset().asSeconds()/60;
	
	// find the best Advance Unit
	if (offs>=120 || offs==60) 
	{
		offs/=60;
		if (offs>=48 || offs==24) 
		{
			offs/=24;
			de->setAdvanceUnits(advDays);
		}
		else
		{
			de->setAdvanceUnits(advHours);
		}
	}
	else
	{
		de->setAdvanceUnits(advMinutes);
	}
	de->setAdvance(offs);
}

void VCalConduit::setRecurrence(KCal::Event *event,const PilotDateEntry &dateEntry)
{
	FUNCTIONSETUP;

	if ((dateEntry.getRepeatType() == repeatNone) ||
		(dateEntry.getRepeatType() == repeatDaily && dateEntry.isEvent()))
	{
		return;
	}

	Recurrence_t *recur = event->recurrence();
	int freq = dateEntry.getRepeatFrequency();
	bool repeatsForever = dateEntry.getRepeatForever();
	QDate endDate;

	if (!repeatsForever)
	{
		endDate = readTm(dateEntry.getRepeatEnd()).date();
#ifdef DEBUG
		DEBUGCONDUIT << fname << "-- end " << endDate.toString() << endl;
#endif
	}
	else
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << "-- noend" << endl;
#endif
	}

	QBitArray dayArray(7);

	switch(dateEntry.getRepeatType())
	{
	case repeatDaily:
		if (repeatsForever) recur->setDaily(freq,0);
		else recur->setDaily(freq,endDate);
		break;
	case repeatWeekly:
		{
		const int *days = dateEntry.getRepeatDays();

#ifdef DEBUG
		DEBUGCONDUIT << fname
			<< ": Got repeat-weekly entry, by-days="
			<< days[0] << " "<< days[1] << " "<< days[2] << " "
			<< days[3]
			<< days[4] << " "<< days[5] << " "<< days[6] << " "
			<< endl;
#endif

		// Rotate the days of the week, since day numbers on the Pilot and
		// in vCal / Events are different.
		//
		if (days[0]) dayArray.setBit(6);
		for (int i = 1; i < 7; i++)
		{
			if (days[i]) dayArray.setBit(i-1);
		}

		if (repeatsForever) recur->setWeekly(freq,dayArray,0);
		else recur->setWeekly(freq,dayArray,endDate);
		}
		break;
	case repeatMonthlyByDay:
		if (repeatsForever)
		{
			recur->setMonthly(Recurrence_t::rMonthlyPos,freq,0);
		}
		else
		{
			recur->setMonthly(Recurrence_t::rMonthlyPos,freq,endDate);
		}

		dayArray.setBit(dateEntry.getRepeatDay() % 7);
		recur->addMonthlyPos((dateEntry.getRepeatDay() / 7) + 1,dayArray);
		break;
	case repeatMonthlyByDate:
		if (repeatsForever)
		{
			recur->setMonthly(Recurrence_t::rMonthlyDay,freq,0);
		}
		else
		{
			recur->setMonthly(Recurrence_t::rMonthlyDay,freq,endDate);
		}
		break;
	case repeatYearly:
		if (repeatsForever)
		{
			recur->setYearly(Recurrence_t::rYearlyDay,freq,0);
		}
		else
		{
			recur->setYearly(Recurrence_t::rYearlyDay,freq,endDate);                }
		break;
	case repeatNone:
	default :
#ifdef DEBUG
		DEBUGCONDUIT << fname
			<< ": Can't handle repeat type "
			<< dateEntry.getRepeatType()
			<< endl;
#endif
		break;
	}
}

void VCalConduit::setRecurrence(PilotDateEntry*dateEntry, const KCal::Event *event)
{
	FUNCTIONSETUP;

	KCal::Recurrence*r=event->recurrence();
	if (!r) return;

	ushort recType=r->doesRecur();
	int freq=r->frequency();
	QDate endDate=r->endDate();
	QDateTime startDt(readTm(dateEntry->getEventStart())), endDt(readTm(dateEntry->getEventEnd()));

	// whether we have to recalc the end date depending on the duration and the recurrence type
	bool needCalc=( (!endDate.isValid()) && (r->duration()>0));
	// TODO: What if duration>0 and the date is valid???
	if (r->duration()==0)
	{
		if (!endDate.isValid())
		{
			dateEntry->setRepeatForever();
		}
		else
		{
			dateEntry->setRepeatEnd(writeTm(endDate));
		}
	}
	dateEntry->setRepeatFrequency(freq);
#ifdef DEBUG
	DEBUGCONDUIT<<" Event: "<<event->summary()<<" ("<<event->description()<<")"<<endl;
	DEBUGCONDUIT<< "duration: "<<r->duration() << ", endDate: "<<endDate.toString()<< ", ValidEndDate: "<<endDate.isValid()<<", NullEndDate: "<<endDate.isNull()<<endl;
#endif

	//  first we have 'fake type of recurrence' when a multi-day
	// event is passed to the pilot, it is converted to an event 
	// which recurs daily a number of times.
	// if the event itself recurs, this will be overridden, and 
	// only the first day will be included in the event!!!!
	if (startDt.daysTo(endDt)) 
	{
		// multi day event
		dateEntry->setRepeatType(repeatDaily);
		dateEntry->setRepeatFrequency(1);
		dateEntry->setRepeatEnd(dateEntry->getEventEnd());
#ifdef DEBUG
		DEBUGCONDUIT << fname <<": Setting single-day recurrence (" << startDt.toString() << 
			" - " << endDt.toString() << ")" <<endl;
#endif
	}
	QBitArray dayArray(7), dayArrayPalm(7);
	switch(recType)
	{
	case KCal::Recurrence::rDaily:
		dateEntry->setRepeatType(repeatDaily);
		if (needCalc)
		{
			dateEntry->setRepeatEnd(writeTm( startDt.addDays(r->duration()) ));
		}
		break;
	case KCal::Recurrence::rWeekly:
		dateEntry->setRepeatType(repeatWeekly);
		dayArray=r->days();
		// rotate the bits by one
		for (int i=0; i<7; i++)
		{
			dayArrayPalm.setBit( (i+1)%7, dayArray[i]);

		}
		dateEntry->setRepeatDays(dayArrayPalm);
		if (needCalc)
		{
			dateEntry->setRepeatEnd(writeTm( startDt.addDays(7*r->duration()) ));
		}
		break;
	case KCal::Recurrence::rMonthlyPos:
		dateEntry->setRepeatType(repeatMonthlyByDay);
		if (r->monthPositions().count()>0)
		{ // Only take the first monthly position, as the palm allows only one
			QPtrList<KCal::Recurrence::rMonthPos> mps=r->monthPositions();
			const KCal::Recurrence::rMonthPos*mp=mps.first();
			int pos=0;
			dayArray=mp->rDays;
			// this is quite clumsy, but I haven't found a better way...
			for (int j=0; j<7; j++)
				if (dayArray[j]) pos=j;
			dateEntry->setRepeatDay(static_cast<DayOfMonthType>(7*(mp->rPos-1) + pos));
		}
		if (needCalc)
		{
			dateEntry->setRepeatEnd(writeTm( startDt.addMonths(r->duration()) ));
		}
		break;
	case KCal::Recurrence::rMonthlyDay:
		dateEntry->setRepeatType(repeatMonthlyByDate);
		if (needCalc)
		{
			dateEntry->setRepeatEnd(writeTm( startDt.addMonths(r->duration()) ));
		}
		break;
	case KCal::Recurrence::rYearlyDay:
		dateEntry->setRepeatType(repeatYearly);
		if (needCalc)
		{
			dateEntry->setRepeatEnd(writeTm( startDt.addYears(r->duration()) ));
		}
		break;
	case KCal::Recurrence::rNone:
		dateEntry->setRepeatType(repeatNone);
		break;
	default:
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": Unknown recurrence type "<< recType << " with frequency "
			<< freq << " and duration " << r->duration() << endl;
#endif
		break;
	}
}

void VCalConduit::setExceptions(KCal::Event *vevent,const PilotDateEntry &dateEntry)
{
	FUNCTIONSETUP;
	if (((dateEntry.getRepeatType() == repeatDaily) &&
		dateEntry.getEvent()) && dateEntry.getExceptionCount())
	{
#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": WARNING Exceptions ignored for multi-day event "
		<< dateEntry.getDescription()
		<< endl ;
#endif
		return;
	}

	for (int i = 0; i < dateEntry.getExceptionCount(); i++)
	{
		vevent->addExDate(readTm(dateEntry.getExceptions()[i]).date());
	}
}

void VCalConduit::setExceptions(PilotDateEntry *dateEntry, const KCal::Event *vevent )
{
	FUNCTIONSETUP;
	struct tm *ex_List;
	
	if (!dateEntry || !vevent) 
	{
		kdWarning() << k_funcinfo << ": NULL dateEntry or NULL vevent given for exceptions. Skipping exceptions" << endl;
		return;
	}
	// first, we need to delete the old exceptions list, if it existed...
	// This is no longer needed, as I fixed PilotDateEntry::setExceptions to do this automatically
/*	ex_List=const_cast<structdateEntry->getExceptions();
	if (ex_List)
		KPILOT_DELETE(ex_List);*/

	size_t excount=vevent->exDates().size();
	if (excount<1)
	{
		dateEntry->setExceptionCount(0);
		dateEntry->setExceptions(0);
		return;
	}

#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Exceptions, before new struct tm" << endl ;
#endif
	// we have exceptions, so allocate mem and copy them there...
	ex_List=new struct tm[excount];
	if (!ex_List)
	{
		kdWarning() << k_funcinfo << ": Couldn't allocate memory for the exceptions" << endl;
		dateEntry->setExceptionCount(0);
		dateEntry->setExceptions(0);
		return;
	}

#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Exceptions, before iterator" << endl ;
#endif
	size_t n=0;
	KCal::DateList::const_iterator it;
	for ( it = vevent->exDates().begin(); it != vevent->exDates().end(); ++it )
	{
		ex_List[n++]=writeTm(*it);
	}
#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Exceptions, before actually setting them" << endl ;
#endif
	dateEntry->setExceptionCount(excount);
	dateEntry->setExceptions(ex_List);
}

// $Log$
// Revision 1.54  2002/04/17 00:28:11  kainhofe
// Removed a few #ifdef DEBUG clauses I had inserted for debugging purposes
//
// Revision 1.53  2002/04/16 23:40:36  kainhofe
// Exceptions no longer crash the daemon, recurrences are correct now, end date is set correctly. Problems: All events are off 1 day, lots of duplicates, exceptions are duplicate, too.
//
// Revision 1.52  2002/04/14 22:18:16  kainhofe
// Implemented the second part of the sync (PC=>Palm), but disabled it, because it corrupts the Palm datebook
//
// Revision 1.51  2002/02/23 20:57:41  adridg
// #ifdef DEBUG stuff
//
// Revision 1.50  2002/01/26 15:01:02  adridg
// Compile fixes and more
//
// Revision 1.49  2002/01/25 21:43:12  adridg
// ToolTips->WhatsThis where appropriate; vcal conduit discombobulated - it doesn't eat the .ics file anymore, but sync is limited; abstracted away more pilot-link
//

