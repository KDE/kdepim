/*******************************************************************************
 * konsolekalendar.cpp                                                         *
 *                                                                             *
 * KonsoleKalendar is a command line interface to KDE calendars                *
 * Copyright (C) 2002-2004  Tuukka Pasanen <illuusio@mailcity.com>             *
 * Copyright (C) 2003-2005  Allen Winter <winter@kde.org>
 *                                                                             *
 * This program is free software; you can redistribute it and/or modify        *
 * it under the terms of the GNU General Public License as published by        *
 * the Free Software Foundation; either version 2 of the License, or           *
 * (at your option) any later version.                                         *
 *                                                                             *
 * This program is distributed in the hope that it will be useful,             *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                *
 * GNU General Public License for more details.                                *
 *                                                                             *
 * You should have received a copy of the GNU General Public License           *
 * along with this program; if not, write to the Free Software                 *
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. *
 *                                                                             *
 * As a special exception, permission is given to link this program            *
 * with any edition of Qt, and distribute the resulting executable,            *
 * without including the source code for Qt in the source distribution.        *
 *                                                                             *
 ******************************************************************************/
/**
 * @file konsolekalendar.cpp
 * Provides the KonsoleKalendar class definition.
 * @author Tuukka Pasanen
 * @author Allen Winter
 */
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <qdatetime.h>
#include <qfile.h>
#include <qtextstream.h>

#include <kdebug.h>
#include <klocale.h>

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

KonsoleKalendar::KonsoleKalendar( KonsoleKalendarVariables *variables )
{
  m_variables = variables;
  // m_Calendar =  new ResourceCalendar;
}

KonsoleKalendar::~KonsoleKalendar()
{
}

bool KonsoleKalendar::importCalendar()
{
  KonsoleKalendarAdd add( m_variables );

  kdDebug() << "konsolecalendar.cpp::importCalendar() | importing now!"
            << endl;
  return( add.addImportedCalendar() );
}

