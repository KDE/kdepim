/********************************************************************************
 *   konsolekalendarexports.cpp                                                 *
 *                                                                              *
 *   KonsoleKalendar is console frontend to calendar                            *
 *   Copyright (C) 2002-2004  Tuukka Pasanen <illuusio@mailcity.com>            *
 *   Copyright (C) 2003-2004  Allen Winter                                      *
 *                                                                              *
 *   This library is free software; you can redistribute it and/or              *
 *   modify it under the terms of the GNU Lesser General Public                 *
 *   License as published by the Free Software Foundation; either               *
 *   version 2.1 of the License, or (at your option) any later version.         *
 *                                                                              *
 *   This library is distributed in the hope that it will be useful,            *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU          *
 *   Lesser General Public License for more details.                            *
 *                                                                              *
 *   You should have received a copy of the GNU Lesser General Public           *
 *   License along with this library; if not, write to the Free Software        *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA  *
 *                                                                              *
 ********************************************************************************/

#include <stdlib.h>
#include <iostream>

#include <qdatetime.h>

#include <kdebug.h>
#include <klocale.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/calendar.h>
#include <libkcal/event.h>
#include <libkcal/htmlexport.h>


#include "konsolekalendarexports.h"

using namespace KCal;
using namespace std;

KonsoleKalendarExports::KonsoleKalendarExports( KonsoleKalendarVariables *variables )
{
  m_variables = variables;
  m_firstEntry = true;
}


KonsoleKalendarExports::~KonsoleKalendarExports()
{
}

bool KonsoleKalendarExports::exportAsTxt( QTextStream *ts, Event *event, QDate date ){

  // Print Date (in our standard format)
  *ts << I18N_NOOP("Date:") << "\t" <<  date.toString("yyyy-MM-dd") << endl;

  // Print Event Starttime - Endtime (in our standard format), for Non-Floating Events Only
  if ( !event->doesFloat() ) {
    *ts << "\t";
    *ts << event->dtStart().time().toString("hh:mm");
    *ts << " - ";
    *ts << event->dtEnd().time().toString("hh:mm");
  }

  // Print Event Summary, Description, etc.
  *ts << endl << I18N_NOOP("Summary:") << endl;
  *ts << "\t" << event->summary() << endl;

  *ts << endl << I18N_NOOP("Location:") << endl;
  if( !event->location().isEmpty() ) {
    *ts << "\t" <<event->location() << endl;
  } else {
    *ts << "\t" << I18N_NOOP("(no location info available)") << endl;
  }

  *ts << I18N_NOOP("Description:") << endl;
  if( !event->description().isEmpty() ) {
    *ts << "\t" << event->description() << endl;
  } else {
    *ts << "\t" << I18N_NOOP("(no description available)") << endl;
  }
  *ts << I18N_NOOP("UID:") << endl;
  *ts << "\t" << event->uid() << endl;
  *ts << "----------------------------------" << endl;

  return true;
}

bool KonsoleKalendarExports::exportAsHuman( QTextStream *ts, Event *event, QDate date, bool sameday ){

  if( !sameday ){
    // Print Date (in our standard format)
    *ts << "----------------------------------" << endl;
    *ts << date.toString("yyyy-MM-dd") << endl;
  }

  // Print Event Starttime - Endtime (in our standard format), for Non-Floating Events Only
  if ( !event->doesFloat() ) {
    *ts << event->dtStart().time().toString("hh:mm");
    *ts << " - ";
    *ts << event->dtEnd().time().toString("hh:mm");
  }

  // Print Event Summary, Description, etc.
  *ts << "\t" << event->summary();

  if( !event->location().isEmpty() ) {
    *ts << ", " << event->location();
  }

  *ts << endl;

  if( !event->description().isEmpty() ) {
    *ts << "\t\t" << event->description();
  } else {
    *ts << "\t\t" << I18N_NOOP("(no description available)");
  }

  *ts << ", " << event->uid() << endl;

  return true;
}

bool KonsoleKalendarExports::exportAsCSV( QTextStream *ts, Event *event, QDate date ){

// startdate,starttime,enddate,endtime,summary,description,UID

  QString delim = ",";  //TODO: the delim character can be an option??

  if ( !event->doesFloat() ) {
    *ts <<          date.toString("yyyy-MM-dd");
    *ts << delim << event->dtStart().time().toString("hh:mm");
    *ts << delim << date.toString("yyyy-MM-dd");
    *ts << delim << event->dtEnd().time().toString("hh:mm");
  } else {
    *ts <<          date.toString("yyyy-MM-dd");
    *ts << delim;
    *ts << delim << date.toString("yyyy-MM-dd");
    *ts << delim;
  }

  QString rdelim = "\\" + delim;
  *ts << delim << event->summary().replace(delim,rdelim);
  *ts << delim << event->location().replace(delim,rdelim);
  *ts << delim << event->description().replace(delim,rdelim);
  *ts << delim << event->uid();
  *ts << endl;

  return true;
}
