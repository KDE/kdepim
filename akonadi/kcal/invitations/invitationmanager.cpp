/*
    Copyright (C) 2010 Bertjan Broeksema <broeksema@kde.org>
    Copyright (C) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "invitationmanager.h"

#include <boost/shared_ptr.hpp>

#include <KMessageBox>

#include <akonadi/kcal/kcalprefs.h>
#include <KCal/Incidence>
#include <KCal/IncidenceFormatter>

#include "../calendaradaptor.h"
#include "../mailscheduler.h"

using namespace Akonadi;
using namespace KCal;

/// Private

namespace Akonadi {

struct InvitationHandler::Private
{
/// Members
  Calendar *mCalendar;
  iTIPMethod mMethod;
  bool mMethodSet;
  QWidget *mParent;

/// Methods
  Private( Calendar *cal );

  InvitationHandler::SendStatus sentInvitation( int messageBoxReturnCode,
                                                const Incidence::Ptr &incidence );

  /**
    We are the organizer. If there is more than one attendee, or if there is
    only one, and it's not the same as the organizer, ask the user to send
    mail.
  */
  bool weAreOrganizerOf( const Incidence::Ptr &incidence );

  /**
    Assumes that we are the organizer. If there is more than one attendee, or if
    there is only one, and it's not the same as the organizer, ask the user to send
    mail.
   */
  bool weNeedToSendMailFor( const Incidence::Ptr &incidence );
};

}

InvitationHandler::Private::Private( Calendar *cal )
  : mCalendar( cal )
  , mMethod( iTIPNoMethod )
  , mMethodSet( false )
  , mParent( 0 )
{
  Q_ASSERT( mCalendar);
}

InvitationHandler::SendStatus InvitationHandler::Private::sentInvitation( int messageBoxReturnCode,
                                                                          const Incidence::Ptr &incidence )
{
  // The value represented by messageBoxReturnCode is the answer on a question
  // which is a variant of: Do you want to send an email to the attendees?
  //
  // Where the email contains an invitation, modification notification or
  // deletion notification.

  if ( messageBoxReturnCode == KMessageBox::Yes ) {

    // We will be sending out a message here. Now make sure there is some summary
    if ( incidence->summary().isEmpty() )
      incidence->setSummary( i18n( "<placeholder>No summary given</placeholder>" ) );

    // Send the mail
    MailScheduler scheduler( mCalendar );
    if( scheduler.performTransaction( incidence, mMethod ) )
      return InvitationHandler::Success;

    messageBoxReturnCode = KMessageBox::questionYesNo( mParent,
                                                       i18n( "Sending group scheduling email failed." ),
                                                       i18n( "Group Scheduling Email" ),
                                                       KGuiItem( i18n( "Abort Update" ) ),
                                                       KGuiItem( i18n( "Do Not Send" ) ) );
    if ( messageBoxReturnCode == KMessageBox::Yes )
      return InvitationHandler::FailAbortUpdate;
    else
      return InvitationHandler::FailKeepUpdate;

  } else if ( messageBoxReturnCode == KMessageBox::No ) {
    return InvitationHandler::Canceled;
  } else {
    Q_ASSERT( false ); // TODO Figure out if this can happen and if so how (maybe by closing the dialog with x)
    return InvitationHandler::Canceled;
  }
}

bool InvitationHandler::Private::weAreOrganizerOf( const Incidence::Ptr &incidence )
{
  return KCalPrefs::instance()->thatIsMe( incidence->organizer().email() );
}

bool InvitationHandler::Private::weNeedToSendMailFor( const Incidence::Ptr &incidence )
{
  Q_ASSERT( weAreOrganizerOf( incidence ) );

  if ( incidence->attendees().isEmpty() )
    return false;

  // At least one attendee
  return ( incidence->attendees().count() > 1 ||
       incidence->attendees().first()->email() != incidence->organizer().email() );
}

/// InvitationSender

InvitationHandler::InvitationHandler( Calendar *cal )
  : d ( new InvitationHandler::Private( cal) )
{ }

