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

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <qdatetime.h>
#include <qfile.h>
#include <qtextstream.h>

#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/resourcecalendar.h>
#include <libkcal/calendarresources.h>
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

KonsoleKalendar::KonsoleKalendar(KonsoleKalendarVariables *variables)
{
  m_variables = variables;
  // m_Calendar =  new ResourceCalendar;
}

KonsoleKalendar::~KonsoleKalendar()
{
}

bool KonsoleKalendar::openCalendar()
{
  // Obsolette to be removed	
	
  /*if( !m_Calendar->load( m_variables->getCalendarFile() ) ) {
    kdDebug() << "konsolekalendar.cpp::openCalendar() | Can't open file: " << m_variables->getCalendarFile() << endl;
    return false;
  } else {
    kdDebug() << "konsolekalendar.cpp::openCalendar() | Successfully opened file: " << m_variables->getCalendarFile() << endl;
    m_variables->setCalendar( m_Calendar );
    return true;
  }*/
}

void KonsoleKalendar::closeCalendar()
{
  //m_Calendar->close();
  //m_variables->getCalendarResources()->close();
  //delete m_Calendar;
}

void KonsoleKalendar::importCalendar()
{
  KonsoleKalendarAdd add( m_variables );

  kdDebug() << "konsolecalendar.cpp::importCalendar() | importing now!"  << endl;
  add.addImportedCalendar();
}

bool KonsoleKalendar::createCalendar()
{
  bool status = false;
  CalendarLocal newCalendar;

  kdDebug() << "konsolekalendar.cpp::createCalendar() | Creating calendar file: " << m_variables->getCalendarFile() << endl;

  if( newCalendar.save( m_variables->getCalendarFile() ) ) {
    newCalendar.close();
    status = true;
  }
  return status;
}

bool KonsoleKalendar::showInstance()
{
  bool status = true;
  QFile f;
  QString title;

  if( m_variables->isDryRun() ) {
    cout << i18n("View Events <Dry Run>:").local8Bit() << endl;
    m_variables->printSpecs("view");
  } else {

    kdDebug() << "konsolekalendar.cpp::showInstance() | open export file" << endl;

    if( m_variables->isExportFile() ) {
      f.setName( m_variables->getExportFile() );
      if ( !f.open( IO_WriteOnly ) ) {
	status = false;
	kdDebug() << "konsolekalendar.cpp::showInstance() | unable to open export file " << m_variables->getExportFile() << endl;
      } // if
    } else {
      f.open( IO_WriteOnly, stdout );
    } // else

    if( status ) {
      kdDebug() << "konsolekalendar.cpp::showInstance() | opened successful" << endl;

      QTextStream ts( &f );

      if( m_variables->getExportType() != HTML ) {

	kdDebug() << "konsolekalendar.cpp::showInstance() | get raw events list" << endl;

	Event::List *eventList;
	CalendarResources *resource = m_variables->getCalendarResources(); 
	      
	eventList = new Event::List ( ((CalendarLocal *) m_variables->getCalendarResources())->rawEvents( 
							     m_variables->getStartDateTime().date(),
							     m_variables->getEndDateTime().date(),
							     false ) );
	//eventList = new Event::List( m_variables->getCalendarResources()->rawEvents() );      

	status = printEventList ( &ts, eventList );

	delete eventList;

      } else {

	kdDebug() << "konsolekalendar.cpp::showInstance() | HTML" << endl;
	      KCal::HtmlExport Export( m_variables->getCalendarResources() );
	title = "Calendar for " + m_variables->getStartDateTime().date().toString(Qt::TextDate);
	if( m_variables->getStartDateTime().date() != m_variables->getEndDateTime().date() ) {
	  title += " - " + m_variables->getEndDateTime().date().toString(Qt::TextDate);
	}
	Export.setTitle( title );
	Export.setEmail( "" );
	Export.setFullName( "" );
	Export.setCredit( "KonsoleKalendar", "http://pim.kde.org/components/konsolekalendar.php" );

	Export.setMonthViewEnabled( false );  // month view would be another export mode, no?
	Export.setEventsEnabled( true );
	Export.setCategoriesEventEnabled( true );
	Export.setAttendeesEventEnabled( true );
	Export.setExcludePrivateEventEnabled( true );
	Export.setExcludeConfidentialEventEnabled( true );
// Not supporting Todos yet
	title = "To-Do List for " + m_variables->getStartDateTime().date().toString(Qt::TextDate);
	if( m_variables->getStartDateTime().date() != m_variables->getEndDateTime().date() ) {
	  title += " - " + m_variables->getEndDateTime().date().toString(Qt::TextDate);
	}
	Export.setTitleTodo( title );
	Export.setTodosEnabled( false );
	Export.setCategoriesTodoEnabled( false );
	Export.setAttendeesTodoEnabled( false );
	Export.setExcludePrivateTodoEnabled( false );
	Export.setExcludeConfidentialTodoEnabled( false );
	Export.setDueDateEnabled( false );

	Export.setDateRange( m_variables->getStartDateTime().date(), m_variables->getEndDateTime().date());

	status = Export.save( &ts );
      }

      f.close();

    }
  }

  return status;
}

