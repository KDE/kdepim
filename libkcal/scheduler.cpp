/*
    This file is part of libkcal.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

// $Id$

#include <qdir.h>
#include <qfile.h>
#include <qtextstream.h>

#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>

#include "event.h"
#include "freebusy.h"
#include "icalformat.h"
#include "calendar.h"

#include "scheduler.h"

using namespace KCal;

ScheduleMessage::ScheduleMessage(IncidenceBase *incidence,int method,ScheduleMessage::Status status)
{
  mIncidence = incidence;
  mMethod = method;
  mStatus = status;
}

QString ScheduleMessage::statusName(ScheduleMessage::Status status)
{
  switch (status) {
    case PublishNew:
      return i18n("Publish");
    case Obsolete:
      return i18n("Obsolete");
    case RequestNew:
      return i18n("New Request");
    case RequestUpdate:
      return i18n("Updated Request");
    default:
      return i18n("Unknown Status: %1").arg(QString::number(status));
  }
}

Scheduler::Scheduler(Calendar *calendar)
{
  mCalendar = calendar;
  mFormat = mCalendar->iCalFormat();
}

Scheduler::~Scheduler()
{
}

bool Scheduler::acceptTransaction(IncidenceBase *incidence,Method method,ScheduleMessage::Status status)
{
  kdDebug() << "Scheduler::acceptTransaction " << endl;
  switch (method) {
    case Publish:
      return acceptPublish(incidence, status, method);
    case Request:
      return acceptRequest(incidence, status);
    case Add:
      return acceptAdd(incidence, status);
    case Cancel:
      return acceptCancel(incidence, status);
    case Declinecounter:
      return acceptDeclineCounter(incidence, status);
    case Reply:
      return acceptReply(incidence, status, method);
    case Refresh:
      return acceptRefresh(incidence, status);
    case Counter:
      return acceptCounter(incidence, status);
    default:
      deleteTransaction(incidence);
      return false;
  }
  deleteTransaction(incidence);
  return false;
}

QString Scheduler::methodName(Method method)
{
  switch (method) {
    case Publish:
      return i18n("Publish");
    case Request:
      return i18n("Request");
    case Refresh:
      return i18n("Refresh");
    case Cancel:
      return i18n("Cancel");
    case Add:
      return i18n("Add");
    case Reply:
      return i18n("Reply");
    case Counter:
      return i18n("Counter");
    case Declinecounter:
      return i18n("Decline Counter");
    default:
      return i18n("Unknown");
  }
}

bool Scheduler::deleteTransaction(IncidenceBase *)
{
  return true;
}

bool Scheduler::acceptPublish(IncidenceBase *incidence,ScheduleMessage::Status status, Method method)
{
  if(incidence->type()=="FreeBusy") {
    return acceptFreeBusy(incidence, method);
  }
  switch (status) {
    case ScheduleMessage::PublishNew:
      if (!mCalendar->getEvent(incidence->uid())) {
	Incidence *inc = static_cast<Incidence *>(incidence);
        mCalendar->addIncidence(inc);
        deleteTransaction(incidence);
      }
      return true;
    case ScheduleMessage::Obsolete:
      return true;
    default:
      deleteTransaction(incidence);
      return false;
  }
  deleteTransaction(incidence);
  return false;
}

bool Scheduler::acceptRequest(IncidenceBase *incidence,ScheduleMessage::Status status)
{
    Incidence *inc = static_cast<Incidence *>(incidence);
    switch (status) {
    case ScheduleMessage::Obsolete:
      return true;
    case ScheduleMessage::RequestNew:
      mCalendar->addIncidence(inc);
      deleteTransaction(incidence);
      return true;
    case ScheduleMessage::RequestUpdate:
      Event *even;
      even = mCalendar->getEvent(incidence->uid());
      if (even) {
        mCalendar->deleteEvent(even);
      }
      mCalendar->addIncidence(inc);
      deleteTransaction(incidence);
      return true;
    default:
      return false;
    }
  deleteTransaction(incidence);
  return false;
}

bool Scheduler::acceptAdd(IncidenceBase *incidence,ScheduleMessage::Status status)
{
  deleteTransaction(incidence);
  return false;
}

bool Scheduler::acceptCancel(IncidenceBase *incidence,ScheduleMessage::Status status)
{
  bool ret = false;
  //get event in calendar
  QPtrList<Event> eventList;
  eventList=mCalendar->getEvents(incidence->dtStart().date(),incidence->dtStart().date(),false);
  Event *ev;
  for ( ev = eventList.first(); ev; ev = eventList.next() ) {
    if (ev->uid()==incidence->uid()) {
      //get matching attendee in calendar
      kdDebug() << "Scheduler::acceptTransaction match found!" << endl;
      mCalendar->deleteEvent(ev);
      ret = true;
    }
  }
  deleteTransaction(incidence);
  return ret;
}

bool Scheduler::acceptDeclineCounter(IncidenceBase *incidence,ScheduleMessage::Status status)
{
  deleteTransaction(incidence);
  return false;
}

//bool Scheduler::acceptFreeBusy(Incidence *incidence,ScheduleMessage::Status status)
//{
//  deleteTransaction(incidence);
//  return false;
//}

bool Scheduler::acceptReply(IncidenceBase *incidence,ScheduleMessage::Status status, Method method)
{
  if(incidence->type()=="FreeBusy") {
    return acceptFreeBusy(incidence, method);
  }
  bool ret = false;
  Event *ev = mCalendar->getEvent(incidence->uid());
  if (ev) {
    //get matching attendee in calendar
    kdDebug(5800) << "Scheduler::acceptTransaction match found!" << endl;
    QPtrList<Attendee> attendeesIn = incidence->attendees();
    QPtrList<Attendee> attendeesEv = ev->attendees();
    Attendee *attIn;
    Attendee *attEv;
    for ( attIn = attendeesIn.first(); attIn; attIn = attendeesIn.next() ) {
      for ( attEv = attendeesEv.first(); attEv; attEv = attendeesEv.next() ) {
        if (attIn->email()==attEv->email()) {
          //update attendee-info
          kdDebug(5800) << "Scheduler::acceptTransaction update attendee" << endl;
          //attEv->setRole(attIn->role());
          attEv->setStatus(attIn->status());
          //attEv->setRSVP(attIn->RSVP());
          ev->setRevision(ev->revision()+1);
          ret = true;
        }
      }
    }
  }
  if (ret) deleteTransaction(incidence);
  return ret;
}

bool Scheduler::acceptRefresh(IncidenceBase *incidence,ScheduleMessage::Status status)
{
  // handled in korganizer's IncomingDialog
  deleteTransaction(incidence);
  return false;
}

bool Scheduler::acceptCounter(IncidenceBase *incidence,ScheduleMessage::Status status)
{
  deleteTransaction(incidence);
  return false;
}

bool Scheduler::acceptFreeBusy(IncidenceBase *incidence, Method method)
{
  FreeBusy *freebusy = static_cast<FreeBusy *>(incidence);

  QString freeBusyDirName = locateLocal("appdata","freebusy");
  kdDebug() << "acceptFreeBusy:: freeBusyDirName: " << freeBusyDirName << endl;

  QString from;
  if(method == Scheduler::Publish) {
    from = freebusy->organizer();
  }
  if((method == Scheduler::Reply) && (freebusy->attendeeCount() == 1)) {
    Attendee *attendee = freebusy->attendees().first();
    from = attendee->email();
  }

  QDir freeBusyDir(freeBusyDirName);
  if (!freeBusyDir.exists()) {
    kdDebug() << "Directory " << freeBusyDirName << " does not exist!" << endl;
    kdDebug() << "Creating directory: " << freeBusyDirName << endl;
    
    if(!freeBusyDir.mkdir(freeBusyDirName, TRUE)) {
      kdDebug() << "Could not create directory: " << freeBusyDirName << endl;
      return false;
    }
  }

  QString filename(freeBusyDirName);
  filename += "/";
  filename += from;
  filename += ".ifb";
  QFile f(filename);

  kdDebug() << "acceptFreeBusy: filename" << filename << endl;

  freebusy->clearAttendees();
  freebusy->setOrganizer(from);

  QString messageText = mFormat->createScheduleMessage(freebusy, Publish);

  if (!f.open(IO_ReadWrite)) {
   kdDebug() << "acceptFreeBusy: Can't open:" << filename << " for writing" << endl;
   return false;
  }
  QTextStream t(&f);
  t << messageText;
  f.close();
  
  deleteTransaction(incidence);
  return true;
}
