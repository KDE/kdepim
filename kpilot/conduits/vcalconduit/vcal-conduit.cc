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

static const char *vcalconduit_id = "$Id:$";

#include <options.h>

#include <qdatetime.h>

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
#define Recurrence_t Recurrence
#define DateList_t DateList
#define DateListIterator_t DateList::ConstIterator
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


VCalConduit::VCalConduit(KPilotDeviceLink *d,
	const char *n,
	const QStringList &a) :
	ConduitAction(d,n,a),
	fCalendar(0L),
	fCurrentDatabase(0L),
	fPreviousDatabase(0L)
{
	FUNCTIONSETUP;
	(void)vcalconduit_id;
}

VCalConduit::~VCalConduit()
{
	FUNCTIONSETUP;
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
	fPreviousDatabase = new PilotLocalDatabase("DatebookDB");
	fCalendar = new KCal::CalendarLocal();

	// Handle lots of error cases.
	//
	if (!fCurrentDatabase || !fPreviousDatabase || !fCalendar) goto error;
	if (!fCurrentDatabase->isDBOpen() ||
		!fPreviousDatabase->isDBOpen()) goto error;
	loadSuccesful = fCalendar->load(fCalendarFile);
	if (!loadSuccesful) goto error;

	QTimer::singleShot(0,this,SLOT(syncRecord()));
	return;

error:
	if (!fCurrentDatabase)
	{
		kdWarning() << k_funcinfo
			<< ": Couldn't open database on Pilot"
			<< endl;
	}
	if (!fPreviousDatabase)
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
	KPILOT_DELETE(fPreviousDatabase);
	KPILOT_DELETE(fCalendar);
	emit syncDone(this);
}

void VCalConduit::syncRecord()
{
	FUNCTIONSETUP;

	PilotRecord *r = fCurrentDatabase->readNextModifiedRec();
	PilotRecord *s = 0L;

	if (!r)
	{
		QTimer::singleShot(0,this,SLOT(cleanup()));
		return;
	}

	s = fPreviousDatabase->readRecordById(r->getID());
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

void VCalConduit::cleanup()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fCurrentDatabase);
	KPILOT_DELETE(fPreviousDatabase);

	fCalendar->save(fCalendarFile);
	KPILOT_DELETE(fCalendar);

	emit syncDone(this);
}


void VCalConduit::addRecord(PilotRecord *r)
{
	FUNCTIONSETUP;

	fPreviousDatabase->writeRecord(r);

	PilotDateEntry de(r);
	KCal::Event *e = eventFromRecord(de);
	e->setOrganizer(fCalendar->getEmail());
	e->setSyncStatus(KCal::Incidence::SYNCNONE);
	e->setSecrecy(r->isSecret() ?
		KCal::Event::SecrecyPrivate :
		KCal::Event::SecrecyPublic);

	fCalendar->addEvent(e);
}

void VCalConduit::deleteRecord(PilotRecord *r, PilotRecord *s)
{
	FUNCTIONSETUP;

	KCal::Event *e = findEvent(r->getID());
	if (e) { fCalendar->deleteEvent(e); }
	fPreviousDatabase->writeRecord(r);
}

void VCalConduit::changeRecord(PilotRecord *r,PilotRecord *s)
{
	FUNCTIONSETUP;
}

KCal::Event *VCalConduit::eventFromRecord(const PilotDateEntry &de)
{
	KCal::Event *e = new KCal::Event;

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

void VCalConduit::setAlarms(KCal::Event *e, const PilotDateEntry &de)
{
	FUNCTIONSETUP;

	if (!de.getAlarm()) return;

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

	// TODO: Fix alarms
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
		DEBUGCONDUIT << fname << "-- end " << endDate.toString() << endl;
	}
	else
	{
		DEBUGCONDUIT << fname << "-- noend" << endl;
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

KCal::Event *VCalConduit::findEvent(recordid_t id)
{
	QList<KCal::Event> events = fCalendar->getAllEvents();

	KCal::Event *event = events.first();
	while(event)
	{
		if (event->pilotId() == id) return event;
		event = events.next();
	}

	return 0L;
}

// $Log:$

