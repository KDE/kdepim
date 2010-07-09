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

#include <KCal/CalendarLocal>
#include <KCal/ICalFormat>
#include <KCal/Incidence>
#include <KCal/Event>
#include <KCal/Todo>
#include <KConfigSkeleton>
#include <KMessageBox>
#include <KStandardDirs>
#include <KSystemTimeZones>

#include <Akonadi/CollectionComboBox>
#include <Akonadi/Item>
#include <Akonadi/KCal/IncidenceMimeTypeVisitor>

#include "combinedincidenceeditor.h"
#include "editorconfig.h"
#include "editoritemmanager.h"
#include "incidencealarm.h"
#include "incidenceattachment.h"
#include "incidencecategories.h"
#include "incidencecompletionpriority.h"
#include "incidencedatetime.h"
#include "incidencedescription.h"
#include "incidencegeneral.h"
#include "incidencerecurrence.h"
#include "incidencesecrecy.h"
#include "incidenceattendee.h"
#include "templatemanagementdialog.h"
#include "ui_eventortododesktop.h"

using namespace IncidenceEditorsNG;

namespace IncidenceEditorsNG {

class EventOrTodoDialogPrivate : public Akonadi::ItemEditorUi
{
  EventOrTodoDialog *q_ptr;
  Q_DECLARE_PUBLIC( EventOrTodoDialog )

public:
  Ui::EventOrTodoDesktop *mUi;
  Akonadi::CollectionComboBox *mCalSelector;
  bool mCloseOnSave;

  Akonadi::EditorItemManager *mItemManager;
  CombinedIncidenceEditor *mEditor;
  IncidenceDateTime *mIeDateTime;

public:
  EventOrTodoDialogPrivate( EventOrTodoDialog *qq );
  ~EventOrTodoDialogPrivate();

  /// General methods
  void handleAlarmCountChange( int newCount );
  void handleRecurrenceChange( int type );
  void loadTemplate( const QString &templateName );
  void manageTemplates();
  void saveTemplate( const QString &templateName );
  void storeTemplatesInConfig( const QStringList &newTemplates );
  void updateAttachmentCount( int newCount );
  void updateAttendeeCount( int newCount );
  void updateButtonStatus( bool isDirty );

  /// ItemEditorUi methods
  virtual bool containsPayloadIdentifiers( const QSet<QByteArray> &partIdentifiers ) const;
  void handleItemSaveFinish();
  virtual bool hasSupportedPayload( const Akonadi::Item &item ) const;
  virtual bool isDirty() const;
  virtual bool isValid();
  virtual void load( const Akonadi::Item &item );
  virtual Akonadi::Item save( const Akonadi::Item &item );
  virtual Akonadi::Collection selectedCollection() const;
  void slotButtonClicked( int button );    enum RecurrenceType {
    None = 0,
    Daily,
    Weekly,
    Monthly,
    Yearly
  };
  virtual void reject( RejectReason reason, const QString &errorMessage = QString() );
};

}

