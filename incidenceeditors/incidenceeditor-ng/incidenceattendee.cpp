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
  , mAttendeeEditor( new AttendeeEditor )
{
  QGridLayout *layout = new QGridLayout( mUi->mAttendeWidgetPlaceHolder );
  layout->setSpacing( 0 );
  layout->addWidget( mAttendeeEditor );

  mAttendeeEditor->setCompletionMode( KGlobalSettings::self()->completionMode() );
  mAttendeeEditor->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );

  mUi->mOrganizerStack->setCurrentIndex( 0 );

  fillOrganizerCombo();
  mUi->mSolveButton->setDisabled( true );
  mUi->mOrganizerLabel->setVisible( false );
}

void IncidenceEditorsNG::IncidenceAttendee::load( KCal::Incidence::ConstPtr incidence )
{
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
    if ( found < 0 ) {
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
  return mAttendeeEditor->isModified();
}

bool IncidenceEditorsNG::IncidenceAttendee::isValid()
{
  //TODO: implement isValid
  return true;
}

void IncidenceEditorsNG::IncidenceAttendee::fillOrganizerCombo()
{
  const QStringList lst = IncidenceEditors::EditorConfig::instance()->fullEmails();
  QStringList uniqueList;
  for ( QStringList::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
    if ( !uniqueList.contains( *it ) ) {
      uniqueList << *it;
    }
  }
  mUi->mOrganizerCombo->addItems( uniqueList );
}
