/* calendarconduit.cc			KPilot
**
** Copyright (C) 2008 by Bertjan Broeksema <b.broeksema@kdemail.net>
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "calendarconduit.h"

#include <akonadi/collection.h>
#include <kcal/alarm.h>
#include <kcal/event.h>
#include <kcal/recurrence.h>

#include "idmapping.h"
#include "options.h"
#include "calendarakonadiproxy.h"
#include "calendarakonadirecord.h"
#include "calendarhhrecord.h"
#include "calendarhhdataproxy.h"
#include "calendarsettings.h"

#define boost_cast( a ) boost::dynamic_pointer_cast<KCal::Event, KCal::Incidence>( a )

class CalendarConduit::Private
{
public:
	Private()
	{
		fCollectionId = -1;
	}
	
	Akonadi::Collection::Id fCollectionId;
};

CalendarConduit::CalendarConduit( KPilotLink *o, const QVariantList &a )
	: RecordConduit( o, a, CSL1( "DatebookDB" ), CSL1( "Calendar Conduit" ) )
	, d( new CalendarConduit::Private )
{
}

CalendarConduit::~CalendarConduit()
{
	KPILOT_DELETE( d );
}

void CalendarConduit::loadSettings()
{
	FUNCTIONSETUP;
	
	CalendarSettings::self()->readConfig();
	d->fCollectionId = CalendarSettings::akonadiCollection();
}

bool CalendarConduit::initDataProxies()
{
	FUNCTIONSETUP;
	
	if( !fDatabase )
	{
		addSyncLogEntry( i18n( "Error: Handheld database is not loaded." ) );
		return false;
	}
	
	if( d->fCollectionId < 0 )
	{
		addSyncLogEntry( i18n( "Error: No valid akonadi collection configured." ) );
		return false;
	}
	
	// At this point we should be able to read the backup and handheld database.
	// However, it might be that Akonadi is not started.
	CalendarAkonadiProxy* tadp = new CalendarAkonadiProxy( fMapping );
	tadp->setCollectionId( d->fCollectionId );
	 
	fPCDataProxy = tadp;
	fHHDataProxy = new CalendarHHDataProxy( fDatabase );
	fHHDataProxy->loadAllRecords();
	fBackupDataProxy = new CalendarHHDataProxy( fLocalDatabase );
	fBackupDataProxy->loadAllRecords();
	fPCDataProxy->loadAllRecords();
	
	return true;
}

bool CalendarConduit::equal( const Record *pcRec, const HHRecord *hhRec ) const
{
	FUNCTIONSETUP;
	
	const CalendarAkonadiRecord* tar = static_cast<const CalendarAkonadiRecord*>( pcRec );
	const CalendarHHRecord* thr = static_cast<const CalendarHHRecord*>( hhRec );
	
	EventPtr pcEvent = boost_cast( tar->item().payload<IncidencePtr>() );
	PilotDateEntry hhEntry = thr->dateEntry();

	// A TEST define which immediatly returns when a TEST fails.
#define TEST( a, b, c ) { if( a != b ) { WARNINGKPILOT << CSL1( c ) << " not equal."; return false; } }
#define TEST1( a, b ) { if( !a ) { WARNINGKPILOT << CSL1( b ) << " was false."; return false; } }
	
	TEST( pcEvent->summary(), hhEntry.getDescription(), "Description" )
	TEST( pcEvent->description(), hhEntry.getNote(), "Note" )
	if( thr->category() != "Unfiled" )
	{
		TEST1( pcEvent->categories().contains( thr->category() ), "Category" )
	}
	TEST( pcEvent->allDay(), hhEntry.doesFloat() , "AllDay" )
	
	// Check start and end times only when the event does not float.
	if( !hhEntry.doesFloat() )
	{
		// Both entries do not float, so lets see if the start and end times are equal.
		TEST( pcEvent->dtStart().dateTime(), hhEntry.dtStart(), "dtStart" )
		TEST( pcEvent->dtEnd().dateTime(), hhEntry.dtEnd(), "dtEnd" )
	}
	else
	{
		DEBUGKPILOT << "ENTRY FLOATS:" << pcEvent->dtStart().dateTime().toString()
			<< hhEntry.dtStart().toString() << " " << thr->toString();
		// The records should be on the same date.
		TEST( pcEvent->dtStart().dateTime(), hhEntry.dtStart(), "DtStart" )
	}
	
	TEST( pcEvent->isAlarmEnabled(), hhEntry.isAlarmEnabled(), "HasAlarm" )
	/* TODO: Find out some way to check this, but do we really want to test for this?
	if( hhEntry.isAlarmEnabled() )
	{
		// Both entries have alarms enabled, lets see if they are set on the same time.
		TEST1( entryOther.dtAlarm(), entryThis.dtAlarm(), "dtAlarm" )
	}
	*/
	
	TEST( pcEvent->recurs(), (hhEntry.getRepeatType() != repeatNone), "Recurs" )
	
	if( pcEvent->recurs() )
	{
		KCal::Recurrence* recurrence = pcEvent->recurrence();
		
		// Both have the same repeat type set, lets see if the repeat settings are
		// also equal then.
		TEST( (recurrence->duration() == -1), hhEntry.getRepeatForever(), "DurationForever" )
		
		if( !hhEntry.getRepeatForever() )
		{
			// Both entries seem to repeat and end the repetition, so lets see if the
			// end time is equal.
			if( hhEntry.isMultiDay() )
			{
				KDateTime hhDt( readTm( hhEntry.getRepeatEnd() ), KDateTime::Spec::LocalZone() );
				KDateTime pcDt = pcEvent->dtEnd();
				TEST( hhDt, pcDt, "DtRepeatEnd" );
			}
			else
			{
				KDateTime hhDt( readTm( hhEntry.getEventEnd() ), KDateTime::Spec::LocalZone() );
				KDateTime pcDt = pcEvent->dtEnd();
				TEST( hhDt, pcDt, "DtEventEnd" );
			}
		}
		
		if( hhEntry.getRepeatType() == repeatMonthlyByDay )
		{
			TEST1( pcEvent->recurrenceType() == KCal::Recurrence::rMonthlyDay, "RepeatMonthlyByDay" )
			
			// TODO: Check the actual day
			//TEST( entryOther.getRepeatDay(), entryThis.getRepeatDay(), "getRepeatDay" )
		}
		
		if( hhEntry.getRepeatType() == repeatWeekly )
		{
			TEST1( pcEvent->recurrenceType() == KCal::Recurrence::rWeekly, "RepeatWeekly" )
			// When repeat type is weekly we should check if the days are the same for
			// both entries.
			QBitArray eventDays = recurrence->days();
			const int* entryDays = hhEntry.getRepeatDays();
			
			TEST( ( entryDays[0] != 0 ), eventDays.at( 6 ), "RepeatDays" )
			
			for( int i = 1; i < 7; i++ )
			{
				// NOTE: Does this work? I'm not sure if the days item is set to 0 if the
				// alarm is not set for that day. And I also don't know if they can be
				// expected to be exactly equal when the alarm is set for a specific day.
				TEST( (entryDays[i] != 0), eventDays.at( i - 1 ), "RepeatDays" )
			}
		}
		
		/*
		// TODO: My (Bertjan) handheld (palm m515) doesn't seem to support exceptions
		// so I'll leave it out for now but it should be something like this:
		exceptionCountEqual = entryOther.getExceptionCount() == entryThis.getExceptionCount();
		if( exceptionCountEqual && entryThis.getExceptionCount() > 0 )
		{
			// Both entries have the same number of exceptions, so lets check if the
			// exceptions are on the same moments.
			int count = entryThis.getExceptionCount();
			tm* thisExceptions = entryThis.getExceptions();
			tm* otherExceptions = entryOther.getExceptions();
			
			for( int i = 0; i < count; i++ )
			{
				exceptionEqual = exceptionEqual && readTm( otherExceptions[i] ) == readTm( thisExceptions[i] );
			}
		}
		*/
		// End of repeat stuff.
	}
	
