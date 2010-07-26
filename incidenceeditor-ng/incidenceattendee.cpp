/*
    Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
    Copyright (C) 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

    Based on old attendeeeditor.cpp:
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

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

#include "incidenceattendee.h"

#include <QtCore/QWeakPointer>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QTreeView>

#include <akonadi/contact/emailaddressselectiondialog.h>

#include <Akonadi/Contact/ContactGroupExpandJob>
#include <Akonadi/Contact/ContactGroupSearchJob>
#include <KABC/Address>
#include <kcalcore/event.h>
#include <KComboBox>
#include <KDebug>
#include <KMessageBox>
#include <KPIMUtils/Email>
#include <KDateTime>

#ifdef KDEPIM_MOBILE_UI
#include "ui_eventortodomoremobile.h"
#else
#include "ui_eventortododesktop.h"
#include "schedulingdialog.h"
#endif

#include "attendeeeditor.h"
#include "attendeeline.h"
#include "conflictresolver.h"
#include "incidencedatetime.h"
#include "freebusyitemmodel.h"
#include "../editorconfig.h"

using namespace IncidenceEditorsNG;

#ifdef KDEPIM_MOBILE_UI
IncidenceAttendee::IncidenceAttendee( QWidget* parent, IncidenceDateTime *dateTime, Ui::EventOrTodoMore* ui )
#else
IncidenceAttendee::IncidenceAttendee( QWidget* parent, IncidenceDateTime *dateTime, Ui::EventOrTodoDesktop* ui )
#endif
  : mUi( ui )
  , mParentWidget( parent )
  , mAttendeeEditor( new AttendeeEditor )
  , mConflictResolver( 0 )
  , mSchedulingDialog()
  , mDateTime( dateTime )
{
  setObjectName( "IncidenceAttendee" );

  QGridLayout *layout = new QGridLayout( mUi->mAttendeWidgetPlaceHolder );
  layout->setSpacing( 0 );
  layout->addWidget( mAttendeeEditor );

  mAttendeeEditor->setCompletionMode( KGlobalSettings::self()->completionMode() );
  mAttendeeEditor->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );

  connect( mAttendeeEditor, SIGNAL( countChanged( int ) ),
           SIGNAL( attendeeCountChanged( int ) ) );
  connect( mAttendeeEditor, SIGNAL( editingFinished( KPIM::MultiplyingLine* ) ),
           SLOT( checkIfExpansionIsNeeded( KPIM::MultiplyingLine* ) ) );

  mUi->mOrganizerStack->setCurrentIndex( 0 );

  fillOrganizerCombo();
  mUi->mSolveButton->setDisabled( false );
  mUi->mOrganizerLabel->setVisible( false );

  mConflictResolver = new ConflictResolver( parent, parent );
  mConflictResolver->setEarliestDate( mDateTime->startDate() );
  mConflictResolver->setEarliestTime( mDateTime->startTime() );
  mConflictResolver->setLatestDate( mDateTime->endDate() );
  mConflictResolver->setLatestTime( mDateTime->endTime() );

  connect( mUi->mSelectButton, SIGNAL( clicked( bool ) ), this, SLOT( slotSelectAddresses() ) );
  connect( mUi->mSolveButton, SIGNAL( clicked( bool ) ), this, SLOT( slotSolveConflictPressed()) );
  connect( mUi->mOrganizerCombo, SIGNAL( activated( QString) ), this, SLOT( slotOrganizerChanged( QString ) ) );
  connect( mUi->mOrganizerCombo, SIGNAL( currentIndexChanged( int ) ), SLOT( checkDirtyStatus() ) );

  connect( mDateTime, SIGNAL( startDateChanged( QDate ) ), this , SLOT( slotEventDurationChanged() ) );
  connect( mDateTime, SIGNAL( endDateChanged( QDate ) ), this , SLOT( slotEventDurationChanged() ) );
  connect( mDateTime, SIGNAL( startTimeChanged( QTime ) ), this , SLOT( slotEventDurationChanged() ) );
  connect( mDateTime, SIGNAL( endTimeChanged( QTime ) ), this , SLOT( slotEventDurationChanged() ) );

  connect( mConflictResolver, SIGNAL( conflictsDetected( int ) ), this, SLOT( slotUpdateConflictLabel( int ) ) );
  slotUpdateConflictLabel( 0 ); //initialize label

//  connect( mAttendeeEditor, SIGNAL( lineAdded(KPIM::MultiplyingLine*) ),
//           SLOT( checkDirtyStatus() ) );
  connect( mAttendeeEditor, SIGNAL( editingFinished( KPIM::MultiplyingLine* ) ),
           SLOT( checkIfExpansionIsNeeded( KPIM::MultiplyingLine* ) ) );
  connect( mAttendeeEditor, SIGNAL( changed( KCalCore::Attendee::Ptr, KCalCore::Attendee::Ptr ) ),
           SLOT( slotAttendeeChanged( KCalCore::Attendee::Ptr,KCalCore::Attendee::Ptr ) ) );
}

void IncidenceAttendee::load( const KCalCore::Incidence::ConstPtr &incidence )
{
  mLoadedIncidence = incidence;
  const bool itsMe = IncidenceEditors::EditorConfig::instance()->thatIsMe( incidence->organizer()->email() );

  if ( itsMe || incidence->organizer()->isEmpty() ) {
    mUi->mOrganizerStack->setCurrentIndex( 0 );

    int found = -1;
    QString fullOrganizer = incidence->organizer()->fullName();
    for ( int i = 0; i < mUi->mOrganizerCombo->count(); ++i ) {
      if ( mUi->mOrganizerCombo->itemText( i ) == fullOrganizer ) {
        found = i;
        mUi->mOrganizerCombo->setCurrentIndex( i );
        break;
      }
    }
    if ( found < 0 && !fullOrganizer.isEmpty() ) {
      mUi->mOrganizerCombo->addItem( fullOrganizer, 0 );
      mUi->mOrganizerCombo->setCurrentIndex( 0 );
    }
  } else { // someone else is the organizer
    mUi->mOrganizerStack->setCurrentIndex( 1 );
    mUi->mOrganizerLabel->setText( incidence->organizer()->fullName() );
  }

  mAttendeeEditor->clear();
  const KCalCore::Attendee::List al = incidence->attendees();
  foreach( const KCalCore::Attendee::Ptr &a, al )
    mAttendeeEditor->addAttendee( a );

  if ( incidence->type() == KCalCore::Incidence::TypeEvent )
    mAttendeeEditor->setActions( AttendeeLine::EventActions );
  else
    mAttendeeEditor->setActions( AttendeeLine::TodoActions );

  // set the default organizer
  slotOrganizerChanged( mUi->mOrganizerCombo->currentText() );
  mWasDirty = false;
}

void IncidenceAttendee::save( const KCalCore::Incidence::Ptr &incidence )
{
  incidence->clearAttendees();

  AttendeeData::List attendees = mAttendeeEditor->attendees();

  foreach( AttendeeData::Ptr attendee, attendees ) {
    Q_ASSERT( attendee );

    bool skip = false;
    if ( KPIMUtils::isValidAddress( attendee->email() ) ) {
        if ( KMessageBox::warningYesNo(
               0,
               i18nc( "@info",
                      "%1 does not look like a valid email address. "
                      "Are you sure you want to invite this participant?",
                      attendee->email() ),
               i18nc( "@title:window", "Invalid Email Address" ) ) != KMessageBox::Yes ) {
          skip = true;
        }
      }
    if( !skip  )
      incidence->addAttendee( attendee );
  }
  incidence->setOrganizer( mUi->mOrganizerCombo->currentText() );
}

bool IncidenceAttendee::isDirty() const
{

  // When the organizer of the loaded incidence is empty, we assume a new Incidence
  // and in that case the current organizer is the default value and we only
  // consider it dirty when the user selected another organizer.
  if ( mLoadedIncidence->organizer()->isEmpty()
       && mUi->mOrganizerCombo->currentIndex() != 0 ) {
    kDebug() << "Default organizer changed.";
    return true;
  }

  KCalCore::Event tmp;
  tmp.setOrganizer( mUi->mOrganizerCombo->currentText() );
  if ( !mLoadedIncidence->organizer()->isEmpty() && *mLoadedIncidence->organizer() != *tmp.organizer() ) {
    kDebug() << "Organizer changed.";
    return true;
  }

  const KCalCore::Attendee::List origList = mLoadedIncidence->attendees();
  AttendeeData::List newList = mAttendeeEditor->attendees();

  // TODO Since we always add the organizer as a default attendee
  // if the local attendee list only has the organizer in it
  // then report back non dirty
  // this is is an ugly hack until IE-NG supports default data
  if( newList.size() == 1 ) {

    QString name;
    QString email;
    bool success = KPIMUtils::extractEmailAddressAndName( mUi->mOrganizerCombo->currentText(), email, name );
    AttendeeData::Ptr att = newList.front();
    if( att->name() == name &&  att->email() == email )
      return false;
  }
  
  // The lists sizes *must* be the same. When the organizer is attending the
  // event as well, he should be in the attendees list as well.
  if ( origList.size() != newList.size() ) {
    kDebug() << "Number of attendees changed";
    return true;
  }

  // Okay, again not the most efficient algorithm, but I'm assuming that in the
  // bulk of the use cases, the number of attendees is not much higher than 10 or so.
  foreach ( const KCalCore::Attendee::Ptr &attendee, origList ) {
    bool found = false;
    for ( int i = 0; i < newList.size(); ++i ) {
      if ( *newList.at( i )->attendee() == *attendee ) {
        newList.removeAt( i );
        found = true;
        break;
      }
    }

    if ( !found ) {
      // One of the attendees in the original list was not found in the new list.
      return true;
    }
  }

  return false;
}

void IncidenceAttendee::fillOrganizerCombo()
{
  mUi->mOrganizerCombo->clear();
  const QStringList lst = IncidenceEditors::EditorConfig::instance()->fullEmails();
  QStringList uniqueList;
  for ( QStringList::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
    if ( !uniqueList.contains( *it ) ) {
      uniqueList << *it;
    }
  }
  mUi->mOrganizerCombo->addItems( uniqueList );
}

void IncidenceAttendee::checkIfExpansionIsNeeded( KPIM::MultiplyingLine *line )
{
  AttendeeData::Ptr data = qSharedPointerDynamicCast<AttendeeData>( line->data() );
  if ( !data )
    return;

  // For some reason, when pressing enter (in stead of tab) the editingFinished()
  // signal is emitted twice. Check if there is already a job running to prevent
  // that we end up with the group members twice.
  if ( mMightBeGroupLines.key( QWeakPointer<KPIM::MultiplyingLine>( line ) ) != 0 )
    return;

  Akonadi::ContactGroupSearchJob *job = new Akonadi::ContactGroupSearchJob();
  job->setQuery( Akonadi::ContactGroupSearchJob::Name, data->name() );
  connect( job, SIGNAL( result( KJob* ) ), this, SLOT( groupSearchResult( KJob* ) ) );

  mMightBeGroupLines.insert( job, QWeakPointer<KPIM::MultiplyingLine>( line ) );
}

void IncidenceAttendee::expandResult( KJob *job )
{
  Akonadi::ContactGroupExpandJob *expandJob = qobject_cast<Akonadi::ContactGroupExpandJob*>( job );
  Q_ASSERT( expandJob );

  const KABC::Addressee::List groupMembers = expandJob->contacts();
  foreach ( const KABC::Addressee &member, groupMembers )
    insertAttendeeFromAddressee( member );
}

void IncidenceAttendee::groupSearchResult( KJob *job )
{
  Akonadi::ContactGroupSearchJob *searchJob = qobject_cast<Akonadi::ContactGroupSearchJob*>( job );
  Q_ASSERT( searchJob );

  Q_ASSERT( mMightBeGroupLines.contains( job ) );
  KPIM::MultiplyingLine *line = mMightBeGroupLines.take( job ).data();

  const KABC::ContactGroup::List contactGroups = searchJob->contactGroups();
  if ( contactGroups.isEmpty() )
    return; // Nothing todo, probably a normal email address was entered

  // TODO: Give the user the possibility to choose a group when there is more than one?!
  KABC::ContactGroup group = contactGroups.first();
  if ( line )
    line->slotPropagateDeletion();

  Akonadi::ContactGroupExpandJob *expandJob = new Akonadi::ContactGroupExpandJob( group, this );
  connect( expandJob, SIGNAL( result( KJob* ) ), this, SLOT( expandResult( KJob* ) ) );
  expandJob->start();
}

void IncidenceAttendee::slotSelectAddresses()
{
  QWeakPointer<Akonadi::EmailAddressSelectionDialog> dialog( new Akonadi::EmailAddressSelectionDialog( mAttendeeEditor ) );
  dialog.data()->view()->view()->setSelectionMode( QAbstractItemView::MultiSelection );

  if ( dialog.data()->exec() == QDialog::Accepted ) {

    Akonadi::EmailAddressSelectionDialog *dialogPtr = dialog.data();
    if ( dialogPtr ) {
      const Akonadi::EmailAddressSelection::List list = dialogPtr->selectedAddresses();
      foreach ( const Akonadi::EmailAddressSelection &selection, list ) {

        if ( selection.item().hasPayload<KABC::ContactGroup>() ) {
          Akonadi::ContactGroupExpandJob *job = new Akonadi::ContactGroupExpandJob( selection.item().payload<KABC::ContactGroup>(), this );
          connect( job, SIGNAL( result( KJob* ) ), this, SLOT( expandResult( KJob* ) ) );
          job->start();
        } else {
          KABC::Addressee contact;
          contact.setName( selection.name() );
          contact.insertEmail( selection.email() );

          if ( selection.item().hasPayload<KABC::Addressee>() )
            contact.setUid( selection.item().payload<KABC::Addressee>().uid() );

          insertAttendeeFromAddressee( contact );
        }
      }
    } else {
      kDebug() << "dialog was already deleted";
    }
  }
}

void IncidenceEditorsNG::IncidenceAttendee::slotSolveConflictPressed()
{
#ifndef KDEPIM_MOBILE_UI
    if( mSchedulingDialog )
      delete mSchedulingDialog.data();

    mSchedulingDialog = new SchedulingDialog( mConflictResolver, mParentWidget );
    mSchedulingDialog->exec();

    delete mSchedulingDialog.data();
#endif
}

void IncidenceAttendee::slotAttendeeChanged( const KCalCore::Attendee::Ptr& oldAttendee,
                                             const KCalCore::Attendee::Ptr& newAttendee )
{
   // if newAttendee's email is empty, we are probably removing an attendee
   if( mConflictResolver->containsAttendee( oldAttendee ) )
      mConflictResolver->removeAttendee( oldAttendee );
   if( !mConflictResolver->containsAttendee( newAttendee ) && !newAttendee->email().isEmpty() )
      mConflictResolver->insertAttendee( newAttendee );

   checkDirtyStatus();
}


void IncidenceAttendee::slotUpdateConflictLabel( int count )
{
    if( count > 0 ) {
      mUi->mSolveButton->setEnabled( true );
      QString label( i18np( "%1 scheduling conflict", "%1 scheduling conflicts", count ) );
      mUi->mConflictsLabel->setText( label );
    } else {
      QString label( i18n( "No scheduling conflicts" ) );
      mUi->mConflictsLabel->setText( label );
    }
}


void IncidenceAttendee::insertAttendeeFromAddressee( const KABC::Addressee& a )
{
  const bool myself = IncidenceEditors::EditorConfig::instance()->thatIsMe( a.preferredEmail() );
  const bool sameAsOrganizer = mUi->mOrganizerCombo &&
                         KPIMUtils::compareEmail( a.preferredEmail(),
                                                  mUi->mOrganizerCombo->currentText(), false );
  KCalCore::Attendee::PartStat partStat = KCalCore::Attendee::NeedsAction;
  bool rsvp = true;

  if ( myself && sameAsOrganizer ) {
    partStat = KCalCore::Attendee::Accepted;
    rsvp = false;
  }
  KCalCore::Attendee::Ptr newAt( new KCalCore::Attendee( a.realName(), a.preferredEmail(), rsvp,
                                  partStat, KCalCore::Attendee::ReqParticipant, a.uid() ) );

  mAttendeeEditor->addAttendee( newAt );
}

void IncidenceAttendee::slotEventDurationChanged()
{
  KDateTime start = mDateTime->currentStartDateTime();
  KDateTime end = mDateTime->currentEndDateTime();

  Q_ASSERT( start < end );

  kDebug() << start << end;

#ifndef KDEPIM_MOBILE_UI
  if( !mSchedulingDialog ) {
    // when we aren't showing the dialog, the search timeframe
    // should be the same as the event's.
    mConflictResolver->setEarliestDateTime( start );
    mConflictResolver->setLatestDateTime( end );
  }
#endif
}

void IncidenceAttendee::slotOrganizerChanged( const QString & newOrganizer )
{
  if ( KPIMUtils::compareEmail( newOrganizer, mOrganizer, false ) )
    return;

  QString name;
  QString email;
  bool success = KPIMUtils::extractEmailAddressAndName( newOrganizer, email, name );

  if ( !success ) {
    return;
  }

  AttendeeData::Ptr currentOrganizerAttendee;
  AttendeeData::Ptr newOrganizerAttendee;

  Q_FOREACH( AttendeeData::Ptr attendee, mAttendeeEditor->attendees() ) {
    if( attendee->fullName() == mOrganizer ) {
      currentOrganizerAttendee = attendee;
    }

    if( attendee->fullName() == newOrganizer ) {
      newOrganizerAttendee = attendee;
    }
  }

  int answer = KMessageBox::No;
  if ( currentOrganizerAttendee ) {
    answer = KMessageBox::questionYesNo(
      mParentWidget,
      i18nc( "@option",
             "You are changing the organizer of this event. "
             "Since the organizer is also attending this event, would you "
             "like to change the corresponding attendee as well?" ) );
  } else
    answer = KMessageBox::Yes;

  if ( answer == KMessageBox::Yes ) {
    if ( currentOrganizerAttendee ) {
      mAttendeeEditor->removeAttendee( currentOrganizerAttendee );
    }

    if ( !newOrganizerAttendee ) {
      bool myself = IncidenceEditors::EditorConfig::instance()->thatIsMe( email );

      bool rsvp = !myself; // if it is the user, dont make him rsvp.
      KCalCore::Attendee::PartStat status = myself ? KCalCore::Attendee::Accepted : KCalCore::Attendee::NeedsAction;

      KCalCore::Attendee::Ptr newAt( new KCalCore::Attendee( name, email, rsvp, status, KCalCore::Attendee::ReqParticipant ) );

      mAttendeeEditor->addAttendee( newAt );
    }
  }
  mOrganizer = newOrganizer;
}


