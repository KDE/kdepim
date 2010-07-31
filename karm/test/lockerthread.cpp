#include <tqthread.h>
#include <tqstring.h>

#include <resourcecalendar.h>
#include <resourcelocal.h>
#include <calendarresources.h>

#include "lockerthread.h"

LockerThread::LockerThread( const TQString &icsfile )
{
  m_gotlock = false;
  m_icsfile = icsfile;
}

/*
void LockerThread::setIcsFile( const TQString &filename )
{
  m_icsfile = filename;
}
*/

void LockerThread::run()
{
  KCal::CalendarResources         *calendars = 0;
  KCal::ResourceCalendar          *calendar  = 0;
  KCal::CalendarResources::Ticket *lock      = 0;

  calendars = new KCal::CalendarResources( TQString::fromLatin1( "UTC" ) );
  calendar  = new KCal::ResourceLocal( m_icsfile );
  lock      = calendars->requestSaveTicket( calendar );
  if ( lock )
  {
    m_gotlock = true;
    calendars->releaseSaveTicket( lock );
  }
  else
  {
    m_gotlock = false;
  }

  delete calendar;
  delete calendars;
}
