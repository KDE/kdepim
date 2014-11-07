/*
  Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
  Copyright (c) 2009-2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

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
#include "attendeeeditor.h"
#include "conflictresolver.h"
#include "editorconfig.h"
#include "incidencedatetime.h"
#include "schedulingdialog.h"
#ifdef KDEPIM_MOBILE_UI
#include "ui_dialogmoremobile.h"
#else
#include "ui_dialogdesktop.h"
#endif

#include <Akonadi/Contact/ContactGroupExpandJob>
#include <Akonadi/Contact/ContactGroupSearchJob>
#include <Akonadi/Contact/EmailAddressSelectionDialog>

#include <KPIMUtils/Email>

#include <QDebug>
#include <QTreeView>
#include <KMessageBox>
#include <KLocalizedString>


using namespace IncidenceEditorNG;

static bool compareAttendees(const KCalCore::Attendee::Ptr &newAttendee,
                             const KCalCore::Attendee::Ptr &originalAttendee)
{
    KCalCore::Attendee::Ptr originalClone(new KCalCore::Attendee(*originalAttendee));

    if (newAttendee->name() != originalAttendee->name()) {
        // What you put into an IncidenceEditorNG::AttendeeLine isn't exactly what you get out.
        // In rare situations, such as "Doe\, John <john.doe@kde.org>", AttendeeLine will normalize
        // the name, and set "Doe, John <john.doe@kde.org>" instead.
        // So, for isDirty() purposes, have that in mind, so we don't return that the editor is dirty
        // when the user didn't edit anything.
        QString dummy;
        QString originalNameNormalized;
        KPIMUtils::extractEmailAddressAndName(originalAttendee->fullName(), dummy, originalNameNormalized);
        originalClone->setName(originalNameNormalized);
    }

    return *newAttendee == *originalClone;
}

#ifdef KDEPIM_MOBILE_UI
IncidenceAttendee::IncidenceAttendee( QWidget *parent, IncidenceDateTime *dateTime,
                                      Ui::EventOrTodoMore *ui )
#else
IncidenceAttendee::IncidenceAttendee( QWidget *parent, IncidenceDateTime *dateTime,
                                      Ui::EventOrTodoDesktop *ui )
#endif
  : mUi( ui ),
    mParentWidget( parent ),
    mAttendeeEditor( new AttendeeEditor ),
    mConflictResolver( 0 ), mDateTime( dateTime )
{
  setObjectName( "IncidenceAttendee" );

  QGridLayout *layout = new QGridLayout( mUi->mAttendeWidgetPlaceHolder );
  layout->setSpacing( 0 );
  layout->addWidget( mAttendeeEditor );

  //QT5 mAttendeeEditor->setCompletionMode( KGlobalSettings::self()->completionMode() );
  mAttendeeEditor->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );

#ifdef KDEPIM_MOBILE_UI
  mAttendeeEditor->setDynamicSizeHint( false );
#endif

  connect(mAttendeeEditor, &AttendeeEditor::countChanged, this, &IncidenceAttendee::attendeeCountChanged);
  connect(mAttendeeEditor, &AttendeeEditor::editingFinished, this, &IncidenceAttendee::checkIfExpansionIsNeeded);

  mUi->mOrganizerStack->setCurrentIndex( 0 );

  fillOrganizerCombo();
  mUi->mSolveButton->setEnabled( false );
  mUi->mOrganizerLabel->setVisible( false );

  mConflictResolver = new ConflictResolver( parent, parent );
  mConflictResolver->setEarliestDate( mDateTime->startDate() );
  mConflictResolver->setEarliestTime( mDateTime->startTime() );
  mConflictResolver->setLatestDate( mDateTime->endDate() );
  mConflictResolver->setLatestTime( mDateTime->endTime() );

  connect(mUi->mSelectButton, &QPushButton::clicked, this, &IncidenceAttendee::slotSelectAddresses);
  connect(mUi->mSolveButton, &QPushButton::clicked, this, &IncidenceAttendee::slotSolveConflictPressed);
  /* Added as part of kolab/issue2297, which is currently under review
  connect(mUi->mOrganizerCombo, static_cast<void (KComboBox::*)(const QString &)>(&KComboBox::activated), this, &IncidenceAttendee::slotOrganizerChanged);
  */
  connect(mUi->mOrganizerCombo, static_cast<void (KComboBox::*)(int)>(&KComboBox::currentIndexChanged), this, &IncidenceAttendee::checkDirtyStatus);

  connect(mDateTime, &IncidenceDateTime::startDateChanged, this, &IncidenceAttendee::slotEventDurationChanged);
  connect(mDateTime, &IncidenceDateTime::endDateChanged, this, &IncidenceAttendee::slotEventDurationChanged);
  connect(mDateTime, &IncidenceDateTime::startTimeChanged, this, &IncidenceAttendee::slotEventDurationChanged);
  connect(mDateTime, &IncidenceDateTime::endTimeChanged, this, &IncidenceAttendee::slotEventDurationChanged);

  connect(mConflictResolver, &ConflictResolver::conflictsDetected, this, &IncidenceAttendee::slotUpdateConflictLabel);

  slotUpdateConflictLabel( 0 ); //initialize label

  connect(mAttendeeEditor, &AttendeeEditor::editingFinished, this, &IncidenceAttendee::checkIfExpansionIsNeeded);
  connect(mAttendeeEditor, &AttendeeEditor::changed, this, &IncidenceAttendee::slotAttendeeChanged);
}