EventOrTodoDialogPrivate::EventOrTodoDialogPrivate( EventOrTodoDialog *qq )
  : q_ptr( qq )
  , mUi( new Ui::EventOrTodoDesktop )
  , mCalSelector( new Akonadi::CollectionComboBox )
  , mCloseOnSave( false )
  , mItemManager( new Akonadi::EditorItemManager( this ) )
  , mEditor( new CombinedIncidenceEditor )
{
  Q_Q( EventOrTodoDialog );
  mUi->setupUi( q->mainWidget() );

  QGridLayout *layout = new QGridLayout( mUi->mCalSelectorPlaceHolder );
  layout->setSpacing( 0 );
  layout->addWidget( mCalSelector );

  mCalSelector->setAccessRightsFilter( Akonadi::Collection::CanCreateItem );

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

  IncidenceAlarm *ieAlarm = new IncidenceAlarm( mUi );
  mEditor->combine( ieAlarm );

  IncidenceAttachment *ieAttachments = new IncidenceAttachment( mUi );
  mEditor->combine( ieAttachments );

  IncidenceRecurrence *ieRecurrence = new IncidenceRecurrence( mIeDateTime, mUi );
  mEditor->combine( ieRecurrence );

  IncidenceSecrecy *ieSecrecy = new IncidenceSecrecy( mUi );
  mEditor->combine( ieSecrecy );

  IncidenceAttendee *ieAttendee= new IncidenceAttendee( mUi );
  mEditor->combine( ieAttendee );

  q->connect( mEditor, SIGNAL(dirtyStatusChanged(bool)),
              SLOT(updateButtonStatus(bool)) );
  q->connect( mItemManager, SIGNAL(itemSaveFinished()),
              SLOT(handleItemSaveFinish()));
  q->connect( ieAlarm, SIGNAL(alarmCountChanged(int)),
              SLOT(handleAlarmCountChange(int)) );
  q->connect( ieRecurrence, SIGNAL(recurrenceChanged(int)),
              SLOT(handleRecurrenceChange(int)) );
  q->connect( ieAttachments, SIGNAL(attachmentCountChanged(int)),
              SLOT(updateAttachmentCount(int)) );
  q->connect( ieAttendee, SIGNAL(attendeeCountChanged(int)),
              SLOT(updateAttendeeCount(int)) );
}

EventOrTodoDialogPrivate::~EventOrTodoDialogPrivate()
{
  delete mItemManager;
  delete mEditor;
}

void EventOrTodoDialogPrivate::handleAlarmCountChange( int newCount )
{
  QString tabText;
  if ( newCount > 0 ) {
    tabText = i18n( "Reminder (%1)", newCount );
  } else {
    tabText = i18n( "Reminder" );
  }

  mUi->mTabWidget->setTabText( 2, tabText );
}

void EventOrTodoDialogPrivate::handleRecurrenceChange( int type )
{
  QString tabText = i18n( "Rec&urrence" );

  // Keep this numbers in sync with the items in mUi->mRecurrenceTypeCombo. I
  // tried adding an enum to IncidenceRecurrence but for whatever reason I could
  // Qt not play nice with namespaced enums in signal/slot connections.
  // Anyways, I don't expect these values to change.
  switch ( type ) {
  case 0: // None
    break;
  case 1: // Daily
    tabText += i18nc( "Daily recurring event, capital first letter only", " (D)" );
    break;
  case 2: // Weekly
    tabText += i18nc( "Weekly recurring event, capital first letter only", " (W)" );
    break;
  case 3: // Monthly
    tabText += i18nc( "Monthly recurring event, capital first letter only", " (M)" );
    break;
  case 4: // Yearly
    tabText += i18nc( "Yearly recurring event, capital first letter only", " (Y)" );
    break;
  }

  mUi->mTabWidget->setTabText( 3, tabText );
}

void EventOrTodoDialogPrivate::loadTemplate( const QString &templateName )
{
  Q_Q( EventOrTodoDialog );

  KCal::CalendarLocal cal( KSystemTimeZones::local() );
  QString fileName = KStandardDirs::locateLocal( "data",
                       "korganizer/templates/" + mEditor->type() + '/' + templateName );

  if ( fileName.isEmpty() ) {
    KMessageBox::error( q, i18nc( "@info", "Unable to find template '%1'.", fileName ) );
    return;
  }

  KCal::ICalFormat format;
  if ( !format.load( &cal, fileName ) ) {
    KMessageBox::error( q, i18nc( "@info", "Error loading template file '%1'.", fileName ) );
    return;
  }

  KCal::Incidence::List incidences = cal.incidences();
  if ( incidences.isEmpty() ) {
    KMessageBox::error( q, i18nc( "@info", "Template does not contain a valid incidence." ) );
    return;
  }

  mIeDateTime->setActiveDate( QDate() );
  mEditor->load( KCal::Incidence::Ptr( incidences.first()->clone() ) );
}


