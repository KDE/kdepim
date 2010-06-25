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

#include <KComboBox>
#include <KDebug>
#include <KMessageBox>
#include <KPIMUtils/Email>

#include <QGridLayout>
#include <QLabel>

#ifdef KDEPIM_MOBILE_UI
IncidenceEditorsNG::IncidenceAttendee::IncidenceAttendee( Ui::EventOrTodoMore* ui )
#else
IncidenceEditorsNG::IncidenceAttendee::IncidenceAttendee( Ui::EventOrTodoDesktop* ui )
#endif
  : mUi( ui )
  , mAttendeeEditor(  new AttendeeEditor )
  , mOrganizerCombo( 0 )
  , mOrganizerLabel( 0 )
{
  gridLayout()->addWidget( mAttendeeEditor, 3, 0, 1, 3 );

  mAttendeeEditor->setCompletionMode( KGlobalSettings::self()->completionMode() );
  mAttendeeEditor->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );

  mOrganizerLabel = new QLabel;
  gridLayout()->addWidget( mOrganizerLabel, 0, 1, 1, 1);
  mOrganizerLabel->hide();

  makeOrganizerCombo();
  mUi->mSolveButton->setDisabled( true );
}

void IncidenceEditorsNG::IncidenceAttendee::load( KCal::Incidence::ConstPtr incidence )
{
  const bool itsMe = IncidenceEditors::EditorConfig::instance()->thatIsMe( incidence->organizer().email() );
  if ( itsMe || incidence->organizer().isEmpty() ) {
    if ( !mOrganizerCombo ) {
      makeOrganizerCombo();
    }
    mOrganizerLabel->hide();
    int found = -1;
    QString fullOrganizer = incidence->organizer().fullName();
    for ( int i = 0; i < mOrganizerCombo->count(); ++i ) {
      if ( mOrganizerCombo->itemText( i ) == fullOrganizer ) {
        found = i;
        mOrganizerCombo->setCurrentIndex( i );
        break;
      }
    }
    if ( found < 0 ) {
      mOrganizerCombo->addItem( fullOrganizer, 0 );
      mOrganizerCombo->setCurrentIndex( 0 );
    }
  } else { // someone else is the organizer
    if ( mOrganizerCombo ) {
      delete mOrganizerCombo;
      mOrganizerCombo = 0;
    }
    mOrganizerLabel->setText( incidence->organizer().fullName() );
    mOrganizerLabel->show();
  }

  KCal::Attendee::List al = incidence->attendees();
  foreach( const KCal::Attendee* a, al ) {
    if( a )
      mAttendeeEditor->addAttendee( *a );
  }
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
  incidence->setOrganizer( mOrganizerCombo->currentText() );
}

bool IncidenceEditorsNG::IncidenceAttendee::isDirty() const
{
  return mAttendeeEditor->isModified();
}

bool IncidenceEditorsNG::IncidenceAttendee::isValid()
{
  //TODO: implement isValid
  return true;
}

void IncidenceEditorsNG::IncidenceAttendee::makeOrganizerCombo()
{
  if ( mOrganizerCombo ) {
      delete mOrganizerCombo;
  }
  mOrganizerCombo = new KComboBox( this );
  gridLayout()->addWidget( mOrganizerCombo, 0, 1, 1, 1);
  QString whatsThis =
    i18nc( "@info:whatsthis",
            "Sets the identity corresponding to "
            "the organizer of this to-do or event. "
            "Identities can be set in the 'Personal' section "
            "of the KOrganizer configuration, or in the "
            "'Personal'->'About Me'->'Password & User Account' "
            "section of the System Settings. In addition, "
            "identities are gathered from your KMail settings "
            "and from your address book. If you choose "
            "to set it globally for KDE in the System Settings, "
            "be sure to check 'Use email settings from "
            "System Settings' in the 'Personal' section of the "
            "KOrganizer configuration." );
  mOrganizerCombo->setWhatsThis( whatsThis );
  mOrganizerCombo->setToolTip(
    i18nc( "@info:tooltip", "Set the organizer identity" ) );

  // Get all emails from KOPrefs (coming from various places),
  // and insert them - removing duplicates
  const QStringList lst = IncidenceEditors::EditorConfig::instance()->fullEmails();
  QStringList uniqueList;
  for ( QStringList::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
    if ( !uniqueList.contains( *it ) ) {
      uniqueList << *it;
    }
  }
  mOrganizerCombo->addItems( uniqueList );
}

QGridLayout* IncidenceEditorsNG::IncidenceAttendee::gridLayout()
{
  QGridLayout *grid = qobject_cast< QGridLayout* >( mUi->mAttendeeWidget->layout() );
  Q_ASSERT( grid );
  return grid;
}