IncidenceAttendee::~IncidenceAttendee() {}

void IncidenceAttendee::load( const KCalCore::Incidence::Ptr &incidence )
{
  mLoadedIncidence = incidence;

  if ( iAmOrganizer() || incidence->organizer()->isEmpty() ) {
    mUi->mOrganizerStack->setCurrentIndex( 0 );

    int found = -1;
    const QString fullOrganizer = incidence->organizer()->fullName();
    const QString organizerEmail = incidence->organizer()->email();
    for ( int i = 0; i < mUi->mOrganizerCombo->count(); ++i ) {
      KCalCore::Person::Ptr organizerCandidate =
        KCalCore::Person::fromFullName( mUi->mOrganizerCombo->itemText( i ) );
      if ( organizerCandidate->email() == organizerEmail ) {
        found = i;
        mUi->mOrganizerCombo->setCurrentIndex( i );
        break;
      }
    }
    if ( found < 0 && !fullOrganizer.isEmpty() ) {
      mUi->mOrganizerCombo->insertItem( 0, fullOrganizer );
      mUi->mOrganizerCombo->setCurrentIndex( 0 );
    }

    mUi->mOrganizerLabel->setVisible( false );
  } else { // someone else is the organizer
    mUi->mOrganizerStack->setCurrentIndex( 1 );
    mUi->mOrganizerLabel->setText( incidence->organizer()->fullName() );
    mUi->mOrganizerLabel->setVisible( true );
  }

  mAttendeeEditor->clear();
  // NOTE: Do this *before* adding the attendees, otherwise the status of the
  //       attendee in the line will be 0 after when returning from load()
  if ( incidence->type() == KCalCore::Incidence::TypeEvent ) {
    mAttendeeEditor->setActions( AttendeeLine::EventActions );
  } else {
    mAttendeeEditor->setActions( AttendeeLine::TodoActions );
  }

  const KCalCore::Attendee::List attendees = incidence->attendees();
  foreach ( const KCalCore::Attendee::Ptr &a, attendees ) {
    mAttendeeEditor->addAttendee( a );
  }

  mWasDirty = false;
}

void IncidenceAttendee::save( const KCalCore::Incidence::Ptr &incidence )
{
  incidence->clearAttendees();
  AttendeeData::List attendees = mAttendeeEditor->attendees();

  foreach ( AttendeeData::Ptr attendee, attendees ) {
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
    if ( !skip ) {
      incidence->addAttendee( attendee );
    }
  }

  // Must not have an organizer for items without attendees
  if ( !incidence->attendeeCount() ) {
    return;
  }

  if ( mUi->mOrganizerStack->currentIndex() == 0 ) {
    incidence->setOrganizer( mUi->mOrganizerCombo->currentText() );
  } else {
    incidence->setOrganizer( mUi->mOrganizerLabel->text() );
  }
}