void EventOrTodoDialogPrivate::manageTemplates()
{
  Q_Q( EventOrTodoDialog );

  QStringList &templates = IncidenceEditors::EditorConfig::instance()->templates( mEditor->type() );
  QPointer<IncidenceEditors::TemplateManagementDialog> dialog(
      new IncidenceEditors::TemplateManagementDialog( q, templates, mEditor->type() ) );
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

  KCal::Incidence::Ptr incidence( new KCal::Event );
  mEditor->save( incidence );

  QString fileName = "templates/" + incidence->type();
  fileName.append( '/' + templateName );
  fileName = KStandardDirs::locateLocal( "data", "korganizer/" + fileName );

  KCal::CalendarLocal cal( KSystemTimeZones::local() );
  cal.addIncidence( incidence->clone() );
  KCal::ICalFormat format;
  format.save( &cal, fileName );
}

void EventOrTodoDialogPrivate::storeTemplatesInConfig( const QStringList &templateNames )
{
  // I find this somewhat broken. templates() returns a reference, maybe it should
  // be changed by adding a setTemplates method.
  IncidenceEditors::EditorConfig::instance()->templates( mEditor->type() ) = templateNames;
  IncidenceEditors::EditorConfig::instance()->config()->writeConfig();
}


void EventOrTodoDialogPrivate::updateAttachmentCount( int newCount )
{
  if ( newCount > 0 ) {
    mUi->mTabWidget->setTabText( 4, i18n( "Attac&htments (%1)", newCount ) );
  } else {
    mUi->mTabWidget->setTabText( 4, i18n( "Attac&htments" ) );
  }
}

void EventOrTodoDialogPrivate::updateAttendeeCount( int newCount )
{
  if ( newCount > 0 ) {
    mUi->mTabWidget->setTabText( 1, i18n( "&Attendees (%1)", newCount ) );
  } else {
    mUi->mTabWidget->setTabText( 1, i18n( "&Attendees" ) );
  }
}


void EventOrTodoDialogPrivate::updateButtonStatus( bool isDirty )
{
  Q_Q( EventOrTodoDialog );
  q->enableButton( KDialog::Apply, isDirty );
  q->enableButton( KDialog::Ok, isDirty );
}


bool EventOrTodoDialogPrivate::containsPayloadIdentifiers( const QSet<QByteArray> &partIdentifiers ) const
{
  return partIdentifiers.contains( QByteArray( "PLD:RFC822" ) );
}

void EventOrTodoDialogPrivate::handleItemSaveFinish()
{
  Q_Q( EventOrTodoDialog );

  if ( mCloseOnSave )
    q->accept();
  else {
    const Akonadi::Item item = mItemManager->item();
    Q_ASSERT( item.isValid() );
    Q_ASSERT( item.hasPayload() );
    Q_ASSERT( item.hasPayload<KCal::Incidence::Ptr>() );
    // Now the item is succesfull saved, reload it in the editor in order to
    // reset the dirty status of the editor.
    mEditor->load( item.payload<KCal::Incidence::Ptr>() );

    // Set the buttons to a reasonable state as well (ok and apply should be
    // disabled at this point).
    q->enableButtonOk( mEditor->isDirty() );
    q->enableButtonCancel( true );
    q->enableButtonApply( mEditor->isDirty() );
  }
}

bool EventOrTodoDialogPrivate::hasSupportedPayload( const Akonadi::Item &item ) const
{
  return item.hasPayload() && item.hasPayload<KCal::Incidence::Ptr>()
    && ( item.hasPayload<KCal::Event::Ptr>() || item.hasPayload<KCal::Todo::Ptr>() );
}

bool EventOrTodoDialogPrivate::isDirty() const
{
  return mEditor->isDirty();
}

bool EventOrTodoDialogPrivate::isValid()
{
  return mEditor->isValid();
}

