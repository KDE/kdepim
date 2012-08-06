/*
  Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

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

#include "editoritemmanager.h"

#include <calendarsupport/utils.h>

#include <Akonadi/Item>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemDeleteJob>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/ItemMoveJob>
#include <Akonadi/Monitor>
#include <Akonadi/Session>
#include <Akonadi/TransactionSequence>

#include <KJob>
#include <KLocale>

#include <QMessageBox>
#include <QPointer>

/// ItemEditorPrivate

namespace IncidenceEditorNG {

class ItemEditorPrivate
{
  EditorItemManager *q_ptr;
  Q_DECLARE_PUBLIC( EditorItemManager )

  public:
    Akonadi::Item mItem;
    Akonadi::Item mPrevItem;
    Akonadi::ItemFetchScope mFetchScope;
    Akonadi::Monitor *mItemMonitor;
    ItemEditorUi *mItemUi;
    bool mIsCounterProposal;
    EditorItemManager::SaveAction currentAction;

  public:
    ItemEditorPrivate( EditorItemManager *qq );
    void itemChanged( const Akonadi::Item &, const QSet<QByteArray> & );
    void itemFetchResult( KJob *job );
    void itemMoveResult( KJob *job );
    void modifyResult( KJob *job );
    void setupMonitor();
    void createMoveAndModifyTransactionJob();
    void moveAndModifyTransactionFinished( KJob *job );
};

ItemEditorPrivate::ItemEditorPrivate( EditorItemManager *qq )
  : q_ptr( qq ), mItemMonitor( 0 ), mIsCounterProposal( false )
{
  mFetchScope.fetchFullPayload();
  mFetchScope.setAncestorRetrieval( Akonadi::ItemFetchScope::Parent );
}

void ItemEditorPrivate::createMoveAndModifyTransactionJob()
{
  Q_Q( EditorItemManager );
  Akonadi::TransactionSequence *transaction = new Akonadi::TransactionSequence;
  q->connect( transaction, SIGNAL(result(KJob*)), SLOT(moveAndModifyTransactionFinished(KJob*)) );
  new Akonadi::ItemModifyJob( mItem, transaction );
  new Akonadi::ItemMoveJob( mItem, mItemUi->selectedCollection(), transaction );
}

void ItemEditorPrivate::moveAndModifyTransactionFinished( KJob *job )
{
  Q_Q( EditorItemManager );
  if ( job->error() ) {
    kError() << "Error while moving and modifying " << job->errorString();
    mItemUi->reject( ItemEditorUi::ItemMoveFailed, job->errorString() );
  } else {
    Akonadi::Item item;
    item.setId( mItem.id() );
    currentAction = EditorItemManager::MoveAndModify;
    q->load( item );
  }
}

void ItemEditorPrivate::itemFetchResult( KJob *job )
{
  Q_ASSERT( job );
  Q_Q( EditorItemManager );

  EditorItemManager::SaveAction action = currentAction;
  currentAction = EditorItemManager::None;

  if ( job->error() ) {
    mItemUi->reject( ItemEditorUi::ItemFetchFailed, job->errorString() );
    return;
  }

  Akonadi::ItemFetchJob *fetchJob = qobject_cast<Akonadi::ItemFetchJob*>( job );
  if ( fetchJob->items().isEmpty() ) {
    mItemUi->reject( ItemEditorUi::ItemFetchFailed );
    return;
  }

  Akonadi::Item item = fetchJob->items().first();
  if ( mItemUi->hasSupportedPayload( item ) ) {
    q->load( item );
    if ( action != EditorItemManager::None ) {
      // Finally enable ok/apply buttons, we've finished loading
      emit q->itemSaveFinished( action );
    }
  } else {
    mItemUi->reject( ItemEditorUi::ItemHasInvalidPayload );
  }
}

void ItemEditorPrivate::itemMoveResult( KJob *job )
{
  Q_ASSERT( job );
  Q_Q( EditorItemManager );

  if ( job->error() ) {
    Akonadi::ItemMoveJob *moveJob = qobject_cast<Akonadi::ItemMoveJob*>( job );
    Q_ASSERT( moveJob );
    //Q_ASSERT( !moveJob->items().isEmpty() );
    // TODO: What is reasonable behavior at this point?
    kError() << "Error while moving item ";// << moveJob->items().first().id() << " to collection "
             //<< moveJob->destinationCollection() << job->errorString();
    emit q->itemSaveFailed( EditorItemManager::Move, job->errorString() );
  } else {
    // Fetch the item again, we want a new mItem, which has an updated parentCollection
    Akonadi::Item item( mItem.id() );
    // set currentAction, so the fetchResult slot emits itemSavedFinished( Move );
    // We could emit it here, but we should only enable ok/apply buttons after the loading
    // is complete
    currentAction = EditorItemManager::Move;
    q->load( item );
  }
}

void ItemEditorPrivate::modifyResult( KJob *job )
{
  Q_ASSERT( job );
  Q_Q( EditorItemManager );

  if ( job->error() ) {
    if ( qobject_cast<Akonadi::ItemModifyJob*>( job ) ) {
      kError() << "Modify failed " << job->errorString();
      emit q->itemSaveFailed( EditorItemManager::Modify, job->errorString() );
    } else {
      kError() << "Creation failed " << job->errorString();
      emit q->itemSaveFailed( EditorItemManager::Create, job->errorString() );
    }
    return;
  }

  if ( Akonadi::ItemModifyJob *modifyJob = qobject_cast<Akonadi::ItemModifyJob*>( job ) ) {
    mItem = modifyJob->item();
    emit q->itemSaveFinished( EditorItemManager::Modify );
  } else {
    Akonadi::ItemCreateJob *createJob = qobject_cast<Akonadi::ItemCreateJob*>( job );
    Q_ASSERT( createJob );
    q->load( createJob->item() );
    emit q->itemSaveFinished( EditorItemManager::Create );
  }

  setupMonitor();
}

void ItemEditorPrivate::setupMonitor()
{
  // Q_Q( EditorItemManager );
  delete mItemMonitor;
  mItemMonitor = new Akonadi::Monitor;
  mItemMonitor->ignoreSession( Akonadi::Session::defaultSession() );
  mItemMonitor->itemFetchScope().fetchFullPayload();
  if ( mItem.isValid() ) {
    mItemMonitor->setItemMonitored( mItem );
  }

//   q->connect( mItemMonitor, SIGNAL(itemChanged(Akonadi::Item,QSet<QByteArray>)),
//               SLOT(itemChanged(Akonadi::Item,QSet<QByteArray>)) );
}

void ItemEditorPrivate::itemChanged( const Akonadi::Item &item,
                                     const QSet<QByteArray> &partIdentifiers )
{
  Q_Q( EditorItemManager );
  if ( mItemUi->containsPayloadIdentifiers( partIdentifiers ) ) {
    QPointer<QMessageBox> dlg = new QMessageBox; //krazy:exclude=qclasses
    dlg->setIcon( QMessageBox::Question );
    dlg->setInformativeText( i18n( "The item has been changed by another application.\n"
                                   "What should be done?" ) );
    dlg->addButton( i18n( "Take over changes" ), QMessageBox::AcceptRole );
    dlg->addButton( i18n( "Ignore and Overwrite changes" ), QMessageBox::RejectRole );

    if ( dlg->exec() == QMessageBox::AcceptRole ) {
      Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( mItem );
      job->setFetchScope( mFetchScope );

      mItem = item;

      q->load( mItem );
    } else {
      mItem.setRevision( item.revision() );
      q->save();
    }

    delete dlg;
  }

  // Overwrite or not, we need to update the revision and the remote id to be able
  // to store item later on.
  mItem.setRevision( item.revision() );
}

/// ItemEditor

EditorItemManager::EditorItemManager( ItemEditorUi *ui )
  : d_ptr( new ItemEditorPrivate( this ) )
{
  Q_D( ItemEditor );
  d->mItemUi = ui;
}

EditorItemManager::~EditorItemManager()
{
  delete d_ptr;
}

Akonadi::Item EditorItemManager::item( ItemState state ) const
{
  Q_D( const ItemEditor );

  switch ( state ) {
  case EditorItemManager::AfterSave:
    if ( d->mItem.hasPayload() ) {
      return d->mItem;
    } else {
      kDebug() << "Won't return mItem because isValid = " << d->mItem.isValid()
               << "; and haPayload is " << d->mItem.hasPayload();
    }
    break;
  case EditorItemManager::BeforeSave:
    if ( d->mPrevItem.hasPayload() ) {
      return d->mPrevItem;
    } else {
      kDebug() << "Won't return mPrevItem because isValid = " << d->mPrevItem.isValid()
               << "; and haPayload is " << d->mPrevItem.hasPayload();
    }
    break;
  default:
    kDebug() << "state = " << state;
    Q_ASSERT_X( false, "EditorItemManager::item", "Unknown enum value" ) ;
  }
  return Akonadi::Item();
}

void EditorItemManager::load( const Akonadi::Item &item )
{
  Q_D( ItemEditor );

  if ( item.hasPayload() ) {
    d->mPrevItem = item;
    d->mItem = item;
    d->mItemUi->load( item );
    d->setupMonitor();
  } else {
    Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( item, this );
    job->setFetchScope( d->mFetchScope );
    connect( job, SIGNAL(result(KJob*)), SLOT(itemFetchResult(KJob*)) );
  }
}

void EditorItemManager::revertLastSave()
{
  Q_D( ItemEditor );

  if ( d->mPrevItem.hasPayload() ) {
    // Modify
    Q_ASSERT( d->mItem.isValid() ); // Really, if this isn't true, then fix the logic somewhere else
    Q_ASSERT( d->mItem.id() == d->mPrevItem.id() ); // managing two different items??

    d->mPrevItem.setRevision( d->mItem.revision() );
    Akonadi::ItemModifyJob *job = new Akonadi::ItemModifyJob( d->mPrevItem );
    if ( !job->exec() ) {
      kDebug() << "Revert failed, could not delete item." << job->errorText();
    }
  } else if ( d->mItem.isValid() ) {
    // No payload in the previous item and the current item is valid, so the last
    // call to save created a new item and reverting that means that we have to
    // delete it.
    Akonadi::ItemDeleteJob *job = new Akonadi::ItemDeleteJob( d->mItem );
    if ( !job->exec() ) {
      kDebug() << "Revert failed, could not delete item." << job->errorText();
    }
  }

  // else, the previous item had no payload *and* the current item is not valid,
  // meaning that no item has been saved yet. Nothing to be done.
}

void EditorItemManager::save()
{
  Q_D( ItemEditor );

  if ( !d->mItemUi->isValid() ) {
    emit itemSaveFailed( d->mItem.isValid() ? Modify : Create, QString() );
    return;
  }

  if ( !d->mItemUi->isDirty() &&
       d->mItemUi->selectedCollection() == d->mItem.parentCollection() ) {
    // Item did not change and was not moved
    emit itemSaveFinished( None );
    return;
  }

  Akonadi::Item updateItem = d->mItemUi->save( d->mItem );
  Q_ASSERT( updateItem.id() == d->mItem.id() );
  d->mItem = updateItem;

  if ( d->mItem.isValid() ) { // A valid item. Means we're modifying.
    Q_ASSERT( d->mItem.parentCollection().isValid() );

    //TODO_SERGIO: incidence changere here
    KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( d->mItem );

    if ( d->mItem.parentCollection() == d->mItemUi->selectedCollection() ) {
      const bool modify = true;//invitationHandler.handleIncidenceAboutToBeModified( incidence ); TODO_SERGIO
      if ( modify ) {
        Akonadi::ItemModifyJob *modifyJob = new Akonadi::ItemModifyJob( d->mItem );
        connect( modifyJob, SIGNAL(result(KJob*)), SLOT(modifyResult(KJob*)) );
      } else {
        emit itemSaveFailed( EditorItemManager::Modify, QString() );
        Akonadi::Item item;
        item.setId( d->mItem.id() );
        load( item );
      }
    } else {
      Q_ASSERT( d->mItemUi->selectedCollection().isValid() );

      if ( d->mItemUi->isDirty() ) {
        const bool modify = true; // invitationHandler.handleIncidenceAboutToBeModified( incidence ); TODO_SERGIO
        if ( modify ) {
          d->createMoveAndModifyTransactionJob();
        } else {
          emit itemSaveFailed( EditorItemManager::Modify, QString() );
          Akonadi::Item item;
          item.setId( d->mItem.id() );
          load( item );
        }
      } else {
        Akonadi::ItemMoveJob *itemMoveJob =
          new Akonadi::ItemMoveJob( d->mItem, d->mItemUi->selectedCollection() );
        connect( itemMoveJob, SIGNAL(result(KJob*)), SLOT(itemMoveResult(KJob*)) );
      }
    }
  } else { // An invalid item. Means we're creating.
    if ( d->mIsCounterProposal ) {
      emit itemSaveFinished( EditorItemManager::Modify );
    } else {
      Q_ASSERT( d->mItemUi->selectedCollection().isValid() );

      Akonadi::ItemCreateJob *createJob =
        new Akonadi::ItemCreateJob( d->mItem, d->mItemUi->selectedCollection() );
      connect( createJob, SIGNAL(result(KJob*)), SLOT(modifyResult(KJob*)) );
    }
  }
}

void EditorItemManager::setFetchScope( const Akonadi::ItemFetchScope &fetchScope )
{
  Q_D( ItemEditor );
  d->mFetchScope = fetchScope;
}

Akonadi::ItemFetchScope &EditorItemManager::fetchScope()
{
  Q_D( ItemEditor );
  return d->mFetchScope;
}

void EditorItemManager::setIsCounterProposal( bool isCounterProposal )
{
  Q_D( ItemEditor );
  d->mIsCounterProposal = isCounterProposal;
}

ItemEditorUi::~ItemEditorUi()
{
}

bool ItemEditorUi::isValid() const
{
  return true;
}

} // namespace

#include "editoritemmanager.moc"