#undef TEST
#undef TEST1
	// No test returned false so the records are equal.
	return true;
}

Record* CalendarConduit::createPCRecord( const HHRecord *hhRec )
{
	FUNCTIONSETUP;
	
	Akonadi::Item item;
	item.setPayload<IncidencePtr>( IncidencePtr( new KCal::Event() ) );
	item.setMimeType( "application/x-vnd.akonadi.calendar.event" );
		
	Record* rec = new CalendarAkonadiRecord( item, fMapping.lastSyncedDate() );
	copy( hhRec, rec );
	
	Q_ASSERT( equal( rec, hhRec ) );
	
	return rec;
}

HHRecord* CalendarConduit::createHHRecord( const Record *pcRec )
{
	FUNCTIONSETUP;
	
	HHRecord* hhRec = new CalendarHHRecord( PilotDateEntry().pack(), "Unfiled" );
	copy( pcRec, hhRec );
	
	Q_ASSERT( equal( pcRec, hhRec ) );
	
	return hhRec;
}

void CalendarConduit::_copy( const Record *from, HHRecord *to )
{
	FUNCTIONSETUP;
	
	const CalendarAkonadiRecord* tar = static_cast<const CalendarAkonadiRecord*>( from );
	CalendarHHRecord* thr = static_cast<CalendarHHRecord*>( to );
	
	PilotDateEntry hhTo = thr->dateEntry();
	EventPtr pcFrom = boost_cast( tar->item().payload<IncidencePtr>() );
	
	if( ( pcFrom->recurrenceType() == KCal::Recurrence::rYearlyDay ) ||
		( pcFrom->recurrenceType() ==  KCal::Recurrence::rYearlyPos ) )
	{
		// Warn ahead of time
		QString message( "Event \"%1\" has a yearly recurrence other than by month, " );
		message += CSL1( "will change this to recurrence by month on handheld." );
		
		emit logMessage( i18n( message.toLatin1(), pcFrom->summary() ) );
	}
	
	// set secrecy, start/end times, alarms, recurrence, exceptions, summary and description:
	if( pcFrom->secrecy() != KCal::Event::SecrecyPublic )
	{
		hhTo.setSecret( true );
	}

	setStartEndTimes( &hhTo, pcFrom );
	setAlarms( &hhTo, pcFrom );
	setRecurrence( &hhTo, pcFrom );
	setExceptions( &hhTo, pcFrom );
	
	hhTo.setDescription( pcFrom->summary() );
	hhTo.setNote( pcFrom->description() );
	hhTo.setLocation( pcFrom->location() );
	
	thr->setDateEntry( hhTo );
}