void EventOrTodoDialogPrivate::load( const Akonadi::Item &item )
{
  Q_ASSERT( hasSupportedPayload( item ) );
  mEditor->load( item.payload<KCal::Incidence::Ptr>() );

  if ( item.hasPayload<KCal::Event::Ptr>() ) {
    mCalSelector->setMimeTypeFilter(
      QStringList() << Akonadi::IncidenceMimeTypeVisitor::eventMimeType() );
  } else {
    mCalSelector->setMimeTypeFilter(
      QStringList() << Akonadi::IncidenceMimeTypeVisitor::todoMimeType() );
  }
}

Akonadi::Item EventOrTodoDialogPrivate::save( const Akonadi::Item &item )
{
  KCal::Event::Ptr event( new KCal::Event );
  // Make sure that we don't loose uid for existing incidence
  if ( mEditor->incidence<KCal::Incidence>() )
    event->setUid( mEditor->incidence<KCal::Incidence>()->uid() );
  mEditor->save( event );

  Akonadi::Item result = item;
  result.setMimeType( Akonadi::IncidenceMimeTypeVisitor::eventMimeType() );
  result.setPayload<KCal::Event::Ptr>( event );
  return result;
}

Akonadi::Collection EventOrTodoDialogPrivate::selectedCollection() const
{
  return mCalSelector->currentCollection();
}

void EventOrTodoDialogPrivate::reject( RejectReason /*reason*/, const QString &errorMessage )
{
  Q_Q( EventOrTodoDialog );
  kDebug() << "Rejecting:" << errorMessage;
  q->deleteLater();
}

/// EventOrTodoDialog

EventOrTodoDialog::EventOrTodoDialog()
  : d_ptr( new EventOrTodoDialogPrivate( this ) )
{
  setButtons( KDialog::Ok | KDialog::Apply | KDialog::Cancel | KDialog::Default );
  setButtonText( KDialog::Apply, i18nc( "@action:button", "&Save" ) );
  setButtonToolTip( KDialog::Apply, i18nc( "@info:tooltip", "Save current changes" ) );
  setButtonToolTip( KDialog::Ok, i18nc( "@action:button", "Save changes and close dialog" ) );
  setButtonToolTip( KDialog::Cancel, i18nc( "@action:button", "Discard changes and close dialog" ) );
  setDefaultButton( Ok );
  enableButton( Ok, false );
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
}

EventOrTodoDialog::~EventOrTodoDialog()
{
  delete d_ptr;
}


void EventOrTodoDialog::load( const Akonadi::Item &item )
{
  Q_D( EventOrTodoDialog );
  Q_ASSERT( d->hasSupportedPayload( item ) );

  if ( item.isValid() ) {
    d->mItemManager->load( item );
  } else {
    d->mEditor->load( item.payload<KCal::Incidence::Ptr>() );
  }

  if ( item.hasPayload<KCal::Event::Ptr>() ) {
    d->mCalSelector->setMimeTypeFilter(
      QStringList() << Akonadi::IncidenceMimeTypeVisitor::eventMimeType() );
  } else {
    d->mCalSelector->setMimeTypeFilter(
      QStringList() << Akonadi::IncidenceMimeTypeVisitor::todoMimeType() );
  }
}

void EventOrTodoDialog::slotButtonClicked( int button )
{
  Q_D( EventOrTodoDialog );

  switch( button ) {
  case KDialog::Ok:
  {
    enableButtonOk( false );
    enableButtonCancel( false );
    enableButtonApply( false );
    d->mCloseOnSave = true;
    d->mItemManager->save();
    KDialog::accept();
    break;
  }
  case KDialog::Apply:
  {
    d->mCloseOnSave = false;
    d->mItemManager->save();
    break;
  }
  case KDialog::Cancel:
    if ( d->mEditor->isDirty() &&
         KMessageBox::questionYesNo( this, i18nc( "@info", "Do you really want to cancel?" ),
                 i18nc( "@title:window", "KOrganizer Confirmation" ) ) == KMessageBox::Yes ) {
      KDialog::reject(); // Discard current changes
    } else if ( !d->mEditor->isDirty() )
      KDialog::reject(); // No pending changes, just close the dialog.
    // else { // the user wasn't finished editting after all }
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
