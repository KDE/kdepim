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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
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

bool Scheduler::acceptTransaction( IncidenceBase *incidence,
                                   Method method,
                                   ScheduleMessage::Status status,
                                   const QString &attendee )
{
  kdDebug(5800) << "Scheduler::acceptTransaction, method="
                << methodName( method ) << endl;

  switch (method) {
    case Publish:
      return acceptPublish(incidence, status, method);
    case Request:
      return acceptRequest( incidence, status, attendee );
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

bool Scheduler::acceptRequest( IncidenceBase *incidence,
                               ScheduleMessage::Status status,
                               const QString &attendee )
{
  Incidence *inc = static_cast<Incidence *>(incidence);
  if ( !inc )
    return false;
  if (inc->type()=="FreeBusy") {
    // reply to this request is handled in korganizer's incomingdialog
    return true;
  }

  const Incidence::List existingIncidences = mCalendar->incidencesFromSchedulingID( inc->uid() );
  kdDebug(5800) << "Scheduler::acceptRequest status=" << ScheduleMessage::statusName( status ) << ": found " << existingIncidences.count() << " incidences with schedulingID " << inc->schedulingID() << endl;
  Incidence::List::ConstIterator incit = existingIncidences.begin();
  for ( ; incit != existingIncidences.end() ; ++incit ) {
    Incidence* const i = *incit;
    kdDebug(5800) << "Considering this found event ("
                  << ( i->isReadOnly() ? "readonly" : "readwrite" )
                  << ") :" << mFormat->toString( i ) << endl;
    // If it's readonly, we can't possible update it.
    if ( i->isReadOnly() )
      continue;
    if ( i->revision() <= inc->revision() ) {
      // The new incidence might be an update for the found one
      bool isUpdate = true;
      // Code for new invitations:
      // If you think we could check the value of "status" to be RequestNew: we can't.
      // It comes from a similar check inside libical, where the event is compared to
      // other events in the calendar. But if we have another version of the event around
      // (e.g. shared folder for a group), the status could be RequestNew, Obsolete or Updated.
      kdDebug(5800) << "looking in " << i->uid() << "'s attendees" << endl;
      // This is supposed to be a new request, not an update - however we want to update
      // the existing one to handle the "clicking more than once on the invitation" case.
      // So check the attendee status of the attendee.
      const KCal::Attendee::List attendees = i->attendees();
      KCal::Attendee::List::ConstIterator ait;
      for ( ait = attendees.begin(); ait != attendees.end(); ++ait ) {
        if( (*ait)->email() == attendee && (*ait)->status() == Attendee::NeedsAction ) {
          // This incidence wasn't created by me - it's probably in a shared folder
          // and meant for someone else, ignore it.
          kdDebug(5800) << "ignoring " << i->uid() << " since I'm still NeedsAction there" << endl;
          isUpdate = false;
          break;
        }
      }
      if ( isUpdate ) {
        if ( i->revision() == inc->revision() &&
             i->lastModified() > inc->lastModified() ) {
          // This isn't an update - the found incidence was modified more recently
          kdDebug(5800) << "This isn't an update - the found incidence was modified more recently" << endl;
          deleteTransaction(incidence);
          return false;
        }
        kdDebug(5800) << "replacing existing incidence " << i->uid() << endl;
        mCalendar->deleteIncidence( i );
        break; // replacing one is enough
      }
    } else {
      // This isn't an update - the found incidence has a bigger revision number
      kdDebug(5800) << "This isn't an update - the found incidence has a bigger revision number" << endl;
      deleteTransaction(incidence);
      return false;
    }
  }

  // Move the uid to be the schedulingID and make a unique UID
  inc->setSchedulingID( inc->uid() );
  inc->setUid( CalFormat::createUniqueId() );
  // in case this is an update and we didn't find the to-be-updated incidence,
  // ask whether we should create a new one, or drop the update
  if ( existingIncidences.count() > 0 || inc->revision() == 0 ||
          KMessageBox::warningYesNo( 0,
              i18n("The event, task or journal to be updated could not be found. "
                  "Maybe it has already been deleted, or the calendar that "
                  "contains it is disabled. Press 'Store' to create a new "
                  "one or 'Throw away' to discard this update." ),
              i18n("Discard this update?"), i18n("Store"), i18n("Throw away") ) == KMessageBox::Yes ) {
    kdDebug(5800) << "Storing new incidence with scheduling uid=" << inc->schedulingID() << " and uid=" << inc->uid() << endl;
    mCalendar->addIncidence(inc);
  }
  deleteTransaction(incidence);
  return true;
}

bool Scheduler::acceptAdd(IncidenceBase *incidence,ScheduleMessage::Status /* status */)
{
  deleteTransaction(incidence);
  return false;
}

bool Scheduler::acceptCancel(IncidenceBase *incidence,ScheduleMessage::Status /* status */)
{
  bool ret = false;
  const IncidenceBase *toDelete = mCalendar->incidenceFromSchedulingID( incidence->uid() );
  if ( !toDelete ) {
      KMessageBox::error( 0,
              i18n("The event, task or journal to be canceled could not be found. "
                  "Maybe it has already been deleted, or the calendar that "
                  "contains it is disabled." ) );
  } else {
    Event *even = mCalendar->event(toDelete->uid());
    if (even) {
      mCalendar->deleteEvent(even);
      ret = true;
    } else {
      Todo *todo = mCalendar->todo(toDelete->uid());
      if (todo) {
        mCalendar->deleteTodo(todo);
        ret = true;
      }
    }
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

  // try harder to find the correct incidence
  if ( !ev && !to ) {
    const Incidence::List list = mCalendar->incidences();
    for ( Incidence::List::ConstIterator it = list.begin(), end = list.end(); it != end; ++it ) {
      if ( (*it)->schedulingID() == incidence->uid() ) {
        ev = dynamic_cast<Event*>( *it );
        to = dynamic_cast<Todo*>( *it );
        break;
      }
    }
  }

  if (ev || to) {
    //get matching attendee in calendar
    kdDebug(5800) << "Scheduler::acceptTransaction match found!" << endl;
    Attendee::List attendeesIn = incidence->attendees();
    Attendee::List attendeesEv;
    Attendee::List attendeesNew;
    if (ev) attendeesEv = ev->attendees();
    if (to) attendeesEv = to->attendees();
    Attendee::List::ConstIterator inIt;
    Attendee::List::ConstIterator evIt;
    for ( inIt = attendeesIn.begin(); inIt != attendeesIn.end(); ++inIt ) {
      Attendee *attIn = *inIt;
      bool found = false;
      for ( evIt = attendeesEv.begin(); evIt != attendeesEv.end(); ++evIt ) {
        Attendee *attEv = *evIt;
        if (attIn->email().lower()==attEv->email().lower()) {
          //update attendee-info
          kdDebug(5800) << "Scheduler::acceptTransaction update attendee" << endl;
          attEv->setStatus(attIn->status());
          attEv->setDelegate(attIn->delegate());
          attEv->setDelegator(attIn->delegator());
          ret = true;
          found = true;
        }
      }
      if ( !found && attIn->status() != Attendee::Declined )
        attendeesNew.append( attIn );
    }

    bool attendeeAdded = false;
    for ( Attendee::List::ConstIterator it = attendeesNew.constBegin(); it != attendeesNew.constEnd(); ++it ) {
      Attendee* attNew = *it;
      QString msg = i18n("%1 wants to attend %2 but was not invited.").arg( attNew->fullName() )
          .arg( ev ? ev->summary() : to->summary() );
      if ( !attNew->delegator().isEmpty() )
        msg = i18n("%1 wants to attend %2 on behalf of %3.").arg( attNew->fullName() )
            .arg( ev ? ev->summary() : to->summary() )
            .arg( attNew->delegator() );
      if ( KMessageBox::questionYesNo( 0, msg, i18n("Uninvited attendee"),
           KGuiItem(i18n("Accept Attendance")), KGuiItem(i18n("Reject Attendance")) )
           != KMessageBox::Yes )
      {
        KCal::Incidence *cancel = dynamic_cast<Incidence*>( incidence );
        if ( cancel )
          cancel->addComment( i18n( "The organizer rejected your attendance at this meeting." ) );
        performTransaction( cancel ? cancel : incidence, Scheduler::Cancel, attNew->fullName() );
        delete cancel;
        continue;
      }

      Attendee *a = new Attendee( attNew->name(), attNew->email(), attNew->RSVP(),
                                  attNew->status(), attNew->role(), attNew->uid() );
      a->setDelegate( attNew->delegate() );
      a->setDelegator( attNew->delegator() );
      if ( ev )
        ev->addAttendee( a );
      else if ( to )
        to->addAttendee( a );
      ret = true;
      attendeeAdded = true;
    }

    // send update about new participants
    if ( attendeeAdded ) {
      if ( ev ) {
        ev->setRevision( ev->revision() + 1 );
        performTransaction( ev, Scheduler::Request );
      }
      if ( to ) {
        to->setRevision( ev->revision() + 1 );
        performTransaction( to, Scheduler::Request );
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
    if ( to ) {
      // for VTODO a REPLY can be used to update the completion status of
      // a task. see RFC2446 3.4.3
      Todo *update = dynamic_cast<Todo*> ( incidence );
      Q_ASSERT( update );
      if ( update && ( to->percentComplete() != update->percentComplete() ) ) {
        to->setPercentComplete( update->percentComplete() );
        to->updated();
      }
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
