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

#include <kdebug.h>
#include <klocale.h>

#include "calendarlocal.h"
#include "calendar.h"
#include "event.h"

#include "konsolekalendar.h"
#include "konsolekalendaradd.h"

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
	kdDebug() << "Succefully opened file: " << m_variables.getCalendarFile() << endl;
	m_variables.setCalendar( m_Calendar );
	return true;
	  
  }
	
}

void KonsoleKalendar::closeCalendar()
{
		delete m_Calendar;
}


void KonsoleKalendar::showInstance()
{
    if( m_variables.isNext() ) {
      showNext();
    }
  
    if( m_variables.isDate() && m_variables.isStartDate() == false ) {
      showDate( m_variables.getDate() );                              
    }
    
    if( m_variables.isStartDate() ) {    
      if(m_variables.isVerbose()){
        kdDebug() << "konsolecalendar.cpp::showInstance() | Start date set" << endl;           
      }
      
      QDateTime start = m_variables.getStartDate( );
      QDateTime end;
      bool loop = false;
      
      if( m_variables.isEndDate() ) {
        end = m_variables.getEndDate( );                
      } else {
         end = start.addDays(30);
      }
    
      while( !loop ) {
        if(m_variables.isVerbose()) {
          kdDebug() << "konsolecalendar.cpp::showInstance() | " << start.toString().local8Bit()  << endl;            
          kdDebug() << "konsolecalendar.cpp::showInstance() | days to end " << start.daysTo( end )  << endl;
        }
      
        showDate( start );
            
        if( !start.daysTo( end )  ) {
          loop = true;
        }
      
        start = start.addDays(1);          
      }     
    }
  

  
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
}
	
void KonsoleKalendar::deleteEvent()
{
}

void KonsoleKalendar::showDate( QDateTime date )
{
  Event *singleEvent;

  QPtrList<Event> eventList(m_Calendar->events( date.date(), TRUE));
  QString tempString;
  QDate current = QDate::currentDate();
      
  if( eventList.count() ) {
    int len = 100;
    tempString = date.toString();
    len -= tempString.length();
  
    cout << endl << tempString.local8Bit() << "\n";

    for( len = len; len < 100; len ++) {
       cout << "-";
    }
      
    cout << endl;
  
    for ( singleEvent = eventList.first(); singleEvent != 0; singleEvent =
          eventList.next() ) {
      if( m_variables.isAll() ) { 
        printEventTime(singleEvent);
        // cout << endl;
        cout << "\t" << singleEvent->summary().local8Bit() << endl;
      } 
       else 
      {      
        if(current.daysTo( date.date() ) == 0) 
	{
          if( m_variables.isVerbose() ) 
	  {
            cout << i18n("Today: ").local8Bit() <<  isHappened(singleEvent) << endl;
          }  
        
          if( isHappened(singleEvent) == false) 
	  {
            printEventTime( singleEvent );
            cout << "\t" << singleEvent->summary().local8Bit() << endl;
          }
        } // if
	 else 
	{
		
          if( m_variables.isVerbose() )
	  {
            cout << i18n("Not today: ").local8Bit() <<  isHappened(singleEvent) << endl;
          }
    
          printEventTime( singleEvent );
          cout << "\t" << singleEvent->summary().local8Bit() << endl;
        }// else
      }//else
    }// for
  } // if
}


void KonsoleKalendar::showNext()
{
  int date = 0;
  bool loop = false;

  // single event
  Event *singleEvent;
  QDate qdate;    
  QString tempString;
  int len = 50;

  while(!loop) {
    QPtrList<Event> eventList(m_Calendar->events(m_variables.getDate().date(), TRUE));
  
    if( eventList.count() ) {
      len = 80;
      tempString = m_variables.getDate().toString();
      len -= tempString.length();
  
      cout << endl << tempString.local8Bit() << " ";

      for( len = len; len < 80; len ++) {
        cout << "-";
      }
      
      cout << endl;
  
      for ( singleEvent = eventList.first(); singleEvent != 0; singleEvent = eventList.next() ){
        printEventTime(singleEvent);
        cout << endl;
        cout << "\t\t" << singleEvent->summary().local8Bit() << endl;

        if (!singleEvent->doesFloat()) {
          loop = true;
          break;
        }
      }
       loop = true;
    }
  
    date ++;
    if(date >= 30) {
      loop = true;
    }

    qdate = m_variables.getDate().date();
    qdate = qdate.addDays(1);
    m_variables.setDate(qdate);
  }
}


bool KonsoleKalendar::isHappened( Event *event )
{
  int minute, hour;
    
  QString sHour, sMinute;
  QString temp;
  QString temp2;
  QTime time( QTime::currentTime() );
    
  temp = event->dtStartStr().remove(0, (event->dtStartStr().find(' ', 0, false) + 1) );
  temp2 = temp;
    
  sHour = temp.remove( (temp.find(':', 0, false) ), ( temp.length() - temp.find(':', 0, false) ));
    
  sMinute = temp2.remove( 0, ( temp2.find(':', 0, false) + 1 ));

  if( m_variables.isVerbose() ) {
    cout << i18n("hours: ").local8Bit() << sHour.latin1() 
         << i18n(" minutes: ").local8Bit() << sMinute.latin1() << endl;
  }
    
  hour = sHour.toInt();
  minute = sMinute.toInt();
    
  if( m_variables.isVerbose() ) {
    cout << i18n("hours: ").local8Bit() << hour 
        << i18n(" minutes: ").local8Bit() << minute << endl;
  }
    
  if( hour >= time.hour() && minute >= time.minute()) {
    if( m_variables.isVerbose() ) {
      cout << i18n("This is valid!").local8Bit();
    }
    return false;
  } 

  return true;
}


/** Print event time */
void KonsoleKalendar::printEventTime(Event *event)
{
  if (!event->doesFloat()) {
    //cout << event->dtStartStr();
      
    // Cut out info only leave times
    //
    cout <<  event->dtStartStr().remove(0, (event->dtStartStr().find(' ', 0, false) + 1) ).local8Bit();
    cout << " - ";  
    cout << event->dtEndStr().remove(0, (event->dtEndStr().find(' ', 0, false) + 1) ).local8Bit();
  }
}