void CalendarConduit::_copy( const HHRecord* from, Record *to  )
{
	FUNCTIONSETUP;
	
	const CalendarHHRecord* thr = static_cast<const CalendarHHRecord*>( from );
	CalendarAkonadiRecord* tar = static_cast<CalendarAkonadiRecord*>( to );
	
	PilotDateEntry hhFrom = thr->dateEntry();
	EventPtr pcTo = boost_cast( tar->item().payload<IncidencePtr>() );
	
	pcTo->setSecrecy( hhFrom.isSecret() ?
		KCal::Event::SecrecyPrivate :
		KCal::Event::SecrecyPublic );
	
	setStartEndTimes( pcTo, hhFrom );
	setAlarms( pcTo, hhFrom );
	setRecurrence( pcTo, hhFrom );
	setExceptions( pcTo, hhFrom );

	pcTo->setSummary( hhFrom.getDescription() );
	pcTo->setDescription( hhFrom.getNote() );
	pcTo->setLocation( hhFrom.getLocation() );
}

void CalendarConduit::setAlarms( PilotDateEntry* de, const EventPtr& e ) const
{
	FUNCTIONSETUP;

	if( !de || !e )
	{
		DEBUGKPILOT << "NULL entry given to setAlarms.";
		return;
	}

	if( !e->isAlarmEnabled() )
	{
		de->setAlarmEnabled( false );
		return;
	}

	// find the first enabled alarm
	KCal::Alarm::List alms = e->alarms();
	const KCal::Alarm* alm = 0;
	
	foreach( const KCal::Alarm* alarm, alms )
	{
		if( alarm->enabled() ) alm = alarm;
	}

	if (!alm )
	{
		DEBUGKPILOT << "no enabled alarm found (should exist!!!)";
		de->setAlarmEnabled( false );
		return;
	}

	// palm and PC offsets have a different sign!!
	int aoffs = -alm->startOffset().asSeconds() / 60;
	int offs = (aoffs > 0) ? aoffs : -aoffs;

	// find the best Advance Unit
	if( offs >= 100 || offs == 60 )
	{
		offs /= 60;
		if( offs >= 48 || offs == 24 )
		{
			offs /= 24;
			de->setAdvanceUnits( advDays );
		}
		else
		{
			de->setAdvanceUnits( advHours );
		}
	}
	else
	{
		de->setAdvanceUnits( advMinutes );
	}
	
	de->setAdvance( (aoffs > 0) ? offs : -offs );
	de->setAlarmEnabled( true );
}

