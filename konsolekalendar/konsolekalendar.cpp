/***************************************************************************
        konsolekalendar.cpp  -  description
           -------------------
    begin                : Sun Jan  6 11:50:14 EET 2002
    copyright            : (C) 2002 by Tuukka Pasanen
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

#include "calendarlocal.h"
#include "calendar.h"
#include "event.h"

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



  if( !m_Calendar->load( m_variables.getCalendarFile() ) )
  {
	  kdDebug() << "Can't open file: " << m_variables.getCalendarFile() << endl;
	  return false;
  }
   else
  {
	kdDebug() << "Successfully opened file: " << m_variables.getCalendarFile() << endl;
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

	kdDebug() << "konsolecalendar.cpp::addEvent | importing now!"  << endl;
	add.addImportedCalendar();


}

void KonsoleKalendar::createCalendar()
{

  QFile fileExists( m_variables.getCalendarFile() );
  bool exists = fileExists.exists();
  fileExists.close();

  if( !exists )
  {
    kdDebug() << "Creating calendar file: " << m_variables.getCalendarFile() << endl;

    CalendarLocal newCalendar;
    newCalendar.save( m_variables.getCalendarFile() );
    newCalendar.close();
  }// if
}



void KonsoleKalendar::showInstance()
{

  Event::List *eventList;

  if( m_variables.isDate() )
  {

     eventList = new Event::List( m_Calendar->rawEventsForDate( m_variables.getDate() ) );
     printEventList( eventList );

  } else {

     eventList = new Event::List ( m_Calendar->rawEvents( m_variables.getStartDate().date(),
                                                         m_variables.getEndDate().date(),
                                                        false )
                                );
     printEventList ( eventList );

  }

  delete eventList;  

}

void KonsoleKalendar::printEventList( Event::List *eventList )
{

  if( eventList->count() ) {

  Event *singleEvent;
  Event::List::ConstIterator it;
  KonsoleKalendarExports exports;


  
    for( it = eventList->begin(); it != eventList->end(); ++it ) {
      singleEvent = *it;

      if( m_variables.getExportType() == HTML ) {
        kdDebug() << "HTML export" << endl;

        exports.exportAsHTML( singleEvent );

      } else if( m_variables.getExportType() == CSV ) {
        kdDebug() << "CSV export" << endl;

        exports.exportAsCSV( singleEvent );

      } else {
        kdDebug() << "TEXT export" << endl;

        exports.exportAsTxt( singleEvent );

      } //else

    }// for

  }// if eventList.count() != 0

}


void KonsoleKalendar::addEvent()
{
  kdDebug() << "konsolecalendar.cpp::addEvent | Create Adding"  << endl;
  KonsoleKalendarAdd add( &m_variables );
  kdDebug() << "konsolecalendar.cpp::addEvent | Adding now!"  << endl;
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

