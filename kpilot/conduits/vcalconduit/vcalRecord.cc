/* vcalRecord.cc                       KPilot
**
** Copyright (C) 2006 by Adriaan de Groot <groot@kde.org>
** Copyright (C) 2002-2003 by Reinhold Kainhofer
** Copyright (C) 2001 by Dan Pilone
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <libkcal/calendar.h>
#include <libkcal/calendarlocal.h>
#include <libkcal/recurrence.h>
#include <libkcal/vcalformat.h>

#include "pilot.h"
#include "pilotDateEntry.h"

#include "kcalRecord.h"
#include "vcalRecord.h"


static void setStartEndTimes(KCal::Event *e, const PilotDateEntry *de)
{
	FUNCTIONSETUP;
	DEBUGKPILOT << fname
		<< "# Start time on Palm: "
		<< readTm(de->getEventStart()).toString() << endl;

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

static void setAlarms(KCal::Event *e, const PilotDateEntry *de)
{
	FUNCTIONSETUP;

	if (!e) return;
	// Delete all the alarms now and add them one by one later on.
	e->clearAlarms();
	if (!de->isAlarmEnabled()) return;

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
		DEBUGKPILOT << fname
			<< ": Unknown advance units "
			<< advanceUnits
			<< endl;
#endif
		advanceUnits=1;
	}

	KCal::Duration adv(-60*advanceUnits*de->getAdvance());
	KCal::Alarm*alm=e->newAlarm();
	if (!alm) return;

	alm->setStartOffset(adv);
	alm->setEnabled(true);
}

static void setRecurrence(KCal::Event *event,const PilotDateEntry *dateEntry)
{
	FUNCTIONSETUP;

	if ((dateEntry->getRepeatType() == repeatNone) || dateEntry->isMultiDay())
	{
#ifdef DEBUG
		DEBUGKPILOT<<fname<<": no recurrence to set"<<endl;
#endif
		return;
	}

	KCal::Recurrence *recur = event->recurrence();
	int freq = dateEntry->getRepeatFrequency();
	bool repeatsForever = dateEntry->getRepeatForever();
	QDate endDate, evt;

	if (!repeatsForever)
	{
		endDate = readTm(dateEntry->getRepeatEnd()).date();
#ifdef DEBUG
		DEBUGKPILOT << fname << "-- end " << endDate.toString() << endl;
	}
	else
	{
		DEBUGKPILOT << fname << "-- noend" << endl;
#endif
	}

	QBitArray dayArray(7);

	switch(dateEntry->getRepeatType())
	{
	case repeatDaily:
		recur->setDaily(freq);
		break;
	case repeatWeekly:
		{
		const int *days = dateEntry->getRepeatDays();

#ifdef DEBUG
		DEBUGKPILOT << fname
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
		recur->setWeekly( freq, dayArray );
		}
		break;
	case repeatMonthlyByDay: {
		// Palm: Day=0(sun)-6(sat); week=0-4, 4=last week; pos=week*7+day
		// libkcal: day=bit0(mon)-bit6(sun); week=-5to-1(from end) and 1-5 (from beginning)
		// Palm->PC: w=pos/7
		// week: if w=4 -> week=-1, else week=w+1;
		// day: day=(pos-1)%7 (rotate by one day!)
		recur->setMonthly( freq );

		int day=dateEntry->getRepeatDay();
		int week=day/7;
		// week=4 means last, otherwise convert to 0-based
		if (week==4) week=-1; else week++;
		dayArray.setBit((day+6) % 7);
		recur->addMonthlyPos(week, dayArray);
		break;}
	case repeatMonthlyByDate:
		recur->setMonthly( freq );
		recur->addMonthlyDate( dateEntry->getEventStart().tm_mday );
		break;
	case repeatYearly:
		recur->setYearly( freq );
		evt=readTm(dateEntry->getEventStart()).date();
		recur->addYearlyMonth( evt.month() );
//		dayArray.setBit((evt.day()-1) % 7);
//		recur->addYearlyMonthPos( ( (evt.day()-1) / 7) + 1, dayArray );
		break;
	case repeatNone:
	default :
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": Can't handle repeat type "
			<< dateEntry->getRepeatType()
			<< endl;
#endif
		break;
	}
	if (!repeatsForever)
	{
		recur->setEndDate(endDate);
	}
}

static void setExceptions(KCal::Event *vevent,const PilotDateEntry *dateEntry)
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
	DEBUGKPILOT << fname
		<< ": WARNING Exceptions ignored for multi-day event "
		<< dateEntry->getDescription()
		<< endl ;
#endif
		return;
	}
	vevent->recurrence()->setExDates(dl);
}

static void setStartEndTimes(PilotDateEntry*de, const KCal::Event *e)
{
	FUNCTIONSETUP;
	struct tm ttm=writeTm(e->dtStart());
	de->setEventStart(ttm);
	de->setFloats( e->doesFloat() );

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




static void setAlarms(PilotDateEntry*de, const KCal::Event *e)
{
	FUNCTIONSETUP;

	if (!de || !e )
	{
#ifdef DEBUG
		DEBUGKPILOT << fname << ": NULL entry given to setAlarms. "<<endl;
#endif
		return;
	}

	if ( !e->isAlarmEnabled() )
	{
		de->setAlarmEnabled( false );
		return;
	}

	// find the first enabled alarm
	KCal::Alarm::List alms=e->alarms();
	KCal::Alarm* alm=0;
	KCal::Alarm::List::ConstIterator it;
	for ( it = alms.begin(); it != alms.end(); ++it ) {
		if ((*it)->enabled()) alm=*it;
	}

	if (!alm )
	{
#ifdef DEBUG
		DEBUGKPILOT << fname << ": no enabled alarm found (should exist!!!)"<<endl;
#endif
		de->setAlarmEnabled( false );
		return;
	}

	// palm and PC offsets have a different sign!!
	int aoffs=-alm->startOffset().asSeconds()/60;
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
	de->setAlarmEnabled( true );
}



static void setRecurrence(PilotDateEntry*dateEntry, const KCal::Event *event)
{
	FUNCTIONSETUP;
	bool isMultiDay=false;

	// first we have 'fake type of recurrence' when a multi-day event is passed to the pilot, it is converted to an event
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
		DEBUGKPILOT << fname <<": Setting single-day recurrence (" << startDt.toString() << " - " << endDt.toString() << ")" <<endl;
#endif
	}


	KCal::Recurrence*r=event->recurrence();
	if (!r) return;
	ushort recType=r->recurrenceType();
	if ( recType==KCal::Recurrence::rNone )
	{
		if (!isMultiDay) dateEntry->setRepeatType(repeatNone);
		return;
	}


	int freq=r->frequency();
	QDate endDate=r->endDate();

	if ( r->duration() < 0 || !endDate.isValid() )
	{
		dateEntry->setRepeatForever();
	}
	else
	{
		dateEntry->setRepeatEnd(writeTm(endDate));
	}
	dateEntry->setRepeatFrequency(freq);
#ifdef DEBUG
	DEBUGKPILOT<<" Event: "<<event->summary()<<" ("<<event->description()<<")"<<endl;
	DEBUGKPILOT<< "duration: "<<r->duration() << ", endDate: "<<endDate.toString()<< ", ValidEndDate: "<<endDate.isValid()<<", NullEndDate: "<<endDate.isNull()<<endl;
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
		// Palm: Day=0(sun)-6(sat); week=0-4, 4=last week; pos=week*7+day
		// libkcal: day=bit0(mon)-bit6(sun); week=-5to-1(from end) and 1-5 (from beginning)
		// PC->Palm: pos=week*7+day
		//  week: if w=-1 -> week=4, else week=w-1
		//  day: day=(daybit+1)%7  (rotate because of the different offset)
		dateEntry->setRepeatType(repeatMonthlyByDay);
		if (r->monthPositions().count()>0)
		{
			// Only take the first monthly position, as the palm allows only one
			QValueList<KCal::RecurrenceRule::WDayPos> mps=r->monthPositions();
			KCal::RecurrenceRule::WDayPos mp=mps.first();
			int week = mp.pos();
			int day = (mp.day()+1) % 7; // rotate because of different offset
			// turn to 0-based and include starting from end of month
			// TODO: We don't handle counting from the end of the month yet!
			if (week==-1) week=4; else week--;
			dateEntry->setRepeatDay(static_cast<DayOfMonthType>(7*week + day));
		}
		break;
	case KCal::Recurrence::rMonthlyDay:
		dateEntry->setRepeatType(repeatMonthlyByDate);
//TODO: is this needed?		dateEntry->setRepeatDay(static_cast<DayOfMonthType>(startDt.day()));
		break;
	case KCal::Recurrence::rYearlyDay:
	case KCal::Recurrence::rYearlyPos:
		DEBUGKPILOT << fname
			<< "! Unsupported yearly recurrence type." << endl;
	case KCal::Recurrence::rYearlyMonth:
		dateEntry->setRepeatType(repeatYearly);
		break;
	case KCal::Recurrence::rNone:
		if (!isMultiDay) dateEntry->setRepeatType(repeatNone);
		break;
	default:
#ifdef DEBUG
		DEBUGKPILOT << fname << ": Unknown recurrence type "<< recType << " with frequency "
			<< freq << " and duration " << r->duration() << endl;
#endif
		break;
	}
}


static void setExceptions(PilotDateEntry *dateEntry, const KCal::Event *vevent )
{
	FUNCTIONSETUP;
	struct tm *ex_List;

	if (!dateEntry || !vevent)
	{
		WARNINGKPILOT << "NULL dateEntry or NULL vevent given for exceptions. Skipping exceptions" << endl;
		return;
	}
	// first, we need to delete the old exceptions list, if it existed...
	// This is no longer needed, as I fixed PilotDateEntry::setExceptions to do this automatically
/*	ex_List=const_cast<structdateEntry->getExceptions();
	if (ex_List)
		KPILOT_DELETE(ex_List);*/

	KCal::DateList exDates = vevent->recurrence()->exDates();
	size_t excount = exDates.size();
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
		WARNINGKPILOT << "Couldn't allocate memory for the exceptions" << endl;
		dateEntry->setExceptionCount(0);
		dateEntry->setExceptions(0);
		return;
	}

	size_t n=0;

	KCal::DateList::ConstIterator dit;
	for (dit = exDates.begin(); dit != exDates.end(); ++dit ) {
		struct tm ttm=writeTm(*dit);
		ex_List[n++]=ttm;
	}
	dateEntry->setExceptionCount(excount);
	dateEntry->setExceptions(ex_List);
}