bool KonsoleKalendar::printEventList( QTextStream *ts, Event::List *eventList )
{

  bool status = true;

  if( eventList->count() ) {

    Event *singleEvent;
    Event::List::ConstIterator it;
    KonsoleKalendarExports exports;

    for( it = eventList->begin(); it != eventList->end(); ++it ) {
      singleEvent = *it;

      if( m_variables->getExportType() == CSV ) {
	status = exports.exportAsCSV( ts, singleEvent );
	kdDebug() << "konsolekalendar.cpp::printEventList() | CSV export" << endl;
      } else {  // Default ExportType is TEXT_KONSOLEKALENDAR
        status = exports.exportAsTxt( ts, singleEvent );
        kdDebug() << "konsolekalendar.cpp::printEventList() | TEXT export" << endl;  
      } //else

    }// for

  } else {

    // if no events
    *ts << "(no events)" << endl;
  }

  return( status );
}


bool KonsoleKalendar::addEvent()
{
  kdDebug() << "konsolecalendar.cpp::addEvent() | Create Adding"  << endl;
  KonsoleKalendarAdd add( m_variables );
  kdDebug() << "konsolecalendar.cpp::addEvent() | Adding Event now!"  << endl;
  return( add.addEvent() );
}

bool KonsoleKalendar::changeEvent()
{

  kdDebug() << "konsolecalendar.cpp::changeEvent() | Create Changing"  << endl;
  KonsoleKalendarChange change( m_variables );
  kdDebug() << "konsolecalendar.cpp::changeEvent() | Changing Event now!"  << endl;
  return( change.changeEvent() );
}

bool KonsoleKalendar::deleteEvent()
{
  kdDebug() << "konsolecalendar.cpp::deleteEvent() | Create Deleting"  << endl;
  KonsoleKalendarDelete del( m_variables );
  kdDebug() << "konsolecalendar.cpp::deleteEvent() | Deleting Event now!"  << endl;
  return( del.deleteEvent() );
}

bool KonsoleKalendar::isEvent( QDateTime startdate, QDateTime enddate, QString summary )
{
  // Search for an event with specified start and end date/time stamps and summaries.

  Event *e;
  Event::List::ConstIterator it;
 
  bool found = false;
  
  Event::List eventList( ((CalendarLocal *) m_variables->getCalendarResources())->
			                    rawEventsForDate( startdate.date(), true ));
  for ( it =  eventList.begin(); it != eventList.end(); ++it ) {
    e = *it;
    if ( e->dtEnd()==enddate && e->summary()==summary ) {
      found = true;
      break;
    }
  }
  return found;
}