void CalendarConduit::setAlarms( EventPtr e, const PilotDateEntry& de ) const
{
	FUNCTIONSETUP;

	if( !e ) return;
	
	// Delete all the alarms now and add them one by one later on.
	e->clearAlarms();
	if( !de.isAlarmEnabled() ) return;
	
	int advanceUnits = de.getAdvanceUnits();

	switch( advanceUnits )
	{
	case advMinutes:
		advanceUnits = 1;
		break;
	case advHours:
		advanceUnits = 60;
		break;
	case advDays:
		advanceUnits = 60 * 24;
		break;
	default:
		WARNINGKPILOT << "Unknown advance units " << advanceUnits;
		advanceUnits = 1;
	}

	KCal::Duration adv( -60 * advanceUnits * de.getAdvance() );
	KCal::Alarm* alm = e->newAlarm();
	if( !alm ) return;

	alm->setStartOffset( adv );
	alm->setEnabled( true );
}

void CalendarConduit::setExceptions( PilotDateEntry* de, const EventPtr& e ) const
{
	FUNCTIONSETUP;
	struct tm* ex_List;
	
	if( !de || !e )
	{
		DEBUGKPILOT << "NULL entry given to setExceptions.";
		return;
	}
	
	KCal::DateList exDates = e->recurrence()->exDates();
	size_t excount = exDates.size();
	if (excount<1)
	{
		de->setExceptionCount( 0 );
		de->setExceptions( 0 );
		return;
	}

	// we have exceptions, so allocate mem and copy them there...
	ex_List = new struct tm[excount];
	if( !ex_List )
	{
		WARNINGKPILOT << "Couldn't allocate memory for the exceptions";
		de->setExceptionCount( 0 );
		de->setExceptions( 0 );
		return;
	}
	
	size_t n = 0;
	
	foreach( const QDate& dt, exDates )
	{
		struct tm ttm = writeTm( dt );
		ex_List[n++] = ttm;
	}
	
	de->setExceptionCount( excount );
	de->setExceptions( ex_List );
}

void CalendarConduit::setExceptions( EventPtr e, const PilotDateEntry& de ) const
{
	FUNCTIONSETUP;

	// Start from an empty exception list, and if necessary, add exceptions.
	// At the end of the function, apply the (possibly empty) exception list.
	KCal::DateList dl;

	if( !( de.isMultiDay() ) && de.getExceptionCount() > 0 )
	{
		for( int i = 0; i < de.getExceptionCount(); i++ )
		{
			dl.append( readTm( de.getExceptions()[i] ).date() );
		}
	}
	else
	{
		return;
	}
	
	e->recurrence()->setExDates(dl);
}

