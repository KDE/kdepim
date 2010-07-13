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

#ifdef KDEPIM_MOBILE_UI
#include "ui_eventortodomoremobile.h"
#else
#include "ui_eventortododesktop.h"
#endif

#include "attendeeeditor.h"
#include "../editorconfig.h"

#include <akonadi/contact/emailaddressselectiondialog.h>
#include <kabc/addressee.h>

#include <KCal/Event>

#include <KComboBox>
#include <KDebug>
#include <KMessageBox>
#include <KPIMUtils/Email>

#include <QGridLayout>
#include <QLabel>
#include <QTreeView>
#include <QWeakPointer>

#ifdef KDEPIM_MOBILE_UI
IncidenceEditorsNG::IncidenceAttendee::IncidenceAttendee( Ui::EventOrTodoMore* ui )
#else
IncidenceEditorsNG::IncidenceAttendee::IncidenceAttendee( Ui::EventOrTodoDesktop* ui )
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

  connect( mAttendeeEditor, SIGNAL( countChanged( int ) ), this, SIGNAL( attendeeCountChanged( int ) ) );

  mUi->mOrganizerStack->setCurrentIndex( 0 );

  fillOrganizerCombo();
  mUi->mSolveButton->setDisabled( true );
  mUi->mOrganizerLabel->setVisible( false );

  connect( mUi->mSelectButton, SIGNAL( clicked( bool ) ), this, SLOT( slotSelectAddresses() ) );
}

void IncidenceEditorsNG::IncidenceAttendee::load( KCal::Incidence::ConstPtr incidence )
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

void IncidenceEditorsNG::IncidenceAttendee::save( KCal::Incidence::Ptr incidence )
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

bool IncidenceEditorsNG::IncidenceAttendee::isDirty() const
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

void IncidenceEditorsNG::IncidenceAttendee::fillOrganizerCombo()
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

void IncidenceEditorsNG::IncidenceAttendee::slotSelectAddresses()
{
  QWeakPointer<Akonadi::EmailAddressSelectionDialog> dialog( new Akonadi::EmailAddressSelectionDialog( mAttendeeEditor ) );
  dialog.data()->view()->view()->setSelectionMode( QAbstractItemView::MultiSelection );

  if ( dialog.data()->exec() == QDialog::Accepted ) {

    Akonadi::EmailAddressSelectionDialog *dialogPtr = dialog.data();
    if ( dialogPtr ) {
      const Akonadi::EmailAddressSelection::List list = dialogPtr->selectedAddresses();
      foreach ( const Akonadi::EmailAddressSelection &selection, list ) {
        KABC::Addressee contact;
        contact.setName( selection.name() );
        contact.insertEmail( selection.email() );

        if ( selection.item().hasPayload<KABC::Addressee>() )
          contact.setUid( selection.item().payload<KABC::Addressee>().uid() );

        insertAttendeeFromAddressee( contact );
      }
    } else {
      kDebug() << "dialog was already deleted";
    }
  }
}

void IncidenceEditorsNG::IncidenceAttendee::insertAttendeeFromAddressee( const KABC::Addressee& a )
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