bool KCalSync::setEvent(KCal::Event *e,
	const PilotDateEntry *de,
	const CategoryAppInfo &info)
{
	FUNCTIONSETUP;
	if (!e)
	{
		DEBUGKPILOT << fname
			<< "! NULL event given... Skipping it" << endl;
		return false;
	}
	if (!de)
	{
		DEBUGKPILOT << fname
			<< "! NULL date entry given... Skipping it" << endl;
		return false;
	}


	e->setSecrecy(de->isSecret() ?
		KCal::Event::SecrecyPrivate :
		KCal::Event::SecrecyPublic);

	e->setPilotId(de->id());

	setStartEndTimes(e,de);
	setAlarms(e,de);
	setRecurrence(e,de);
	setExceptions(e,de);

	e->setSummary(de->getDescription());
	e->setDescription(de->getNote());
	e->setLocation(de->getLocation());

	// used by e.g. Agendus and Datebk
	setCategory(e, de, info);

	// NOTE: This MUST be done last, since every other set* call
	// calls updated(), which will trigger an
	// setSyncStatus(SYNCMOD)!!!
	e->setSyncStatus(KCal::Incidence::SYNCNONE);

	return true;
}

bool KCalSync::setDateEntry(PilotDateEntry *de,
	const KCal::Event *e,
	const CategoryAppInfo &info)
{
	FUNCTIONSETUP;
	if (!de || !e) {
		DEBUGKPILOT << fname
			<< ": NULL event given... Skipping it" << endl;
		return false;
	}

	// set secrecy, start/end times, alarms, recurrence, exceptions, summary and description:
	if (e->secrecy()!=KCal::Event::SecrecyPublic)
	{
		de->setSecret( true );
	}

	setStartEndTimes(de, e);
	setAlarms(de, e);
	setRecurrence(de, e);
	setExceptions(de, e);
	de->setDescription(e->summary());
	de->setNote(e->description());
	de->setLocation(e->location());
	setCategory(de, e, info);
	return true;
}