void CalendarConduit::setRecurrence( PilotDateEntry* de, const EventPtr& e ) const
{
	FUNCTIONSETUP;
	
	if( !de || !e )
	{
		DEBUGKPILOT << "NULL entry given to setRecurrence.";
		return;
	}
	
	bool isMultiDay = false;

	// first we have 'fake type of recurrence' when a multi-day event is passed to
	// the pilot, it is converted to an event which recurs daily a number of times.
	// if the event itself recurs, this will be overridden, and only the first day
	// will be included in the event!!!!
	
	QDateTime startDt( readTm( de->getEventStart() ) );
	QDateTime endDt( readTm( de->getEventEnd() ) );
	
	if( startDt.daysTo( endDt ) )
	{
		isMultiDay = true;
		de->setRepeatType( repeatDaily );
		de->setRepeatFrequency( 1 );
		de->setRepeatEnd( de->getEventEnd() );
		
		DEBUGKPILOT << "Setting single-day recurrence (" << startDt.toString()
			<< " - " << endDt.toString() << ")";
	}
	
	KCal::Recurrence* r = e->recurrence();
	
	if( !r ) return;
	
	ushort recType = r->recurrenceType();
	if( recType == KCal::Recurrence::rNone )
	{
		if( !isMultiDay ) de->setRepeatType( repeatNone );
		return;
	}

	int freq = r->frequency();
	QDate endDate = r->endDate();

	if( r->duration() < 0 || !endDate.isValid() )
	{
		de->setRepeatForever();
	}
	else
	{
		de->setRepeatEnd(writeTm(endDate));
	}
	de->setRepeatFrequency(freq);
	
	DEBUGKPILOT << "Event: " << e->summary() << " (" << e->description() << ")";
	DEBUGKPILOT << "duration:" << r->duration() << ", endDate:" 
		<< endDate.toString() << ", ValidEndDate: " << endDate.isValid() 
		<< ", NullEndDate:" << endDate.isNull();

	QBitArray dayArray(7);
	QBitArray dayArrayPalm(7);
	
	switch( recType )
	{
	case KCal::Recurrence::rDaily:
		de->setRepeatType( repeatDaily );
		break;
		
	case KCal::Recurrence::rWeekly:
		de->setRepeatType( repeatWeekly );
		dayArray = r->days();
		
		// rotate the bits by one
		for (int i=0; i<7; i++)
		{
			dayArrayPalm.setBit( (i+1) % 7, dayArray[i] );
		}
		
		de->setRepeatDays( dayArrayPalm );
		break;
		
	case KCal::Recurrence::rMonthlyPos:
		// Palm: Day=0(sun)-6(sat); week=0-4, 4=last week; pos=week*7+day
		// libkcal: day=bit0(mon)-bit6(sun); week=-5to-1(from end) and 1-5 (from beginning)
		// PC->Palm: pos=week*7+day
		//  week: if w=-1 -> week=4, else week=w-1
		//  day: day=(daybit+1)%7  (rotate because of the different offset)
		de->setRepeatType( repeatMonthlyByDay );
		if (r->monthPositions().count()>0)
		{
			// Only take the first monthly position, as the palm allows only one
			KCal::RecurrenceRule::WDayPos mp = r->monthPositions().first();
			int week = mp.pos();
			int day = ( mp.day() + 1 ) % 7; // rotate because of different offset
			// turn to 0-based and include starting from end of month
			// TODO: We don't handle counting from the end of the month yet!
			if( week == -1 ) week = 4; else week--;
			de->setRepeatDay( static_cast<DayOfMonthType>( 7 * week + day ) );
		}
		break;
		
	case KCal::Recurrence::rMonthlyDay:
		de->setRepeatType( repeatMonthlyByDate );
		//TODO: is this needed?
		//dateEntry->setRepeatDay( static_cast<DayOfMonthType>( startDt.day() ) );
		break;
		
	case KCal::Recurrence::rYearlyDay:
	case KCal::Recurrence::rYearlyPos:
		WARNINGKPILOT << "Unsupported yearly recurrence type.";
		break;
		
	case KCal::Recurrence::rYearlyMonth:
		de->setRepeatType( repeatYearly );
		break;
		
	case KCal::Recurrence::rNone:
		if( !isMultiDay ) de->setRepeatType( repeatNone );
		break;
		
	default:
		WARNINGKPILOT << "Unknown recurrence type " << recType
			<<" with frequency " << freq
			<< " and duration " << r->duration();
		break;
	}
}

