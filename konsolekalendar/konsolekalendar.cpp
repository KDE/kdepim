/***************************************************************************
                 konsolekalendar.cpp
                 -------------------
    begin                : Sun Jan  6 11:50:14 EET 2002
    copyright            : (C) 2002-2003 by Tuukka Pasanen
    copyright            : (C) 2003 by Allen Winter
    email                : illuusio@mailcity.com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h>
#include <iostream>

#include <qdatetime.h>
#include <qfile.h>

#include <kdebug.h>
#include <klocale.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/calendar.h>
#include <libkcal/event.h>
#include <libkcal/htmlexport.h>

#include "konsolekalendar.h"
#include "konsolekalendaradd.h"
#include "konsolekalendarchange.h"
#include "konsolekalendardelete.h"
#include "konsolekalendarexports.h"

using namespace KCal;
using namespace std;

KonsoleKalendar::KonsoleKalendar(KonsoleKalendarVariables &variables)
{
  m_variables = variables;
  m_Calendar =  new CalendarLocal;
}

KonsoleKalendar::~KonsoleKalendar()
{
}


bool KonsoleKalendar::openCalendar()
{
  if( !m_Calendar->load( m_variables.getCalendarFile() ) ) {
    kdDebug() << "konsolekalendar.cpp::openCalendar() | Can't open file: " << m_variables.getCalendarFile() << endl;
    return false;
  } else {
    kdDebug() << "konsolekalendar.cpp::openCalendar() | Successfully opened file: " << m_variables.getCalendarFile() << endl;
    m_variables.setCalendar( m_Calendar );
    return true;
  }
}

void KonsoleKalendar::closeCalendar()
{
  m_Calendar->close();
  delete m_Calendar;
}

void KonsoleKalendar::importCalendar()
{
  KonsoleKalendarAdd add( &m_variables );

  kdDebug() << "konsolecalendar.cpp::importCalendar() | importing now!"  << endl;
  add.addImportedCalendar();
}

bool KonsoleKalendar::createCalendar()
{

  QFile fileExists( m_variables.getCalendarFile() );
  bool exists = fileExists.exists();
  fileExists.close();

  if( !exists ) {
    kdDebug() << "konsolekalendar.cpp::createCalendar() | Creating calendar file: " << m_variables.getCalendarFile() << endl;

    CalendarLocal newCalendar;
    if( newCalendar.save( m_variables.getCalendarFile() ) ) {
      newCalendar.close();
      return true;
    }
  }
  
  return false;
}

void KonsoleKalendar::showInstance()
{

  if( m_variables.isDryRun() ) {
    cout << i18n("View Events <Dry Run>:").local8Bit() << endl;
    m_variables.printSpecs("view");
  } else {
    kdDebug() << "konsolekalendar.cpp::showInstance() | get raw events list" << endl;

    Event::List *eventList;

    if( m_variables.getExportType() != HTML ) {

      eventList = new Event::List ( m_Calendar->rawEvents( m_variables.getStartDateTime().date(),
							   m_variables.getEndDateTime().date(),
							   false ) );

      printEventList ( eventList );

      delete eventList;

    }else if( m_variables.getExportType() == HTML &&  
              m_variables.getExportFile().contains("none.html") == false ){
      kdDebug() << "konsolekalendar.cpp::showInstance() | HTML" << endl;
      KCal::HtmlExport Export( m_Calendar );
      Export.setEmail( "" );
      Export.setFullName( "" );
  
      Export.setMonthViewEnabled( true );
      Export.setEventsEnabled( true );
      Export.setTodosEnabled( true );
      Export.setCategoriesEventEnabled( true );
      Export.setAttendeesEventEnabled( true );
      Export.setExcludePrivateEventEnabled( true );
      Export.setExcludeConfidentialEventEnabled( true );
      Export.setCategoriesTodoEnabled( true );
      Export.setAttendeesTodoEnabled( true );
      Export.setExcludePrivateTodoEnabled( true );
      Export.setExcludeConfidentialTodoEnabled( true );
      Export.setDueDateEnabled( true );

      Export.setDateRange( m_variables.getStartDateTime().date(), m_variables.getEndDateTime().date());

      Export.save( m_variables.getExportFile() );

      //cout << output.local8Bit() << endl;
    } else {    
	cout << "Can't initialize export. If you are trying to export as HTML" << endl;
	cout << "try use --export-file <file name> switch." << endl;
		 
    }
	  

  }
}

void KonsoleKalendar::printEventList( Event::List *eventList )
{

  if( eventList->count() ) {

    Event *singleEvent;
    Event::List::ConstIterator it;
    KonsoleKalendarExports exports;

    for( it = eventList->begin(); it != eventList->end(); ++it ) {
      singleEvent = *it;


      if( m_variables.getExportType() == CSV ) {
       kdDebug() << "konsolekalendar.cpp::printEventList() | CSV export" << endl;
       exports.exportAsCSV( singleEvent );
      } else if( m_variables.getExportType() == TEXT_KORGANIZER ) {
          kdDebug() << "konsolekalendar.cpp::printEventList() | TEXT-KORGANIZER export" << endl;
          exports.exportAsTxtKorganizer( singleEvent );
      } else {  // Default ExportType is TEXT_KONSOLEKALENDAR
        kdDebug() << "konsolekalendar.cpp::printEventList() | TEXT export" << endl;
        exports.exportAsTxt( singleEvent );

      } //else

    }// for

  }// if eventList.count() != 0

}


void KonsoleKalendar::addEvent()
{
  kdDebug() << "konsolecalendar.cpp::addEvent() | Create Adding"  << endl;
  KonsoleKalendarAdd add( &m_variables );
  kdDebug() << "konsolecalendar.cpp::addEvent() | Adding Event now!"  << endl;
  add.addEvent();
}

void KonsoleKalendar::changeEvent()
{

  KonsoleKalendarChange change( &m_variables );
  change.changeEvent();
}

void KonsoleKalendar::deleteEvent()
{
  KonsoleKalendarDelete del( &m_variables );
  del.deleteEvent();
}

bool KonsoleKalendar::isEvent( QDateTime startdate, QDateTime enddate, QString summary )
{
  // Search for an event with specified start and end date/time stamps and summaries.

  Event *e;
  Event::List::ConstIterator it;
 
  bool found = false;
  Event::List eventList(m_Calendar->events( startdate.date(), TRUE));
  for ( it =  eventList.begin(); it != eventList.end(); ++it ) {
    e = *it;
    if ( e->dtEnd()==enddate && e->summary()==summary ) {
      found = true;
      break;
    }
  }
  return(found);
}
