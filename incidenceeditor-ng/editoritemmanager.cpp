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

#include "editoritemmanager.h"

#include <Akonadi/Item>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemDeleteJob>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/ItemMoveJob>
#include <Akonadi/Monitor>
#include <Akonadi/Session>

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

  public:
    ItemEditorPrivate( EditorItemManager *qq );
    void itemChanged( const Akonadi::Item &, const QSet<QByteArray> & );
    void itemFetchResult( KJob *job );
    void itemMoveResult( KJob *job );
    void modifyResult( KJob *job );
    void setupMonitor();
};

ItemEditorPrivate::ItemEditorPrivate( EditorItemManager *qq )
  : q_ptr( qq ), mItemMonitor( 0 )
{
  mFetchScope.fetchFullPayload();
  mFetchScope.setAncestorRetrieval( Akonadi::ItemFetchScope::Parent );
}

void ItemEditorPrivate::itemFetchResult( KJob *job )
{
  Q_ASSERT( job );
  Q_Q( EditorItemManager );

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
  } else {
    mItemUi->reject( ItemEditorUi::ItemHasInvalidPayload );
  }
}

void ItemEditorPrivate::itemMoveResult( KJob *job )
{
  Q_ASSERT( job );

  if ( job->error() ) {
    // TODO: What is reasonable behavior at this point?
    kError() << job->errorString();
//     mItemUi->reject( ItemEditorUi::ItemFetchFailed, job->errorString() );
    return;
  }
}

void ItemEditorPrivate::modifyResult( KJob *job )
{
  Q_ASSERT( job );
  Q_Q( EditorItemManager );

  if ( job->error() ) {
    if ( qobject_cast<Akonadi::ItemModifyJob*>( job ) ) {
      emit q->itemSaveFailed( EditorItemManager::Modify, job->errorString() );
    } else {
      emit q->itemSaveFailed( EditorItemManager::Create, job->errorString() );
    }
    return;
  }

  if ( Akonadi::ItemModifyJob *modifyJob = qobject_cast<Akonadi::ItemModifyJob*>( job ) ) {
    mItem = modifyJob->item();
    emit q->itemSaveFinished( EditorItemManager::Modify );
  } else {
    Akonadi::ItemCreateJob *createJob = qobject_cast<Akonadi::ItemCreateJob*>( job );
    Q_ASSERT(createJob);
    mItem = createJob->item();
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

//   q->connect( mItemMonitor, SIGNAL(itemChanged(const Akonadi::Item&,const QSet<QByteArray>&)),
//               SLOT(itemChanged(const Akonadi::Item&,const QSet<QByteArray>&)) );
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
    if ( d->mItem.isValid() && d->mItem.hasPayload() ) {
      return d->mItem;
    }
    break;
  case EditorItemManager::BeforeSave:
    if ( d->mPrevItem.isValid() && d->mPrevItem.hasPayload() ) {
      return d->mPrevItem;
    }
    break;
  }
  return Akonadi::Item();
}

void EditorItemManager::load( const Akonadi::Item &item )
{
  Q_ASSERT( item.isValid() );
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
    return;
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

    if ( d->mItem.parentCollection() == d->mItemUi->selectedCollection() ) {
      Akonadi::ItemModifyJob *modifyJob = new Akonadi::ItemModifyJob( d->mItem );
      connect( modifyJob, SIGNAL(result(KJob*)), SLOT(modifyResult(KJob*)) );
    } else {
      Q_ASSERT( d->mItemUi->selectedCollection().isValid() );

      if ( d->mItemUi->isDirty() ) {
        Q_ASSERT_X( false, "ItemEditor::save()",
                    "Moving of modified items not implemented yet" );
      // 1) ItemModify( d->mItem );
      // 2) ItemMove( d->mItem,d->mItemUi->selectedCollection() )
      } else {
        Akonadi::ItemMoveJob *imjob =
          new Akonadi::ItemMoveJob( d->mItem, d->mItemUi->selectedCollection() );
        connect( imjob, SIGNAL(result(KJob*)), SLOT(itemMoveResult(KJob*)) );
      }
    }
  } else { // An invalid item. Means we're creating.
    Q_ASSERT( d->mItemUi->selectedCollection().isValid() );

    Akonadi::ItemCreateJob *createJob =
      new Akonadi::ItemCreateJob( d->mItem, d->mItemUi->selectedCollection() );
    connect( createJob, SIGNAL(result(KJob*)), SLOT(modifyResult(KJob*)) );
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

ItemEditorUi::~ItemEditorUi()
{
}

bool ItemEditorUi::isValid() const
{
  return true;
}

}

#include "editoritemmanager.moc"
