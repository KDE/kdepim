/* vcalBase.cc			Base class for KOrganizer Conduits
**
** Copyright (C) 2001 by Adriaan de Groot, Cornelius Schumacher
**
** This is the base class for the todo and datebook conduits
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

#include <cstdlib>

#include "options.h"

#include <qfile.h>
#include <qdatetime.h>

#include <kmessagebox.h>
#include <kdebug.h>

#include <calendarlocal.h>
#include <icalformat.h>

#include "kpilotConfig.h"

#include "vcalBase.h"


VCalBaseConduit::VCalBaseConduit(BaseConduit::eConduitMode mode,
                                 BaseConduit::DatabaseSource source) :
  BaseConduit(mode,source),
  fTimeZone(0), 
  calName(),         // That's QString::null
  fCalendar(0) 
{
}

VCalBaseConduit::~VCalBaseConduit()
{
  delete fCalendar;
}

bool VCalBaseConduit::getCalendar(const QString &group)
{
  FUNCTIONSETUP;
  
  if (fCalendar) {
    kdWarning() << __FUNCTION__
		<< ": Already have a calendar file."
		<< endl;
    return true;
  }
  
  KConfig& config = KPilotConfig::getConfig(group);
  (void) getDebugLevel(config);
  calName = config.readEntry("CalFile");
  fFirstTime = getFirstTime(config);
  
  DEBUGCONDUIT << fname
	       << ": Calendar file is " << calName
	       << ( fFirstTime ? " (first time!)" : "" )
	       << endl;
  
  QCString s = QFile::encodeName(calName);
  
  fCalendar = new CalendarLocal();
  
  if(!fCalendar->load(s)) {
    kdError(CONDUIT_AREA) << __FUNCTION__
			  << ": Couldn't open "
			  << calName
			  << endl;
    
    return false;
  } else {
    DEBUGCONDUIT << fname
		 << ": Got calendar!"
		 << endl;
    fTimeZone = fCalendar->getTimeZone();
    return true;
  }
}


void VCalBaseConduit::saveVCal()
{
  if (fCalendar) {
    ICalFormat *format = new ICalFormat(fCalendar);
    fCalendar->save(QFile::encodeName(calName),format);
  }

  fFirstTime = false;
}


void VCalBaseConduit::noCalendarError(const QString &conduitName)
{
  QString message = i18n(
      "The %1 could not open the file `%2'. "
      "Please configure the conduit with the correct "
      "filename and try again.")
      .arg(conduitName)
      .arg(calName);
	
  KMessageBox::error(0, message,i18n("%1 Fatal Error").arg(conduitName));
}

void VCalBaseConduit::setSummary(Incidence *incidence,const char *summary)
{
  incidence->setSummary(summary);
}

void VCalBaseConduit::setNote(Incidence *incidence,const char *s)
{
  incidence->setDescription(s);
}

void VCalBaseConduit::setSecret(Incidence *vevent,bool secret)
{
  vevent->setSecrecy(secret ? Incidence::SecrecyPrivate :
                              Incidence::SecrecyPublic);
}

QDateTime VCalBaseConduit::readTm(const struct tm &t)
{
  QDateTime dt;
  dt.setDate(QDate(1900 + t.tm_year, t.tm_mon + 1, t.tm_mday));
  dt.setTime(QTime(t.tm_hour, t.tm_min, t.tm_sec));
  return dt;
}

struct tm VCalBaseConduit::writeTm(const QDateTime &dt)
{
  struct tm t;

  t.tm_wday = 0; // unimplemented
  t.tm_yday = 0; // unimplemented
  t.tm_isdst = 0; // unimplemented

  t.tm_year = dt.date().year() - 1900;
  t.tm_mon = dt.date().month() - 1;
  t.tm_mday = dt.date().day();
  t.tm_hour = dt.time().hour();
  t.tm_min = dt.time().minute();
  t.tm_sec = dt.time().second();

  return t;
}

void VCalBaseConduit::deleteRecord(PilotRecord *rec)
{
  kdDebug() << "VCalBaseConduit::deleteRecord() not yet implemented" << endl;
}

void VCalBaseConduit::deleteFromPilot(int entryType)
{
  QValueList<recordid_t> deletedList;

  /* Build a list of records in the pilot calendar that are not
     found in the vcal and thus probably have been deleted. */
  
  // Get all entries from Pilot
  PilotRecord *rec;
  int index = 0;
  while ((rec = readRecordByIndex(index++)) != 0) {
    bool found = false;
    if ((entryType == TypeTodo) && findTodo(rec->getID())) found = true;
    if ((entryType == TypeEvent) && findEvent(rec->getID())) found = true;
    if (!found) {
      DEBUGCONDUIT << __FUNCTION__
		   << ": record "
		   << rec->getID()
		   << " found on pilot, but not in vcalendar. "
		   << "Scheduling it for deletion."
		   << endl;
      deletedList.append(rec->getID());
    }
    delete rec;
  }

  // Disable deletion to prevent data loss in case of logs.
  // Will be removed, when the todo and datebook conduits are thoroughly tested
  // TODO: Reenable deleteFromPilot()
  return;
  
  // Now process the list of deleted records. 
  for (QValueList<recordid_t>::Iterator it = deletedList.begin();
       it != deletedList.end(); ++it) {
    PilotRecord *r = readRecordById(*it);
    if (r) {
      DEBUGCONDUIT << __FUNCTION__ << ": deleting record " << *it
		   << endl;
      r->setAttrib(~dlpRecAttrDeleted);
      recordid_t rid = writeRecord(r);
      delete r;
      if (rid != *it)
	DEBUGCONDUIT << __FUNCTION__
		     << ": writeRecord() returned "
		     << rid << endl;
    } else
      kdWarning(CONDUIT_AREA) << __FUNCTION__
			      << ": readRecordById() failed for record"
			      << *it << endl;
  }
}

Todo *VCalBaseConduit::findTodo(recordid_t id)
{
  QList<Todo> todos = calendar()->getTodoList();
  
  Todo *todo = todos.first();
  while(todo) {
    if (todo->pilotId() == id) return todo;
    todo = todos.next();
  }
  
  return 0;
}

Event *VCalBaseConduit::findEvent(recordid_t id)
{
  QList<Event> events = calendar()->getAllEvents();
  
  Event *event = events.first();
  while(event) {
    if (event->pilotId() == id) return event;    
    event = events.next();
  }
  
  return 0;
}

// $Log$
// Revision 1.8  2001/06/05 22:58:40  adridg
// General rewrite, cleanup thx. Philipp Hullmann
//
// Revision 1.7  2001/05/25 16:06:52  adridg
// DEBUG breakage
//
// Revision 1.6  2001/04/23 06:29:30  adridg
// Patches for bug #23385 and probably #23289
//
// Revision 1.5  2001/04/18 21:20:29  adridg
// Response to bug #24291
//
// Revision 1.4  2001/04/16 13:54:17  adridg
// --enable-final file inclusion fixups
//
// Revision 1.3  2001/04/01 17:32:06  adridg
// Fiddling around with date properties
//
// Revision 1.2  2001/03/24 16:11:06  adridg
// Fixup some date-to-vcs functions
//
// Revision 1.1  2001/03/10 18:26:04  adridg
// Refactored vcal conduit and todo conduit
//