void CalendarConduit::setRecurrence( EventPtr e, const PilotDateEntry& de ) const
{
	FUNCTIONSETUP;

	if( ( de.getRepeatType() == repeatNone) || de.isMultiDay() )
	{
		DEBUGKPILOT << "No recurrence to set.";
		return;
	}

	KCal::Recurrence *recur = e->recurrence();
	int freq = de.getRepeatFrequency();
	bool repeatsForever = de.getRepeatForever();
	QDate endDate;
	QDate evt;

	if( !repeatsForever )
	{
		endDate = readTm( de.getRepeatEnd() ).date();
	}

	QBitArray dayArray(7);

	switch( de.getRepeatType() )
	{
		case repeatDaily:
			recur->setDaily( freq );
			break;
		
		case repeatWeekly:
		{
			const int *days = de.getRepeatDays();
	
			DEBUGKPILOT << "Got repeat-weekly entry, by-days = "
				<< days[0] << " " << days[1] << " " << days[2] << " "
				<< days[3] << " " << days[4] << " " << days[5] << " " << days[6];
	
			// Rotate the days of the week, since day numbers on the Pilot and
			// in vCal / Events are different.
			//
			if( days[0] ) dayArray.setBit( 6 );
			
			for( int i = 1; i < 7; i++ )
			{
				if( days[i] )
				{
					dayArray.setBit( i - 1 );
				}
			}
			recur->setWeekly( freq, dayArray );
		}
		break;
		
		case repeatMonthlyByDay:
		{
			// Palm: Day=0(sun)-6(sat); week=0-4, 4=last week; pos=week*7+day
			// libkcal: day=bit0(mon)-bit6(sun); week=-5to-1(from end) and 1-5 (from beginning)
			// Palm->PC: w=pos/7
			// week: if w=4 -> week=-1, else week=w+1;
			// day: day=(pos-1)%7 (rotate by one day!)
			recur->setMonthly( freq );
	
			int day = de.getRepeatDay();
			int week = day / 7;
			// week=4 means last, otherwise convert to 0-based
			if( week == 4 ) week = -1; else week++;
			dayArray.setBit( ( day + 6 ) % 7 );
			recur->addMonthlyPos( week, dayArray );
			break;
		}
	
		case repeatMonthlyByDate:
			recur->setMonthly( freq );
			recur->addMonthlyDate( de.getEventStart().tm_mday );
			break;
		
		case repeatYearly:
			recur->setYearly( freq );
			evt = readTm( de.getEventStart() ).date();
			recur->addYearlyMonth( evt.month() );
			break;
		
		case repeatNone:
		default :
			WARNINGKPILOT << "Can't handle repeat type " << de.getRepeatType();
			break;
	}
	
	if( !repeatsForever )
	{
		recur->setEndDate( endDate );
	}
}

void CalendarConduit::setStartEndTimes( PilotDateEntry* de, const EventPtr& e ) const
{
	FUNCTIONSETUP;
	
	if( !de || !e )
	{
		DEBUGKPILOT << "NULL entry given to setStartEndTimes.";
		return;
	}
	
	struct tm ttm = writeTm( e->dtStart().dateTime() );
	de->setEventStart( ttm );
	de->setFloats( e->allDay() );
	
	if( e->hasEndDate() && e->dtEnd().isValid() )
	{
		ttm = writeTm( e->dtEnd().dateTime() );
	}
	else
	{
		ttm = writeTm( e->dtStart().dateTime() );
	}
	
	de->setEventEnd( ttm );
}

void CalendarConduit::setStartEndTimes( EventPtr e, const PilotDateEntry& de ) const
{
	FUNCTIONSETUP;

	e->setDtStart( KDateTime( readTm( de.getEventStart() ), KDateTime::Spec::LocalZone() ) );
	e->setAllDay( de.isEvent() );

	if( de.isMultiDay() )
	{
		e->setDtEnd( KDateTime( readTm( de.getRepeatEnd() ), KDateTime::Spec::LocalZone() ) );
	}
	else
	{
		e->setDtEnd( KDateTime( readTm( de.getEventEnd() ), KDateTime::Spec::LocalZone() ) );
	}
}

#undef boost_cast
