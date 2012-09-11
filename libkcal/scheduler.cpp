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

#include "calhelper.h"
#include "event.h"
#include "todo.h"
#include "freebusy.h"
#include "icalformat.h"
#include "calendar.h"
#include "calendarresources.h"
#include "freebusycache.h"
#include "assignmentvisitor.h"

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
      return acceptCancel(incidence, status,  attendee );
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
      if ( calInc && newInc ) {
        if ( (newInc->revision() > calInc->revision()) ||
             (newInc->revision() == calInc->revision() &&
               newInc->lastModified() > calInc->lastModified() ) ) {
          AssignmentVisitor visitor;
          const QString oldUid = calInc->uid();
          if ( !visitor.assign( calInc, newInc ) ) {
            kdError(5800) << "assigning different incidence types" << endl;
          } else {
            calInc->setUid( oldUid );
            calInc->setSchedulingID( newInc->uid() );
            res = true;
          }
        }
      }
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

  Incidence::List existingIncidences = mCalendar->incidencesFromSchedulingID( inc->uid() );
  kdDebug(5800) << "Scheduler::acceptRequest status=" << ScheduleMessage::statusName( status )
                << ": found " << existingIncidences.count() << " incidences with schedulingID "
                << inc->schedulingID() << endl;


  if ( existingIncidences.count() > 1 ) {
    // We must process our own incidences first, so we do a little sort here.
    // This situation basically means we're watching a shared calendar from an attendee that
    // was also invited, so, kontact is showing two events, on each agenda, which are actually
    // the same event.
    kdDebug(5800) << "Scheduler::acceptRequest: found more than one existing incidence!" << endl;
    Incidence::List existingIncidencesCopy;
    Incidence::List::ConstIterator it = existingIncidences.begin();
    for ( ; it != existingIncidences.end() ; ++it ) {
      Incidence *i = *it;
      if ( CalHelper::isMyCalendarIncidence( mCalendar, i ) ) {
        existingIncidencesCopy.prepend( i );
      } else {
        existingIncidencesCopy.append( i );
      }
    }
    existingIncidences = existingIncidencesCopy;
  }

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
        bool res = true;
        AssignmentVisitor visitor;
        const QString oldUid = i->uid();
        Incidence *incidenceCopy = inc->clone();
        incidenceCopy->setSyncStatus( i->syncStatus() );
        const bool incidencesAreEqual = ( *i == *incidenceCopy );

        if ( !incidencesAreEqual ) { // If they are equal, lets not bother the resource with update()s
          kdDebug(5800) << "Scheduler::acceptRequest(): incidences are different, assigning" << endl;
          if ( visitor.assign( i, incidenceCopy ) ) {
            i->startUpdates();
            i->setUids( oldUid, incidenceCopy->uid() );
            i->endUpdates();
          } else {
            kdError(5800) << "assigning different incidence types" << endl;
            res = false;
          }
          delete incidenceCopy;
        } else {
          kdDebug() << "Scheduler::acceptRequest(): incidences are equal, skipping." << endl;
        }

        deleteTransaction( incidence );
        return res;
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
  // notify the user in case this is an update and we didn't find the to-be-updated incidence
  if ( existingIncidences.count() == 0 && inc->revision() > 0 ) {
    KMessageBox::information(
      0,
      i18n( "<qt>"
            "You accepted an invitation update, but an earlier version of the "
            "item could not be found in your calendar.<p>"
            "This may have occurred because:<ul>"
            "<li>the organizer did not include you in the original invitation</li>"
            "<li>you did not accept the original invitation yet</li>"
            "<li>you deleted the original invitation from your calendar</li>"
            "<li>you no longer have access to the calendar containing the invitation</li>"
            "</ul>"
            "This is not a problem, but we thought you should know.</qt>" ),
      i18n( "Cannot find invitation to be updated" ), "AcceptCantFindIncidence" );
  }
  kdDebug(5800) << "Storing new incidence with scheduling uid=" << inc->schedulingID()
                << " and uid=" << inc->uid() << endl;

  CalendarResources *stdcal = dynamic_cast<CalendarResources *>( mCalendar );
  if( stdcal && !stdcal->hasCalendarResources() ) {
    KMessageBox::sorry(
      0,
      i18n( "No calendars found, unable to save the invitation." ) );
    return false;
  }

  // FIXME: This is a nasty hack, since we need to set a parent for the
  //        resource selection dialog. However, we don't have any UI methods
  //        in the calendar, only in the CalendarResources::DestinationPolicy
  //        So we need to type-cast it and extract it from the CalendarResources
  QWidget *tmpparent = 0;
  if ( stdcal ) {
    tmpparent = stdcal->dialogParentWidget();
    stdcal->setDialogParentWidget( 0 );
  }