InvitationHandler::~InvitationHandler()
{
  delete d;
}

bool InvitationHandler::receiveInvitation( const QString& receiver,
                                           const QString& iCal,
                                           const QString& type )
{
  const QString action = type;
  ICalFormat mFormat;

  CalendarAdaptor adaptor( d->mCalendar, d->mParent );
  QScopedPointer<ScheduleMessage> message( mFormat.parseScheduleMessage( &adaptor, iCal ) );
  if ( !message ) {
    QString errorMessage = i18n( "Unknown error while parsing iCal invitation" );
    if ( mFormat.exception() )
      errorMessage = i18n( "Error message: %1", mFormat.exception()->message() );

    kDebug() << "Error parsing" << errorMessage;
    KMessageBox::detailedError( d->mParent,
                                i18n( "Error while processing an invitation or update." ),
                                errorMessage );
    return false;
  }

  iTIPMethod method = static_cast<iTIPMethod>( message->method() );
  ScheduleMessage::Status status = message->status();
  Incidence *incidence = dynamic_cast<Incidence*>( message->event() );
  if( !incidence )
    return false;

  MailScheduler scheduler( d->mCalendar );
  if ( action.startsWith( QLatin1String( "accepted" ) ) ||
       action.startsWith( QLatin1String( "tentative" ) ) ||
       action.startsWith( QLatin1String( "delegated" ) ) ||
       action.startsWith( QLatin1String( "counter" ) ) ) {
    // Find myself and set my status. This can't be done in the scheduler,
    // since this does not know the choice I made in the KMail bpf
    const Attendee::List attendees = incidence->attendees();
    foreach ( Attendee *attendee, attendees ) {
      if ( attendee->email() == receiver ) {
        if ( action.startsWith( QLatin1String( "accepted" ) ) ) {
          attendee->setStatus( Attendee::Accepted );
        } else if ( action.startsWith( QLatin1String( "tentative" ) ) ) {
          attendee->setStatus( Attendee::Tentative );
        } else if ( KCalPrefs::instance()->outlookCompatCounterProposals() &&
                    action.startsWith( QLatin1String( "counter" ) ) ) {
          attendee->setStatus( Attendee::Tentative );
        } else if ( action.startsWith( QLatin1String( "delegated" ) ) ) {
          attendee->setStatus( Attendee::Delegated );
        }
        break;
      }
    }
    if ( KCalPrefs::instance()->outlookCompatCounterProposals() ||
         !action.startsWith( QLatin1String( "counter" ) ) ) {
      scheduler.acceptTransaction( incidence, method, status, receiver );
    }
  } else if ( action.startsWith( QLatin1String( "cancel" ) ) ) {
    // Delete the old incidence, if one is present
    scheduler.acceptTransaction( incidence, iTIPCancel, status, receiver );
  } else if ( action.startsWith( QLatin1String( "reply" ) ) ) {
    if ( method != iTIPCounter ) {
      scheduler.acceptTransaction( incidence, method, status, QString() );
    } else {
      scheduler.acceptCounterProposal( incidence );
      // send update to all attendees
      setMethod( iTIPRequest );
      sendIncidenceModifiedMessage( Incidence::Ptr( incidence->clone() ), false );
    }
  } else {
    kError() << "Unknown incoming action" << action;
  }

  if ( action.startsWith( QLatin1String( "counter" ) ) )
    emit editorRequested( Incidence::Ptr( incidence->clone() ) );

  return true;
}

void InvitationHandler::setMethod( iTIPMethod method )
{
  d->mMethod = method;
  d->mMethodSet = true;
}

