/* vcal-conduit.cc                      KPilot
**
** Copyright (C) 2002 Reinhold Kainhofer
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
#include <unistd.h>

#include <qdatetime.h>
#include <qtimer.h>

#include <pilotUser.h>
#include <kconfig.h>

//#include <calendar.h>
#include <calendarlocal.h>
//#include <event.h>


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
//#include <pilotDateEntry.h>

//#include "vcal-conduitbase.h"
//#include "vcal-factory.h"
#include "vcal-conduit.moc"





VCalConduitPrivate::VCalConduitPrivate(KCal::Calendar *b) :
	VCalConduitPrivateBase(b)
{
	fAllEvents.setAutoDelete(false);
}

void VCalConduitPrivate::addIncidence(KCal::Incidence*e)
{
	fAllEvents.append(dynamic_cast<KCal::Event*>(e));
	fCalendar->addEvent(dynamic_cast<KCal::Event*>(e));
}

int VCalConduitPrivate::updateIncidences()
{
	fAllEvents = fCalendar->events();
	fAllEvents.setAutoDelete(false);
	return fAllEvents.count();
}


void VCalConduitPrivate::removeIncidence(KCal::Incidence *e)
{
	// use dynamic_cast which returns a null pointer if the class does not match...
	fAllEvents.remove(dynamic_cast<KCal::Event*>(e));
	fCalendar->deleteEvent(dynamic_cast<KCal::Event*>(e));
}


KCal::Incidence *VCalConduitPrivate::findIncidence(recordid_t id)
{
	KCal::Event *event = fAllEvents.first();
	while (event!=0)
	{
		if ((recordid_t)event->pilotId() == id) return event;
		event = fAllEvents.next();
	}
	return 0L;
}


KCal::Incidence *VCalConduitPrivate::getNextIncidence()
{
	if (reading) return fAllEvents.next();
	reading=true;
	return fAllEvents.first();
}


KCal::Incidence *VCalConduitPrivate::getNextModifiedIncidence()
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



/****************************************************************************
 *                          VCalConduit class                               *
 ****************************************************************************/

VCalConduit::VCalConduit(KPilotDeviceLink *d,
	const char *n,
	const QStringList &a) : VCalConduitBase(d,n,a)
{
	FUNCTIONSETUP;
	(void) vcalconduit_id;
}


VCalConduit::~VCalConduit()
{
//	FUNCTIONSETUP;
};

VCalConduitPrivateBase* VCalConduit::newVCalPrivate(KCal::Calendar *fCalendar) {
	return new VCalConduitPrivate(fCalendar);
};

const QString VCalConduit::getTitle(PilotAppCategory*de)
{
	PilotDateEntry*d=dynamic_cast<PilotDateEntry*>(de);
	if (d) return QString(d->getDescription());
	return "";
}



PilotRecord*VCalConduit::recordFromIncidence(PilotAppCategory*de, const KCal::Incidence*e)
{
	FUNCTIONSETUP;
	if (!de || !e)
	{
#ifdef DEBUG
		DEBUGCONDUIT<<fname<<": got null entry or null incidence."<<endl;
#endif
		return NULL;
	}
	return recordFromIncidence(dynamic_cast<PilotDateEntry*>(de), dynamic_cast<const KCal::Event*>(e));
}

PilotRecord*VCalConduit::recordFromIncidence(PilotDateEntry*de, const KCal::Event*e)
{
	FUNCTIONSETUP;
	if (!de || !e) {
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
DEBUGCONDUIT<<"-------- "<<e->summary()<<endl;
	return de->pack();
}


KCal::Incidence *VCalConduit::incidenceFromRecord(KCal::Incidence *e, const PilotAppCategory *de)
{
	return dynamic_cast<KCal::Incidence*>(incidenceFromRecord(dynamic_cast<KCal::Event*>(e), dynamic_cast<const PilotDateEntry*>(de)));
}


KCal::Event *VCalConduit::incidenceFromRecord(KCal::Event *e, const PilotDateEntry *de)
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
	e->setSecrecy(de->isSecret() ?
		KCal::Event::SecrecyPrivate :
		KCal::Event::SecrecyPublic);

	e->setPilotId(de->getID());
	e->setSyncStatus(KCal::Incidence::SYNCNONE);

	setStartEndTimes(e,de);
	setAlarms(e,de);
	setRecurrence(e,de);
	setExceptions(e,de);

	e->setSummary(de->getDescription());
#ifdef DEBUG
		DEBUGCONDUIT<<fname<<": DESCRIPTION: "<<de->getDescription()<<"  ---------------------------------------------------"<<endl;
#endif
	e->setDescription(de->getNote());

	return e;
}


