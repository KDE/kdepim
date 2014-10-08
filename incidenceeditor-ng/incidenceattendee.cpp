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
#include "attendeecomboboxdelegate.h"
#include "freebusymodel/freebusyitemmodel.h"
#ifdef KDEPIM_MOBILE_UI
#include "ui_dialogmoremobile.h"
#else
#include "ui_dialogdesktop.h"
#endif

#include <Akonadi/Contact/ContactGroupExpandJob>
#include <Akonadi/Contact/ContactGroupSearchJob>
#include <Akonadi/Contact/EmailAddressSelectionDialog>

#include <KCalUtils/Stringify>
#include <KPIMUtils/Email>

#include <KDebug>
#include <KMessageBox>

#include <QTreeView>

using namespace IncidenceEditorNG;

#ifdef KDEPIM_MOBILE_UI
IncidenceAttendee::IncidenceAttendee( QWidget *parent, IncidenceDateTime *dateTime,
                                      Ui::EventOrTodoMore *ui )
#else
IncidenceAttendee::IncidenceAttendee( QWidget *parent, IncidenceDateTime *dateTime,
                                      Ui::EventOrTodoDesktop *ui )
#endif
  : mUi( ui ),
    mParentWidget( parent ),
    mStateDelegate(new AttendeeComboBoxDelegate(this)),
    mRoleDelegate(new AttendeeComboBoxDelegate(this)),
    mResponseDelegate(new AttendeeComboBoxDelegate(this)),
    mConflictResolver( 0 ), mDateTime( dateTime )
{
  KCalCore::Attendee::List attendees;
  KCalCore::Attendee::Ptr attendee(new KCalCore::Attendee("", ""));
  attendees.append(attendee);
  mDataModel = new AttendeeTableModel(attendees, this);
  mDataModel->setKeepEmpty(true);
  mDataModel->setRemoveEmptyLines(true);

  #ifdef KDEPIM_MOBILE_UI
  mRoleDelegate->addItem(DesktopIcon("meeting-participant", 48),
                        KCalUtils::Stringify::attendeeRole(KCalCore::Attendee::ReqParticipant));
  mRoleDelegate->addItem(DesktopIcon("meeting-participant-optional", 48),
                        KCalUtils::Stringify::attendeeRole(KCalCore::Attendee::OptParticipant));
  mRoleDelegate->addItem(DesktopIcon("meeting-observer", 48),
                        KCalUtils::Stringify::attendeeRole(KCalCore::Attendee::NonParticipant));
  mRoleDelegate->addItem(DesktopIcon("meeting-chair", 48),
                        KCalUtils::Stringify::attendeeRole(KCalCore::Attendee::Chair));

  mResponseDelegate->addItem( DesktopIcon( "meeting-participant-request-response", 48 ),
                             i18nc( "@item:inlistbox", "Request Response" ) );
  mResponseDelegate->addItem( DesktopIcon( "meeting-participant-no-response", 48 ),
                             i18nc( "@item:inlistbox", "Request No Response" ) );

  #else
  mRoleDelegate->addItem(SmallIcon("meeting-participant"),
                        KCalUtils::Stringify::attendeeRole(KCalCore::Attendee::ReqParticipant));
  mRoleDelegate->addItem(SmallIcon("meeting-participant-optional"),
                        KCalUtils::Stringify::attendeeRole(KCalCore::Attendee::OptParticipant));
  mRoleDelegate->addItem(SmallIcon("meeting-observer"),
                        KCalUtils::Stringify::attendeeRole(KCalCore::Attendee::NonParticipant));
  mRoleDelegate->addItem(SmallIcon("meeting-chair"),
                        KCalUtils::Stringify::attendeeRole(KCalCore::Attendee::Chair));

  mResponseDelegate->addItem( SmallIcon( "meeting-participant-request-response" ),
                             i18nc( "@item:inlistbox", "Request Response" ) );
  mResponseDelegate->addItem( SmallIcon( "meeting-participant-no-response" ),
                             i18nc( "@item:inlistbox", "Request No Response" ) );

  #endif

  mStateDelegate->setWhatsThis( i18nc( "@info:whatsthis",
                                    "Edits the current attendance status of the attendee." ) );

  mRoleDelegate->setWhatsThis( i18nc( "@info:whatsthis",
                                   "Edits the role of the attendee." ) );

  mResponseDelegate->setToolTip( i18nc( "@info:tooltip", "Request a response from the attendee" ) );
  mResponseDelegate->setWhatsThis( i18nc( "@info:whatsthis",
                                       "Edits whether to send an email to the "
                                       "attendee to request a response concerning "
                                       "attendance." ) );


  setObjectName( "IncidenceAttendee" );

  AttendeeFilterProxyModel *filterProxyModel = new AttendeeFilterProxyModel(this);
  filterProxyModel->setDynamicSortFilter(true);
  filterProxyModel->setSourceModel(mDataModel);

#ifdef KDEPIM_MOBILE_UI
#else
  connect(mUi->mGroupSubstitution, SIGNAL(clicked(bool)),
          SLOT(slotGroupSubstitutionPressed()));

  mUi->mAttendeeTable->setModel(filterProxyModel);

  mAttendeeDelegate = new AttendeeLineEditDelegate(this);
  mAttendeeDelegate->setCompletionMode( KGlobalSettings::self()->completionMode() );

  mUi->mAttendeeTable->setItemDelegateForColumn(AttendeeTableModel::Role, roleDelegate());
  mUi->mAttendeeTable->setItemDelegateForColumn(AttendeeTableModel::FullName, attendeeDelegate());
  mUi->mAttendeeTable->setItemDelegateForColumn(AttendeeTableModel::Status, stateDelegate());
  mUi->mAttendeeTable->setItemDelegateForColumn(AttendeeTableModel::Response, responseDelegate());
#endif

  mUi->mOrganizerStack->setCurrentIndex( 0 );

  fillOrganizerCombo();
  mUi->mSolveButton->setEnabled( false );
  mUi->mOrganizerLabel->setVisible( false );

  mConflictResolver = new ConflictResolver( parent, parent );
  mConflictResolver->setEarliestDate( mDateTime->startDate() );
  mConflictResolver->setEarliestTime( mDateTime->startTime() );
  mConflictResolver->setLatestDate( mDateTime->endDate() );
  mConflictResolver->setLatestTime( mDateTime->endTime() );

  connect( mUi->mSelectButton, SIGNAL(clicked(bool)),
           this, SLOT(slotSelectAddresses()) );
  connect( mUi->mSolveButton, SIGNAL(clicked(bool)),
           this, SLOT(slotSolveConflictPressed()) );
  /* Added as part of kolab/issue2297, which is currently under review
  connect( mUi->mOrganizerCombo, SIGNAL(activated(QString)),
           this, SLOT(slotOrganizerChanged(QString)) );
  */
  connect( mUi->mOrganizerCombo, SIGNAL(currentIndexChanged(int)),
           SLOT(checkDirtyStatus()) );

  connect( mDateTime, SIGNAL(startDateChanged(QDate)),
           this, SLOT(slotEventDurationChanged()) );
  connect( mDateTime, SIGNAL(endDateChanged(QDate)),
           this, SLOT(slotEventDurationChanged()) );
  connect( mDateTime, SIGNAL(startTimeChanged(QTime)),
           this, SLOT(slotEventDurationChanged()) );
  connect( mDateTime, SIGNAL(endTimeChanged(QTime)),
           this, SLOT(slotEventDurationChanged()) );

  connect( mConflictResolver, SIGNAL(conflictsDetected(int)),
           this, SLOT(slotUpdateConflictLabel(int)) );

  connect( mConflictResolver->model(),  SIGNAL(rowsInserted(const QModelIndex&, int, int)),
    SLOT(slotFreeBusyAdded(const QModelIndex&, int, int)) );
  connect(mConflictResolver->model(), SIGNAL(layoutChanged()), SLOT(updateFBStatus()));
  connect(mConflictResolver->model(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
      SLOT(slotFreeBusyChanged(const QModelIndex&, const QModelIndex&)));

  slotUpdateConflictLabel( 0 ); //initialize label

  // confict resolver (should show also resources)
  connect(mDataModel, SIGNAL(layoutChanged()), SLOT(slotConflictResolverLayoutChanged()));
  connect(mDataModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int , int)), SLOT(slotConflictResolverAttendeeRemoved(const QModelIndex&,int,int)));
  connect(mDataModel, SIGNAL(rowsInserted(const QModelIndex&, int , int)), SLOT(slotConflictResolverAttendeeAdded(const QModelIndex&,int,int)));
  connect(mDataModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), SLOT(slotConflictResolverAttendeeChanged(const QModelIndex&, const QModelIndex&)));

  //Group substitution
  connect(filterProxyModel, SIGNAL(layoutChanged()), SLOT(slotGroupSubstitutionLayoutChanged()));
  connect(filterProxyModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int , int)), SLOT(slotGroupSubstitutionAttendeeRemoved(const QModelIndex&,int,int)));
  connect(filterProxyModel, SIGNAL(rowsInserted(const QModelIndex&, int , int)), SLOT(slotGroupSubstitutionAttendeeAdded(const QModelIndex&,int,int)));
  connect(filterProxyModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), SLOT(slotGroupSubstitutionAttendeeChanged(const QModelIndex&, const QModelIndex&)));

  connect(filterProxyModel, SIGNAL(rowsInserted(const QModelIndex&, int , int)), SLOT(updateCount()));
  connect(filterProxyModel, SIGNAL(rowsRemoved(const QModelIndex&, int , int)), SLOT(updateCount()));
  // only update when FullName is changed
  connect(filterProxyModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), SLOT(updateCount()));
  connect(filterProxyModel, SIGNAL(layoutChanged()), SLOT(updateCount()));
  connect(filterProxyModel, SIGNAL(layoutChanged()), SLOT(filterLayoutChanged()));
}

