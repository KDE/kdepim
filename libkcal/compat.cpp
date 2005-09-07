/*
    This file is part of libkcal.

    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "compat.h"

#include <kdebug.h>

#include <qregexp.h>

#include "incidence.h"

using namespace KCal;

Compat *CompatFactory::createCompat( const QString &productId )
{
//  kdDebug(5800) << "CompatFactory::createCompat(): '" << productId << "'"
//                << endl;

  Compat *compat = 0;

  int korg = productId.find( "KOrganizer" );
  int outl9 = productId.find( "Outlook 9.0" );
//   int kcal = productId.find( "LibKCal" );

  // TODO: Use the version of LibKCal to determine the compat class...
  if ( korg >= 0 ) {
    int versionStart = productId.find( " ", korg );
    if ( versionStart >= 0 ) {
      int versionStop = productId.find( QRegExp( "[ /]" ), versionStart + 1 );
      if ( versionStop >= 0 ) {
        QString version = productId.mid( versionStart + 1,
                                         versionStop - versionStart - 1 );
//        kdDebug(5800) << "Found KOrganizer version: " << version << endl;

        int versionNum = version.section( ".", 0, 0 ).toInt() * 10000 +
                         version.section( ".", 1, 1 ).toInt() * 100 +
                         version.section( ".", 2, 2 ).toInt();
        int releaseStop = productId.find( "/", versionStop );
        QString release;
        if ( releaseStop > versionStop ) {
          release = productId.mid( versionStop+1, releaseStop-versionStop-1 );
        }
//        kdDebug(5800) << "KOrganizer release: \"" << release << "\"" << endl;

//        kdDebug(5800) << "Numerical version: " << versionNum << endl;

        if ( versionNum < 30100 ) {
          compat = new CompatPre31;
        } else if ( versionNum < 30200 ) {
          compat = new CompatPre32;
        } else if ( versionNum == 30200 && release == "pre" ) {
          kdDebug(5800) << "Generating compat for KOrganizer 3.2 pre " << endl;
          compat = new Compat32PrereleaseVersions;
        } else if ( versionNum < 30400 ) {
          compat = new CompatPre34;
        } else if ( versionNum < 30500 ) {
          compat = new CompatPre35;
        }
      }
    }
  } else if ( outl9 >= 0 ) {
    kdDebug(5800) << "Generating compat for Outlook < 2000 (Outlook 9.0)" << endl;
    compat = new CompatOutlook9;
  }

  if ( !compat ) compat = new Compat;

  return compat;
}

void Compat::fixEmptySummary( Incidence *incidence )
{
  // some stupid vCal exporters ignore the standard and use Description
  // instead of Summary for the default field. Correct for this: Copy the
  // first line of the description to the summary (if summary is just one
  // line, move it)
  if (incidence->summary().isEmpty() &&
      !(incidence->description().isEmpty())) {
    QString oldDescription = incidence->description().stripWhiteSpace();
    QString newSummary( oldDescription );
    newSummary.remove( QRegExp("\n.*") );
    incidence->setSummary( newSummary );
    if ( oldDescription == newSummary )
      incidence->setDescription("");
  }
}

void Compat::fixRecurrence( Incidence */*incidence*/ )
{
  // Prevent use of compatibility mode during subsequent changes by the application
//  incidence->recurrence()->setCompatVersion();
}

/** Before kde 3.5, the start date was not automatically a recurring date. So
    if the start date doesn't match the recurrence rule, we need to add an ex
    date for the date start. If a duration was given, the DTSTART was only counted
    if it matched, so by accident this was already the correct behavior, so
    we don't need to adjust the duration... */
void CompatPre35::fixRecurrence( Incidence *incidence )
{
  Recurrence* recurrence = incidence->recurrence();
  if (recurrence ) {
    QDateTime start( incidence->dtStart() );
    // kde < 3.5 only had one rrule, so no need to loop over all RRULEs.
    RecurrenceRule *r = recurrence->defaultRRule();
    if ( r && !r->dateMatchesRules( start )  ) {
      recurrence->addExDateTime( start );
    }
  }

  // Call base class method now that everything else is done
  Compat::fixRecurrence( incidence );
}

int CompatPre34::fixPriority( int prio )
{
  if ( 0<prio && prio<6 ) {
    // adjust 1->1, 2->3, 3->5, 4->7, 5->9
    return 2*prio - 1;
  } else return prio;
}

