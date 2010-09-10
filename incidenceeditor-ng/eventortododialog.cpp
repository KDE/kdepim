/*
  Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
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

#include "eventortododialog.h"

#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/utils.h>

#include <KCalCore/MemoryCalendar>
#include <KCalCore/ICalFormat>
#include <KCalCore/Incidence>
#include <KCalCore/Event>
#include <KCalCore/Todo>
#include <KConfigSkeleton>
#include <KMessageBox>
#include <KStandardDirs>
#include <KSystemTimeZones>

#include <Akonadi/CollectionComboBox>
#include <Akonadi/Item>

#include "combinedincidenceeditor.h"
#include "editorconfig.h"
#include "incidencealarm.h"
#include "incidenceattachment.h"
#include "incidencecategories.h"
#include "incidencecompletionpriority.h"
#include "incidencedatetime.h"
#include "incidencedescription.h"
#include "incidencewhatwhere.h"
#include "incidencerecurrence.h"
#include "incidencesecrecy.h"
#include "incidenceattendee.h"
#include "invitationdispatcher.h"
#include "templatemanagementdialog.h"
#include "ui_eventortododesktop.h"

using namespace IncidenceEditorNG;

namespace IncidenceEditorNG {

enum Tabs {
  GeneralTab = 0,
  AttendeesTab,
  AlarmsTab,
  RecurrenceTab,
  AttachmentsTab,
};

class EventOrTodoDialogPrivate : public CalendarSupport::ItemEditorUi
{
  EventOrTodoDialog *q_ptr;
  Q_DECLARE_PUBLIC( EventOrTodoDialog )

  public:
    Ui::EventOrTodoDesktop *mUi;
    Akonadi::CollectionComboBox *mCalSelector;
    bool mCloseOnSave;

    CalendarSupport::EditorItemManager *mItemManager;
    CalendarSupport::InvitationDispatcher *mInvitationDispatcher;

    CombinedIncidenceEditor *mEditor;
    IncidenceDateTime *mIeDateTime;
    IncidenceAttendee *mIeAttendee;
    IncidenceRecurrence *mIeRecurrence;

  public:
    EventOrTodoDialogPrivate( EventOrTodoDialog *qq );
    ~EventOrTodoDialogPrivate();

    /// General methods
    void handleAlarmCountChange( int newCount );
    void handleRecurrenceChange( IncidenceEditorNG::RecurrenceType type );
    void loadTemplate( const QString &templateName );
    void manageTemplates();
    void saveTemplate( const QString &templateName );
    void storeTemplatesInConfig( const QStringList &newTemplates );
    void updateAttachmentCount( int newCount );
    void updateAttendeeCount( int newCount );
    void updateButtonStatus( bool isDirty );

    /// ItemEditorUi methods
    virtual bool containsPayloadIdentifiers( const QSet<QByteArray> &partIdentifiers ) const;
    void handleItemSaveFinish( CalendarSupport::EditorItemManager::SaveAction );
    void handleItemSaveFail( CalendarSupport::EditorItemManager::SaveAction,
                             const QString &errorMessage );
    virtual bool hasSupportedPayload( const Akonadi::Item &item ) const;
    virtual bool isDirty() const;
    virtual bool isValid() const;
    virtual void load( const Akonadi::Item &item );
    virtual Akonadi::Item save( const Akonadi::Item &item );
    virtual Akonadi::Collection selectedCollection() const;
    void slotButtonClicked( int button );

    virtual void reject( RejectReason reason, const QString &errorMessage = QString() );
};

}

EventOrTodoDialogPrivate::EventOrTodoDialogPrivate( EventOrTodoDialog *qq )
  : q_ptr( qq ),
    mUi( new Ui::EventOrTodoDesktop ),
    mCalSelector( new Akonadi::CollectionComboBox ),
    mCloseOnSave( false ),
    mItemManager( new CalendarSupport::EditorItemManager( this ) ),
    mInvitationDispatcher( 0 ),
    mEditor( new CombinedIncidenceEditor )
{
  Q_Q( EventOrTodoDialog );
  mUi->setupUi( q->mainWidget() );

  QGridLayout *layout = new QGridLayout( mUi->mCalSelectorPlaceHolder );
  layout->setSpacing( 0 );
  layout->addWidget( mCalSelector );

  mCalSelector->setAccessRightsFilter( Akonadi::Collection::CanCreateItem );

  if ( CalendarSupport::KCalPrefs::instance()->useGroupwareCommunication() ) {
    mInvitationDispatcher = new CalendarSupport::InvitationDispatcher( 0, q );
    mInvitationDispatcher->setItemManager( mItemManager );
  }

  // Now instantiate the logic of the dialog. These editors update the ui, validate
  // fields and load/store incidences in the ui.
  IncidenceWhatWhere *ieGeneral = new IncidenceWhatWhere( mUi );
  mEditor->combine( ieGeneral );

  IncidenceCategories *ieCategories = new IncidenceCategories( mUi );
  mEditor->combine( ieCategories );

  mIeDateTime = new IncidenceDateTime( mUi );
  mEditor->combine( mIeDateTime );

  IncidenceCompletionPriority *ieCompletionPriority = new IncidenceCompletionPriority( mUi );
  mEditor->combine( ieCompletionPriority );

  IncidenceDescription *ieDescription = new IncidenceDescription( mUi );
  mEditor->combine( ieDescription );

  IncidenceAlarm *ieAlarm = new IncidenceAlarm( mIeDateTime, mUi );
  mEditor->combine( ieAlarm );

  IncidenceAttachment *ieAttachments = new IncidenceAttachment( mUi );
  mEditor->combine( ieAttachments );

  mIeRecurrence = new IncidenceRecurrence( mIeDateTime, mUi );
  mEditor->combine( mIeRecurrence );

  IncidenceSecrecy *ieSecrecy = new IncidenceSecrecy( mUi );
  mEditor->combine( ieSecrecy );

  mIeAttendee = new IncidenceAttendee( qq, mIeDateTime, mUi );
  mEditor->combine( mIeAttendee );

  q->connect( mEditor, SIGNAL(dirtyStatusChanged(bool)),
              SLOT(updateButtonStatus(bool)) );
  q->connect( mItemManager,
              SIGNAL(itemSaveFinished(CalendarSupport::EditorItemManager::SaveAction)),
              SLOT(handleItemSaveFinish(CalendarSupport::EditorItemManager::SaveAction)));
  q->connect( mItemManager,
              SIGNAL(itemSaveFailed(CalendarSupport::EditorItemManager::SaveAction,QString)),
              SLOT(handleItemSaveFail(CalendarSupport::EditorItemManager::SaveAction, QString)));
  q->connect( ieAlarm, SIGNAL(alarmCountChanged(int)),
              SLOT(handleAlarmCountChange(int)) );
  q->connect( mIeRecurrence, SIGNAL(recurrenceChanged(IncidenceEditorNG::RecurrenceType)),
              SLOT(handleRecurrenceChange(IncidenceEditorNG::RecurrenceType)) );
  q->connect( ieAttachments, SIGNAL(attachmentCountChanged(int)),
              SLOT(updateAttachmentCount(int)) );
  q->connect( mIeAttendee, SIGNAL(attendeeCountChanged(int)),
              SLOT(updateAttendeeCount(int)) );
}

EventOrTodoDialogPrivate::~EventOrTodoDialogPrivate()
{
  delete mItemManager;
  delete mEditor;
  delete mUi;
}

void EventOrTodoDialogPrivate::handleAlarmCountChange( int newCount )
{
  QString tabText;
  if ( newCount > 0 ) {
    tabText = i18nc( "@title:tab Tab to configure the reminders of an event or todo",
                     "Reminder (%1)", newCount );
  } else {
    tabText = i18nc( "@title:tab Tab to configure the reminders of an event or todo", "Reminder" );
  }

  mUi->mTabWidget->setTabText( AlarmsTab, tabText );
}

void EventOrTodoDialogPrivate::handleRecurrenceChange( IncidenceEditorNG::RecurrenceType type )
{
  QString tabText = i18nc( "@title:tab Tab to configure the recurrence of an event or todo",
                           "Rec&urrence" );

  // Keep this numbers in sync with the items in mUi->mRecurrenceTypeCombo. I
  // tried adding an enum to IncidenceRecurrence but for whatever reason I could
  // Qt not play nice with namespaced enums in signal/slot connections.
  // Anyways, I don't expect these values to change.
  switch ( type ) {
  case RecurrenceTypeNone:
    break;
  case RecurrenceTypeDaily:
    tabText += i18nc( "@title:tab Daily recurring event, capital first letter only", " (D)" );
    break;
  case RecurrenceTypeWeekly:
    tabText += i18nc( "@title:tab Weekly recurring event, capital first letter only", " (W)" );
    break;
  case RecurrenceTypeMonthly:
    tabText += i18nc( "@title:tab Monthly recurring event, capital first letter only", " (M)" );
    break;
  case RecurrenceTypeYearly:
    tabText += i18nc( "@title:tab Yearly recurring event, capital first letter only", " (Y)" );
    break;
  default:
    Q_ASSERT_X( false, "handleRecurrenceChange", "Fix your program" );
  }

  mUi->mTabWidget->setTabText( RecurrenceTab, tabText );
}

void EventOrTodoDialogPrivate::loadTemplate( const QString &templateName )
{
  Q_Q( EventOrTodoDialog );

  KCalCore::MemoryCalendar::Ptr cal( new KCalCore::MemoryCalendar( KSystemTimeZones::local() ) );

  QStringList typeStrings;
  typeStrings << "Event" << "Todo" << "Journal";
  const QString fileName = KStandardDirs::locateLocal( "data",
                                                       "korganizer/templates/" +
                                                       typeStrings[mEditor->type()] + '/' +
                                                       templateName );

  if ( fileName.isEmpty() ) {
    KMessageBox::error( q, i18nc( "@info", "Unable to find template '%1'.", fileName ) );
    return;
  }

  KCalCore::ICalFormat format;
  if ( !format.load( cal, fileName ) ) {
    KMessageBox::error( q, i18nc( "@info", "Error loading template file '%1'.", fileName ) );
    return;
  }

  KCalCore::Incidence::List incidences = cal->incidences();
  if ( incidences.isEmpty() ) {
    KMessageBox::error( q, i18nc( "@info", "Template does not contain a valid incidence." ) );
    return;
  }

  mIeDateTime->setActiveDate( QDate() );
  mEditor->load( KCalCore::Incidence::Ptr( incidences.first()->clone() ) );
}

void EventOrTodoDialogPrivate::manageTemplates()
{
  Q_Q( EventOrTodoDialog );

  QStringList &templates =
    IncidenceEditorNG::EditorConfig::instance()->templates( mEditor->type() );
  QPointer<IncidenceEditorNG::TemplateManagementDialog> dialog(
      new IncidenceEditorNG::TemplateManagementDialog( q, templates, mEditor->type() ) );
  q->connect( dialog, SIGNAL( loadTemplate( const QString& ) ),
              SLOT( loadTemplate( const QString& ) ) );
  q->connect( dialog, SIGNAL( templatesChanged( const QStringList& ) ),
              SLOT( storeTemplatesInConfig( const QStringList& ) ) );
  q->connect( dialog, SIGNAL( saveTemplate( const QString& ) ),
              SLOT( saveTemplate( const QString& ) ) );
  dialog->exec();
  delete dialog;
}

void EventOrTodoDialogPrivate::saveTemplate( const QString &templateName )
{
  Q_ASSERT( ! templateName.isEmpty() );

  KCalCore::Incidence::Ptr incidence( new KCalCore::Event );
  mEditor->save( incidence );

  QString fileName = "templates/" + incidence->type();
  fileName.append( '/' + templateName );
  fileName = KStandardDirs::locateLocal( "data", "korganizer/" + fileName );

  KCalCore::MemoryCalendar::Ptr cal( new KCalCore::MemoryCalendar( KSystemTimeZones::local() ) );
  cal->addIncidence( KCalCore::Incidence::Ptr( incidence->clone() ) );
  KCalCore::ICalFormat format;
  format.save( cal, fileName );
}

void EventOrTodoDialogPrivate::storeTemplatesInConfig( const QStringList &templateNames )
{
  // I find this somewhat broken. templates() returns a reference, maybe it should
  // be changed by adding a setTemplates method.
  IncidenceEditorNG::EditorConfig::instance()->templates( mEditor->type() ) = templateNames;
  IncidenceEditorNG::EditorConfig::instance()->config()->writeConfig();
}

void EventOrTodoDialogPrivate::updateAttachmentCount( int newCount )
{
  if ( newCount > 0 ) {
    mUi->mTabWidget->setTabText( AttachmentsTab,
                                 i18nc( "@title:tab Tab to modify attachments of an event or todo",
                                        "Attac&hments (%1)", newCount ) );
  } else {
    mUi->mTabWidget->setTabText( AttachmentsTab,
                                 i18nc( "@title:tab Tab to modify attachments of an event or todo",
                                        "Attac&hments" ) );
  }
}

void EventOrTodoDialogPrivate::updateAttendeeCount( int newCount )
{
  if ( newCount > 0 ) {
    mUi->mTabWidget->setTabText( AttendeesTab,
                                 i18nc( "@title:tab Tab to modify attendees of an event or todo",
                                        "&Attendees (%1)", newCount ) );
  } else {
    mUi->mTabWidget->setTabText( AttendeesTab,
                                 i18nc( "@title:tab Tab to modify attendees of an event or todo",
                                        "&Attendees" ) );
  }
}

void EventOrTodoDialogPrivate::updateButtonStatus( bool isDirty )
{
  Q_Q( EventOrTodoDialog );
  q->enableButton( KDialog::Apply, isDirty );
}

bool EventOrTodoDialogPrivate::containsPayloadIdentifiers(
  const QSet<QByteArray> &partIdentifiers ) const
{
  return partIdentifiers.contains( QByteArray( "PLD:RFC822" ) );
}

void EventOrTodoDialogPrivate::handleItemSaveFail( CalendarSupport::EditorItemManager::SaveAction,
                                                   const QString &errorMessage )
{
  Q_Q( EventOrTodoDialog );

  bool retry = false;

  if ( !errorMessage.isEmpty() ) {
    const QString message = i18nc( "@info",
                                   "Unable to store the incidence in the calendar. Try again?\n\n "
                                   "Reason: %1", errorMessage );
    retry = ( KMessageBox::warningYesNo( q, message ) == KMessageBox::Yes );
  }

  if ( retry ) {
    mItemManager->save();
  } else {
    updateButtonStatus( mEditor->isDirty() );
    q->enableButtonOk( true );
    q->enableButtonCancel( true );
  }
}

void EventOrTodoDialogPrivate::handleItemSaveFinish(
  CalendarSupport::EditorItemManager::SaveAction )
{
  Q_Q( EventOrTodoDialog );

  if ( mCloseOnSave ) {
    q->accept();
  } else {
    const Akonadi::Item item = mItemManager->item();
    Q_ASSERT( item.isValid() );
    Q_ASSERT( item.hasPayload() );
    Q_ASSERT( item.hasPayload<KCalCore::Incidence::Ptr>() );
    // Now the item is succesfull saved, reload it in the editor in order to
    // reset the dirty status of the editor.
    mEditor->load( item.payload<KCalCore::Incidence::Ptr>() );

    // Set the buttons to a reasonable state as well (ok and apply should be
    // disabled at this point).
    q->enableButtonOk( true );
    q->enableButtonCancel( true );
    q->enableButtonApply( mEditor->isDirty() );
  }
}

bool EventOrTodoDialogPrivate::hasSupportedPayload( const Akonadi::Item &item ) const
{
  return CalendarSupport::incidence( item );
}

bool EventOrTodoDialogPrivate::isDirty() const
{
  return mEditor->isDirty();
}

bool EventOrTodoDialogPrivate::isValid() const
{
  return mEditor->isValid();
}

void EventOrTodoDialogPrivate::load( const Akonadi::Item &item )
{
  Q_Q( EventOrTodoDialog );

  Q_ASSERT( hasSupportedPayload( item ) );

  if ( CalendarSupport::hasJournal( item ) ) {
    //mUi->mTabWidget->removeTab( 5 );
    mUi->mTabWidget->removeTab( AttachmentsTab );
    mUi->mTabWidget->removeTab( RecurrenceTab );
    mUi->mTabWidget->removeTab( AlarmsTab );
  }

  mEditor->load( CalendarSupport::incidence( item ) );

  const KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( item );
  const QStringList allEmails = IncidenceEditorNG::EditorConfig::instance()->allEmails();
  KCalCore::Attendee::Ptr me = incidence->attendeeByMails( allEmails );

  if ( incidence->attendeeCount() > 1 &&
       me && ( me->status() == KCalCore::Attendee::NeedsAction ||
               me->status() == KCalCore::Attendee::Tentative ||
               me->status() == KCalCore::Attendee::InProcess ) ) {
    mUi->mInvitationBar->show();
  } else {
    mUi->mInvitationBar->hide();
  }

  mCalSelector->setMimeTypeFilter( QStringList() << incidence->mimeType() );
  if ( item.parentCollection().isValid() ) {
    mCalSelector->setDefaultCollection( item.parentCollection() );
  }

  if ( mEditor->type() == KCalCore::Incidence::TypeTodo ) {
    q->setWindowIcon( SmallIcon( "view-calendar-tasks" ) );
  } else if ( mEditor->type() == KCalCore::Incidence::TypeEvent ) {
    q->setWindowIcon( SmallIcon( "view-calendar-day" ) );
  } else if  ( mEditor->type() == KCalCore::Incidence::TypeJournal ) {
    q->setWindowIcon( SmallIcon( "view-pim-journal" ) );
  }

  // Initialize tab's titles
  updateAttachmentCount( incidence->attachments().size() );
  handleRecurrenceChange( mIeRecurrence->currentRecurrenceType() );
  handleAlarmCountChange( incidence->alarms().count() );

  q->show();
}

Akonadi::Item EventOrTodoDialogPrivate::save( const Akonadi::Item &item )
{
  Q_ASSERT( mEditor->incidence<KCalCore::Incidence>() );

  KCalCore::Incidence::Ptr incidenceInEditor = mEditor->incidence<KCalCore::Incidence>();
  KCalCore::Incidence::Ptr newIncidence;

  Akonadi::Item result = item;
  if ( incidenceInEditor->type() == KCalCore::Incidence::TypeEvent ) {
    newIncidence = KCalCore::Event::Ptr( new KCalCore::Event );
  } else if ( incidenceInEditor->type() == KCalCore::Incidence::TypeTodo ) {
    newIncidence = KCalCore::Todo::Ptr( new KCalCore::Todo );
  } else if ( incidenceInEditor->type() == KCalCore::Incidence::TypeJournal ) {
    newIncidence = KCalCore::Journal::Ptr( new KCalCore::Journal );
  } else {
    Q_ASSERT_X( false, "save", "Invalid Incidence type" );
  }

  result.setMimeType( newIncidence->mimeType() );

  mEditor->save( newIncidence );

  // TODO: Remove this once we support moving of events/todo's
  mCalSelector->setEnabled( false );

  // Make sure that we don't loose uid for existing incidence
  newIncidence->setUid( mEditor->incidence<KCalCore::Incidence>()->uid() );
  result.setPayload<KCalCore::Incidence::Ptr>( newIncidence );
  return result;
}

Akonadi::Collection EventOrTodoDialogPrivate::selectedCollection() const
{
  return mCalSelector->currentCollection();
}

void EventOrTodoDialogPrivate::reject( RejectReason reason, const QString &errorMessage )
{
  Q_UNUSED( reason );

  Q_Q( EventOrTodoDialog );
  kDebug() << "Rejecting:" << errorMessage;
  q->deleteLater();
}

/// EventOrTodoDialog

EventOrTodoDialog::EventOrTodoDialog( QWidget *parent, Qt::WFlags flags )
  : IncidenceDialog( parent, flags ), d_ptr( new EventOrTodoDialogPrivate( this ) )
{
  Q_D( EventOrTodoDialog );

  resize( QSize( 600, 500 ).expandedTo( minimumSizeHint() ) );
  d->mUi->mTabWidget->setCurrentIndex( 0 );
  d->mUi->mSummaryEdit->setFocus();

  setButtons( KDialog::Ok | KDialog::Apply | KDialog::Cancel | KDialog::Default );
  setButtonToolTip( KDialog::Apply,
                    i18nc( "@info:tooltip", "Save current changes" ) );
  setButtonToolTip( KDialog::Ok,
                    i18nc( "@action:button", "Save changes and close dialog" ) );
  setButtonToolTip( KDialog::Cancel,
                    i18nc( "@action:button", "Discard changes and close dialog" ) );
  setDefaultButton( Ok );
  enableButton( Apply, false );

  setButtonText( Default, i18nc( "@action:button", "Manage &Templates..." ) );
  setButtonToolTip( Default,
                    i18nc( "@info:tooltip",
                           "Apply or create templates for this item" ) );
  setButtonWhatsThis( Default,
                      i18nc( "@info:whatsthis",
                             "Push this button to run a tool that helps "
                             "you manage a set of templates. Templates "
                             "can make creating new items easier and faster "
                             "by putting your favorite default values into "
                             "the editor automatically." ) );

  setModal( false );
  showButtonSeparator( false );

  // TODO: Implement these.
  connect( d->mUi->mAcceptInvitationButton, SIGNAL(clicked()),
           d->mIeAttendee, SLOT(acceptForMe()) );
  connect( d->mUi->mAcceptInvitationButton, SIGNAL(clicked()),
           d->mUi->mInvitationBar, SLOT(hide()) );
  connect( d->mUi->mDeclineInvitationButton, SIGNAL(clicked()),
           d->mIeAttendee, SLOT(declineForMe()) );
  connect( d->mUi->mDeclineInvitationButton, SIGNAL(clicked()),
           d->mUi->mInvitationBar, SLOT(hide()) );
}

EventOrTodoDialog::~EventOrTodoDialog()
{
  delete d_ptr;
}

void EventOrTodoDialog::load( const Akonadi::Item &item, const QDate &activeDate )
{
  Q_D( EventOrTodoDialog );
  d->mIeDateTime->setActiveDate( activeDate );

  if ( item.isValid() ) {
    d->mItemManager->load( item );
    // TODO: Remove this once we support moving of events/todo's
    d->mCalSelector->setEnabled( false );
  } else {
    Q_ASSERT( d->hasSupportedPayload( item ) );
    d->load( item );
    show();
  }
}

void EventOrTodoDialog::selectCollection( const Akonadi::Collection &collection )
{
  Q_D( EventOrTodoDialog );
  if ( collection.isValid() ) {
    d->mCalSelector->setDefaultCollection( collection );
  } else {
    d->mCalSelector->setCurrentIndex( 0 );
  }
}

void EventOrTodoDialog::setIsCounterProposal( bool isCounterProposal )
{
  Q_D( EventOrTodoDialog );
  d->mInvitationDispatcher->setIsCounterProposal( isCounterProposal );
}

QObject *EventOrTodoDialog::typeAheadReceiver() const
{
  Q_D( const EventOrTodoDialog );
  return d->mUi->mSummaryEdit;
}

void EventOrTodoDialog::slotButtonClicked( int button )
{
  Q_D( EventOrTodoDialog );

  switch( button ) {
  case KDialog::Ok:
  {
    if ( d->mEditor->isDirty() ) {
      enableButtonOk( false );
      enableButtonCancel( false );
      enableButtonApply( false );
      d->mCloseOnSave = true;
      d->mItemManager->save();
    } else {
      close();
    }
    break;
  }
  case KDialog::Apply:
  {
    enableButtonOk( false );
    enableButtonCancel( false );
    enableButtonApply( false );

    d->mCloseOnSave = false;
    d->mItemManager->save();
    break;
  }
  case KDialog::Cancel:
    if ( d->mEditor->isDirty() &&
         KMessageBox::questionYesNo( this, i18nc( "@info", "Do you really want to cancel?" ),
                 i18nc( "@title:window", "KOrganizer Confirmation" ) ) == KMessageBox::Yes ) {
      KDialog::reject(); // Discard current changes
    } else if ( !d->mEditor->isDirty() ) {
      KDialog::reject(); // No pending changes, just close the dialog.
    } // else { // the user wasn't finished editting after all }
    break;
  case KDialog::Default:
    d->manageTemplates();
    break;
  default:
    Q_ASSERT( false ); // Shouldn't happen
    break;
  }
}

#include "eventortododialog.moc"
