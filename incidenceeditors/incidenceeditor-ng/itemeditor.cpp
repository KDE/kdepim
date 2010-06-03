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

#include "itemeditor.h"

#include <QtCore/QPointer>
#include <QtGui/QMessageBox>

#include <KLocale>

#include <Akonadi/Item>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/ItemMoveJob>
#include <Akonadi/Monitor>
#include <Akonadi/Session>

using namespace Akonadi;

/// ItemEditorPrivate

namespace Akonadi {

class ItemEditorPrivate
{
  ItemEditor *q_ptr;
  Q_DECLARE_PUBLIC( ItemEditor )

  public:
    Item mItem;
    Monitor *mItemMonitor;
    ItemEditorUi *mItemUi;

  public:
    ItemEditorPrivate( ItemEditor *qq );
    void itemChanged( const Akonadi::Item&, const QSet<QByteArray>& );
    void itemFetchResult( KJob *job );
    void itemMoveResult( KJob *job );
    void modifyResult( KJob *job );
    void setupMonitor();
};

}

ItemEditorPrivate::ItemEditorPrivate( ItemEditor *qq )
  : q_ptr( qq ), mItemMonitor( 0 )
{ }

void ItemEditorPrivate::itemFetchResult( KJob *job )
{
  Q_ASSERT( job );
  Q_Q( ItemEditor );

  if ( job->error() ) {
    mItemUi->reject( ItemEditorUi::ItemFetchFailed, job->errorString() );
    return;
  }

  ItemFetchJob *fetchJob = qobject_cast<ItemFetchJob*>( job );
  if ( fetchJob->items().isEmpty() ) {
    mItemUi->reject( ItemEditorUi::ItemFetchFailed );
    return;
  }

  Item item = fetchJob->items().first();
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
//     mItemUi->reject( ItemEditorUi::ItemFetchFailed, job->errorString() );
    return;
  }
}

void ItemEditorPrivate::modifyResult( KJob *job )
{
  Q_ASSERT( job );
  Q_Q( ItemEditor );

  if ( job->error() ) {
    emit q->itemSaveResult( ItemEditor::Failure, job->errorString() );
    return;
  }

  if ( ItemModifyJob *modifyJob = qobject_cast<ItemModifyJob*>( job ) ) {
    mItem = modifyJob->item();
    emit q->itemSaveResult( ItemEditor::Succes );
  } else {
    ItemCreateJob *createJob = qobject_cast<ItemCreateJob*>( job );
    Q_ASSERT(createJob);
    mItem = createJob->item();
    emit q->itemSaveResult( ItemEditor::Succes );
  }

  setupMonitor();
}

void ItemEditorPrivate::setupMonitor()
{
  Q_Q( ItemEditor );
  delete mItemMonitor;
  mItemMonitor = new Akonadi::Monitor;
  mItemMonitor->ignoreSession( Akonadi::Session::defaultSession() );
  mItemMonitor->itemFetchScope().fetchFullPayload();
  if ( mItem.isValid() )
    mItemMonitor->setItemMonitored( mItem );

  q->connect( mItemMonitor, SIGNAL( itemChanged( const Akonadi::Item&, const QSet<QByteArray>& ) ),
              SLOT( itemChanged( const Akonadi::Item&, const QSet<QByteArray>& ) ) );
}

void ItemEditorPrivate::itemChanged( const Akonadi::Item &item,
                                     const QSet<QByteArray> &partIdentifiers )
{
  Q_Q( ItemEditor );
  if ( mItemUi->containsPayloadIdentifiers( partIdentifiers ) ) {
    QPointer<QMessageBox> dlg = new QMessageBox; //krazy:exclude=qclasses
    dlg->setIcon( QMessageBox::Question );
    dlg->setInformativeText( i18n( "The item has been changed by another application.\nWhat should be done?" ) );
    dlg->addButton( i18n( "Take over changes" ), QMessageBox::AcceptRole );
    dlg->addButton( i18n( "Ignore and Overwrite changes" ), QMessageBox::RejectRole );

    if ( dlg->exec() == QMessageBox::AcceptRole ) {
      Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( mItem );
      job->fetchScope().fetchFullPayload();
      job->fetchScope().setAncestorRetrieval( Akonadi::ItemFetchScope::Parent );

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

ItemEditor::ItemEditor( ItemEditorUi *ui )
  : d_ptr( new ItemEditorPrivate( this ) )
{
  Q_D( ItemEditor );
  d->mItemUi = ui;
}


ItemEditor::~ItemEditor()
{
  delete d_ptr;
}


void ItemEditor::load( const Akonadi::Item &item )
{
  Q_ASSERT( item.isValid() );
  Q_D( ItemEditor );

  if ( item.hasPayload() ) {
    d->mItem = item;
    d->mItemUi->load( item );
    d->setupMonitor();
  } else {
    ItemFetchJob *job = new ItemFetchJob( item, this );
    job->fetchScope().fetchFullPayload();
    job->fetchScope().setAncestorRetrieval( Akonadi::ItemFetchScope::Parent );

    connect( job, SIGNAL(result(KJob*)), SLOT(itemFetchResult(KJob*)) );
    return;
  }
}

void ItemEditor::save()
{
  Q_D( ItemEditor );

  d->mItem = d->mItemUi->save( d->mItem );

  if ( d->mItem.isValid() ) { // A valid item needs to be modified.
    Q_ASSERT( d->mItem.parentCollection().isValid() );

    if ( d->mItem.parentCollection() == d->mItemUi->selectedCollection() ) {
      ItemModifyJob *modifyJob = new ItemModifyJob( d->mItem );
      connect( modifyJob, SIGNAL(result(KJob*)), SLOT(modifyResult(KJob*)) );
    } else {
      Q_ASSERT( d->mItemUi->selectedCollection().isValid() );

      if ( d->mItemUi->isDirty() )
        Q_ASSERT_X( false, "ItemEditor::save()",
                    "Moving of modified items not implemented yet" );
        // 1) ItemModify( d->mItem );
        // 2) ItemMove( d->mItem,d->mItemUi->selectedCollection() )
      else {
        ItemMoveJob *imjob = new ItemMoveJob( d->mItem, d->mItemUi->selectedCollection() );
        connect( imjob, SIGNAL(result(KJob*)), SLOT(itemMoveResult(KJob*)) );
      }
    }
  } else { // An invalid item needs to be created.
    Q_ASSERT( d->mItemUi->selectedCollection().isValid() );

    ItemCreateJob *createJob =
      new ItemCreateJob( d->mItem, d->mItemUi->selectedCollection() );
    connect( createJob, SIGNAL(result(KJob*)), SLOT(modifyResult(KJob*)) );
  }
}

#include "itemeditor.moc"
