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

#include <klocale.h>
#include <kdebug.h>

#include "event.h"
#include "icalformat.h"
#include "calendar.h"

#include "scheduler.h"

using namespace KCal;

ScheduleMessage::ScheduleMessage(Incidence *event,int method,ScheduleMessage::Status status)
{
  mEvent = event;
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

bool Scheduler::acceptTransaction(Incidence *incidence,Method method,ScheduleMessage::Status status)
{
  kdDebug() << "Scheduler::acceptTransaction " << endl;
  switch (method) {
    case Publish:
      return acceptPublish(incidence, status);
    case Request:
      return acceptRequest(incidence, status);
    case Add:
      return acceptAdd(incidence, status);
    case Cancel:
      return acceptCancel(incidence, status);
    case Declinecounter:
      return acceptDeclineCounter(incidence, status);
//    case FreeBusy:
//      return acceptFreeBusy(incidence, status);
    case Reply:
      return acceptReply(incidence, status);
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

bool Scheduler::deleteTransaction(Incidence *)
{
  return true;
}

bool Scheduler::acceptPublish(Incidence *incidence,ScheduleMessage::Status status)
{
  switch (status) {
    case ScheduleMessage::PublishNew:
      if (!mCalendar->getEvent(incidence->VUID())) {
        mCalendar->addIncidence(incidence);
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

bool Scheduler::acceptRequest(Incidence *incidence,ScheduleMessage::Status status)
{
    switch (status) {
    case ScheduleMessage::Obsolete:
      return true;
    case ScheduleMessage::RequestNew:
      mCalendar->addIncidence(incidence);
      deleteTransaction(incidence);
      return true;
    case ScheduleMessage::RequestUpdate:
      Event *even;
      even = mCalendar->getEvent(incidence->VUID());
      if (even) {
        mCalendar->deleteEvent(even);
      }
      mCalendar->addIncidence(incidence);
      deleteTransaction(incidence);
      return true;
    default:
      return false;
    }
  deleteTransaction(incidence);
  return false;
}

bool Scheduler::acceptAdd(Incidence *incidence,ScheduleMessage::Status status)
{
  deleteTransaction(incidence);
  return false;
}

bool Scheduler::acceptCancel(Incidence *incidence,ScheduleMessage::Status status)
{
  bool ret = false;
  //get event in calendar
  QPtrList<Event> eventList;
  eventList=mCalendar->getEvents(incidence->dtStart().date(),incidence->dtStart().date(),false);
  Event *ev;
  for ( ev = eventList.first(); ev; ev = eventList.next() ) {
    if (ev->VUID()==incidence->VUID()) {
      //get matching attendee in calendar
      kdDebug() << "Scheduler::acceptTransaction match found!" << endl;
      mCalendar->deleteEvent(ev);
      ret = true;
    }
  }
  deleteTransaction(incidence);
  return ret;
}

bool Scheduler::acceptDeclineCounter(Incidence *incidence,ScheduleMessage::Status status)
{
  deleteTransaction(incidence);
  return false;
}

//bool Scheduler::acceptFreeBusy(Incidence *incidence,ScheduleMessage::Status status)
//{
//  deleteTransaction(incidence);
//  return false;
//}

bool Scheduler::acceptReply(Incidence *incidence,ScheduleMessage::Status status)
{
  bool ret = false;
  Event *ev = mCalendar->getEvent(incidence->VUID());
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
  deleteTransaction(incidence);
  return ret;
}

bool Scheduler::acceptRefresh(Incidence *incidence,ScheduleMessage::Status status)
{
  // handled in korganizer's IncomingDialog
  deleteTransaction(incidence);
  return false;
}

bool Scheduler::acceptCounter(Incidence *incidence,ScheduleMessage::Status status)
{
  deleteTransaction(incidence);
  return false;
}