void VCalConduit::setStartEndTimes(KCal::Event *e,const PilotDateEntry *de)
{
	FUNCTIONSETUP;
	e->setDtStart(readTm(de->getEventStart()));
	e->setFloats(de->isEvent());

	if (de->isMultiDay())
	{
		e->setDtEnd(readTm(de->getRepeatEnd()));
	}
	else
	{
		e->setDtEnd(readTm(de->getEventEnd()));
	}
}


void VCalConduit::setStartEndTimes(PilotDateEntry*de, const KCal::Event *e)
{
	FUNCTIONSETUP;
	struct tm ttm=writeTm(e->dtStart());
	de->setEventStart(ttm);
	de->setEvent(e->doesFloat());

	if (e->hasEndDate() && e->dtEnd().isValid())
	{
		ttm=writeTm(e->dtEnd());
	}
	else
	{
		ttm=writeTm(e->dtStart());
	}
	de->setEventEnd(ttm);
}


void VCalConduit::setAlarms(KCal::Event *e, const PilotDateEntry *de)
{
	FUNCTIONSETUP;

	if (!e) return;
	// Delete all the alarms now and add them one by one later on.
	e->clearAlarms();
	if (!de->getAlarm()) return;

//	QDateTime alarmDT = readTm(de->getEventStart());
	int advanceUnits = de->getAdvanceUnits();

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

	KCal::Duration adv(-60*advanceUnits*de->getAdvance());
	KCal::Alarm*alm=e->newAlarm();
	if (!alm) return;

	alm->setOffset(adv);
	alm->setEnabled(true);
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

	if ( !e->isAlarmEnabled() )
	{
		de->setAlarm(0);
		return;
	}

	// find the first enabled alarm
	QPtrList<KCal::Alarm> alms=e->alarms();
	KCal::Alarm* alm=0, *alarm=0;
	for (QPtrListIterator<KCal::Alarm> it(alms); (alarm = it.current()) != 0; ++it) {
		if (alarm->enabled()) alm=alarm;
	}

	if (!alm )
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": no enabled alarm found (should exist!!!)"<<endl;
#endif
		de->setAlarm(0);
		return;
	}

	// palm and PC offsets have a different sign!!
	int aoffs=-alm->offset().asSeconds()/60;
	int offs=(aoffs>0)?aoffs:-aoffs;

	// find the best Advance Unit
	if (offs>=100 || offs==60)
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
	de->setAdvance((aoffs>0)?offs:-offs);
	de->setAlarm(1);
}


