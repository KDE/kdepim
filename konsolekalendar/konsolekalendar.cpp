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
  return true;  // to shut up compiler warning
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

  if( m_variables->isDryRun() ) {
// TODO: put back after string freeze
//    cout << i18n("Create Calendar <Dry Run>:").local8Bit() << m_variables->getCalendarFile() << endl;
  } else {

    kdDebug() << "konsolekalendar.cpp::createCalendar() | Creating calendar file: " << m_variables->getCalendarFile() << endl;

    if( m_variables->isVerbose() ) {
// TODO: put back after string freeze
//      cout << i18n("Create Calendar <Verbose>:").local8Bit() << m_variables->getCalendarFile() << endl;
    }

    if( newCalendar.save( m_variables->getCalendarFile() ) ) {
      newCalendar.close();
      status = true;
    }
  }
  return status;
}

bool KonsoleKalendar::showInstance()
{
  bool status = true;
  QFile f;
  QString title;
  Event::List *eventList;
  Event *event;

  if( m_variables->isDryRun() ) {
    cout << i18n("View Events <Dry Run>:").local8Bit() << endl;
    printSpecs();
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

      if( m_variables->isVerbose() ) {
// TODO: put back after string freeze
//	cout << i18n("View Event <Verbose>:").local8Bit() << endl;
	printSpecs();
      }

      QTextStream ts( &f );

      if( m_variables->getExportType() != HTML ) {

	if( m_variables->getAll() ) {
	  kdDebug() << "konsolekalendar.cpp::showInstance() | view all events sorted list" << endl;
	  Event::List sortedList = allEventsSorted( );
	  status = printEventList ( &ts, &sortedList );
	} else if( m_variables->isUID() ) {
	  kdDebug() << "konsolekalendar.cpp::showInstance() | view events by uid list" << endl;
	  event = m_Calendar->event( m_variables->getUID() );
	  status = printEvent ( &ts, event );
	} else {
	  kdDebug() << "konsolekalendar.cpp::showInstance() | view raw events within date range list" << endl;
	  eventList = new Event::List ( ((CalendarLocal *) m_variables->getCalendarResources())->rawEvents( 
					  m_variables->getStartDateTime().date(),
					  m_variables->getEndDateTime().date(),
					  true ) );

	  status = printEventList ( &ts, eventList );
	  delete eventList;
	}

      } else {

	QDate firstdate, lastdate;
	if( m_variables->getAll() ) {
	  // TODO: this is broken since the date on last() may not be last date (this is the case for me)
	  kdDebug() << "konsolekalendar.cpp::showInstance() | HTML view all events sorted list" << endl;
	  eventList = new Event::List ( m_Calendar->rawEvents( ) );
	  firstdate = eventList->first()->dtStart().date();
	  lastdate = eventList->last()->dtStart().date();
	  delete eventList;
	} else if( m_variables->isUID() ) {
	  // TODO
	  kdDebug() << "konsolekalendar.cpp::showInstance() | HTML view events by uid list" << endl;
	  kdError() << i18n("Sorry, export to HTML by UID is not supported yet").local8Bit() << endl;
	  return( false );
	} else {
	  kdDebug() << "konsolekalendar.cpp::showInstance() | HTML view raw events within date range list" << endl;
	  firstdate = m_variables->getStartDateTime().date();
	  lastdate = m_variables->getStartDateTime().date();
	}

	KCal::HtmlExport Export( m_variables->getCalendarResources() );

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
	title = "To-Do List for " + firstdate.toString(Qt::TextDate);
	if( firstdate != lastdate ) {
	  title += " - " + lastdate.toString(Qt::TextDate);
	}
	Export.setTitleTodo( title );
	Export.setTodosEnabled( false );
	Export.setCategoriesTodoEnabled( false );
	Export.setAttendeesTodoEnabled( false );
	Export.setExcludePrivateTodoEnabled( false );
	Export.setExcludeConfidentialTodoEnabled( false );
	Export.setDueDateEnabled( false );

	Export.setDateRange( firstdate, lastdate );

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

      status = printEvent( ts, singleEvent );
      if( ! status )break;

    }// for

  } else {

    // if no events
    *ts << "(no events)" << endl;
  }

  return( status );
}

bool KonsoleKalendar::printEvent( QTextStream *ts, Event *event )
{

  bool status = false;
  KonsoleKalendarExports exports;

  if( event )
  {
    if( m_variables->getExportType() == CSV ) {
      status = exports.exportAsCSV( ts, event );
      kdDebug() << "konsolekalendar.cpp::printEvent() | CSV export" << endl;
    } else {  // Default ExportType is TEXT_KONSOLEKALENDAR
      status = exports.exportAsTxt( ts, event );
      kdDebug() << "konsolekalendar.cpp::printEvent() | TEXT export" << endl;  
    } //else
  } //if

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

Event::List KonsoleKalendar::allEventsSorted()
{
  Event::List *eventList = new Event::List ( m_Calendar->rawEvents( ) );

  // Sort based on dtStart.toTime_t()
  Event::List::ConstIterator it;
  Event::List eventListSorted;
  Event::List::Iterator sortIt;
  for ( it = eventList->begin(); it != eventList->end(); ++it ) {
    sortIt = eventListSorted.begin();
    while ( sortIt != eventListSorted.end() &&
            (*it)->dtStart().toTime_t() >= (*sortIt)->dtStart().toTime_t() ) {
      ++sortIt;
    }
    eventListSorted.insert( sortIt, *it );
  }
  return ( eventListSorted );
}

void KonsoleKalendar::printSpecs()
{
  cout << i18n("  What:  ").local8Bit() << m_variables->getSummary().local8Bit() << endl;
  cout << i18n("  Begin: ").local8Bit() << m_variables->getStartDateTime().toString(Qt::TextDate).local8Bit() << endl;
  cout << i18n("  End:   ").local8Bit() << m_variables->getEndDateTime().toString(Qt::TextDate).local8Bit() << endl;
  if( m_variables->getFloating() == true ) {
    cout << i18n("  No Time Associated with Event").local8Bit() << endl;
  }
  cout << i18n("  Desc:  ").local8Bit() << m_variables->getDescription().local8Bit() << endl;
}
