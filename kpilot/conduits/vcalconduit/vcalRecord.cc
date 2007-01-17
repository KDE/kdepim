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

#include "vcalRecord.h"

bool VCalRecord::setEvent(KCal::Event *e, const PilotDateEntry *de)
{
	FUNCTIONSETUP;
	if (!e) {
		DEBUGCONDUIT << fname
			<< "! NULL event given... Skipping it" << endl;
		return false;
	}

	e->setSyncStatus(KCal::Incidence::SYNCNONE);
	e->setSecrecy(de->isSecret() ?
		KCal::Event::SecrecyPrivate :
		KCal::Event::SecrecyPublic);

	e->setPilotId(de->id());
	e->setSyncStatus(KCal::Incidence::SYNCNONE);

	setStartEndTimes(e,de);
	setAlarms(e,de);
	setRecurrence(e,de);
	setExceptions(e,de);

	e->setSummary(de->getDescription());
	e->setDescription(de->getNote());
	e->setLocation(de->getLocation());

	// used by e.g. Agendus and Datebk
	setCategory(e, de);

	return true;
}

void VCalRecord::setStartEndTimes(KCal::Event *e, const PilotDateEntry *de)
{
	FUNCTIONSETUP;
	DEBUGCONDUIT << fname
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

void VCalRecord::setAlarms(KCal::Event *e, const PilotDateEntry *de)
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

	alm->setStartOffset(adv);
	alm->setEnabled(true);
}

void VCalRecord::setRecurrence(KCal::Event *event,const PilotDateEntry *dateEntry)
{
	FUNCTIONSETUP;

	if ((dateEntry->getRepeatType() == repeatNone) || dateEntry->isMultiDay())
	{
#ifdef DEBUG
		DEBUGCONDUIT<<fname<<": no recurrence to set"<<endl;
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
		DEBUGCONDUIT << fname << "-- end " << endDate.toString() << endl;
	}
	else
	{
		DEBUGCONDUIT << fname << "-- noend" << endl;
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
		DEBUGCONDUIT << fname
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

void VCalRecord::setExceptions(KCal::Event *vevent,const PilotDateEntry *dateEntry)
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
	vevent->recurrence()->setExDates(dl);
}

void VCalRecord::setCategory(KCal::Event *e, const PilotDateEntry *de)
{
	FUNCTIONSETUP;

	if (!e || !de)
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": error.  unable to set kcal category. e: [" <<
			e << "], de: [" << de << "]" << endl;
#endif
		return;
	}

	QStringList cats=e->categories();
	int cat = de->category();
	QString newcat = de->getCategoryLabel();
#ifdef DEBUG
	DEBUGCONDUIT << fname << ": palm category id: [" << cat <<
		"], label: [" << de->getCategoryLabel() << "]" << endl;
#endif
	if ( (0<cat) && (cat< (int)Pilot::CATEGORY_COUNT) )
	{
		if (!cats.contains(newcat))
		{
			// if this event only has one category associated with it, then we can
			// safely assume that what we should be doing here is changing it to match
			// the palm.  if there's already more than one category in the event, however, we
			// won't cause data loss--we'll just append what the palm has to the
			// event's categories
			if (cats.count() <=1) cats.clear();

			cats.append( newcat );
			e->setCategories(cats);
		}
	}
#ifdef DEBUG
	DEBUGCONDUIT << fname << ": kcal categories now: [" << cats.join(",") << "]" << endl;
#endif
}


bool VCalRecord::setDateEntry(PilotDateEntry*de, const KCal::Event*e)
{
	FUNCTIONSETUP;
	if (!de || !e) {
		DEBUGCONDUIT << fname
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
	setCategory(de, e);
	return true;
}

void VCalRecord::setStartEndTimes(PilotDateEntry*de, const KCal::Event *e)
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




void VCalRecord::setAlarms(PilotDateEntry*de, const KCal::Event *e)
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
		DEBUGCONDUIT << fname << ": no enabled alarm found (should exist!!!)"<<endl;
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



void VCalRecord::setRecurrence(PilotDateEntry*dateEntry, const KCal::Event *event)
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
		DEBUGCONDUIT << fname <<": Setting single-day recurrence (" << startDt.toString() << " - " << endDt.toString() << ")" <<endl;
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
		DEBUGCONDUIT << fname
			<< "! Unsupported yearly recurrence type." << endl;
	case KCal::Recurrence::rYearlyMonth:
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


void VCalRecord::setExceptions(PilotDateEntry *dateEntry, const KCal::Event *vevent )
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
		kdWarning() << k_funcinfo << ": Couldn't allocate memory for the exceptions" << endl;
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


void VCalRecord::setCategory(PilotDateEntry *de, const KCal::Event *e)
{
	FUNCTIONSETUP;

	if (!de || !e)
	{
		return;
	}

	QString deCategory;
	QStringList eventCategories = e->categories();
	if (eventCategories.size() < 1)
	{
		// This event has no categories.
		(PilotRecordBase *)de->setCategory(Pilot::Unfiled);
		return;
	}

	// Quick check: does the record (not unfiled) have an entry
	// in the categories list? If so, use that.
	if (de->category() != Pilot::Unfiled)
	{
		deCategory = de->getCategoryLabel();
		if (eventCategories.contains(deCategory))
		{
			// Found, so leave the category unchanged.
			return;
		}
	}

	QStringList availableHandheldCategories = Pilot::categoryNames(&(de->appInfo().category));

	// Either the record is unfiled, and should be filed, or
	// it has a category set which is not in the list of
	// categories that the event has. So go looking for
	// a category that is available both for the event
	// and on the handheld.
	for ( QStringList::ConstIterator it = eventCategories.begin();
		it != eventCategories.end(); ++it )
	{
		// Odd, an empty category string.
		if ( (*it).isEmpty() )
		{
			continue;
		}

		if (availableHandheldCategories.contains(*it))
		{
			de->setCategory(*it);
			return;
		}
	}

	de->setCategory(Pilot::Unfiled);
}

