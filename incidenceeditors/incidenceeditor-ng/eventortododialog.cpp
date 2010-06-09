/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include "eventortododialog.h"

#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QMessageBox>
#include <QtGui/QTabWidget>

#include <KLocale>
#include <KPushButton>

#include <KCal/Event>
#include <KCal/Todo>

#include <Akonadi/CollectionComboBox>
#include <Akonadi/Item>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/Monitor>
#include <Akonadi/Session>
#include <Akonadi/KCal/IncidenceMimeTypeVisitor>

#include "incidenceeditorgeneralpage.h"
#include "incidenceattachmenteditor.h"

using namespace Akonadi;
using namespace IncidenceEditorsNG;

/// Class EventOrTodoDialogPrivate

namespace IncidenceEditorsNG {

class EventOrTodoDialogPrivate
{
  EventOrTodoDialog *q;
public:

  Akonadi::Item mItem;
  Akonadi::Monitor *mMonitor;

  bool mAcceptOnSuccessFullSave;

  Akonadi::CollectionComboBox *mCalSelector;
  IncidenceAttachmentEditor *mAttachtmentPage;
  IncidenceEditorGeneralPage *mGeneralPage;
  QTabWidget *mTabWidget;

public:
  EventOrTodoDialogPrivate( EventOrTodoDialog *qq );

  void itemFetchResult( KJob *job );
  void itemChanged( const Akonadi::Item&, const QSet<QByteArray>& );
  void modifyFinished( KJob *job );
  void save();
  void setupMonitor();
  void slotButtonClicked( KDialog::ButtonCode button );
  void updateAttachmentCount( int newCount );
  void updateButtonStatus( bool isDirty );
};

}

EventOrTodoDialogPrivate::EventOrTodoDialogPrivate( EventOrTodoDialog *qq )
  : q( qq )
  , mMonitor( 0 )
  , mAcceptOnSuccessFullSave( false )
  , mCalSelector( new Akonadi::CollectionComboBox( q->mainWidget() ) )
  , mAttachtmentPage( new IncidenceAttachmentEditor( q->mainWidget() ) )
  , mGeneralPage( new IncidenceEditorGeneralPage( q->mainWidget() ) )
  , mTabWidget( new QTabWidget( q->mainWidget() ) )
{
  mGeneralPage->combine( mAttachtmentPage );
}

void EventOrTodoDialogPrivate::setupMonitor()
{
  delete mMonitor;
  mMonitor = new Akonadi::Monitor;
  mMonitor->ignoreSession( Akonadi::Session::defaultSession() );
  mMonitor->itemFetchScope().fetchFullPayload();
  if ( mItem.isValid() )
    mMonitor->setItemMonitored( mItem );

  q->connect( mMonitor, SIGNAL( itemChanged( const Akonadi::Item&, const QSet<QByteArray>& ) ),
              SLOT( itemChanged( const Akonadi::Item&, const QSet<QByteArray>& ) ) );
}


void EventOrTodoDialogPrivate::itemChanged( const Akonadi::Item &item,
                                            const QSet<QByteArray> &partIdentifiers )
{
  if ( partIdentifiers.contains( QByteArray( "PLD:RFC822" ) ) ) {
    QPointer<QMessageBox> dlg = new QMessageBox( q ); //krazy:exclude=qclasses
    dlg->setIcon( QMessageBox::Question );
    dlg->setInformativeText( i18n( "The incidence has been changed by someone else.\nWhat should be done?" ) );
    dlg->addButton( i18n( "Take over changes" ), QMessageBox::AcceptRole );
    dlg->addButton( i18n( "Ignore and Overwrite changes" ), QMessageBox::RejectRole );

    if ( dlg->exec() == QMessageBox::AcceptRole ) {
      Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( mItem );
      job->fetchScope().fetchFullPayload();
      job->fetchScope().setAncestorRetrieval( Akonadi::ItemFetchScope::Parent );

      Q_ASSERT( item.hasPayload<KCal::Incidence::Ptr>() );
      mItem = item;

      q->enableButton( KDialog::Ok, false );
      q->enableButton( KDialog::Apply, false );
      q->load( mItem );
    } else {
      mItem.setRevision( item.revision() );
      save();
    }

    delete dlg;
  }

  // Overwrite or not we need to update the revision and the remote id to be able
  // to store item later on.
  mItem.setRevision( item.revision() );
}