bool KonsoleKalendar::createCalendar()
{
  bool status = false;
  CalendarLocal newCalendar;

  if ( m_variables->isDryRun() ) {
    cout << i18n( "Create Calendar <Dry Run>: %1" ).
      arg( m_variables->getCalendarFile() ).local8Bit()
         << endl;
  } else {
    kdDebug() << "konsolekalendar.cpp::createCalendar() | "
              << "Creating calendar file: "
              << m_variables->getCalendarFile().local8Bit()
              << endl;

    if ( m_variables->isVerbose() ) {
      cout << i18n( "Create Calendar <Verbose>: %1" ).
        arg( m_variables->getCalendarFile() ).local8Bit()
           << endl;
    }

    if ( newCalendar.save( m_variables->getCalendarFile() ) ) {
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

  if ( m_variables->isDryRun() ) {
    cout << i18n( "View Events <Dry Run>:" ).local8Bit()
         << endl;
    printSpecs();
  } else {

    kdDebug() << "konsolekalendar.cpp::showInstance() | "
              << "open export file"
              << endl;

    if ( m_variables->isExportFile() ) {
      f.setName( m_variables->getExportFile() );
      if ( !f.open( IO_WriteOnly ) ) {
	status = false;
	kdDebug() << "konsolekalendar.cpp::showInstance() | "
                  << "unable to open export file "
                  << m_variables->getExportFile()
                  << endl;
      }
    } else {
      f.open( IO_WriteOnly, stdout );
    }

    if ( status ) {
      kdDebug() << "konsolekalendar.cpp::showInstance() | "
                << "opened successful"
                << endl;

      if ( m_variables->isVerbose() ) {
	cout << i18n( "View Event <Verbose>:" ).local8Bit()
             << endl;
	printSpecs();
      }

      QTextStream ts( &f );

      if ( m_variables->getExportType() != ExportTypeHTML &&
           m_variables->getExportType() != ExportTypeMonthHTML ) {

	if ( m_variables->getAll() ) {
	  kdDebug() << "konsolekalendar.cpp::showInstance() | "
                    << "view all events sorted list"
                    << endl;

	  Event::List sortedList =
            m_variables->getCalendar()->events( EventSortStartDate );

          QDate dt, firstdate, lastdate;
          firstdate = sortedList.first()->dtStart().date();
          lastdate = sortedList.last()->dtStart().date();
          for ( dt = firstdate;
                dt <= lastdate && status != false;
                dt = dt.addDays(1) ) {
            Event::List events = m_variables->getCalendar()->events( dt, true );
            status = printEventList( &ts, &events, dt );
          }

	} else if ( m_variables->isUID() ) {
	  kdDebug() << "konsolekalendar.cpp::showInstance() | "
                    << "view events by uid list"
                    << endl;
	  //TODO: support a list of UIDs
	  event = m_variables->getCalendar()->event( m_variables->getUID() );
	  //If this UID represents a recurring Event,
          //only the first day of the Event will be printed
	  status = printEvent ( &ts, event, event->dtStart().date() );

        } else if ( m_variables->isNext() ) {
          kdDebug() << "konsolekalendar.cpp::showInstance() | "
                    << "Show next activity in calendar"
                    << endl;

          QDateTime datetime = m_variables->getStartDateTime();
          datetime = datetime.addDays( 720 );

	  QDate dt;
	  for ( dt = m_variables->getStartDateTime().date();
                dt <= datetime.date() && status != false;
                dt = dt.addDays(1) ) {
	    Event::List events = m_variables->getCalendar()->events( dt, true );
	    status = printEventList( &ts, &events, dt );

	    // when we get next event we exit..
	    if ( events.count() ) {
	      kdDebug() << "konsolekalendar.cpp::showInstance() | "
                        << "Next event"
                        << endl;
	      return true;
	    }
          }
	} else {
	  kdDebug() << "konsolekalendar.cpp::showInstance() | "
                    << "view raw events within date range list"
                    << endl;

	  QDate dt;
	  for ( dt = m_variables->getStartDateTime().date();
                dt <= m_variables->getEndDateTime().date() && status != false;
                dt = dt.addDays(1) ) {
	    Event::List events = m_variables->getCalendar()->events( dt, true );
	    status = printEventList( &ts, &events, dt );
	  }
	}
      } else {
	QDate firstdate, lastdate;
	if ( m_variables->getAll() ) {
          // TODO: this is broken since the date on last() may not be last date
          // (this is the case for me)
	  kdDebug() << "konsolekalendar.cpp::showInstance() | "
                    << "HTML view all events sorted list"
                    << endl;
          eventList =
            new Event::List ( m_variables->getCalendar()->rawEvents() );
	  firstdate = eventList->first()->dtStart().date();
	  lastdate = eventList->last()->dtStart().date();
          delete eventList;
	} else if ( m_variables->isUID() ) {
	  // TODO
	  kdDebug() << "konsolekalendar.cpp::showInstance() | "
                    << "HTML view events by uid list" << endl;
	  cout << i18n("Sorry, export to HTML by UID is not supported yet")
            .local8Bit() << endl;
	  return( false );
	} else {
	  kdDebug() << "konsolekalendar.cpp::showInstance() | "
                    << "HTML view raw events within date range list"
                    << endl;
	  firstdate = m_variables->getStartDateTime().date();
	  lastdate = m_variables->getEndDateTime().date();
	}

        HTMLExportSettings htmlSettings( "Konsolekalendar" );

        //TODO: get progname and url from the values set in main
        htmlSettings.setCreditName( "KonsoleKalendar" );
        htmlSettings.setCreditURL( "http://pim.kde.org/components/konsolekalendar.php" );

        htmlSettings.setExcludePrivate( true );
        htmlSettings.setExcludeConfidential( true );

        htmlSettings.setEventView( false );
        htmlSettings.setMonthView( false );
        if ( m_variables->getExportType() == ExportTypeMonthHTML ) {
          title = i18n( "Events:" );
          htmlSettings.setMonthView( true );
        } else {
          if ( firstdate == lastdate ) {
            title = i18n( "Events: %1" )
                    .arg(  firstdate.toString( Qt::TextDate ) );
          } else {
            title = i18n( "Events: %1 - %2" )
                    .arg( firstdate.toString( Qt::TextDate ) )
                    .arg( lastdate.toString( Qt::TextDate ) );
          }
          htmlSettings.setEventView( true );
        }
        htmlSettings.setEventTitle( title );
        htmlSettings.setEventAttendees( true );
// Not supporting Todos yet
//         title = "To-Do List for " + firstdate.toString(Qt::TextDate);
//         if ( firstdate != lastdate ) {
//           title += " - " + lastdate.toString(Qt::TextDate);
//         }
        htmlSettings.setTodoListTitle( title );
        htmlSettings.setTodoView( false );
//         htmlSettings.setTaskCategories( false );
//         htmlSettings.setTaskAttendees( false );
//         htmlSettings.setTaskDueDate( true );

        htmlSettings.setDateStart( QDateTime( firstdate ) );
        htmlSettings.setDateEnd( QDateTime( lastdate ) ) ;

        KCal::HtmlExport *Export;
        if ( !m_variables->isCalendarResources() ) {
          Export = new HtmlExport( m_variables->getCalendar(), &htmlSettings );
        } else {
          Export = new HtmlExport( m_variables->getCalendarResources(), &htmlSettings );
        }
	status = Export->save( &ts );
        delete Export;
      }
      f.close();
    }
  }
  return status;
}

bool KonsoleKalendar::printEventList( QTextStream *ts,
                                      Event::List *eventList, QDate date )
{
  bool status = true;

  if ( eventList->count() ) {
    Event *singleEvent;
    Event::List::ConstIterator it;

    for ( it = eventList->begin();
          it != eventList->end() && status != false;
          ++it ) {
      singleEvent = *it;

      status = printEvent( ts, singleEvent, date );
    }
  }

  return( status );
}

bool KonsoleKalendar::printEvent( QTextStream *ts, Event *event, QDate dt )
{
  bool status = false;
  bool sameDay = true;
  KonsoleKalendarExports exports;

  if ( event ) {
    switch ( m_variables->getExportType() ) {

    case ExportTypeCSV:
      kdDebug() << "konsolekalendar.cpp::printEvent() | "
                << "CSV export"
                << endl;
      status = exports.exportAsCSV( ts, event, dt );
      break;

    case ExportTypeTextShort:
      kdDebug()
        << "konsolekalendar.cpp::printEvent() | "
        << "TEXT-SHORT export"
        << endl;
      if ( dt.daysTo( m_saveDate ) ) {
        sameDay = false;
        m_saveDate = dt;
      }
      status = exports.exportAsTxtShort( ts, event, dt, sameDay );
      break;

    case ExportTypeHTML:
      // this is handled separately for now
      break;

    default:// Default export-type is ExportTypeText
      kdDebug() << "konsolekalendar.cpp::printEvent() | "
                << "TEXT export"
                << endl;
      status = exports.exportAsTxt( ts, event, dt );
      break;
    }
  }
  return( status );
}

bool KonsoleKalendar::addEvent()
{
  kdDebug() << "konsolecalendar.cpp::addEvent() | "
            << "Create Adding"
            << endl;
  KonsoleKalendarAdd add( m_variables );
  kdDebug() << "konsolecalendar.cpp::addEvent() | "
            << "Adding Event now!"
            << endl;
  return( add.addEvent() );
}

bool KonsoleKalendar::changeEvent()
{

  kdDebug() << "konsolecalendar.cpp::changeEvent() | "
            << "Create Changing"
            << endl;
  KonsoleKalendarChange change( m_variables );
  kdDebug() << "konsolecalendar.cpp::changeEvent() | "
            << "Changing Event now!"
            << endl;
  return( change.changeEvent() );
}

bool KonsoleKalendar::deleteEvent()
{
  kdDebug() << "konsolecalendar.cpp::deleteEvent() | "
            << "Create Deleting"
            << endl;
  KonsoleKalendarDelete del( m_variables );
  kdDebug() << "konsolecalendar.cpp::deleteEvent() | "
            << "Deleting Event now!"
            << endl;
  return( del.deleteEvent() );
}

bool KonsoleKalendar::isEvent( QDateTime startdate,
                               QDateTime enddate, QString summary )
{
  // Search for an event with specified start and end datetime stamp and summary

  Event *event;
  Event::List::ConstIterator it;

  bool found = false;

  Event::List eventList( m_variables->getCalendar()->
                         rawEventsForDate( startdate.date(), true ) );
  for ( it = eventList.begin(); it != eventList.end(); ++it ) {
    event = *it;
    if ( event->dtEnd() == enddate && event->summary() == summary ) {
      found = true;
      break;
    }
  }
  return found;
}

void KonsoleKalendar::printSpecs()
{
  cout << i18n( "  What:  %1" ).
    arg( m_variables->getSummary() ).local8Bit()
       << endl;

  cout << i18n( "  Begin: %1" ).
    arg( m_variables->getStartDateTime().toString( Qt::TextDate ) ).local8Bit()
       << endl;

  cout << i18n( "  End:   %1" ).
    arg( m_variables->getEndDateTime().toString( Qt::TextDate ) ).local8Bit()
       << endl;

  if ( m_variables->getFloating() == true ) {
    cout << i18n( "  No Time Associated with Event" ).local8Bit()
         << endl;
  }

  cout << i18n( "  Desc:  %1" ).
    arg( m_variables->getDescription() ).local8Bit()
       << endl;

  cout << i18n( "  Location:  %1" ).
    arg( m_variables->getLocation() ).local8Bit()
       << endl;
}
