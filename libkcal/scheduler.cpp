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

#include <qdir.h>
#include <qfile.h>
#include <qtextstream.h>

#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>

#include "event.h"
#include "todo.h"
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
  mFormat = new ICalFormat();
}

Scheduler::~Scheduler()
{
  delete mFormat;
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
      return QString::fromLatin1("Publish");
    case Request:
      return QString::fromLatin1("Request");
    case Refresh:
      return QString::fromLatin1("Refresh");
    case Cancel:
      return QString::fromLatin1("Cancel");
    case Add:
      return QString::fromLatin1("Add");
    case Reply:
      return QString::fromLatin1("Reply");
    case Counter:
      return QString::fromLatin1("Counter");
    case Declinecounter:
      return QString::fromLatin1("Decline Counter");
    default:
      return QString::fromLatin1("Unknown");
  }
}

QString Scheduler::translatedMethodName(Method method)
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
      return i18n("counter proposal","Counter");
    case Declinecounter:
      return i18n("decline counter proposal","Decline Counter");
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
    case ScheduleMessage::Unknown:
    case ScheduleMessage::PublishNew:
      if (!mCalendar->event(incidence->uid())) {
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
  if (inc->type()=="FreeBusy") {
    // reply to this request is handled in korganizer's incomingdialog
    return true;
  } else {
    Event *even = mCalendar->event(incidence->uid());
    if (even) {
      if ( even->revision()<=inc->revision() ) {
        if ( even->revision()==inc->revision() &&
             even->lastModified()>inc->lastModified()) {
          deleteTransaction(incidence);
          return false;
        }
        mCalendar->deleteEvent(even);
      } else {
        deleteTransaction(incidence);
        return false;
      }
    } else {
      Todo *todo = mCalendar->todo(incidence->uid());
      if (todo) {
        if ( todo->revision()<=inc->revision() ) {
          if ( todo->revision()==inc->revision() &&
               todo->lastModified()>inc->lastModified()) {
            deleteTransaction(incidence);
            return false;
          }
          mCalendar->deleteTodo(todo);
        } else {
          deleteTransaction(incidence);
          return false;
        }
      }
    }
  }
  mCalendar->addIncidence(inc);
  deleteTransaction(incidence);
  return true;
}

bool Scheduler::acceptAdd(IncidenceBase *incidence,ScheduleMessage::Status status)
{
  deleteTransaction(incidence);
  return false;
}

bool Scheduler::acceptCancel(IncidenceBase *incidence,ScheduleMessage::Status status)
{
  bool ret = false;
  Event *even = mCalendar->event(incidence->uid());
  if (even) {
    mCalendar->deleteEvent(even);
    ret = true;
  } else {
    Todo *todo = mCalendar->todo(incidence->uid());
    if (todo) {
      mCalendar->deleteTodo(todo);
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
  Event *ev = mCalendar->event(incidence->uid());
  Todo *to = mCalendar->todo(incidence->uid());
  if (ev || to) {
    //get matching attendee in calendar
    kdDebug(5800) << "Scheduler::acceptTransaction match found!" << endl;
    QPtrList<Attendee> attendeesIn = incidence->attendees();
    QPtrList<Attendee> attendeesEv;
    if (ev) attendeesEv = ev->attendees();
    if (to) attendeesEv = to->attendees();
    Attendee *attIn;
    Attendee *attEv;
    for ( attIn = attendeesIn.first(); attIn; attIn = attendeesIn.next() ) {
      for ( attEv = attendeesEv.first(); attEv; attEv = attendeesEv.next() ) {
        if (attIn->email()==attEv->email()) {
          //update attendee-info
          kdDebug(5800) << "Scheduler::acceptTransaction update attendee" << endl;
          attEv->setStatus(attIn->status());
	  attEv->setRSVP(false);
	  // better to not update the sequence number with replys
          //if (ev) ev->setRevision(ev->revision()+1);
	  //if (to) to->setRevision(to->revision()+1);
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
