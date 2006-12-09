/* KPilot
**
** Copyright (C) 2002-2003 Reinhold Kainhofer
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"
#include <libkcal/calendar.h>
#include <libkcal/recurrence.h>

#define Recurrence_t KCal::Recurrence
#include <pilotDateEntry.h>
#include <pilotDatabase.h>

#include "vcal-conduit.moc"
#include "vcalconduitSettings.h"

// Include for testpurposes
#include <libkcal/calendarlocal.h>
#include <libkcal/vcalformat.h>

extern "C"
{

unsigned long version_conduit_vcal = Pilot::PLUGIN_API;

}




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
	FUNCTIONSETUP;
	if (!fCalendar) return 0;
	fAllEvents = fCalendar->events();
	fAllEvents.setAutoDelete(false);
	return fAllEvents.count();
}


void VCalConduitPrivate::removeIncidence(KCal::Incidence *e)
{
	// use dynamic_cast which returns a null pointer if the class does not match...
	fAllEvents.remove(dynamic_cast<KCal::Event*>(e));
	if (!fCalendar) return;
	fCalendar->deleteEvent(dynamic_cast<KCal::Event*>(e));
	// now just in case we're in the middle of reading through our list
	// and we delete something, set reading to false so we start at the
	// top again next time and don't have problems with our iterator
	reading = false;
}


KCal::Incidence *VCalConduitPrivate::findIncidence(recordid_t id)
{
	KCal::Event::List::ConstIterator it;
	for( it = fAllEvents.begin(); it != fAllEvents.end(); ++it ) {
		KCal::Event *event = *it;
		if ((recordid_t)event->pilotId() == id) return event;
	}
	return 0L;
}

KCal::Incidence *VCalConduitPrivate::findIncidence(PilotRecordBase *tosearch)
{
	PilotDateEntry*entry=dynamic_cast<PilotDateEntry*>(tosearch);
	if (!entry) return 0L;

	QString title=entry->getDescription();
	QDateTime dt=readTm( entry->getEventStart() );

	KCal::Event::List::ConstIterator it;
	for( it = fAllEvents.begin(); it != fAllEvents.end(); ++it ) {
		KCal::Event *event = *it;
		if ( (event->dtStart() == dt) && (event->summary() == title) ) return event;
	}
	return 0L;
}



KCal::Incidence *VCalConduitPrivate::getNextIncidence()
{
	FUNCTIONSETUP;

	if (reading) {
		++fAllEventsIterator;
	} else {
		reading=true;
		fAllEventsIterator = fAllEvents.begin();
	}
	// At end of list, or empty list.
	return (fAllEventsIterator == fAllEvents.end()) ? 0L : *fAllEventsIterator;
}

/** Find the next incidence in the list which ddoes not have the SYNCNONE flag set. The
 *  current position is always stored in the iteratoor fAllEventsIterator, so we can just
 *  start from there. Only if reading==false, we haven't yet started goind through the
 *  incidents, so start at fAllEvents.begin() in that case */
KCal::Incidence *VCalConduitPrivate::getNextModifiedIncidence()
{
	FUNCTIONSETUP;
	KCal::Event*e=0L;
	if (!reading)
	{
		// Start from the top
		reading=true;
		fAllEventsIterator = fAllEvents.begin();
	}
	else
	{
		// Move on from current position
		++fAllEventsIterator;
	}

	// Fetch (new) current if possible.
	if ( fAllEventsIterator != fAllEvents.end() ) e = *fAllEventsIterator;
	// Then walk the list until we find an unsynced entry
	while ( fAllEventsIterator != fAllEvents.end() &&
		e && e->syncStatus()!=KCal::Incidence::SYNCMOD && e->pilotId() > 0)
	{
		e = (++fAllEventsIterator != fAllEvents.end()) ? *fAllEventsIterator : 0L;
	}
	return (fAllEventsIterator == fAllEvents.end()) ? 0L : 	*fAllEventsIterator;
}



/****************************************************************************
 *                          VCalConduit class                               *
 ****************************************************************************/

VCalConduit::VCalConduit(KPilotLink *d,
	const char *n,
	const QStringList &a) : VCalConduitBase(d,n,a)
{
	FUNCTIONSETUP;
	fConduitName=i18n("Calendar");
}


VCalConduit::~VCalConduit()
{
//	FUNCTIONSETUP;
}

VCalConduitPrivateBase* VCalConduit::newVCalPrivate(KCal::Calendar *fCalendar) {
	return new VCalConduitPrivate(fCalendar);
}

void VCalConduit::_getAppInfo()
{
	FUNCTIONSETUP;
	// get the address application header information
	unsigned char *buffer =
		new unsigned char[Pilot::MAX_APPINFO_SIZE];
	int appLen = fDatabase->readAppBlock(buffer,Pilot::MAX_APPINFO_SIZE);

	unpack_AppointmentAppInfo(&fAppointmentAppInfo, buffer, appLen);
	delete[]buffer;
	buffer = NULL;

#ifdef DEBUG
	DEBUGCONDUIT << fname << " lastUniqueId"
		<< fAppointmentAppInfo.category.lastUniqueID << endl;
#endif
	for (int i = 0; i < 16; i++)
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << " cat " << i << " =" <<
			fAppointmentAppInfo.category.name[i] << endl;
#endif
	}

}