/** The recurrence has a specified number of repetitions.
    Pre-3.2, this was extended by the number of exception dates.
    This is also rfc 2445-compliant. The duration of an RRULE also counts
    events that are later excluded via EXDATE or EXRULE. */
void CompatPre32::fixRecurrence( Incidence *incidence )
{
  Recurrence* recurrence = incidence->recurrence();
  if ( recurrence->doesRecur() &&  recurrence->duration() > 0 ) {
    recurrence->setDuration( recurrence->duration() + incidence->recurrence()->exDates().count() );
  }
  // Call base class method now that everything else is done
  CompatPre35::fixRecurrence( incidence );
}

/** Before kde 3.1, floating events (events without a date) had 0:00 of their
    last day as the end date. E.g. 28.5.2005  0:00 until 28.5.2005 0:00 for an
    event that lasted the whole day on May 28, 2005. According to RFC 2445, the
    end date for such an event needs to be 29.5.2005 0:00.

    Update: We misunderstood rfc 2445 in this regard. For all-day events, the
    DTEND is the last day of the event. See a mail from the Author or rfc 2445:
         http://www.imc.org/ietf-calendar/archive1/msg03648.html
    However, as all other applications also got this wrong, we'll just leave it 
    as it is and use the wrong interpretation (was also discussed on 
    ietf-calsify)*/
void CompatPre31::fixFloatingEnd( QDate &endDate )
{
  endDate = endDate.addDays( 1 );
}

void CompatPre31::fixRecurrence( Incidence *incidence )
{
  CompatPre32::fixRecurrence( incidence );

  Recurrence *recur = incidence->recurrence();
  RecurrenceRule *r = 0;
  if ( recur ) r = recur->defaultRRule();
  if ( recur && r ) {
    int duration = r->duration();
    if ( duration > 0 ) {
      // Backwards compatibility for KDE < 3.1.
      // rDuration was set to the number of time periods to recur,
      // with week start always on a Monday.
      // Convert this to the number of occurrences.
      r->setDuration( -1 );
      QDate end( r->startDt().date() );
      bool doNothing = false;
      // # of periods:
      int tmp = ( duration - 1 ) * r->frequency();
      switch ( r->recurrenceType() ) {
        case RecurrenceRule::rWeekly: {
          end = end.addDays( tmp * 7 + 7 - end.dayOfWeek() );
          break; }
        case RecurrenceRule::rMonthly: {
          int month = end.month() - 1 + tmp;
          end.setYMD( end.year() + month / 12, month % 12 + 1, 31 );
          break; }
        case RecurrenceRule::rYearly: {
          end.setYMD( end.year() + tmp, 12, 31);
          break; }
        default:
          doNothing = true;
          break;
      }
      if ( !doNothing ) {
        duration = r->durationTo( QDateTime( end, QTime( 0, 0, 0 ) ) );
        r->setDuration( duration );
      }
    }

    /* addYearlyNum */
    // Dates were stored as day numbers, with a fiddle to take account of leap years.
    // Convert the day number to a month.
    QValueList<int> days = r->byYearDays();
    if ( !days.isEmpty() ) {
      QValueList<int> months = r->byMonths();
      for ( QValueListConstIterator<int> it = days.begin(); it != days.end(); ++it ) {
        int newmonth = QDate( r->startDt().date().year(), 1, 1).addDays( (*it) - 1 ).month();
        if ( !months.contains( newmonth ) )
          months.append( newmonth );
      }
      r->setByMonths( months );
      days.clear();
      r->setByYearDays( days );
    }
  }


}

/** In Outlook 9, alarms have the wrong sign. I.e. RFC 2445 says that negative
    values for the trigger are before the event's start. Outlook/exchange,
    however used positive values. */
void CompatOutlook9::fixAlarms( Incidence *incidence )
{
  if ( !incidence ) return;
  Alarm::List alarms = incidence->alarms();
  Alarm::List::Iterator it;
  for ( it = alarms.begin(); it != alarms.end(); ++it ) {
    Alarm *al = *it;
    if ( al && al->hasStartOffset() ) {
      Duration offsetDuration = al->startOffset();
      int offs = offsetDuration.asSeconds();
      if ( offs>0 )
        offsetDuration = Duration( -offs );
      al->setStartOffset( offsetDuration );
    }
  }
}