void EventOrTodoDialogPrivate::itemFetchResult( KJob *job )
{
  Q_ASSERT( job );

  if ( job->error() ) {
    kDebug() << "ItemFetch failed" << job->errorString();
    q->reject();
    return;
  }

  ItemFetchJob *fetchJob = qobject_cast<ItemFetchJob*>( job );
  if ( fetchJob->items().isEmpty() ) {
    kDebug() << "No items returned by fetch job";
    q->reject();
    return;
  }

  Item item = fetchJob->items().first();
  if ( item.hasPayload<KCal::Event::Ptr>() || item.payload<KCal::Todo::Ptr>() ) {
    mItem = item;
    q->load( item );

    //TODO read-only ATM till we support moving of existing incidences from
    // one collection to another.
    mCalSelector->setEnabled( false );
    mCalSelector->setDefaultCollection( item.parentCollection() );
    q->show();
  } else {
    kDebug() << "Item as invalid payload type";
    q->reject();
  }
}

void EventOrTodoDialogPrivate::modifyFinished( KJob *job )
{
  Q_ASSERT( job );

  if ( job->error() ) {
    kDebug() << "Item modify or create failed:" << job->errorString();
    return;
  }

  if ( mAcceptOnSuccessFullSave )
    q->accept();
  else if ( ItemModifyJob *modifyJob = qobject_cast<ItemModifyJob*>( job ) ) {
    mItem = modifyJob->item();
    mGeneralPage->load( mItem.payload<KCal::Incidence::Ptr>() );
  } else {
    ItemCreateJob *createJob = qobject_cast<ItemCreateJob*>( job );
    Q_ASSERT(createJob);
    mItem = createJob->item();
    mGeneralPage->load( mItem.payload<KCal::Incidence::Ptr>() );
  }

  setupMonitor();
}


void EventOrTodoDialogPrivate::save()
{
  mGeneralPage->save( mItem.payload<KCal::Incidence::Ptr>() );

  if ( mItem.isValid() ) { // A valid item needs to be modified.
    ItemModifyJob *modifyJob = new ItemModifyJob( mItem );
    q->connect( modifyJob, SIGNAL(result(KJob*)), SLOT(modifyFinished(KJob*)) );
  } else { // An invalid item needs to be created.
    if ( mItem.hasPayload<KCal::Event::Ptr>() )
      mItem.setMimeType( IncidenceMimeTypeVisitor::eventMimeType() );
    else
      mItem.setMimeType( IncidenceMimeTypeVisitor::todoMimeType() );

    ItemCreateJob *createJob =
      new ItemCreateJob( mItem, mCalSelector->currentCollection() );
    q->connect( createJob, SIGNAL(result(KJob*)), SLOT(modifyFinished(KJob*)) );

    //TODO read-only ATM till we support moving of existing incidences from
    // one collection to another.
    mCalSelector->setEnabled( false );
  }
}

void EventOrTodoDialogPrivate::updateButtonStatus( bool isDirty )
{
  q->enableButton( KDialog::Apply, isDirty );
  q->enableButton( KDialog::Ok, isDirty );
}

void EventOrTodoDialogPrivate::updateAttachmentCount( int newCount )
{
  mTabWidget->setTabText( 2,
    i18nc( "@title:tab event or todo attachments", "A&ttachments (%1)",
           newCount ) );
}

/// Class EventOrTodoDialog