TryAgain:
  bool success = false;
  if ( stdcal ) {
    success = stdcal->addIncidence( inc );
  } else {
    success = mCalendar->addIncidence( inc );
  }

  if ( !success ) {
    ErrorFormat *e = stdcal ? stdcal->exception() : 0;

    if ( e && e->errorCode() == KCal::ErrorFormat::UserCancel &&
         KMessageBox::warningYesNo(
           0,
           i18n( "You canceled the save operation. Therefore, the appointment will not be "
                 "stored in your calendar even though you accepted the invitation. "
                 "Are you certain you want to discard this invitation? " ),
           i18n( "Discard this invitation?" ),
           i18n( "Discard" ), i18n( "Go Back to Folder Selection" ) ) == KMessageBox::Yes ) {
      KMessageBox::information(
        0,
        i18n( "The invitation \"%1\" was not saved to your calendar "
              "but you are still listed as an attendee for that appointment.\n"
              "If you mistakenly accepted the invitation or do not plan to attend, please notify "
              "the organizer %2 and ask them to remove you from the attendee list.").
        arg( inc->summary(),  inc->organizer().fullName() ) );
      deleteTransaction( incidence );
      return true;
    } else {
      goto TryAgain;
    }

    // We can have a failure if the user pressed [cancel] in the resource
    // selectdialog, so check the exception.
    if ( !e ||
         ( e && ( e->errorCode() != KCal::ErrorFormat::UserCancel &&
                  e->errorCode() != KCal::ErrorFormat::NoWritableFound ) ) ) {
      QString errMessage = i18n( "Unable to save %1 \"%2\"." ).
                           arg( i18n( inc->type() ) ).
                           arg( inc->summary() );
      KMessageBox::sorry( 0, errMessage );
    }
    return false;
  }

  deleteTransaction( incidence );
  return true;
}

bool Scheduler::acceptAdd(IncidenceBase *incidence,ScheduleMessage::Status /* status */)
{
  deleteTransaction(incidence);
  return false;
}