bool IncidenceAttendee::isDirty() const
{
  if ( iAmOrganizer() ) {
    KCalCore::Event tmp;
    tmp.setOrganizer( mUi->mOrganizerCombo->currentText() );

    if ( mLoadedIncidence->organizer()->email() != tmp.organizer()->email() ) {
      qDebug() << "Organizer changed. Old was " << mLoadedIncidence->organizer()->name()
               << mLoadedIncidence->organizer()->email() << "; new is " << tmp.organizer()->name()
               << tmp.organizer()->email();
      return true;
    }
  }

  const KCalCore::Attendee::List originalList = mLoadedIncidence->attendees();
  AttendeeData::List newList = mAttendeeEditor->attendees();

  // The lists sizes *must* be the same. When the organizer is attending the
  // event as well, he should be in the attendees list as well.
  if ( originalList.size() != newList.size() ) {
    return true;
  }

  // Okay, again not the most efficient algorithm, but I'm assuming that in the
  // bulk of the use cases, the number of attendees is not much higher than 10 or so.
  foreach ( const KCalCore::Attendee::Ptr &attendee, originalList ) {
    bool found = false;
    for ( int i = 0; i < newList.size(); ++i ) {
      if ( compareAttendees( newList.at( i )->attendee(), attendee) ) {
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

void IncidenceAttendee::changeStatusForMe( KCalCore::Attendee::PartStat stat )
{
  const IncidenceEditorNG::EditorConfig *config = IncidenceEditorNG::EditorConfig::instance();
  Q_ASSERT( config );

  AttendeeData::List attendees = mAttendeeEditor->attendees();
  mAttendeeEditor->clear();

  foreach ( const AttendeeData::Ptr &attendee, attendees ) {
    if ( config->thatIsMe( attendee->email() ) ) {
      attendee->setStatus( stat );
    }
    mAttendeeEditor->addAttendee( attendee );
  }

  checkDirtyStatus();
}

void IncidenceAttendee::acceptForMe()
{
  changeStatusForMe( KCalCore::Attendee::Accepted );
}

void IncidenceAttendee::declineForMe()
{
  changeStatusForMe( KCalCore::Attendee::Declined );
}

void IncidenceAttendee::fillOrganizerCombo()
{
  mUi->mOrganizerCombo->clear();
  const QStringList lst = IncidenceEditorNG::EditorConfig::instance()->fullEmails();
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
  if ( !data ) {
    qDebug() << "dynamic cast failed";
    return;
  }

  // For some reason, when pressing enter (instead of tab) the editingFinished()
  // signal is emitted twice. Check if there is already a job running to prevent
  // that we end up with the group members twice.
  if ( mMightBeGroupLines.key( QWeakPointer<KPIM::MultiplyingLine>( line ) ) != 0 ) {
    return;
  }

  Akonadi::ContactGroupSearchJob *job = new Akonadi::ContactGroupSearchJob();
  job->setQuery( Akonadi::ContactGroupSearchJob::Name, data->name() );
  connect(job, &Akonadi::ContactGroupSearchJob::result, this, &IncidenceAttendee::groupSearchResult);

  mMightBeGroupLines.insert( job, QWeakPointer<KPIM::MultiplyingLine>( line ) );
}

void IncidenceAttendee::expandResult( KJob *job )
{
  Akonadi::ContactGroupExpandJob *expandJob = qobject_cast<Akonadi::ContactGroupExpandJob*>( job );
  Q_ASSERT( expandJob );

  const KContacts::Addressee::List groupMembers = expandJob->contacts();
  foreach ( const KContacts::Addressee &member, groupMembers ) {
    insertAttendeeFromAddressee( member );
  }
}

void IncidenceAttendee::groupSearchResult( KJob *job )
{
  Akonadi::ContactGroupSearchJob *searchJob = qobject_cast<Akonadi::ContactGroupSearchJob*>( job );
  Q_ASSERT( searchJob );

  Q_ASSERT( mMightBeGroupLines.contains( job ) );
  KPIM::MultiplyingLine *line = mMightBeGroupLines.take( job ).data();

  const KContacts::ContactGroup::List contactGroups = searchJob->contactGroups();
  if ( contactGroups.isEmpty() ) {
    return; // Nothing todo, probably a normal email address was entered
  }

  // TODO: Give the user the possibility to choose a group when there is more than one?!
  KContacts::ContactGroup group = contactGroups.first();
  if ( line ) {
    line->slotPropagateDeletion();
  }

  Akonadi::ContactGroupExpandJob *expandJob = new Akonadi::ContactGroupExpandJob( group, this );
  connect(expandJob, &Akonadi::ContactGroupExpandJob::result, this, &IncidenceAttendee::expandResult);
  expandJob->start();
}

void IncidenceAttendee::slotSelectAddresses()
{
#ifndef KDEPIM_MOBILE_UI
  QWidget *dialogParent = mAttendeeEditor;
#else
  QWidget *dialogParent = 0;
#endif
  QWeakPointer<Akonadi::EmailAddressSelectionDialog> dialog(
    new Akonadi::EmailAddressSelectionDialog( dialogParent ) );
  dialog.data()->view()->view()->setSelectionMode( QAbstractItemView::ExtendedSelection );

  if ( dialog.data()->exec() == QDialog::Accepted ) {

    Akonadi::EmailAddressSelectionDialog *dialogPtr = dialog.data();
    if ( dialogPtr ) {
      const Akonadi::EmailAddressSelection::List list = dialogPtr->selectedAddresses();
      foreach ( const Akonadi::EmailAddressSelection &selection, list ) {

        if ( selection.item().hasPayload<KContacts::ContactGroup>() ) {
          Akonadi::ContactGroupExpandJob *job =
            new Akonadi::ContactGroupExpandJob(
              selection.item().payload<KContacts::ContactGroup>(), this );
          connect(job, &Akonadi::ContactGroupExpandJob::result, this, &IncidenceAttendee::expandResult);
          job->start();
        } else {
          KContacts::Addressee contact;
          contact.setName( selection.name() );
          contact.insertEmail( selection.email() );

          if ( selection.item().hasPayload<KContacts::Addressee>() ) {
            contact.setUid( selection.item().payload<KContacts::Addressee>().uid() );
          }
          insertAttendeeFromAddressee( contact );
        }
      }
    } else {
      qDebug() << "dialog was already deleted";
    }
  }

  if ( dialog.data() ) {
    dialog.data()->deleteLater();
  }
}

void IncidenceEditorNG::IncidenceAttendee::slotSolveConflictPressed()
{
  const int duration = mDateTime->startTime().secsTo( mDateTime->endTime() );
  QScopedPointer<SchedulingDialog> dialog( new SchedulingDialog( mDateTime->startDate(),
                                                                 mDateTime->startTime(),
                                                                 duration, mConflictResolver,
                                                                 mParentWidget ) );
  dialog->slotUpdateIncidenceStartEnd( mDateTime->currentStartDateTime(),
                                       mDateTime->currentEndDateTime() );
  if ( dialog->exec() == QDialog::Accepted ) {
    qDebug () << dialog->selectedStartDate() << dialog->selectedStartTime();
    mDateTime->setStartDate( dialog->selectedStartDate() );
    mDateTime->setStartTime( dialog->selectedStartTime() );
  }
}

void IncidenceAttendee::slotAttendeeChanged( const KCalCore::Attendee::Ptr &oldAttendee,
                                             const KCalCore::Attendee::Ptr &newAttendee )
{
  // if newAttendee's email is empty, we are probably removing an attendee
  if ( mConflictResolver->containsAttendee( oldAttendee ) ) {
    mConflictResolver->removeAttendee( oldAttendee );
  }
  if ( !mConflictResolver->containsAttendee( newAttendee ) && !newAttendee->email().isEmpty() ) {
    mConflictResolver->insertAttendee( newAttendee );
  }
  checkDirtyStatus();
}

void IncidenceAttendee::slotUpdateConflictLabel( int count )
{
  if ( mAttendeeEditor->attendees().count() > 0 ) {
    mUi->mSolveButton->setEnabled( true );
    if ( count > 0 ) {
      QString label = i18ncp( "@label Shows the number of scheduling conflicts",
                              "%1 conflict",
                              "%1 conflicts", count );
      mUi->mConflictsLabel->setText( label );
      mUi->mConflictsLabel->setVisible( true );
    } else {
      mUi->mConflictsLabel->setVisible( false );
    }
  } else {
    mUi->mSolveButton->setEnabled( false );
    mUi->mConflictsLabel->setVisible( false );
  }
}

bool IncidenceAttendee::iAmOrganizer() const
{
  if ( mLoadedIncidence ) {
    const IncidenceEditorNG::EditorConfig *config = IncidenceEditorNG::EditorConfig::instance();
    return config->thatIsMe( mLoadedIncidence->organizer()->email() );
  }

  return true;
}

void IncidenceAttendee::insertAttendeeFromAddressee( const KContacts::Addressee &a )
{
  const bool sameAsOrganizer = mUi->mOrganizerCombo &&
                               KPIMUtils::compareEmail( a.preferredEmail(),
                                                        mUi->mOrganizerCombo->currentText(),
                                                        false );
  KCalCore::Attendee::PartStat partStat = KCalCore::Attendee::NeedsAction;
  bool rsvp = true;

  if ( iAmOrganizer() && sameAsOrganizer ) {
    partStat = KCalCore::Attendee::Accepted;
    rsvp = false;
  }
  KCalCore::Attendee::Ptr newAt( new KCalCore::Attendee( a.realName(), a.preferredEmail(),
                                                         rsvp,
                                                         partStat,
                                                         KCalCore::Attendee::ReqParticipant,
                                                         a.uid() ) );

  mAttendeeEditor->addAttendee( newAt );
}

void IncidenceAttendee::slotEventDurationChanged()
{
  const KDateTime start = mDateTime->currentStartDateTime();
  const KDateTime end = mDateTime->currentEndDateTime();

  if ( start >= end ) { // This can happen, especially for todos.
    return;
  }

  mConflictResolver->setEarliestDateTime( start );
  mConflictResolver->setLatestDateTime( end );
}

void IncidenceAttendee::slotOrganizerChanged( const QString &newOrganizer )
{
  if ( KPIMUtils::compareEmail( newOrganizer, mOrganizer, false ) ) {
    return;
  }

  QString name;
  QString email;
  bool success = KPIMUtils::extractEmailAddressAndName( newOrganizer, email, name );

  if ( !success ) {
    qWarning() << "Could not extract email address and name";
    return;
  }

  AttendeeData::Ptr currentOrganizerAttendee;
  AttendeeData::Ptr newOrganizerAttendee;

  Q_FOREACH ( AttendeeData::Ptr attendee, mAttendeeEditor->attendees() ) {
    if ( attendee->fullName() == mOrganizer ) {
      currentOrganizerAttendee = attendee;
    }

    if ( attendee->fullName() == newOrganizer ) {
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
  } else {
    answer = KMessageBox::Yes;
  }

  if ( answer == KMessageBox::Yes ) {
    if ( currentOrganizerAttendee ) {
      mAttendeeEditor->removeAttendee( currentOrganizerAttendee );
    }

    if ( !newOrganizerAttendee ) {
      bool rsvp = !iAmOrganizer(); // if it is the user, don't make him rsvp.
      KCalCore::Attendee::PartStat status = iAmOrganizer() ? KCalCore::Attendee::Accepted
                                            : KCalCore::Attendee::NeedsAction;

      KCalCore::Attendee::Ptr newAt(
        new KCalCore::Attendee( name, email, rsvp, status, KCalCore::Attendee::ReqParticipant ) );

      mAttendeeEditor->addAttendee( newAt );
    }
  }
  mOrganizer = newOrganizer;
}

void IncidenceAttendee::printDebugInfo() const
{
  qDebug() << "I'm organizer   : " << iAmOrganizer();
  qDebug() << "Loaded organizer: "<< mLoadedIncidence->organizer()->email();

  if ( iAmOrganizer() ) {
    KCalCore::Event tmp;
    tmp.setOrganizer( mUi->mOrganizerCombo->currentText() );
    qDebug() << "Organizer combo: " << tmp.organizer()->email();
  }

  const KCalCore::Attendee::List originalList = mLoadedIncidence->attendees();
  AttendeeData::List newList = mAttendeeEditor->attendees();
  qDebug() << "List sizes: " << originalList.count() << newList.count();

  if ( originalList.count() != newList.count() ) {
    return;
  }

  // Okay, again not the most efficient algorithm, but I'm assuming that in the
  // bulk of the use cases, the number of attendees is not much higher than 10 or so.
  foreach ( const KCalCore::Attendee::Ptr &attendee, originalList ) {
    bool found = false;
    for ( int i = 0; i < newList.count(); ++i ) {
      if ( *newList.at( i )->attendee() == *attendee ) {
        newList.removeAt( i );
        found = true;
        break;
      }
    }

    if ( !found ) {
      qDebug() << "Attendee not found: " << attendee->email()
               << attendee->name()
               << attendee->status()
               << attendee->RSVP()
               << attendee->role()
               << attendee->uid()
               << attendee->delegate()
               << attendee->delegator()
               << "; we have:";
      for ( int i = 0; i < newList.count(); ++i ) {
        KCalCore::Attendee::Ptr attendee = newList.at( i )->attendee();
        qDebug() << "Attendee: " << attendee->email()
                 << attendee->name()
                 << attendee->status()
                 << attendee->RSVP()
                 << attendee->role()
                 << attendee->uid()
                 << attendee->delegate()
                 << attendee->delegator();
      }

      return;
    }
  }
}
