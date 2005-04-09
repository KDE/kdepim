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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"
#include <qtextcodec.h>
#include <libkcal/calendar.h>
#include <libkcal/recurrence.h>
#define Recurrence_t KCal::Recurrence
#include <pilotDateEntry.h>
#include <pilotDatabase.h>
#include "vcal-conduit.moc"
#include "vcal-factory.h"

extern "C"
{

long version_conduit_vcal = KPILOT_PLUGIN_API;
const char *id_conduit_vcal = "$Id$";

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

KCal::Incidence *VCalConduitPrivate::findIncidence(PilotAppCategory*tosearch)
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
		e && e->syncStatus()==KCal::Incidence::SYNCNONE && e->pilotId() > 0)
	{
		++fAllEventsIterator;
		e=*fAllEventsIterator;
	}
	return (fAllEventsIterator == fAllEvents.end()) ? 0L : 	*fAllEventsIterator;
}



/****************************************************************************
 *                          VCalConduit class                               *
 ****************************************************************************/

VCalConduit::VCalConduit(KPilotDeviceLink *d,
	const char *n,
	const QStringList &a) : VCalConduitBase(d,n,a)
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGCONDUIT << id_conduit_vcal << endl;
#endif
	fConduitName=i18n("Calendar");
	(void) id_conduit_vcal;
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
		new unsigned char[PilotDateEntry::APP_BUFFER_SIZE];
	int appLen = fDatabase->readAppBlock(buffer,PilotDateEntry::APP_BUFFER_SIZE);

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

const QString VCalConduit::getTitle(PilotAppCategory*de)
{
	PilotDateEntry*d=dynamic_cast<PilotDateEntry*>(de);
	if (d) return QString(d->getDescription());
	return QString::null;
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
	setCategory(de, e);
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

   // We don't want this, do we?
//	e->setOrganizer(fCalendar->getEmail());
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
		de->setAlarm(0);
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
		de->setAlarm(0);
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
	QDate endDate, evt;

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
	case repeatMonthlyByDay: {
		// Palm: Day=0(sun)-6(sat); week=0-4, 4=last week; pos=week*7+day
		// libkcal: day=bit0(mon)-bit6(sun); week=-5to-1(from end) and 1-5 (from beginning)
		// Palm->PC: w=pos/7
		// week: if w=4 -> week=-1, else week=w+1;
		// day: day=(pos-1)%7 (rotate by one day!)
		if (repeatsForever)
		{
			recur->setMonthly(Recurrence_t::rMonthlyPos,freq,-1);
		}
		else
		{
			recur->setMonthly(Recurrence_t::rMonthlyPos,freq,endDate);
		}

		int day=dateEntry->getRepeatDay();
		int week=day/7;
		// week=4 means last, otherwise convert to 0-based
		if (week==4) week=-1; else week++;
		dayArray.setBit((day+6) % 7);
		recur->addMonthlyPos(week, dayArray);
		break;}
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
			recur->setYearly(Recurrence_t::rYearlyMonth,freq,-1);
		}
		else
		{
			recur->setYearly(Recurrence_t::rYearlyMonth,freq,endDate);
		}
		evt=readTm(dateEntry->getEventStart()).date();
		recur->addYearlyNum( evt.month() );
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
		// Palm: Day=0(sun)-6(sat); week=0-4, 4=last week; pos=week*7+day
		// libkcal: day=bit0(mon)-bit6(sun); week=-5to-1(from end) and 1-5 (from beginning)
		// PC->Palm: pos=week*7+day
		//  week: if w=-1 -> week=4, else week=w-1
		//  day: day=(daybit+1)%7  (rotate because of the different offset)
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
			int week=mp->rPos;
			if (mp->negative) week*=-1;
			int day=(pos+1) % 7; // rotate because of different offset
			// turn to 0-based and include starting from end of month
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


void VCalConduit::setCategory(PilotDateEntry *de, const KCal::Event *e)
{
	if (!de || !e) return;
	de->setCategory(_getCat(e->categories(), de->getCategoryLabel()));
}

/**
 * _getCat returns the id of the category from the given categories list. If none of the categories exist
 * on the palm, the "Nicht abgelegt" (don't know the english name) is used.
 */

QString VCalConduit::_getCat(const QStringList cats, const QString curr) const
{
	int j;
	if (cats.size()<1) return QString::null;
	if (cats.contains(curr)) return curr;
	for ( QStringList::ConstIterator it = cats.begin(); it != cats.end(); ++it ) {
		for (j=1; j<=15; j++)
		{
			QString catName = PilotAppCategory::codec()->
			  toUnicode(fAppointmentAppInfo.category.name[j]);
			if (!(*it).isEmpty() && !(*it).compare( catName ) )
			{
				return catName;
			}
		}
	}
	// If we have a free label, return the first possible cat
	QString lastName(QString::fromLatin1(fAppointmentAppInfo.category.name[15]));
	if (lastName.isEmpty()) return cats.first();
	return QString::null;
}

void VCalConduit::setCategory(KCal::Event *e, const PilotDateEntry *de)
{
	if (!e || !de) return;
	QStringList cats=e->categories();
	int cat=de->getCat();
	if (0<cat && cat<=15)
	{
		QString newcat=PilotAppCategory::codec()->toUnicode(fAppointmentAppInfo.category.name[cat]);
		if (!cats.contains(newcat))
		{
			cats.append( newcat );
			e->setCategories(cats);
		}
	}
}


PilotAppCategory*VCalConduit::newPilotEntry(PilotRecord*r)
{
	if (r) return new PilotDateEntry(fAppointmentAppInfo,r);
	else return new PilotDateEntry(fAppointmentAppInfo);
}

KCal::Incidence*VCalConduit::newIncidence()
{
  return new KCal::Event;
}

VCalConduitSettings *VCalConduit::config() {
  return VCalConduitFactory::config();
}