InvitationHandler::SendStatus InvitationHandler::sendIncidenceCreatedMessage( const Incidence::Ptr &incidence )
{
  Q_ASSERT( d->mMethodSet );
  /// When we created the incidence, we *must* be the organizer.
  Q_ASSERT( d->weAreOrganizerOf( incidence ) );

  int messageBoxReturnCode;

  if ( !d->weNeedToSendMailFor( incidence ) )
    return InvitationHandler::NoSendingNeeded;

  QString question;
  if ( incidence->type() == "Event" ) {
    question = i18n( "The event \"%1\" includes other people.\n"
                     "Do you want to email the invitation to the attendees?",
                     incidence->summary() );
  } else if ( incidence->type() == "Todo" ) {
    question = i18n( "The todo \"%1\" includes other people.\n"
                     "Do you want to email the invitation to the attendees?",
                     incidence->summary() );
  } else {
    question = i18n( "This incidence includes other people. "
                     "Should an email be sent to the attendees?" );
  }

  messageBoxReturnCode = KMessageBox::questionYesNo( d->mParent, question, i18n( "Group Scheduling Email" ),
                                                     KGuiItem( i18n( "Send Email" ) ),
                                                     KGuiItem( i18n( "Do Not Send" ) ) );


  return d->sentInvitation( messageBoxReturnCode, incidence );
}

InvitationHandler::SendStatus InvitationHandler::sendIncidenceModifiedMessage( const Incidence::Ptr &incidence,
                                                                               bool attendeeStatusChanged )
{
  Q_ASSERT( d->mMethodSet );

  // For a modified incidence, either we are the organizer or someone else.
  if ( d->weAreOrganizerOf( incidence ) ) {

    if ( d->weNeedToSendMailFor( incidence ) ) {
      QString question;
      if ( incidence->type() == "Event" ) {
        question = i18n( "You changed the invitation \"%1\".\n"
                         "Do you want to email the attendees an update message?",
                         incidence->summary() );
      }

      const int messageBoxReturnCode = KMessageBox::questionYesNo( d->mParent, question, i18n( "Group Scheduling Email" ),
                                                                   KGuiItem( i18n( "Send Email" ) ),
                                                                   KGuiItem( i18n( "Do Not Send" ) ) );
      return d->sentInvitation( messageBoxReturnCode, incidence );

    } else
      return NoSendingNeeded;

  } else if ( incidence->type() == "Todo" ) {

    if ( d->mMethod == iTIPRequest ) // This is an update to be sent to the organizer
      setMethod( iTIPReply );

    QString question = i18n( "Do you want to send a status update to the "
                             "organizer of this task?" );
    const int messageBoxReturnCode = KMessageBox::questionYesNo( d->mParent, question, QString(),
                                                                 KGuiItem( i18n( "Send Update" ) ),
                                                                 KGuiItem( i18n( "Do Not Send" ) ) );
    return d->sentInvitation( messageBoxReturnCode, incidence );

  } else if ( incidence->type() == "Event" ) {

    QString question;
    if ( attendeeStatusChanged && d->mMethod == iTIPRequest ) {

      question = i18n( "Your status as an attendee of this event changed. "
                       "Do you want to send a status update to the event organizer?" );
      d->mMethod = iTIPReply;
      const int messageBoxReturnCode = KMessageBox::questionYesNo( d->mParent, question, QString(),
                                                                   KGuiItem( i18n( "Send Update" ) ),
                                                                   KGuiItem( i18n( "Do Not Send" ) ) );
      return d->sentInvitation( messageBoxReturnCode, incidence );

    } else {

      question = i18n( "You are not the organizer of this event. Editing it will "
                       "bring your calendar out of sync with the organizer's calendar. "
                       "Do you really want to edit it?" );
      const int messageBoxReturnCode = KMessageBox::warningYesNo( d->mParent, question );
      return d->sentInvitation( messageBoxReturnCode, incidence );

    }
  }

  Q_ASSERT( false ); // Shouldn't happen.
  return NoSendingNeeded;
}