IncidenceAttendee::~IncidenceAttendee()
{
}

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

  KCalCore::Attendee::List attendees;
  foreach(const KCalCore::Attendee::Ptr &a, incidence->attendees()) {
      attendees << KCalCore::Attendee::Ptr(new KCalCore::Attendee(*a));
  }

  mDataModel->setAttendees(attendees);
  slotUpdateConflictLabel(0);

  setActions( incidence->type() );

  mWasDirty = false;
}

void IncidenceAttendee::save( const KCalCore::Incidence::Ptr &incidence )
{
  incidence->clearAttendees();
  KCalCore::Attendee::List attendees = mDataModel->attendees();

  foreach(KCalCore::Attendee::Ptr attendee, attendees) {
    Q_ASSERT(attendee);

    bool skip = false;
    if (attendee->fullName().isEmpty()) {
      continue;
    }
    if (KPIMUtils::isValidAddress(attendee->email())) {
      if (KMessageBox::warningYesNo(
        0,
        i18nc("@info",
              "%1 does not look like a valid email address. "
              "Are you sure you want to invite this participant?",
              attendee->email()),
              i18nc("@title:window", "Invalid Email Address")) != KMessageBox::Yes) {
        skip = true;
      }
    }
    if (!skip) {
      incidence->addAttendee(attendee);
    }
  }

  // Must not have an organizer for items without attendees
  if (!incidence->attendeeCount()) {
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
      kDebug() << "Organizer changed. Old was " << mLoadedIncidence->organizer()->name()
               << mLoadedIncidence->organizer()->email() << "; new is " << tmp.organizer()->name()
               << tmp.organizer()->email();
      return true;
    }
  }

  const KCalCore::Attendee::List originalList = mLoadedIncidence->attendees();
  KCalCore::Attendee::List newList;

  foreach(KCalCore::Attendee::Ptr attendee, mDataModel->attendees()) {
    if (!attendee->fullName().isEmpty()) {
      newList.append(attendee);
    }
  }

  // The lists sizes *must* be the same. When the organizer is attending the
  // event as well, he should be in the attendees list as well.
  if (originalList.size() != newList.size()) {
    return true;
  }

  // Okay, again not the most efficient algorithm, but I'm assuming that in the
  // bulk of the use cases, the number of attendees is not much higher than 10 or so.
  foreach(const KCalCore::Attendee::Ptr &attendee, originalList) {
    bool found = false;
    for (int i = 0; i < newList.count(); ++i) {
      if (*(newList[i]) == *attendee) {
        newList.remove(i);
        found = true;
        break;
      }
    }

    if (!found) {
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

  for (int i=0;i<mDataModel->rowCount();i++) {
    QModelIndex index = mDataModel->index(i, AttendeeTableModel::Email);
    if ( config->thatIsMe( mDataModel->data(index, Qt::DisplayRole).toString() ) ) {
      index = mDataModel->index(i, AttendeeTableModel::Status);
      mDataModel->setData(index, stat);
      break;
    }
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

void IncidenceAttendee::checkIfExpansionIsNeeded(const KCalCore::Attendee::Ptr attendee )
{
    QString fullname = attendee->fullName();

    // stop old job
    KJob *oldJob = mMightBeGroupJobs.key(attendee);
    if ( oldJob !=  0 ) {
        disconnect(oldJob);
        oldJob->deleteLater();
        mMightBeGroupJobs.remove(oldJob);
    }

    mGroupList.remove(attendee);

    if (!fullname.isEmpty()) {
        Akonadi::ContactGroupSearchJob *job = new Akonadi::ContactGroupSearchJob();
        job->setQuery( Akonadi::ContactGroupSearchJob::Name, fullname);
        connect( job, SIGNAL(result(KJob*)), this, SLOT(groupSearchResult(KJob*)) );

        mMightBeGroupJobs.insert( job, attendee );
    }
}

void IncidenceAttendee::groupSearchResult( KJob *job )
{
  Akonadi::ContactGroupSearchJob *searchJob = qobject_cast<Akonadi::ContactGroupSearchJob*>( job );
  Q_ASSERT(searchJob);

  Q_ASSERT(mMightBeGroupJobs.contains(job));
  KCalCore::Attendee::Ptr attendee = mMightBeGroupJobs.take(job);

  const KABC::ContactGroup::List contactGroups = searchJob->contactGroups();
  if ( contactGroups.isEmpty() ) {
    updateGroupExpand();
    return; // Nothing todo, probably a normal email address was entered
  }

  // TODO: Give the user the possibility to choose a group when there is more than one?!
  KABC::ContactGroup group = contactGroups.first();

  int row = dataModel()->attendees().indexOf(attendee);
  QModelIndex index = dataModel()->index(row, AttendeeTableModel::CuType);
  dataModel()->setData(index,  KCalCore::Attendee::Group);

  mGroupList.insert(attendee,  group);
  updateGroupExpand();
}

void IncidenceAttendee::updateGroupExpand()
{
#ifndef KDEPIM_MOBILE_UI
    mUi->mGroupSubstitution->setEnabled(mGroupList.count() > 0);
#endif
}


void IncidenceAttendee::slotGroupSubstitutionPressed()
{
    foreach (KCalCore::Attendee::Ptr attendee, mGroupList.keys()) {
        Akonadi::ContactGroupExpandJob *expandJob = new Akonadi::ContactGroupExpandJob( mGroupList.value(attendee), this );
        connect( expandJob, SIGNAL(result(KJob*)), this, SLOT(expandResult(KJob*)) );
        mExpandGroupJobs.insert(expandJob, attendee);
        expandJob->start();
    }
}

void IncidenceAttendee::expandResult( KJob *job )
{
#ifndef KDEPIM_MOBILE_UI
  Akonadi::ContactGroupExpandJob *expandJob = qobject_cast<Akonadi::ContactGroupExpandJob*>( job );
  Q_ASSERT( expandJob );
  Q_ASSERT(mExpandGroupJobs.contains(job));
  KCalCore::Attendee::Ptr attendee = mExpandGroupJobs.take(job);

  int row = dataModel()->attendees().indexOf(attendee);

  dataModel()->removeRow(row);
  const KABC::Addressee::List groupMembers = expandJob->contacts();
  foreach ( const KABC::Addressee &member, groupMembers ) {
      KCalCore::Attendee::Ptr newAt(new KCalCore::Attendee(member.realName(), member.preferredEmail(),
          attendee->RSVP(),
          attendee->status(),
          attendee->role(),
          member.uid()));
      dataModel()->insertAttendee(row, newAt);
  }
#endif
}

void IncidenceAttendee::slotSelectAddresses()
{
  QWeakPointer<Akonadi::EmailAddressSelectionDialog> dialog(
    new Akonadi::EmailAddressSelectionDialog( ) );
  dialog.data()->view()->view()->setSelectionMode( QAbstractItemView::ExtendedSelection );

  if ( dialog.data()->exec() == QDialog::Accepted ) {

    Akonadi::EmailAddressSelectionDialog *dialogPtr = dialog.data();
    if ( dialogPtr ) {
      const Akonadi::EmailAddressSelection::List list = dialogPtr->selectedAddresses();
      foreach ( const Akonadi::EmailAddressSelection &selection, list ) {

        if ( selection.item().hasPayload<KABC::ContactGroup>() ) {
          Akonadi::ContactGroupExpandJob *job =
            new Akonadi::ContactGroupExpandJob(
              selection.item().payload<KABC::ContactGroup>(), this );
          connect( job, SIGNAL(result(KJob*)), this, SLOT(expandResult(KJob*)) );
          job->start();
        } else {
          KABC::Addressee contact;
          contact.setName( selection.name() );
          contact.insertEmail( selection.email() );

          if ( selection.item().hasPayload<KABC::Addressee>() ) {
            contact.setUid( selection.item().payload<KABC::Addressee>().uid() );
          }
          insertAttendeeFromAddressee( contact );
        }
      }
    } else {
      kDebug() << "dialog was already deleted";
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
  if ( dialog->exec() == KDialog::Accepted ) {
    kDebug () << dialog->selectedStartDate() << dialog->selectedStartTime();
    mDateTime->setStartDate( dialog->selectedStartDate() );
    mDateTime->setStartTime( dialog->selectedStartTime() );
  }
}

void IncidenceAttendee::slotConflictResolverAttendeeChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
#ifndef KDEPIM_MOBILE_UI
  if (AttendeeTableModel::FullName <= bottomRight.column() && AttendeeTableModel::FullName >= topLeft.column()) {
       for (int i = topLeft.row(); i <= bottomRight.row(); i++) {
           QModelIndex email = dataModel()->index(i, AttendeeTableModel::Email);
           KCalCore::Attendee::Ptr attendee = dataModel()->data(email,  AttendeeTableModel::AttendeeRole).value<KCalCore::Attendee::Ptr>();
           if (mConflictResolver->containsAttendee(attendee)) {
               mConflictResolver->removeAttendee(attendee);
           }
           if (!dataModel()->data(email).toString().isEmpty()) {
               mConflictResolver->insertAttendee(attendee);
           }
       }
  }
  checkDirtyStatus();
#endif
}

void IncidenceAttendee::slotConflictResolverAttendeeAdded(const QModelIndex &index, int first, int last)
{
    for (int i = first; i <= last; i++) {
        QModelIndex email = dataModel()->index(i, AttendeeTableModel::Email, index);
        if (!dataModel()->data(email).toString().isEmpty()) {
            mConflictResolver->insertAttendee( dataModel()->data(email, AttendeeTableModel::AttendeeRole).value<KCalCore::Attendee::Ptr>() );
        }
    }
    checkDirtyStatus();
}

void IncidenceAttendee::slotConflictResolverAttendeeRemoved(const QModelIndex &index, int first, int last)
{
    for (int i = first; i <= last; i++) {
        QModelIndex email = dataModel()->index(i, AttendeeTableModel::Email, index);
        if (!dataModel()->data(email).toString().isEmpty()) {
            mConflictResolver->removeAttendee(dataModel()->data(email, AttendeeTableModel::AttendeeRole).value<KCalCore::Attendee::Ptr>());
        }
    }
    checkDirtyStatus();
}

void IncidenceAttendee::slotConflictResolverLayoutChanged()
{
    KCalCore::Attendee::List attendees = mDataModel->attendees();
    mConflictResolver->clearAttendees();
    foreach(KCalCore::Attendee::Ptr attendee, attendees) {
        if (!attendee->email().isEmpty()) {
            mConflictResolver->insertAttendee(attendee);
        }
    }
    checkDirtyStatus();
}

void IncidenceAttendee::slotFreeBusyAdded(const QModelIndex &parent, int first, int last)
{
    // We are only interested in toplevel changes
    if (parent.isValid()) {
        return;
    }
    QAbstractItemModel *model = mConflictResolver->model();
    for (int i = first; i <= last; i++) {
        QModelIndex index = model->index(i, 0, parent);
        const KCalCore::Attendee::Ptr &attendee = model->data(index, FreeBusyItemModel::AttendeeRole).value<KCalCore::Attendee::Ptr>();
        const KCalCore::FreeBusy::Ptr &fb = model->data(index, FreeBusyItemModel::FreeBusyRole).value<KCalCore::FreeBusy::Ptr>();
        if (attendee) {
            updateFBStatus(attendee, fb);
        }
    }
}

void IncidenceAttendee::slotFreeBusyChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    // We are only interested in toplevel changes
    if (topLeft.parent().isValid()) {
        return;
    }
    QAbstractItemModel *model = mConflictResolver->model();
    for (int i = topLeft.row(); i <= bottomRight.row(); i++) {
        QModelIndex index = model->index(i, 0);
        const KCalCore::Attendee::Ptr &attendee = model->data(index, FreeBusyItemModel::AttendeeRole).value<KCalCore::Attendee::Ptr>();
        const KCalCore::FreeBusy::Ptr &fb = model->data(index, FreeBusyItemModel::FreeBusyRole).value<KCalCore::FreeBusy::Ptr>();
        if (attendee) {
            updateFBStatus(attendee, fb);
        }
    }
}

void IncidenceAttendee::updateFBStatus()
{
    QAbstractItemModel *model = mConflictResolver->model();
    for (int i = 0; i < model->rowCount(); i++) {
        QModelIndex index = model->index(i, 0);
        const KCalCore::Attendee::Ptr &attendee = model->data(index, FreeBusyItemModel::AttendeeRole).value<KCalCore::Attendee::Ptr>();
        const KCalCore::FreeBusy::Ptr &fb = model->data(index, FreeBusyItemModel::FreeBusyRole).value<KCalCore::FreeBusy::Ptr>();
        if (attendee) {
            updateFBStatus(attendee, fb);
        }
    }
}

void IncidenceAttendee::updateFBStatus(const KCalCore::Attendee::Ptr &attendee, const KCalCore::FreeBusy::Ptr &fb)
{
    KCalCore::Attendee::List attendees = mDataModel->attendees();
    KDateTime startTime = mDateTime->currentStartDateTime();
    KDateTime endTime = mDateTime->currentEndDateTime();
    QAbstractItemModel *model = mConflictResolver->model();
    if (attendees.contains(attendee)) {
        int row = dataModel()->attendees().indexOf(attendee);
        QModelIndex attendeeIndex = dataModel()->index(row, AttendeeTableModel::Available);
        if (fb) {
            KCalCore::Period::List busyPeriods = fb->busyPeriods();
            for ( KCalCore::Period::List::Iterator it = busyPeriods.begin(); it != busyPeriods.end(); ++it ) {
                // periods started before and laping into the incidence (s < startTime && e >= startTime)
                // periods starting in the time of incidende (s >= startTime && s <= endTime)
                if ( ((*it).start() < startTime && (*it).end() > startTime) ||
                    ((*it).start() >= startTime && (*it).start() <= endTime) ) {
                    switch (attendee->status()) {
                    case KCalCore::Attendee::Accepted:
                        dataModel()->setData(attendeeIndex, AttendeeTableModel::Accepted);
                        return;
                    default:
                        dataModel()->setData(attendeeIndex, AttendeeTableModel::Busy);
                        return;
                    }
                }
            }
            dataModel()->setData(attendeeIndex, AttendeeTableModel::Free);
        } else {
            dataModel()->setData(attendeeIndex, AttendeeTableModel::Unkown);
        }
    }
}

void IncidenceAttendee::slotUpdateConflictLabel( int count )
{
  kDebug() <<  "slotUpdateConflictLabel";
  if ( attendeeCount() > 0 ) {
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

void IncidenceAttendee::slotGroupSubstitutionAttendeeChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (AttendeeTableModel::FullName <= bottomRight.column() && AttendeeTableModel::FullName >= topLeft.column()) {
        for (int i = topLeft.row(); i <= bottomRight.row(); i++) {
            QModelIndex email = dataModel()->index(i, AttendeeTableModel::Email);
            KCalCore::Attendee::Ptr attendee = dataModel()->data(email,  AttendeeTableModel::AttendeeRole).value<KCalCore::Attendee::Ptr>();
            checkIfExpansionIsNeeded(attendee);
        }
    }
    updateGroupExpand();
}

void IncidenceAttendee::slotGroupSubstitutionAttendeeAdded(const QModelIndex &index, int first, int last)
{
    Q_UNUSED(index);
    for (int i = first; i <= last; i++) {
        QModelIndex email = dataModel()->index(i, AttendeeTableModel::Email);
        KCalCore::Attendee::Ptr attendee = dataModel()->data(email,  AttendeeTableModel::AttendeeRole).value<KCalCore::Attendee::Ptr>();
        checkIfExpansionIsNeeded(attendee);
    }
    updateGroupExpand();
}

void IncidenceAttendee::slotGroupSubstitutionAttendeeRemoved(const QModelIndex &index, int first, int last)
{
    Q_UNUSED(index);
    for (int i = first; i <= last; i++) {
        QModelIndex email = dataModel()->index(i, AttendeeTableModel::Email);
        KCalCore::Attendee::Ptr attendee = dataModel()->data(email,  AttendeeTableModel::AttendeeRole).value<KCalCore::Attendee::Ptr>();
        KJob *job = mMightBeGroupJobs.key(attendee);
        if (job) {
            disconnect(job);
            job->deleteLater();
            mMightBeGroupJobs.remove(job);
        }
        job = mExpandGroupJobs.key(attendee);
        if (job) {
            disconnect(job);
            job->deleteLater();
            mExpandGroupJobs.remove(job);
        }
        mGroupList.remove(attendee);
      }
    updateGroupExpand();
}

void IncidenceAttendee::slotGroupSubstitutionLayoutChanged()
{
    foreach(KJob *job, mMightBeGroupJobs.keys()) {
        disconnect(job);
        job->deleteLater();
    }
    foreach(KJob *job, mExpandGroupJobs.keys()) {
        disconnect(job);
        job->deleteLater();
    }
    mMightBeGroupJobs.clear();
    mExpandGroupJobs.clear();
    mGroupList.clear();

#ifndef KDEPIM_MOBILE_UI
    QAbstractItemModel *model = mUi->mAttendeeTable->model();
    if (!model ) {
        return;
    }
    for(int i=0;i< model->rowCount(QModelIndex());i++) {
        QModelIndex index = model->index(i,AttendeeTableModel::FullName);
        if (!model->data(index).toString().isEmpty()) {
            QModelIndex email = dataModel()->index(i, AttendeeTableModel::Email);
            KCalCore::Attendee::Ptr attendee = dataModel()->data(email,  AttendeeTableModel::AttendeeRole).value<KCalCore::Attendee::Ptr>();
            checkIfExpansionIsNeeded(attendee);
        }
    }

    updateGroupExpand();
#endif
}

bool IncidenceAttendee::iAmOrganizer() const
{
  if ( mLoadedIncidence ) {
    const IncidenceEditorNG::EditorConfig *config = IncidenceEditorNG::EditorConfig::instance();
    return config->thatIsMe( mLoadedIncidence->organizer()->email() );
  }

  return true;
}

void IncidenceAttendee::insertAttendeeFromAddressee( const KABC::Addressee &a,  int pos/*=-1*/)
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
  if (pos < 0 ) {
      pos = dataModel()->rowCount() - 1;
  }

  dataModel()->insertAttendee(pos, newAt);
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
  updateFBStatus();
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
    kWarning() << "Could not extract email address and name";
    return;
  }

  int currentOrganizerAttendee = -1;
  int newOrganizerAttendee = -1;

  for(int i=0; i<mDataModel->rowCount(); i++) {
    QModelIndex index = mDataModel->index(i,AttendeeTableModel::FullName);
    QString fullName = mDataModel->data(index,Qt::DisplayRole).toString();
    if ( fullName == mOrganizer ) {
      currentOrganizerAttendee = i;
    }

    if ( fullName == newOrganizer ) {
      newOrganizerAttendee = i;
    }
  }

  int answer = KMessageBox::No;
  if ( currentOrganizerAttendee > -1) {
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
    if ( currentOrganizerAttendee > -1 ) {
      mDataModel->removeRows(currentOrganizerAttendee,1);
    }

    if ( newOrganizerAttendee == -1 ) {
      bool rsvp = !iAmOrganizer(); // if it is the user, don't make him rsvp.
      KCalCore::Attendee::PartStat status = iAmOrganizer() ? KCalCore::Attendee::Accepted
                                            : KCalCore::Attendee::NeedsAction;

      KCalCore::Attendee::Ptr newAt(
        new KCalCore::Attendee( name, email, rsvp, status, KCalCore::Attendee::ReqParticipant ) );

      mDataModel->insertAttendee(mDataModel->rowCount(), newAt);
    }
  }
  mOrganizer = newOrganizer;
}

AttendeeTableModel* IncidenceAttendee::dataModel()
{
  return mDataModel;
}

AttendeeComboBoxDelegate* IncidenceAttendee::responseDelegate()
{
  return mResponseDelegate;
}

AttendeeComboBoxDelegate* IncidenceAttendee::roleDelegate()
{
  return mRoleDelegate;
}

AttendeeComboBoxDelegate* IncidenceAttendee::stateDelegate()
{
  return mStateDelegate;
}

AttendeeLineEditDelegate *IncidenceAttendee::attendeeDelegate()
{
  return mAttendeeDelegate;
}


void IncidenceAttendee::filterLayoutChanged()
{
#ifndef KDEPIM_MOBILE_UI
    QHeaderView* headerView = mUi->mAttendeeTable->horizontalHeader();
    headerView->setResizeMode(AttendeeTableModel::Role,  QHeaderView::ResizeToContents);
    headerView->setResizeMode(AttendeeTableModel::FullName, QHeaderView::Stretch);
    headerView->setResizeMode(AttendeeTableModel::Status,  QHeaderView::ResizeToContents);
    headerView->setResizeMode(AttendeeTableModel::Response,  QHeaderView::ResizeToContents);
    headerView->setSectionHidden(AttendeeTableModel::CuType, true);
    headerView->setSectionHidden(AttendeeTableModel::Name, true);
    headerView->setSectionHidden(AttendeeTableModel::Email, true);
    headerView->setSectionHidden(AttendeeTableModel::Available, true);
#endif
}


void IncidenceAttendee::updateCount()
{
  emit attendeeCountChanged(attendeeCount());

  checkDirtyStatus();
}

int IncidenceAttendee::attendeeCount() const
{
#ifndef KDEPIM_MOBILE_UI
    int c=0;
    QModelIndex index;
    QAbstractItemModel *model = mUi->mAttendeeTable->model();
    if (!model ) {
      return 0;
    }
    for(int i=0;i< model->rowCount(QModelIndex());i++) {
      index = model->index(i,AttendeeTableModel::FullName);
      if (!model->data(index).toString().isEmpty()) {
        c++;
      }
    }
    return c;
#endif
    return 0;
}

void IncidenceAttendee::setActions( KCalCore::Incidence::IncidenceType actions )
{
  mStateDelegate->clear();
  if ( actions ==  KCalCore::Incidence::TypeEvent ) {
#ifdef KDEPIM_MOBILE_UI
    mStateDelegate->addItem( DesktopIcon( "task-attention", 48 ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::NeedsAction ) );
    mStateDelegate->addItem( DesktopIcon( "task-accepted", 48 ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Accepted ) );
    mStateDelegate->addItem( DesktopIcon( "task-reject", 48 ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Declined ) );
    mStateDelegate->addItem( DesktopIcon( "task-attempt", 48 ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Tentative ) );
    mStateDelegate->addItem( DesktopIcon( "task-delegate", 48 ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Delegated ) );
#else
    mStateDelegate->addItem( SmallIcon( "task-attention" ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::NeedsAction ) );
    mStateDelegate->addItem( SmallIcon( "task-accepted" ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Accepted ) );
    mStateDelegate->addItem( SmallIcon( "task-reject" ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Declined ) );
    mStateDelegate->addItem( SmallIcon( "task-attempt" ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Tentative ) );
    mStateDelegate->addItem( SmallIcon( "task-delegate" ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Delegated ) );
#endif
  } else {
#ifdef KDEPIM_MOBILE_UI
    mStateDelegate->addItem( DesktopIcon( "task-attention", 48 ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::NeedsAction ) );
    mStateDelegate->addItem( DesktopIcon( "task-accepted", 48 ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Accepted ) );
    mStateDelegate->addItem( DesktopIcon( "task-reject", 48 ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Declined ) );
    mStateDelegate->addItem( DesktopIcon( "task-attempt", 48 ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Tentative ) );
    mStateDelegate->addItem( DesktopIcon( "task-delegate", 48 ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Delegated ) );
    mStateDelegate->addItem( DesktopIcon( "task-complete", 48 ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Completed ) );
    mStateDelegate->addItem( DesktopIcon( "task-ongoing", 48 ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::InProcess ) );
#else
    mStateDelegate->addItem( SmallIcon( "task-attention" ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::NeedsAction ) );
    mStateDelegate->addItem( SmallIcon( "task-accepted" ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Accepted ) );
    mStateDelegate->addItem( SmallIcon( "task-reject" ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Declined ) );
    mStateDelegate->addItem( SmallIcon( "task-attempt" ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Tentative ) );
    mStateDelegate->addItem( SmallIcon( "task-delegate" ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Delegated ) );
    mStateDelegate->addItem( SmallIcon( "task-complete" ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Completed ) );
    mStateDelegate->addItem( SmallIcon( "task-ongoing" ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::InProcess ) );
#endif
  }
}


void IncidenceAttendee::printDebugInfo() const
{
  kDebug() << "I'm organizer   : " << iAmOrganizer();
  kDebug() << "Loaded organizer: "<< mLoadedIncidence->organizer()->email();

  if ( iAmOrganizer() ) {
    KCalCore::Event tmp;
    tmp.setOrganizer( mUi->mOrganizerCombo->currentText() );
    kDebug() << "Organizer combo: " << tmp.organizer()->email();
  }

  const KCalCore::Attendee::List originalList = mLoadedIncidence->attendees();
  KCalCore::Attendee::List newList;

  foreach(KCalCore::Attendee::Ptr attendee, mDataModel->attendees()) {
    if (!attendee->fullName().isEmpty()) {
      newList.append(attendee);
    }
  }

  // Okay, again not the most efficient algorithm, but I'm assuming that in the
  // bulk of the use cases, the number of attendees is not much higher than 10 or so.
  foreach ( const KCalCore::Attendee::Ptr &attendee, originalList ) {
    bool found = false;
    for ( int i = 0; i < newList.count(); ++i ) {
      if ( newList[i] == attendee ) {
        newList.remove(i);
        found = true;
        break;
      }
    }

    if ( !found ) {
      kDebug() << "Attendee not found: " << attendee->email()
               << attendee->name()
               << attendee->status()
               << attendee->RSVP()
               << attendee->role()
               << attendee->uid()
               << attendee->cuType()
               << attendee->delegate()
               << attendee->delegator()
               << "; we have:";
      for ( int i = 0; i < newList.count(); ++i ) {
        KCalCore::Attendee::Ptr attendee = newList[i];
        kDebug() << "Attendee: " << attendee->email()
                 << attendee->name()
                 << attendee->status()
                 << attendee->RSVP()
                 << attendee->role()
                 << attendee->uid()
                 << attendee->cuType()
                 << attendee->delegate()
                 << attendee->delegator();
      }

      return;
    }
  }
}