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
#include "individualmailcomponentfactory.h"

#include <calendarsupport/utils.h>
#include <calendarsupport/kcalprefs.h>

#include <Akonadi/Item>
#include <Akonadi/ItemDeleteJob>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/ItemMoveJob>
#include <Akonadi/Monitor>
#include <Akonadi/Session>

#include <KJob>
#include <KLocalizedString>

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
    Akonadi::IncidenceChanger *mChanger;

  public:
    ItemEditorPrivate( Akonadi::IncidenceChanger *changer, EditorItemManager *qq );
    void itemChanged( const Akonadi::Item &, const QSet<QByteArray> & );
    void itemFetchResult( KJob *job );
    void itemMoveResult( KJob *job );
    void onModifyFinished( int changeId, const Akonadi::Item &item,
                           Akonadi::IncidenceChanger::ResultCode resultCode,
                           const QString &errorString );

    void onCreateFinished( int changeId,
                           const Akonadi::Item &item,
                           Akonadi::IncidenceChanger::ResultCode resultCode,
                           const QString &errorString );

    void setupMonitor();
    void moveJobFinished( KJob *job );
};

ItemEditorPrivate::ItemEditorPrivate( Akonadi::IncidenceChanger *changer, EditorItemManager *qq )
  : q_ptr( qq ), mItemMonitor( 0 ), mIsCounterProposal( false )
{
  mFetchScope.fetchFullPayload();
  mFetchScope.setAncestorRetrieval( Akonadi::ItemFetchScope::Parent );

  mChanger = changer ? changer : new Akonadi::IncidenceChanger( new IndividualMailComponentFactory(qq), qq );

  qq->connect( mChanger,
              SIGNAL(modifyFinished(int,Akonadi::Item,Akonadi::IncidenceChanger::ResultCode,QString)),
              qq, SLOT(onModifyFinished(int,Akonadi::Item,Akonadi::IncidenceChanger::ResultCode,QString)) );

  qq->connect( mChanger,
              SIGNAL(createFinished(int,Akonadi::Item,Akonadi::IncidenceChanger::ResultCode,QString)),
              qq, SLOT(onCreateFinished(int,Akonadi::Item,Akonadi::IncidenceChanger::ResultCode,QString)) );
}

void ItemEditorPrivate::moveJobFinished( KJob *job )
{
  Q_Q( EditorItemManager );
  if ( job->error() ) {
    kError() << "Error while moving and modifying " << job->errorString();
    mItemUi->reject( ItemEditorUi::ItemMoveFailed, job->errorString() );
  } else {
    Akonadi::Item item( mItem.id() );
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
    Q_UNUSED( moveJob );
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

void ItemEditorPrivate::onModifyFinished( int, const Akonadi::Item &item,
                                          Akonadi::IncidenceChanger::ResultCode resultCode,
                                          const QString &errorString )
{
  Q_Q( EditorItemManager );
  if ( resultCode == Akonadi::IncidenceChanger::ResultCodeSuccess ) {
    if ( mItem.parentCollection() == mItemUi->selectedCollection() ||
         mItem.storageCollectionId() == mItemUi->selectedCollection().id()) {
      mItem = item;
      emit q->itemSaveFinished( EditorItemManager::Modify );
      setupMonitor();
    } else { // There's a collection move too.
      Akonadi::ItemMoveJob *moveJob = new Akonadi::ItemMoveJob( mItem, mItemUi->selectedCollection() );
      q->connect( moveJob, SIGNAL(result(KJob*)), SLOT(moveJobFinished(KJob*)) );
    }
  } else if ( resultCode == Akonadi::IncidenceChanger::ResultCodeUserCanceled ) {
    emit q->itemSaveFailed( EditorItemManager::Modify, QString() );
    q->load( Akonadi::Item( mItem.id() ) );
  } else {
    kError() << "Modify failed " << errorString;
    emit q->itemSaveFailed( EditorItemManager::Modify, errorString );
  }
}

void ItemEditorPrivate::onCreateFinished( int,
                                          const Akonadi::Item &item,
                                          Akonadi::IncidenceChanger::ResultCode resultCode,
                                          const QString &errorString )
{
  Q_Q( EditorItemManager );
  if ( resultCode == Akonadi::IncidenceChanger::ResultCodeSuccess ) {
    q->load( item );
    emit q->itemSaveFinished( EditorItemManager::Create );
    setupMonitor();
  } else {
    kError() << "Creation failed " << errorString;
    emit q->itemSaveFailed( EditorItemManager::Create, errorString );
  }
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

EditorItemManager::EditorItemManager( ItemEditorUi *ui, Akonadi::IncidenceChanger *changer )
  : d_ptr( new ItemEditorPrivate( changer, this ) )
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

  d->mChanger->setGroupwareCommunication( CalendarSupport::KCalPrefs::instance()->useGroupwareCommunication() );

  Akonadi::Item updateItem = d->mItemUi->save( d->mItem );
  Q_ASSERT( updateItem.id() == d->mItem.id() );
  d->mItem = updateItem;

  if ( d->mItem.isValid() ) { // A valid item. Means we're modifying.
    Q_ASSERT( d->mItem.parentCollection().isValid() );
    KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( d->mItem );

    KCalCore::Incidence::Ptr oldPayload = CalendarSupport::incidence( d->mPrevItem );
    if ( d->mItem.parentCollection() == d->mItemUi->selectedCollection()
         || d->mItem.storageCollectionId() == d->mItemUi->selectedCollection().id()) {
      d->mChanger->modifyIncidence( d->mItem, oldPayload );
    } else {
      Q_ASSERT( d->mItemUi->selectedCollection().isValid() );
      Q_ASSERT( d->mItem.parentCollection().isValid() );

      // ETM and the KSelectionProxyModel has a bug wrt collections moves, so this is disabled.
      // To test this, enable the collection combo-box and remove the following assert.
      kError() << "Moving between collections is disabled for now: "
               << d->mItemUi->selectedCollection().id()
               << d->mItem.parentCollection().id();
      Q_ASSERT_X( false, "save()", "Moving between collections is disabled for now" );

      if ( d->mItemUi->isDirty() ) {
        d->mChanger->modifyIncidence( d->mItem, oldPayload );
      } else {
        Akonadi::ItemMoveJob *itemMoveJob =
          new Akonadi::ItemMoveJob( d->mItem, d->mItemUi->selectedCollection() );
        connect( itemMoveJob, SIGNAL(result(KJob*)), SLOT(itemMoveResult(KJob*)) );
      }
    }
  } else { // An invalid item. Means we're creating.
    if ( d->mIsCounterProposal ) {
      // We don't write back to akonadi, that will be done in ITipHandler.
      emit itemSaveFinished( EditorItemManager::Modify );
    } else {
      Q_ASSERT( d->mItemUi->selectedCollection().isValid() );
      KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( d->mItem );
      d->mChanger->createIncidence( incidence, d->mItemUi->selectedCollection() );
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

Akonadi::Collection EditorItemManager::collection() const
{
  Q_D( const ItemEditor );
  return d->mChanger->lastCollectionUsed();
}

ItemEditorUi::~ItemEditorUi()
{
}

bool ItemEditorUi::isValid() const
{
  return true;
}

} // namespace

#include "moc_editoritemmanager.cpp"