EventOrTodoDialog::EventOrTodoDialog( QWidget *parent )
  : KDialog( parent )
  , d_ptr( new EventOrTodoDialogPrivate( this ) )
{
  Q_D( EventOrTodoDialog );
  
  // Calendar selector
  d->mCalSelector->setAccessRightsFilter( Akonadi::Collection::CanCreateItem );
  //mCalSelector->setDefaultCollection( KCalPrefs::instance()->defaultCollection() );

  QLabel *callabel = new QLabel( i18n( "Calendar:" ), mainWidget() );
  callabel->setBuddy( d->mCalSelector );

  QHBoxLayout *callayout = new QHBoxLayout;
  callayout->setSpacing( KDialog::spacingHint() );
  callayout->addWidget( callabel );
  callayout->addWidget( d->mCalSelector, 1 );

  // Tab widget and pages
  d->mTabWidget->addTab( d->mGeneralPage,
                      i18nc( "@title:tab general event or todo settings", "&General" ) );
  d->mTabWidget->addTab( new QWidget(),
                      i18nc( "@title:tab event or todo attendees", "&Attendees" ) );
  d->mTabWidget->addTab( d->mAttachtmentPage,
                      i18nc( "@title:tab event or todo attachments", "A&ttachments (%1)",
                             d->mAttachtmentPage->attachmentCount() ) );

  // Overall layout of the complete dialog
  QVBoxLayout *layout = new QVBoxLayout( mainWidget() );
  layout->setMargin( 0 );
  layout->setSpacing( 0 );
  layout->addLayout( callayout );
  layout->addWidget( d->mTabWidget );

  mainWidget()->setLayout( layout );
  setButtons( KDialog::Ok | KDialog::Apply | KDialog::Cancel );
  setButtonText( KDialog::Apply, i18nc( "@action:button", "&Save" ) );
  setButtonToolTip( KDialog::Apply,
                    i18nc( "@info:tooltip", "Save current changes" ) );
  setButtonToolTip( KDialog::Ok,
                    i18nc( "@action:button", "Save changes and close dialog" ) );
  setButtonToolTip( KDialog::Cancel,
                    i18nc( "@action:button", "Discard changes and close dialog" ) );
  setDefaultButton( Ok );
  enableButton( Ok, false );
  enableButton( Apply, false );
  setModal( false );
  showButtonSeparator( false );

  connect( d->mGeneralPage, SIGNAL(dirtyStatusChanged(bool)),
           SLOT(updateButtonStatus(bool)) );
  connect( d->mAttachtmentPage, SIGNAL(attachmentCountChanged(int)),
           SLOT(updateAttachmentCount(int)) );
}

EventOrTodoDialog::~EventOrTodoDialog()
{
  delete d_ptr;
}

void EventOrTodoDialog::load( const Akonadi::Item &item )
{
  Q_D( EventOrTodoDialog );

  d->mItem = item;
  d->setupMonitor();

  if ( !item.isValid() ) {
    // We're creating a new item
    Q_ASSERT( item.hasPayload() );
    Q_ASSERT( item.hasPayload<KCal::Incidence::Ptr>() );
    Q_ASSERT( item.hasPayload<KCal::Event::Ptr>() || item.payload<KCal::Todo::Ptr>() );

    KCal::Incidence::Ptr incidence = item.payload<KCal::Incidence::Ptr>();
    setCaption( i18nc( "@title:window",
                     "New %1", QString( incidence->type() ) ) );
    d->mGeneralPage->load( incidence );
    show();
  } else if ( item.hasPayload() ) {

    if ( item.hasPayload<KCal::Event::Ptr>() ) {
      d->mCalSelector->setMimeTypeFilter(
        QStringList() << IncidenceMimeTypeVisitor::eventMimeType() );
    } else {
      d->mCalSelector->setMimeTypeFilter(
        QStringList() << IncidenceMimeTypeVisitor::todoMimeType() );
    }

    KCal::Incidence::Ptr incidence = item.payload<KCal::Incidence::Ptr>();
    setCaption( i18nc( "@title:window",
                     "Edit %1: %2", QString( incidence->type() ), incidence->summary() ) );
    d->mGeneralPage->load( incidence );
  } else {
    ItemFetchJob *job = new ItemFetchJob( item, this );
    job->fetchScope().fetchFullPayload();
    job->fetchScope().setAncestorRetrieval( Akonadi::ItemFetchScope::Parent );

    connect( job, SIGNAL(result(KJob*)), SLOT(itemFetchResult(KJob*)) );
    return;
  }

  if ( item.hasPayload<KCal::Event::Ptr>() )
    d->mCalSelector->setMimeTypeFilter(
      QStringList() << IncidenceMimeTypeVisitor::eventMimeType() );
  else
    d->mCalSelector->setMimeTypeFilter(
      QStringList() << IncidenceMimeTypeVisitor::todoMimeType() );
}

void EventOrTodoDialog::slotButtonClicked( int button )
{
  Q_D( EventOrTodoDialog );

  switch ( button ) {
  case KDialog::Apply:
    d->save();
    break;
  case KDialog::Ok:
    d->mAcceptOnSuccessFullSave = true;
    d->save();
    break;
  default:
    KDialog::slotButtonClicked( button );
  }
}


#include "moc_eventortododialog.cpp"
