/* calendarhhrecord.cc			KPilot
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

#include "calendarhhrecord.h"

#include <QtCore/QDateTime>

#include "options.h"

CalendarHHRecord::CalendarHHRecord( PilotRecord *record, const QString &category )
	: HHRecord( record, category )
{
	FUNCTIONSETUPL(5);
	DEBUGKPILOT << "id: [" << id() << "], description: [" << toString() << "]";

}

PilotDateEntry CalendarHHRecord::dateEntry() const
{
	FUNCTIONSETUP;
	
	return PilotDateEntry( fRecord );
}

QString CalendarHHRecord::description() const
{
	return dateEntry().getDescription();
}

bool CalendarHHRecord::equal( const HHRecord* other ) const
{
	FUNCTIONSETUP;
	
	const CalendarHHRecord* hhOther = static_cast<const CalendarHHRecord*>( other );

	// As soon as a test fails it will return and print a message which one failed.
#define TEST( a, b, c ) { if( a != b ) { DEBUGKPILOT << CSL1( c ) << " not equal"; return false; } }

	PilotDateEntry entryOther = hhOther->dateEntry();
	PilotDateEntry entryThis = dateEntry();
	
	TEST( entryOther.doesFloat(), entryThis.doesFloat(), "doesFloat" )
	
	// Check start and end times only when the event does not float.
	if( !entryThis.doesFloat() )
	{
		// Both entries do not float, so lets see if the start and end times are equal.
		TEST( entryOther.dtStart(), entryThis.dtStart(), "dtStart" )
		TEST( entryOther.dtEnd(), entryThis.dtEnd(), "dtStart" )
	}
	
	TEST( entryOther.isAlarmEnabled(), entryThis.isAlarmEnabled(), "isAlarmEnabled" )
	
	if( entryThis.isAlarmEnabled() )
	{
		// Both entries have alarms enabled, lets see if they are set on the same time.
		TEST( entryOther.dtAlarm(), entryThis.dtAlarm(), "dtAlarm" )
	}
	
	TEST( entryOther.getRepeatType(), entryThis.getRepeatType(), "getRepeatType" )
	
	if( entryThis.getRepeatType() != repeatNone )
	{
		// Both have the same repeat type set, lets see if the repeat settings are
		// also equal then.
		TEST( entryOther.getRepeatForever(), entryThis.getRepeatForever(), "getRepeatForever" )
		
		if( !entryOther.getRepeatForever() )
		{
			// Both entries seem to repeat and end the repetition, so lets see if the
			// end time is equal.
			TEST( entryOther.dtRepeatEnd(), entryThis.dtRepeatEnd(), "dtRepeatEnd" )
		}
		
		if( entryThis.getRepeatType() == repeatMonthlyByDay )
		{
			// When repeat type is repeatMonthlyByDay then repeatDay should be equal.
			TEST( entryOther.getRepeatDay(), entryThis.getRepeatDay(), "getRepeatDay" )
		}
		
		if( entryOther.getRepeatType() == repeatWeekly )
		{
			// When repeat type is weekly we should check if the days are the same for
			// both entries.
			const int* otherDays = entryOther.getRepeatDays();
			const int* thisDays = entryOther.getRepeatDays();
			
			for( int i = 0; i < 7; ++i )
			{
				// NOTE: Does this work? I'm not sure if the days item is set to 0 if the
				// alarm is not set for that day. And I also don't know if they can be
				// expected to be exactly equal when the alarm is set for a specific day.
				TEST( (otherDays[i] == 0), (thisDays[i] == 0), "getRepeatDays" )
			}
		}
		
		// TODO: My (Bertjan) handheld (palm m515) doesn't seem to support exceptions
		// so I'll leave it out for now but it should be something like this:
		/*
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
	
	TEST( entryOther.getDescription(), entryThis.getDescription(), "getDescription" )
	TEST( entryOther.getNote(), entryThis.getNote(), "getNote" )

#undef TEST
	// Everything passed it seems so the records should be equal.
	return true;
}

void CalendarHHRecord::setDateEntry( const PilotDateEntry& entry, bool keepPrevCategory )
{
	FUNCTIONSETUP;
	
	PilotRecord* record = entry.pack();
	
	if( keepPrevCategory )
	{
		record->setCategory( fRecord->category() );
	}
	
	KPILOT_DELETE( fRecord );
	fRecord = record;
}

QString CalendarHHRecord::toString() const
{
	PilotDateEntry de = dateEntry();
	QString rs = id();
	rs += CSL1( ":" ) + de.getDescription();
	rs += CSL1( ":" ) + readTm( de.getEventStart() ).toString();
	return rs;
}
