/*
    This file is part of libkcal.

    Copyright (c) 2001,2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#include "freebusycache.h"

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
    case PublishUpdate:
      return i18n("Updated Publish");
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

struct Scheduler::Private
{
  Private() : mFreeBusyCache( 0 ) {}

  FreeBusyCache *mFreeBusyCache;
};

Scheduler::Scheduler(Calendar *calendar)
{
  mCalendar = calendar;
  mFormat = new ICalFormat();
  mFormat->setTimeZone( calendar->timeZoneId(), !calendar->isLocalTime() );

  d = new Private;
}

Scheduler::~Scheduler()
{
  delete d;

  delete mFormat;
}

void Scheduler::setFreeBusyCache( FreeBusyCache *c )
{
  d->mFreeBusyCache = c;
}

FreeBusyCache *Scheduler::freeBusyCache() const
{
  return d->mFreeBusyCache;
}

bool Scheduler::acceptTransaction(IncidenceBase *incidence,Method method,ScheduleMessage::Status status)
{
  kdDebug(5800) << "Scheduler::acceptTransaction, method="
                << methodName( method ) << endl;

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
      break;
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

bool Scheduler::acceptPublish( IncidenceBase *newIncBase,
                               ScheduleMessage::Status status, Method method )
{
  if( newIncBase->type() == "FreeBusy" ) {
    return acceptFreeBusy( newIncBase, method );
  }

  bool res = false;
  kdDebug(5800) << "Scheduler::acceptPublish, status="
            << ScheduleMessage::statusName( status ) << endl;
  Incidence *newInc = static_cast<Incidence *>( newIncBase );
  Incidence *calInc = mCalendar->incidence( newIncBase->uid() );
  switch ( status ) {
    case ScheduleMessage::Unknown:
    case ScheduleMessage::PublishNew:
    case ScheduleMessage::PublishUpdate:
      res = true;
      if ( calInc ) {
        if ( (newInc->revision() > calInc->revision()) ||
             (newInc->revision() == calInc->revision() &&
               newInc->lastModified() > calInc->lastModified() ) ) {
          mCalendar->deleteIncidence( calInc );
        } else
          res = false;
      }
      if ( res )
        mCalendar->addIncidence( newInc );
      break;
    case ScheduleMessage::Obsolete:
      res = true;
      break;
    default:
      break;
  }
  deleteTransaction( newIncBase );
  return res;
}

bool Scheduler::acceptRequest(IncidenceBase *newIncBase, ScheduleMessage::Status /* status */)
{
  if (newIncBase->type()=="FreeBusy") {
    // reply to this request is handled in korganizer's incomingdialog
    return true;
  }
  Incidence *newInc = dynamic_cast<Incidence *>( newIncBase );
  if ( newInc ) {
    bool res = true;
    Incidence *exInc = mCalendar->incidenceFromSchedulingID( newIncBase->uid() );
    if ( exInc ) {
      res = false;
      if ( (newInc->revision() > exInc->revision()) ||
           (newInc->revision() == exInc->revision() &&
             newInc->lastModified()>exInc->lastModified()) ) {
        mCalendar->deleteIncidence( exInc );
        res = true;
      }
    }
    if ( res ) {
      // Move the uid to be the schedulingID and make a unique UID
      newInc->setSchedulingID( newInc->uid() );
      newInc->setUid( CalFormat::createUniqueId() );

      mCalendar->addIncidence(newInc);
    }
    deleteTransaction( newIncBase );
    return res;
  }
  return false;
}

bool Scheduler::acceptAdd(IncidenceBase *incidence,ScheduleMessage::Status /* status */)
{
  deleteTransaction(incidence);
  return false;
}

bool Scheduler::acceptCancel(IncidenceBase *incidence,ScheduleMessage::Status /* status */)
{
  bool ret = false;
  Incidence *inc = mCalendar->incidence( incidence->uid() );
  if ( inc ) {
    mCalendar->deleteIncidence( inc );
    ret = true;
  }
  deleteTransaction(incidence);
  return ret;
}

bool Scheduler::acceptDeclineCounter(IncidenceBase *incidence,ScheduleMessage::Status /* status */)
{
  deleteTransaction(incidence);
  return false;
}

//bool Scheduler::acceptFreeBusy(Incidence *incidence,ScheduleMessage::Status status)
//{
//  deleteTransaction(incidence);
//  return false;
//}

bool Scheduler::acceptReply(IncidenceBase *incidence,ScheduleMessage::Status /* status */, Method method)
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
    Attendee::List attendeesIn = incidence->attendees();
    Attendee::List attendeesEv;
    if (ev) attendeesEv = ev->attendees();
    if (to) attendeesEv = to->attendees();
    Attendee::List::ConstIterator inIt;
    Attendee::List::ConstIterator evIt;
    for ( inIt = attendeesIn.begin(); inIt != attendeesIn.end(); ++inIt ) {
      Attendee *attIn = *inIt;
      for ( evIt = attendeesEv.begin(); evIt != attendeesEv.end(); ++evIt ) {
        Attendee *attEv = *evIt;
        if (attIn->email().lower()==attEv->email().lower()) {
          //update attendee-info
          kdDebug(5800) << "Scheduler::acceptTransaction update attendee" << endl;
          attEv->setStatus(attIn->status());
          ret = true;
        }
      }
    }
    if ( ret ) {
      // We set at least one of the attendees, so the incidence changed
      // Note: This should not result in a sequence number bump
      if ( ev )
        ev->updated();
      else if ( to )
        to->updated();
    }
  } else
    kdError(5800) << "No incidence for scheduling\n";
  if (ret) deleteTransaction(incidence);
  return ret;
}

bool Scheduler::acceptRefresh(IncidenceBase *incidence,ScheduleMessage::Status /* status */)
{
  // handled in korganizer's IncomingDialog
  deleteTransaction(incidence);
  return false;
}

bool Scheduler::acceptCounter(IncidenceBase *incidence,ScheduleMessage::Status /* status */)
{
  deleteTransaction(incidence);
  return false;
}

bool Scheduler::acceptFreeBusy(IncidenceBase *incidence, Method method)
{
  if ( !d->mFreeBusyCache ) {
    kdError() << "KCal::Scheduler: no FreeBusyCache." << endl;
    return false;
  }

  FreeBusy *freebusy = static_cast<FreeBusy *>(incidence);

  kdDebug(5800) << "acceptFreeBusy:: freeBusyDirName: " << freeBusyDir() << endl;

  Person from;
  if(method == Scheduler::Publish) {
    from = freebusy->organizer();
  }
  if((method == Scheduler::Reply) && (freebusy->attendeeCount() == 1)) {
    Attendee *attendee = freebusy->attendees().first();
    from = attendee->email();
  }

  if ( !d->mFreeBusyCache->saveFreeBusy( freebusy, from ) ) return false;

  deleteTransaction(incidence);
  return true;
}