void VCalConduit::setRecurrence(KCal::Event *event,const PilotDateEntry *dateEntry)
{
	FUNCTIONSETUP;

	if ((dateEntry->getRepeatType() == repeatNone) || dateEntry->isMultiDay())
	{
#ifdef DEBUG
		DEBUGCONDUIT<<fname<<": no recurrence to set"<<endl;
#endif
		return;
	}

	Recurrence_t *recur = event->recurrence();
	int freq = dateEntry->getRepeatFrequency();
	bool repeatsForever = dateEntry->getRepeatForever();
	QDate endDate;

	if (!repeatsForever)
	{
		endDate = readTm(dateEntry->getRepeatEnd()).date();
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

	switch(dateEntry->getRepeatType())
	{
	case repeatDaily:
		if (repeatsForever) recur->setDaily(freq,-1);
		else recur->setDaily(freq,endDate);
		break;
	case repeatWeekly:
		{
		const int *days = dateEntry->getRepeatDays();

#ifdef DEBUG
		DEBUGCONDUIT << fname
			<< ": Got repeat-weekly entry, by-days="
			<< days[0] << " "<< days[1] << " "<< days[2] << " "
			<< days[3] << " "
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

		if (repeatsForever) recur->setWeekly(freq,dayArray,-1);
		else recur->setWeekly(freq,dayArray,endDate);
		}
		break;
	case repeatMonthlyByDay:
		if (repeatsForever)
		{
			recur->setMonthly(Recurrence_t::rMonthlyPos,freq,-1);
		}
		else
		{
			recur->setMonthly(Recurrence_t::rMonthlyPos,freq,endDate);
		}

		dayArray.setBit((dateEntry->getRepeatDay()-1) % 7);
		recur->addMonthlyPos(( (dateEntry->getRepeatDay()-1) / 7) + 1, dayArray);
		break;
	case repeatMonthlyByDate:
		if (repeatsForever)
		{
			recur->setMonthly(Recurrence_t::rMonthlyDay,freq,-1);
		}
		else
		{
			recur->setMonthly(Recurrence_t::rMonthlyDay,freq,endDate);
		}
		recur->addMonthlyDay( dateEntry->getEventStart().tm_mday );
		break;
	case repeatYearly:
		if (repeatsForever)
		{
			recur->setYearly(Recurrence_t::rYearlyDay,freq,-1);
		}
		else
		{
			recur->setYearly(Recurrence_t::rYearlyDay,freq,endDate);
		}
		recur->addYearlyNum( readTm(dateEntry->getEventStart()).date().dayOfYear() );
		break;
	case repeatNone:
	default :
#ifdef DEBUG
		DEBUGCONDUIT << fname
			<< ": Can't handle repeat type "
			<< dateEntry->getRepeatType()
			<< endl;
#endif
		break;
	}
}


void VCalConduit::setRecurrence(PilotDateEntry*dateEntry, const KCal::Event *event)
{
	FUNCTIONSETUP;
	bool isMultiDay=false;

	//  first we have 'fake type of recurrence' when a multi-day event is passed to the pilot, it is converted to an event
	// which recurs daily a number of times. if the event itself recurs, this will be overridden, and
	// only the first day will be included in the event!!!!
	QDateTime startDt(readTm(dateEntry->getEventStart())), endDt(readTm(dateEntry->getEventEnd()));
	if (startDt.daysTo(endDt))
	{
		isMultiDay=true;
		dateEntry->setRepeatType(repeatDaily);
		dateEntry->setRepeatFrequency(1);
		dateEntry->setRepeatEnd(dateEntry->getEventEnd());
#ifdef DEBUG
		DEBUGCONDUIT << fname <<": Setting single-day recurrence (" << startDt.toString() << " - " << endDt.toString() << ")" <<endl;
#endif
	}


	KCal::Recurrence*r=event->recurrence();
	if (!r) return;
	ushort recType=r->doesRecur();
	if (recType==KCal::Recurrence::rNone)
	{
		if (!isMultiDay) dateEntry->setRepeatType(repeatNone);
		return;
	}


	int freq=r->frequency();
	QDate endDate=r->endDate();

	if (!endDate.isValid())
	{
		dateEntry->setRepeatForever();
	}
	else
	{
		dateEntry->setRepeatEnd(writeTm(endDate));
	}
	dateEntry->setRepeatFrequency(freq);
#ifdef DEBUG
	DEBUGCONDUIT<<" Event: "<<event->summary()<<" ("<<event->description()<<")"<<endl;
	DEBUGCONDUIT<< "duration: "<<r->duration() << ", endDate: "<<endDate.toString()<< ", ValidEndDate: "<<endDate.isValid()<<", NullEndDate: "<<endDate.isNull()<<endl;
#endif

	QBitArray dayArray(7), dayArrayPalm(7);
	switch(recType)
	{
	case KCal::Recurrence::rDaily:
		dateEntry->setRepeatType(repeatDaily);
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
		break;
	case KCal::Recurrence::rMonthlyPos:
		dateEntry->setRepeatType(repeatMonthlyByDay);
		if (r->monthPositions().count()>0)
		{ 
			// Only take the first monthly position, as the palm allows only one
			QPtrList<KCal::Recurrence::rMonthPos> mps=r->monthPositions();
			const KCal::Recurrence::rMonthPos*mp=mps.first();
			int pos=0;
			dayArray=mp->rDays;
			// this is quite clumsy, but I haven't found a better way...
			for (int j=0; j<7; j++)
				if (dayArray[j]) pos=j;
			dateEntry->setRepeatDay(static_cast<DayOfMonthType>(7*(mp->rPos-1) + pos));
		}
		break;
	case KCal::Recurrence::rMonthlyDay:
		dateEntry->setRepeatType(repeatMonthlyByDate);
//TODO: is this needed?		dateEntry->setRepeatDay(static_cast<DayOfMonthType>(startDt.day()));
		break;
	case KCal::Recurrence::rYearlyDay:
		dateEntry->setRepeatType(repeatYearly);
		break;
	case KCal::Recurrence::rNone:
		if (!isMultiDay) dateEntry->setRepeatType(repeatNone);
		break;
	default:
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": Unknown recurrence type "<< recType << " with frequency "
			<< freq << " and duration " << r->duration() << endl;
#endif
		break;
	}
}


void VCalConduit::setExceptions(KCal::Event *vevent,const PilotDateEntry *dateEntry)
{
	FUNCTIONSETUP;
	
	// Start from an empty exception list, and if necessary, add exceptions.
	// At the end of the function, apply the (possibly empty) exception list.
	KCal::DateList dl;

	if ( !(dateEntry->isMultiDay() ) && dateEntry->getExceptionCount()>0 )
	{
		for (int i = 0; i < dateEntry->getExceptionCount(); i++)
		{
//			vevent->addExDate(readTm(dateEntry->getExceptions()[i]).date());
			dl.append(readTm(dateEntry->getExceptions()[i]).date());
		}
	}
	else
	{
#ifdef DEBUG
	if (dateEntry->getExceptionCount()>0)
	DEBUGCONDUIT << fname
		<< ": WARNING Exceptions ignored for multi-day event "
		<< dateEntry->getDescription()
		<< endl ;
#endif
		return;
	}
	vevent->setExDates(dl);
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

	// we have exceptions, so allocate mem and copy them there...
	ex_List=new struct tm[excount];
	if (!ex_List)
	{
		kdWarning() << k_funcinfo << ": Couldn't allocate memory for the exceptions" << endl;
		dateEntry->setExceptionCount(0);
		dateEntry->setExceptions(0);
		return;
	}

	size_t n=0;

	KCal::DateList exDates = vevent->exDates();
	KCal::DateList::ConstIterator dit;
	for (dit = exDates.begin(); dit != exDates.end(); ++dit ) {
		struct tm ttm=writeTm(*dit);
		ex_List[n++]=ttm;
	}
	dateEntry->setExceptionCount(excount);
	dateEntry->setExceptions(ex_List);
}

// $Log$
// Revision 1.70  2002/07/23 00:45:18  kainhofe
// Fixed several bugs with recurrences.
//
// Revision 1.69  2002/07/20 18:17:04  cschumac
// Renamed Calendar::getAllEvents() to Calendar::events().
// Removed get prefix from Calendar functions returning events.
// Finally we now have a consistent naming scheme in Calendar.
//
// Revision 1.68  2002/06/12 22:11:17  kainhofe
// Proper cleanup, libkcal still has some problems marking records modified on loading
//
// Revision 1.67  2002/05/18 13:08:56  kainhofe
// dirty flag is now cleared, conflict resolution shows the correct item title and asks the correct question
//
// Revision 1.66  2002/05/14 23:07:49  kainhofe
// Added the conflict resolution code. the Palm and PC precedence is currently swapped, and will be improved in the next few days, anyway...
//
// Revision 1.65  2002/05/01 21:18:23  kainhofe
// Reworked the settings dialog, added various different sync options
//
// Revision 1.51.2.3  2002/04/28 12:58:54  kainhofe
// Calendar conduit now works, no memory leaks, timezone still shifted. Todo conduit mostly works, for my large list it crashes when saving the calendar file.
//
// Revision 1.63  2002/04/22 22:51:51  kainhofe
// Added the first version of the todo conduit, fixed a check for a null pointer in the datebook conduit
//
// Revision 1.62  2002/04/21 17:39:01  kainhofe
// recurrences without enddate work now
//
// Revision 1.61  2002/04/21 17:07:12  kainhofe
// Fixed some memory leaks, old alarms and exceptions are deleted before new are added, Alarms are now correct
//
// Revision 1.60  2002/04/20 18:05:50  kainhofe
// No duplicates any more in the calendar
//
// Revision 1.59  2002/04/20 17:38:02  kainhofe
// recurrence now correctly written to the palm, no longer crashes
//
// Revision 1.58  2002/04/20 14:21:26  kainhofe
// Alarms are now written to the palm. Some bug fixes, extensive testing. Exceptions still crash the palm ;-(((
//
// Revision 1.57  2002/04/19 19:34:11  kainhofe
// didn't compile
//
// Revision 1.56  2002/04/19 19:10:29  kainhofe
// added some comments describin the sync logic, deactivated the sync again (forgot it when I commited last time)
//
// Revision 1.55  2002/04/17 20:47:04  kainhofe
// Implemented the alarm sync
//
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