bool Scheduler::acceptCancel( IncidenceBase *incidence,
                              ScheduleMessage::Status status,
                              const QString &attendee )
{
  Incidence *inc = static_cast<Incidence *>( incidence );
  if ( !inc ) {
    return false;
  }

  if ( inc->type() == "FreeBusy" ) {
    // reply to this request is handled in korganizer's incomingdialog
    return true;
  }

  const Incidence::List existingIncidences = mCalendar->incidencesFromSchedulingID( inc->uid() );
  kdDebug(5800) << "Scheduler::acceptCancel="
                << ScheduleMessage::statusName( status )
                << ": found " << existingIncidences.count()
                << " incidences with schedulingID " << inc->schedulingID()
                << endl;

  // Remove existing incidences that aren't stored in my calendar as we
  // will never attempt to remove those -- even if we have write-access.
  Incidence::List myExistingIncidences;
  Incidence::List::ConstIterator incit = existingIncidences.begin();
  for ( ; incit != existingIncidences.end() ; ++incit ) {
    Incidence *i = *incit;
    if ( CalHelper::isMyCalendarIncidence( mCalendar, i ) ) {
      myExistingIncidences.append( i );
    }
  }

  bool ret = false;
  incit = myExistingIncidences.begin();
  for ( ; incit != myExistingIncidences.end() ; ++incit ) {
    Incidence *i = *incit;
    kdDebug(5800) << "Considering this found event ("
                  << ( i->isReadOnly() ? "readonly" : "readwrite" )
                  << ") :" << mFormat->toString( i ) << endl;

    // If it's readonly, we can't possible remove it.
    if ( i->isReadOnly() ) {
      continue;
    }

    // Code for new invitations:
    // We cannot check the value of "status" to be RequestNew because
    // "status" comes from a similar check inside libical, where the event
    // is compared to other events in the calendar. But if we have another
    // version of the event around (e.g. shared folder for a group), the
    // status could be RequestNew, Obsolete or Updated.
    kdDebug(5800) << "looking in " << i->uid() << "'s attendees" << endl;

    // This is supposed to be a new request, not an update - however we want
    // to update the existing one to handle the "clicking more than once
    // on the invitation" case. So check the attendee status of the attendee.
    bool isMine = true;
    const KCal::Attendee::List attendees = i->attendees();
    KCal::Attendee::List::ConstIterator ait;
    for ( ait = attendees.begin(); ait != attendees.end(); ++ait ) {
      if ( (*ait)->email() == attendee &&
           (*ait)->status() == Attendee::NeedsAction ) {
        // This incidence wasn't created by me - it's probably in a shared
        // folder and meant for someone else, ignore it.
        kdDebug(5800) << "ignoring " << i->uid()
                      << " since I'm still NeedsAction there" << endl;
        isMine = false;
        break;
      }
    }

    if ( isMine ) {
      kdDebug(5800) << "removing existing incidence " << i->uid() << endl;
      if ( i->type() == "Event" ) {
        Event *event = mCalendar->event( i->uid() );
        ret = ( event && mCalendar->deleteEvent( event ) );
      } else if ( i->type() == "Todo" ) {
        Todo *todo = mCalendar->todo( i->uid() );
        ret = ( todo && mCalendar->deleteTodo( todo ) );
      }
      deleteTransaction( incidence );
      return ret;
    }
  }

  // in case we didn't find the to-be-removed incidence
  if ( myExistingIncidences.count() > 0 && inc->revision() > 0 ) {
    KMessageBox::information(
      0,
      i18n( "The event or task could not be removed from your calendar. "
            "Maybe it has already been deleted or is not owned by you. "
            "Or it might belong to a read-only or disabled calendar." ) );
  }
  deleteTransaction( incidence );
  return ret;
}

bool Scheduler::acceptCancel(IncidenceBase *incidence,ScheduleMessage::Status /* status */)
{
  const IncidenceBase *toDelete = mCalendar->incidenceFromSchedulingID( incidence->uid() );

  bool ret = true;
  if ( toDelete ) {
    if ( toDelete->type() == "Event" ) {
      Event *event = mCalendar->event( toDelete->uid() );
      ret = ( event && mCalendar->deleteEvent( event ) );
    } else if ( toDelete->type() == "Todo" ) {
      Todo *todo = mCalendar->todo( toDelete->uid() );
      ret = ( todo && mCalendar->deleteTodo( todo ) );
    }
  } else {
    // only complain if we failed to determine the toDelete incidence
    // on non-initial request.
    Incidence *inc = static_cast<Incidence *>( incidence );
    if ( inc->revision() > 0 ) {
      ret = false;
    }
  }

  if ( !ret ) {
    KMessageBox::information(
      0,
      i18n( "The event or task to be canceled could not be removed from your calendar. "
            "Maybe it has already been deleted or is not owned by you. "
            "Or it might belong to a read-only or disabled calendar." ) );
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
          Incidence *incidence = ev ? static_cast<Incidence*>( ev ) :
                                      static_cast<Incidence*>( to );
          incidence->setFieldDirty( Incidence::FieldAttendees );
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
      bool sendMail = false;
      if ( ev || to ) {
        if ( KMessageBox::questionYesNo( 0, i18n( "An attendee was added to the incidence. "
                                                  "Do you want to email the attendees an update message?" ),
                                         i18n( "Attendee Added" ), i18n( "Send Messages" ),
                                         i18n( "Do Not Send" ) ) == KMessageBox::Yes ) {
          sendMail = true;
        }
      }

      if ( ev ) {
        ev->setRevision( ev->revision() + 1 );
        if ( sendMail )
          performTransaction( ev, Scheduler::Request );
      }
      if ( to ) {
        to->setRevision( to->revision() + 1 );
        if ( sendMail )
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