const QString VCalConduit::getTitle(PilotRecordBase *de)
{
	PilotDateEntry*d=dynamic_cast<PilotDateEntry*>(de);
	if (d) return QString(d->getDescription());
	return QString::null;
}



PilotRecord*VCalConduit::recordFromIncidence(PilotRecordBase *de, const KCal::Incidence*e)
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
	if (e->secrecy()!=KCal::Event::SecrecyPublic) de->setSecret( true );

	setStartEndTimes(de, e);
	setAlarms(de, e);
	setRecurrence(de, e);
	setExceptions(de, e);
	de->setDescription(e->summary());
	de->setNote(e->description());
	de->setLocation(e->location());
	setCategory(de, e);
DEBUGCONDUIT<<"-------- "<<e->summary()<<endl;
	return de->pack();
}


KCal::Incidence *VCalConduit::incidenceFromRecord(KCal::Incidence *e, const PilotRecordBase  *de)
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

	e->setSyncStatus(KCal::Incidence::SYNCNONE);
	e->setSecrecy(de->isSecret() ?
		KCal::Event::SecrecyPrivate :
		KCal::Event::SecrecyPublic);

	e->setPilotId(de->id());
#ifdef DEBUG
		DEBUGCONDUIT<<fname<<": set KCal item to pilotId: [" << e->pilotId() << "]..."<<endl;
#endif
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
	e->setLocation(de->getLocation());

	// used by e.g. Agendus and Datebk
	setCategory(e, de);

	return e;
}


void VCalConduit::setStartEndTimes(KCal::Event *e,const PilotDateEntry *de)
{
	FUNCTIONSETUP;
	e->setDtStart(readTm(de->getEventStart()));
#ifdef DEBUG
	DEBUGCONDUIT<<"Start time on Palm: "<<readTm(de->getEventStart()).toString()<<", on PC: "<<e->dtStart().toString()<<endl;
#endif
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


void VCalConduit::setAlarms(KCal::Event *e, const PilotDateEntry *de)
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


void VCalConduit::setRecurrence(PilotDateEntry*dateEntry, const KCal::Event *event)
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
		emit logMessage(i18n("Event \"%1\" has a yearly recurrence other than by month, will change this to recurrence by month on handheld.").arg(event->summary()));
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
	vevent->recurrence()->setExDates(dl);
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


void VCalConduit::setCategory(PilotDateEntry *de, const KCal::Event *e)
{
	FUNCTIONSETUP;

	if (!de || !e) return;

	QString cat = _getCat(e->categories(), de->getCategoryLabel());

#ifdef DEBUG
	DEBUGCONDUIT << fname << " setting KCal category: "
		<< "[" << de->getCategoryLabel() << "]"
		<< " to pilot category: "
		<< "[" << cat << "]"
		<< endl;
#endif

	de->setCategory(cat);
}

/*
 * _getCat returns the id of the category from the given categories list. 
 * If none of the categories exist on the handheld, the "Unfiled" category 
 * is used.
 */

QString VCalConduit::_getCat(const QStringList cats, const QString curr) const
{
	FUNCTIONSETUP;

	// Event has no categories
	if (cats.size()<1)
	{
		return QString::null;
	}

#ifdef DEBUG
	DEBUGCONDUIT << fname << "Looking for category [" << curr << "]" << endl;
	DEBUGCONDUIT << fname << "Looking in list [" << cats.join( "," ) << "]" << endl;
#endif

	// Category matches list already
	if (cats.contains(curr))
	{
		return curr;
	}

	// Since the current category is not in the list, instead look for a
	// category name from the handheld AppInfo that is in the list
	// of categories.
	for ( QStringList::ConstIterator it = cats.begin(); it != cats.end(); ++it )
	{
		// Odd, an empty category string.
		if ( (*it).isEmpty() )
		{
			continue;
		}

		for (unsigned int j=1; j<Pilot::CATEGORY_COUNT; j++)
		{
			QString catName = Pilot::fromPilot(fAppointmentAppInfo.category.name[j]);
			// This category from the event matches the AppInfo category?
			if ( !(*it).compare( catName ) )
			{
				return catName;
			}
		}
	}

	return QString::null;
}

void VCalConduit::setCategory(KCal::Event *e, const PilotDateEntry *de)
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


PilotRecordBase * VCalConduit::newPilotEntry(PilotRecord*r)
{
	if (r) return new PilotDateEntry(fAppointmentAppInfo,r);
	else return new PilotDateEntry(fAppointmentAppInfo);
}

KCal::Incidence* VCalConduit::newIncidence()
{
  return new KCal::Event;
}

static VCalConduitSettings *config_vcal = 0L;

VCalConduitSettings *VCalConduit::theConfig() {
	if (!config_vcal)
	{
		config_vcal = new VCalConduitSettings(CSL1("Calendar"));
	}

	return config_vcal;
}

VCalConduitSettings *VCalConduit::config() {
	return theConfig();
}



// vim: ts=4:sw=4:noexpandtab:

