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

#include "../mailscheduler.h"

using namespace Akonadi;
using namespace KCal;

/// Private

namespace Akonadi {

struct InvitationHandler::Private
{
/// Members
  Akonadi::Calendar *mCalendar;
  iTIPMethod mMethod;
  bool mMethodSet;
  QWidget *mParent;

/// Methods
  Private( Akonadi::Calendar *cal );

  InvitationHandler::Action sentInvitation( int messageBoxReturnCode,
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

InvitationHandler::Private::Private( Akonadi::Calendar *cal )
  : mCalendar( cal )
  , mMethod( iTIPNoMethod )
  , mMethodSet( false )
  , mParent( 0 )
{
  Q_ASSERT( mCalendar);
}

InvitationHandler::Action InvitationHandler::Private::sentInvitation( int messageBoxReturnCode,
                                                                    const Incidence::Ptr &incidence )
{
  if ( messageBoxReturnCode == KMessageBox::Yes ) {

    // We will be sending out a message here. Now make sure there is some summary
    if ( incidence->summary().isEmpty() )
      incidence->setSummary( i18n( "<placeholder>No summary given</placeholder>" ) );

    // Send the mail
    MailScheduler scheduler( mCalendar );
    if( scheduler.performTransaction( incidence, mMethod ) )
      return InvitationsSent;

    messageBoxReturnCode = KMessageBox::questionYesNo( mParent,
                                                       i18n( "Sending group scheduling email failed." ),
                                                       i18n( "Group Scheduling Email" ),
                                                       KGuiItem( i18n( "Abort Update" ) ),
                                                       KGuiItem( i18n( "Do Not Send" ) ) );
//    return messageBoxReturnCode == KMessageBox::No;
  } else if ( messageBoxReturnCode == KMessageBox::No ) {
    return CanceledByUser;
  } else {
    Q_ASSERT( false ); // TODO Figure out if this can happen and if so how (maybe by closing the dialog with x)
    return CanceledByUser;
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

InvitationHandler::InvitationHandler( Akonadi::Calendar *cal )
  : d ( new InvitationHandler::Private( cal) )
{ }

InvitationHandler::~InvitationHandler()
{
  delete d;
}

void InvitationHandler::setMethod( iTIPMethod method )
{
  d->mMethod = method;
  d->mMethodSet = true;
}

InvitationHandler::Action InvitationHandler::sendIncidenceCreatedMessage( const Incidence::Ptr &incidence )
{
  Q_ASSERT( d->mMethodSet );
  /// When we created the incidence, we *must* be the organizer.
  Q_ASSERT( d->weAreOrganizerOf( incidence ) );

  int messageBoxReturnCode;

  if ( !d->weNeedToSendMailFor( incidence ) )
    return CanceledByUser;

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

InvitationHandler::Action InvitationHandler::sendIncidenceModifiedMessage( const Incidence::Ptr &incidence, bool attendeeStatusChanged )
{
  Q_ASSERT( d->mMethodSet );

  int messageBoxReturnCode;

  // For a modified incidence, either we are the organizer or someone else.
  if ( d->weAreOrganizerOf( incidence ) ) {

    if ( d->weNeedToSendMailFor( incidence ) ) {
      QString question;
      if ( incidence->type() == "Event" ) {
        question = i18n( "You changed the invitation \"%1\".\n"
                         "Do you want to email the attendees an update message?",
                         incidence->summary() );
      }

      messageBoxReturnCode = KMessageBox::questionYesNo( d->mParent, question, i18n( "Group Scheduling Email" ),
                                                         KGuiItem( i18n( "Send Email" ) ),
                                                         KGuiItem( i18n( "Do Not Send" ) ) );
    } else
      return NoSendingNeeded;

  } else if ( incidence->type() == "Todo" ) {
    if ( d->mMethod == iTIPRequest ) // This is an update to be sent to the organizer
      d->mMethod = iTIPReply;

    QString question = i18n( "Do you want to send a status update to the "
                             "organizer of this task?" );

    messageBoxReturnCode = KMessageBox::questionYesNo( d->mParent, question, QString(),
                                                       KGuiItem( i18n( "Send Update" ) ),
                                                       KGuiItem( i18n( "Do Not Send" ) ) );
  } else if ( incidence->type() == "Event" ) {
    QString question;
    if ( attendeeStatusChanged && d->mMethod == iTIPRequest ) {
      question = i18n( "Your status as an attendee of this event changed. "
                       "Do you want to send a status update to the event organizer?" );
      d->mMethod = iTIPReply;
      messageBoxReturnCode = KMessageBox::questionYesNo( d->mParent, question, QString(),
                                                         KGuiItem( i18n( "Send Update" ) ),
                                                         KGuiItem( i18n( "Do Not Send" ) ) );
    } else {
      question = i18n( "You are not the organizer of this event. Editing it will "
                       "bring your calendar out of sync with the organizer's calendar. "
                       "Do you really want to edit it?" );
      messageBoxReturnCode = KMessageBox::warningYesNo( d->mParent, question );
//      return rc == KMessageBox::Yes;
    }
  }

  return d->sentInvitation( messageBoxReturnCode, incidence );
}

InvitationHandler::Action InvitationHandler::sendIncidenceDeletedMessage( const Incidence::Ptr &incidence )
{
  Q_ASSERT( d->mMethodSet );
  Q_ASSERT( incidence->type() == "Event" || incidence->type() == "Todo" );

  int messageBoxReturnCode;

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

      messageBoxReturnCode = KMessageBox::questionYesNo( d->mParent, question, i18n( "Group Scheduling Email" ),
                                                         KGuiItem( i18n( "Send Email" ) ),
                                                         KGuiItem( i18n( "Do Not Send" ) ) );
    } else
      return NoSendingNeeded;

  } else if ( incidence->type() == "Todo" ) {
    if ( d->mMethod == iTIPRequest ) // This is an update to be sent to the organizer
      d->mMethod = iTIPReply;

    QString question = i18n( "Do you want to send a status update to the "
                             "organizer of this task?" );

    messageBoxReturnCode = KMessageBox::questionYesNo( d->mParent, question, QString(),
                                                       KGuiItem( i18n( "Send Update" ) ),
                                                       KGuiItem( i18n( "Do Not Send" ) ) );
  } else if ( incidence->type() == "Event" ) {

    const QStringList myEmails = KCalPrefs::instance()->allEmails();
    bool askConfirmation = false;
    for ( QStringList::ConstIterator it = myEmails.begin(); it != myEmails.end(); ++it ) {
      QString email = *it;
      Attendee *me = incidence->attendeeByMail(email);
      if ( me &&
           ( me->status() == Attendee::Accepted ||
             me->status() == Attendee::Delegated ) ) {
        askConfirmation = true;
        break;
      }
    }

    if ( !askConfirmation ) {
//      return true;
    }

    QString question = i18n( "You had previously accepted an invitation to this event. "
                             "Do you want to send an updated response to the organizer "
                             "declining the invitation?" );
    messageBoxReturnCode = KMessageBox::questionYesNo( d->mParent, question,
                                                       i18n( "Group Scheduling Email" ),
                                                       KGuiItem( i18n( "Send Update" ) ),
                                                       KGuiItem( i18n( "Do Not Send" ) ) );
//    setDoNotNotify( rc == KMessageBox::No );

  }

  return d->sentInvitation( messageBoxReturnCode, incidence );
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
