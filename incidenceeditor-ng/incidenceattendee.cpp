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
#include <KCal/Event>
#include <KComboBox>
#include <KDebug>
#include <KMessageBox>
#include <KPIMUtils/Email>

#ifdef KDEPIM_MOBILE_UI
#include "ui_eventortodomoremobile.h"
#else
#include "ui_eventortododesktop.h"
#endif

#include "attendeeeditor.h"
#include "attendeeline.h"
#include "../editorconfig.h"

using namespace IncidenceEditorsNG;

#ifdef KDEPIM_MOBILE_UI
IncidenceAttendee::IncidenceAttendee( Ui::EventOrTodoMore* ui )
#else
IncidenceAttendee::IncidenceAttendee( Ui::EventOrTodoDesktop* ui )
#endif
  : mUi( ui )
  , mAttendeeEditor( new AttendeeEditor )
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
  mUi->mSolveButton->setDisabled( true );
  mUi->mOrganizerLabel->setVisible( false );

  connect( mUi->mSelectButton, SIGNAL( clicked( bool ) ), this, SLOT( slotSelectAddresses() ) );
}

void IncidenceAttendee::load( KCal::Incidence::ConstPtr incidence )
{
  mOrigIncidence = incidence;
  const bool itsMe = IncidenceEditors::EditorConfig::instance()->thatIsMe( incidence->organizer().email() );

  if ( itsMe || incidence->organizer().isEmpty() ) {
    mUi->mOrganizerStack->setCurrentIndex( 0 );

    int found = -1;
    QString fullOrganizer = incidence->organizer().fullName();
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
    mUi->mOrganizerLabel->setText( incidence->organizer().fullName() );
  }

  KCal::Attendee::List al = incidence->attendees();
  foreach( const KCal::Attendee* a, al ) {
    if( a )
      mAttendeeEditor->addAttendee( *a );
  }

  if ( IncidenceEditor::incidence<KCal::Event::ConstPtr>( ) )
    mAttendeeEditor->setActions( AttendeeLine::EventActions );
  else
    mAttendeeEditor->setActions( AttendeeLine::TodoActions );
}

void IncidenceAttendee::save( KCal::Incidence::Ptr incidence )
{
  incidence->clearAttendees();

  AttendeeData::List attendees = mAttendeeEditor->attendees();

  foreach( AttendeeData::Ptr attPtr, attendees ) {
    KCal::Attendee *attendee = attPtr.data();
    Q_ASSERT( attendee );
    // we create a new attendee object because the original
    // is guarded by qsharedpointer, and the Incidence
    // takes control of the attendee.
    attendee = new KCal::Attendee( *attendee );

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
  //TODO check free busy ?
  if( !mOrigIncidence  )
    return false;
  KCal::Attendee::List origList = mOrigIncidence->attendees();
  AttendeeData::List newList = mAttendeeEditor->attendees();

  if( origList.size() != newList.size() )
    return true;
  
  foreach( const AttendeeData::Ptr a, newList ) {
    KCal::Attendee *attendee = a.data();
    Q_ASSERT( attendee );
    if( !origList.contains( attendee ) )
      return true;
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
  job->setQuery( Akonadi::ContactGroupSearchJob::Name, data->email() );
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

  const KABC::ContactGroup::List contactGroups = searchJob->contactGroups();
  if ( contactGroups.isEmpty() )
    return; // Nothing todo, probably a normal email address was entered

  // TODO: Give the user the possibility to choose a group when there is more than one?!
  KABC::ContactGroup group = contactGroups.first();

  KPIM::MultiplyingLine *line = mMightBeGroupLines.take( job ).data();
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

void IncidenceAttendee::insertAttendeeFromAddressee( const KABC::Addressee& a )
{
  const bool myself = IncidenceEditors::EditorConfig::instance()->thatIsMe( a.preferredEmail() );
  const bool sameAsOrganizer = mUi->mOrganizerCombo &&
                         KPIMUtils::compareEmail( a.preferredEmail(),
                                                  mUi->mOrganizerCombo->currentText(), false );
  KCal::Attendee::PartStat partStat = KCal::Attendee::NeedsAction;
  bool rsvp = true;

  if ( myself && sameAsOrganizer ) {
    partStat = KCal::Attendee::Accepted;
    rsvp = false;
  }
  KCal::Attendee newAt( a.realName(), a.preferredEmail(), rsvp,
                                  partStat, KCal::Attendee::ReqParticipant, a.uid() );;
  mAttendeeEditor->addAttendee( newAt );
}


