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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
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

void Compat::fixRecurrence( Incidence *incidence )
{
  // Prevent use of compatibility mode during subsequent changes by the application
  incidence->recurrence()->setCompatVersion();
}

int CompatPre34::fixPriority( int prio )
{
  if ( 0<prio && prio<6 ) {
    // adjust 1->1, 2->3, 3->5, 4->7, 5->9
    return 2*prio - 1;
  } else return prio;
}

void CompatPre32::fixRecurrence( Incidence *incidence )
{
  Recurrence* recurrence = incidence->recurrence();
  if ( recurrence->doesRecur() != Recurrence::rNone  &&  recurrence->duration() > 0 ) {
    // The recurrence has a specified number of repetitions.
    // Pre-3.2, this was extended by the number of exception dates.
    recurrence->setDuration( recurrence->duration() + incidence->exDates().count() );
  }
  // Call base class method now that everything else is done
  Compat::fixRecurrence( incidence );
}

void CompatPre31::fixFloatingEnd( QDate &endDate )
{
  endDate = endDate.addDays( 1 );
}

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