InvitationHandler::SendStatus InvitationHandler::sendIncidenceDeletedMessage( const Incidence::Ptr &incidence )
{
  Q_ASSERT( d->mMethodSet );
  Q_ASSERT( incidence->type() == "Event" || incidence->type() == "Todo" );

  // For a modified incidence, either we are the organizer or someone else.
  if ( d->weAreOrganizerOf( incidence ) ) {

    if ( d->weNeedToSendMailFor( incidence ) ) {
      QString question;
      if ( incidence->type() == "Event" ) {
        question = i18n( "You removed the invitation \"%1\".\n"
                         "Do you want to email the attendees that the event is canceled?",
                         incidence->summary() );
      } else if ( incidence->type() == "Todo" ) {
        question = i18n( "You removed the invitation \"%1\".\n"
                         "Do you want to email the attendees that the todo is canceled?",
                         incidence->summary() );
      }

      int messageBoxReturnCode = KMessageBox::questionYesNo( d->mParent, question, i18n( "Group Scheduling Email" ),
                                                             KGuiItem( i18n( "Send Email" ) ),
                                                             KGuiItem( i18n( "Do Not Send" ) ) );
      return d->sentInvitation( messageBoxReturnCode, incidence );
    } else
      return NoSendingNeeded;

  } else if ( incidence->type() == "Todo" ) {

    if ( d->mMethod == iTIPRequest ) // This is an update to be sent to the organizer
      setMethod( iTIPReply );

    const QString question = i18n( "Do you want to send a status update to the "
                                   "organizer of this task?" );
    int messageBoxReturnCode = KMessageBox::questionYesNo( d->mParent, question, QString(),
                                                           KGuiItem( i18n( "Send Update" ) ),
                                                           KGuiItem( i18n( "Do Not Send" ) ) );
    return d->sentInvitation( messageBoxReturnCode, incidence );

  } else if ( incidence->type() == "Event" ) {

    const QStringList myEmails = KCalPrefs::instance()->allEmails();
    bool incidenceAcceptedBefore = false;
    for ( QStringList::ConstIterator it = myEmails.begin(); it != myEmails.end(); ++it ) {
      QString email = *it;
      Attendee *me = incidence->attendeeByMail(email);
      if ( me &&
           ( me->status() == Attendee::Accepted ||
             me->status() == Attendee::Delegated ) ) {
        incidenceAcceptedBefore = true;
        break;
      }
    }

    if ( incidenceAcceptedBefore ) {
      QString question = i18n( "You had previously accepted an invitation to this event. "
                               "Do you want to send an updated response to the organizer "
                               "declining the invitation?" );
      int messageBoxReturnCode = KMessageBox::questionYesNo( d->mParent, question,
                                                             i18n( "Group Scheduling Email" ),
                                                             KGuiItem( i18n( "Send Update" ) ),
                                                             KGuiItem( i18n( "Do Not Send" ) ) );
      return d->sentInvitation( messageBoxReturnCode, incidence );
    } else {
      // We did not accept the event before and delete it from our calendar agian,
      // so there is no need to notify people.
      return InvitationHandler::NoSendingNeeded;
    }
  }

  Q_ASSERT( false ); // Shouldn't happen.
  return NoSendingNeeded;
}

void InvitationHandler::sendCounterProposal( const Event::Ptr &oldEvent, const Event::Ptr &newEvent ) const
{
  if ( !oldEvent || !newEvent || *oldEvent == *newEvent ||
       !KCalPrefs::instance()->mUseGroupwareCommunication ) {
    return;
  }
  if ( KCalPrefs::instance()->outlookCompatCounterProposals() ) {
    Incidence *tmp = oldEvent->clone();
    tmp->setSummary( i18n( "Counter proposal: %1", newEvent->summary() ) );
    tmp->setDescription( newEvent->description() );
    tmp->addComment( i18n( "Proposed new meeting time: %1 - %2",
                           IncidenceFormatter::dateToString( newEvent->dtStart() ),
                           IncidenceFormatter::dateToString( newEvent->dtEnd() ) ) );
    MailScheduler scheduler( d->mCalendar );
    scheduler.performTransaction( tmp, iTIPReply );
    delete tmp;
  } else {
    MailScheduler scheduler( d->mCalendar );
    scheduler.performTransaction( newEvent, iTIPCounter );
  }
}

#include "invitationmanager.moc"
