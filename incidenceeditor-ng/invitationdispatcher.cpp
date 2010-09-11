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

#include "invitationdispatcher.h"

#include <calendarsupport/calendar.h>
#include <calendarsupport/invitationhandler.h>
#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/utils.h>

#include <KCalCore/Incidence>

namespace IncidenceEditorNG {

class InvitationDispatcherPrivate
{
  public: /// Members
    EditorItemManager *mManager;
    CalendarSupport::InvitationHandler mInvitationHandler;
    bool mIsCounterProposal;

  public: /// Functions
    InvitationDispatcherPrivate( CalendarSupport::Calendar *calendar );
    bool myAttendeeStatusChanged( const KCalCore::Incidence::Ptr &oldInc,
                                  const KCalCore::Incidence::Ptr &newInc );
    void processItemSave( EditorItemManager::SaveAction action );
    void sentEventInvitationMessage();
    void sentEventModifiedMessage();
    void resetManager();
};

InvitationDispatcherPrivate::InvitationDispatcherPrivate( CalendarSupport::Calendar *calendar )
  : mManager( 0 ), mInvitationHandler( calendar ), mIsCounterProposal( false )
{ }

bool InvitationDispatcherPrivate::myAttendeeStatusChanged( const KCalCore::Incidence::Ptr &oldInc,
                                                           const KCalCore::Incidence::Ptr &newInc )
{
  KCalCore::Attendee::Ptr oldMe( oldInc->attendeeByMails(
                                   CalendarSupport::KCalPrefs::instance()->allEmails() ) );
  KCalCore::Attendee::Ptr newMe( newInc->attendeeByMails(
                                   CalendarSupport::KCalPrefs::instance()->allEmails() ) );
  if ( oldMe && newMe && ( oldMe->status() != newMe->status() ) ) {
    return true;
  }

  return false;
}

void InvitationDispatcherPrivate::sentEventInvitationMessage()
{
  const KCalCore::Incidence::Ptr newInc =
    CalendarSupport::incidence( mManager->item( EditorItemManager::AfterSave ) );
  const CalendarSupport::InvitationHandler::SendStatus status =
      mInvitationHandler.sendIncidenceCreatedMessage( KCalCore::iTIPRequest, newInc );

  switch ( status ) {
  case CalendarSupport::InvitationHandler::FailAbortUpdate:
    // Okay, at this point we have a new event which is already stored
    // in our calendar, and we need to undo the save.
    mManager->revertLastSave();
    break;
  default: // Canceled, FailKeepUpdate, NoSendingNeeded, Success
    // Canceled       : The user explicitly said that he doesn't want to sent
    //                  a message to the organizer or attendees, keep the
    //                  updated event.
    // FailKeepUpdate : Sending failed, but the user said that he wants to keep
    //                  the updated event.
    // NoSendingNeeded: Everything fine, no need to do anything.
    // Succes         : Everything fine, no need to do anything.
    //
    break;
  }
}

void InvitationDispatcherPrivate::sentEventModifiedMessage()
{
  const KCalCore::Incidence::Ptr oldInc =
    CalendarSupport::incidence( mManager->item( EditorItemManager::BeforeSave ) );
  const KCalCore::Incidence::Ptr newInc =
    CalendarSupport::incidence( mManager->item( EditorItemManager::AfterSave ) );

  CalendarSupport::InvitationHandler::SendStatus status =
    CalendarSupport::InvitationHandler::Success;

  if ( mIsCounterProposal ) {
    status = mInvitationHandler.sendCounterProposal( oldInc, newInc );
  } else {
    const bool attendeeStatusChanged = myAttendeeStatusChanged( oldInc, newInc );
    status = mInvitationHandler.sendIncidenceModifiedMessage( KCalCore::iTIPRequest,
                                                              newInc,
                                                              attendeeStatusChanged );
  }

  switch ( status ) {
  case CalendarSupport::InvitationHandler::FailAbortUpdate:
    // Okay, at this point we have an modified event which is already stored
    // in our calendar, and we need to undo the last changes.
    mManager->revertLastSave();
    break;
  default: // Canceled, FailKeepUpdate, NoSendingNeeded, Success
    // Canceled       : The user explicitly said that he doesn't want to sent
    //                  a message to the organizer or attendees, keep the
    //                  updated event.
    // FailKeepUpdate : Sending failed, but the user said that he wants to keep
    //                  the updated event.
    // NoSendingNeeded: Everything fine, no need to do anything.
    // Succes         : Everything fine, no need to do anything.
    //
    break;
  }
}

void InvitationDispatcherPrivate::processItemSave( EditorItemManager::SaveAction action )
{
  Q_ASSERT( mManager != 0 );

  // At this point the Incidence is saved and it actually was changed.

  switch( action ) {
  case EditorItemManager::Create:
    sentEventInvitationMessage();
    break;
  case EditorItemManager::Modify:
    sentEventModifiedMessage();
    break;
  default:
    return;
  }
}

void InvitationDispatcherPrivate::resetManager()
{
  mManager = 0;
}

/// InvitationDispatcher

InvitationDispatcher::InvitationDispatcher( CalendarSupport::Calendar *calendar, QObject *parent )
  : QObject( parent ), d_ptr( new InvitationDispatcherPrivate( calendar ) )
{
}

InvitationDispatcher::~InvitationDispatcher()
{
  delete d_ptr;
}

void InvitationDispatcher::setIsCounterProposal( bool isCounterProposal )
{
  Q_D( InvitationDispatcher );
  d->mIsCounterProposal = isCounterProposal;
}

void InvitationDispatcher::setItemManager( EditorItemManager *manager )
{
  Q_D( InvitationDispatcher );
  Q_ASSERT( manager );

  if ( d->mManager ) {
    disconnect( d->mManager, SIGNAL(destroyed()) );
    disconnect( d->mManager,
                SIGNAL(itemSaveFinished(CalendarSupport::EditorItemManager::SaveAction)) );
  }

  d->mManager = manager;
  connect( manager, SIGNAL( destroyed() ), SLOT( resetManager() ) );

  qRegisterMetaType<EditorItemManager::SaveAction>( "EditorItemManager::SaveAction" );

  connect( manager, SIGNAL(itemSaveFinished(EditorItemManager::SaveAction)),
           SLOT(processItemSave(EditorItemManager::SaveAction)), Qt::QueuedConnection );
}

}

#include "invitationdispatcher.moc"

