/*
  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (C) 2010 Sérgio Martins <iamsergio@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#include "incidencechanger2.h"
#include "incidencechanger2_p.h"
#include "history.h"
#include "utils.h"

#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/ItemDeleteJob>

#include <KJob>
#include <KLocale>
#include <KMessageBox>

using namespace Akonadi;
using namespace KCalCore;
using namespace CalendarSupport;

IncidenceChanger2::Private::Private( IncidenceChanger2 *qq ) : q( qq )
{
  mLatestChangeId = 0;
  mShowDialogsOnError = true;
  mHistory = new History( q );
  mDestinationPolicy = DestinationPolicyDefault;
  mRespectsCollectionRights = false;

  qRegisterMetaType<QVector<Akonadi::Item::Id> >( "QVector<Akonadi::Item::Id>" );

  qRegisterMetaType<CalendarSupport::IncidenceChanger2::ResultCode>(
    "CalendarSupport::IncidenceChanger2::ResultCode" );
}

IncidenceChanger2::Private::~Private()
{
  delete mHistory;

  if ( !mAtomicOperations.isEmpty() ||
       !mQueuedModifications.isEmpty() ||
       !mModificationsInProgress.isEmpty() ) {
    kDebug() << "Normal if the application was being used. "
                "But might indicate a memory leak if it wasn't";
  }
}

bool IncidenceChanger2::Private::atomicOperationIsValid( uint atomicOperationId ) const
{
  // Changes must be done between startAtomicOperation() and endAtomicOperation()
  return mAtomicOperations.contains( atomicOperationId ) &&
         !mAtomicOperations[atomicOperationId]->endCalled;
}

bool IncidenceChanger2::Private::hasRights( const Collection &collection,
                                            IncidenceChanger2::ChangeType changeType ) const
{
  bool result = false;
  switch( changeType ) {
    case ChangeTypeCreate:
      result = collection.rights() & Akonadi::Collection::CanCreateItem;
      break;
    case ChangeTypeModify:
      result = collection.rights() & Akonadi::Collection::CanChangeItem;
      break;
    case ChangeTypeDelete:
      result = collection.rights() & Akonadi::Collection::CanDeleteItem;
      break;
    default:
      Q_ASSERT_X( false, "hasRights", "invalid type" );
  }

  return !collection.isValid() || !mRespectsCollectionRights || result;
}

void IncidenceChanger2::Private::queueModification( const Change &change )
{
  // If there's already a change queued we just discard it
  // and send the newer change, which already includes
  // previous modifications
  const Akonadi::Item::Id id = change.newItem.id();
  if ( mQueuedModifications.contains( id ) ) {
    mQueuedModifications.take( id );
  }

  mQueuedModifications[id] = change;
}

void IncidenceChanger2::Private::performNextModification( Akonadi::Item::Id id )
{
  mModificationsInProgress.remove( id );

  if ( mQueuedModifications.contains( id ) ) {
    const Change change = mQueuedModifications[id];
    performModification( change );
  }
}

void IncidenceChanger2::Private::handleCreateJobResult( KJob *job )
{
  QString errorString;
  ResultCode resultCode = ResultCodeSuccess;

  const Change change = mChangeForJob.take( job );

  const ItemCreateJob *j = qobject_cast<const ItemCreateJob*>( job );
  Item item = j->item();

  if ( j->error() ) {
    item = change.newItem;
    resultCode = ResultCodeJobError;
    errorString = j->errorString();
    kError() << errorString;
    if ( mShowDialogsOnError ) {
      KMessageBox::sorry( change.parent,
                          i18n( "Error while trying to create calendar item. Error was: %1",
                                errorString ) );
    }
    if ( change.atomicOperationId != 0 ) {
      rollbackAtomicOperation( change.atomicOperationId );
    }
  } else {
    // for user undo/redo
    if ( change.recordToHistory ) {
      mHistory->recordCreation( item, change.atomicOperationId );
    }

    atomicOperationStuff( change );
  }

  emit q->createFinished( change.changeId, item, resultCode, errorString );
}

void IncidenceChanger2::Private::handleDeleteJobResult( KJob *job )
{
  QString errorString;
  ResultCode resultCode = ResultCodeSuccess;

  const Change change = mChangeForJob.take( job );

  const ItemDeleteJob *j = qobject_cast<const ItemDeleteJob*>( job );
  const Item::List items = j->deletedItems();

  QVector<Item::Id> itemIdList;
  foreach ( const Item &item, items ) {
    itemIdList.append( item.id() );
  }

  if ( j->error() ) {
    resultCode = ResultCodeJobError;
    errorString = j->errorString();
    kError() << errorString;
    if ( mShowDialogsOnError ) {
      KMessageBox::sorry( change.parent,
                          i18n( "Error while trying to delete calendar item. Error was: %1",
                                errorString ) );
    }

    foreach ( const Item &item, items ) {
      // Werent deleted due to error
      mDeletedItemIds.remove( item.id() );
    }

    if ( change.atomicOperationId != 0 ) {
      rollbackAtomicOperation( change.atomicOperationId );
    }
  } else { // success
    foreach ( const Item &item, items ) {
      mLatestRevisionByItemId.remove( item.id() );
      if ( change.recordToHistory ) {
        //TODO: check return value
        //TODO: make History support a list of items
        mHistory->recordDeletion( item, change.atomicOperationId );
      }
    }
    atomicOperationStuff( change );
  }

  emit q->deleteFinished( change.changeId, itemIdList, resultCode, errorString );
}

void IncidenceChanger2::Private::handleModifyJobResult( KJob *job )
{
  QString errorString;
  ResultCode resultCode = ResultCodeSuccess;
  const Change change = mChangeForJob.take( job );

  const ItemModifyJob *j = qobject_cast<const ItemModifyJob*>( job );
  const Item item = j->item();
  if ( j->error() ) {
    if ( deleteAlreadyCalled( item.id() ) ) {
      // User deleted the item almost at the same time he changed it. We could just return success
      // but the delete is probably already recorded to History, and that would make undo not work
      // in the proper order.
      resultCode = ResultCodeAlreadyDeleted;
      errorString = j->errorString();
      kWarning() << "Trying to change item " << item.id() << " while deletion is in progress.";
    } else {
      resultCode = ResultCodeJobError;
      errorString = j->errorString();
      kError() << errorString;
    }
    if ( mShowDialogsOnError ) {
      KMessageBox::sorry( change.parent,
                          i18n( "Error while trying to modify calendar item. Error was: %1",
                                errorString ) );
    }

    if ( change.atomicOperationId != 0 ) {
      rollbackAtomicOperation( change.atomicOperationId );
    }
  } else { // success
    mLatestRevisionByItemId[item.id()] = item.revision();
    if ( change.recordToHistory && change.originalItem.isValid() ) {
      mHistory->recordModification( change.originalItem, item, change.atomicOperationId );
    }

    atomicOperationStuff( change );
  }

  emit q->modifyFinished( change.changeId, item, resultCode, errorString );

  qRegisterMetaType<Akonadi::Item::Id>( "Akonadi::Item::Id" );
  QMetaObject::invokeMethod( this, "performNextModification",
                             Qt::QueuedConnection,
                             Q_ARG( Akonadi::Item::Id, item.id() ) );
}

void IncidenceChanger2::Private::atomicOperationStuff( const Change &change )
{
  if ( change.atomicOperationId != 0 ) {
    AtomicOperation *a = mAtomicOperations[change.atomicOperationId];
    a->numCompletedChanges++;

    if ( a->numCompletedChanges == a->numChanges && a->endCalled ) {
      // endAtomicOperation() was already called, and all jobs completed
      delete mAtomicOperations.take( change.atomicOperationId );
    }
  }
}

bool IncidenceChanger2::Private::deleteAlreadyCalled( Akonadi::Item::Id id ) const
{
  return mDeletedItemIds.contains( id );
}

// Does a queued emit, with QMetaObject::invokeMethod
void IncidenceChanger2::Private::emitCreateFinished(
  int changeId, const Akonadi::Item &item,
  CalendarSupport::IncidenceChanger2::ResultCode resultCode,
  const QString &errorString )
{
  QMetaObject::invokeMethod( q, "createFinished", Qt::QueuedConnection,
                             Q_ARG( int, changeId ),
                             Q_ARG( Akonadi::Item, item ),
                             Q_ARG( CalendarSupport::IncidenceChanger2::ResultCode, resultCode ),
                             Q_ARG( QString, errorString ) );
}

// Does a queued emit, with QMetaObject::invokeMethod
void IncidenceChanger2::Private::emitModifyFinished(
  int changeId, const Akonadi::Item &item,
  CalendarSupport::IncidenceChanger2::ResultCode resultCode,
  const QString &errorString )
{
  QMetaObject::invokeMethod( q, "modifyFinished", Qt::QueuedConnection,
                             Q_ARG( int, changeId ),
                             Q_ARG( Akonadi::Item, item ),
                             Q_ARG( CalendarSupport::IncidenceChanger2::ResultCode, resultCode ),
                             Q_ARG( QString, errorString ) );
}

// Does a queued emit, with QMetaObject::invokeMethod
void IncidenceChanger2::Private::emitDeleteFinished(
  int changeId, const QVector<Akonadi::Item::Id> &itemIdList,
  CalendarSupport::IncidenceChanger2::ResultCode resultCode,
  const QString &errorString )
{
  QMetaObject::invokeMethod( q, "deleteFinished", Qt::QueuedConnection,
                             Q_ARG( int, changeId ),
                             Q_ARG( QVector<Akonadi::Item::Id>, itemIdList ),
                             Q_ARG( CalendarSupport::IncidenceChanger2::ResultCode, resultCode ),
                             Q_ARG( QString, errorString ) );
}

void IncidenceChanger2::Private::rollbackAtomicOperation( uint atomicOperationId )
{
  Q_ASSERT( mAtomicOperations.contains( atomicOperationId ) );
  mAtomicOperations[atomicOperationId]->history->undoAll();
}

IncidenceChanger2::IncidenceChanger2() : QObject(),
                                         d( new Private( this ) )
{
}

IncidenceChanger2::~IncidenceChanger2()
{
  delete d;
}

int IncidenceChanger2::createIncidence( const Incidence::Ptr &incidence,
                                        const Collection &collection,
                                        uint atomicOperationId,
                                        bool recordToHistory,
                                        QWidget *parent )
{
  if ( !incidence ) {
    kWarning() << "An invalid payload is not allowed.";
    return -1;
  }

  if ( atomicOperationId != 0 && !d->atomicOperationIsValid( atomicOperationId ) ) {
    Q_ASSERT_X( false, "createIncidence()",
                       "endAtomicOperation() already called for that atomicOperationId"
                       " or startAtomicOperation() not even called" );
    return -1;
  }

  const Change change( ++d->mLatestChangeId, atomicOperationId, recordToHistory, parent );
  Collection collectionToUse;

  if ( atomicOperationId != 0 && d->mAtomicOperations[atomicOperationId]->rollbackInProgress ) {
    // rollback is in progress, no more changes allowed.
    // TODO: better message, and i18n
    const QString errorMessage = "One change belonging to a group of changes failed."
                                 "Undoing in progress.";

    d->emitCreateFinished( change.changeId, Item(), ResultCodeRollback, errorMessage );
    return change.changeId;
  }

  if ( collection.isValid() && d->hasRights( collection, ChangeTypeCreate ) ) {
    // The collection passed always has priority
    collectionToUse = collection;
  } else {
    switch( d->mDestinationPolicy ) {
      case DestinationPolicyDefault:
        if ( d->mDefaultCollection.isValid() &&
             d->hasRights( d->mDefaultCollection, ChangeTypeCreate ) ) {
          collectionToUse = d->mDefaultCollection;
          break;
        }
        // else fallthrough, and ask the user.
      case DestinationPolicyAsk:
      {
        int dialogCode;
        const QStringList mimeTypes( incidence->mimeType() );
        collectionToUse = CalendarSupport::selectCollection( parent,
                                                             dialogCode/*by-ref*/,
                                                             mimeTypes,
                                                             d->mDefaultCollection );
        if ( dialogCode != QDialog::Accepted ) {
          kDebug() << "No valid collection to use.";
          return -2;
        }

        // TODO: add unit test for these two situations after reviewing API
        if ( !collectionToUse.isValid() || !d->hasRights( collectionToUse, ChangeTypeCreate ) ) {
          kError() << "Invalid collection selected. Can't create incidence.";
          return -2;
        }
      }
      break;
      case DestinationPolicyNeverAsk:
      {
        const bool rights = d->hasRights( d->mDefaultCollection, ChangeTypeCreate );
        if ( d->mDefaultCollection.isValid() && rights ) {
          collectionToUse = d->mDefaultCollection;
        } else {
          // error is not i18n'd, should be a bug in the application using IncidenceChanger.
          d->emitCreateFinished( change.changeId,
                                 Item(),
                                 ResultCodeInvalidDefaultCollection,
                                 QString::fromLatin1( "Default collection is invalid or doesn't have "
                                                "rights and DestinationPolicyNeverAsk was used. "
                                                "; rights = %1").arg(rights) );
          return change.changeId;
        }
      }
      break;
    default:
      // Never happens
      Q_ASSERT_X( false, "createIncidence()", "unknown destination policy" );
      return -1;
    }
  }

  Item item;
  item.setPayload<Incidence::Ptr>( incidence );
  item.setMimeType( incidence->mimeType() );
  ItemCreateJob *createJob = new ItemCreateJob( item, collectionToUse );

  d->mChangeForJob.insert( createJob, change );

  if ( atomicOperationId != 0 ) {
    d->mAtomicOperations[atomicOperationId]->numChanges++;
  }

  // QueuedConnection because of possible sync exec calls.
  connect( createJob, SIGNAL(result(KJob*)),
           d, SLOT(handleCreateJobResult(KJob*)), Qt::QueuedConnection );

  return change.changeId;
}

int IncidenceChanger2::deleteIncidence( const Item &item,
                                        uint atomicOperationId,
                                        bool recordToHistory,
                                        QWidget *parent )
{
  Item::List list;
  list.append( item );

  return deleteIncidences( list, atomicOperationId, recordToHistory, parent );
}

int IncidenceChanger2::deleteIncidences( const Item::List &items,
                                         uint atomicOperationId,
                                         bool recordToHistory,
                                         QWidget *parent )
{
  if ( items.isEmpty() ) {
    kWarning() << "Delete what?";
    return -1;
  }

  if ( atomicOperationId != 0 && !d->atomicOperationIsValid( atomicOperationId ) ) {
    Q_ASSERT_X( false, "deleteIncidence()",
                       "endAtomicOperation() already called for that atomicOperationId"
                       " or startAtomicOperation() not even called" );
    return -1;
  }

  foreach ( const Item &item, items ) {
    if ( !item.isValid() ) {
      kWarning() << "Items must be valid!";
      return -1;
    }

    if ( !d->hasRights( item.parentCollection(), ChangeTypeDelete ) ) {
      kWarning() << "Item " << item.id() << " can't be deleted due to ACL restrictions";
      return -2;
    }
  }

  Item::List itemsToDelete;

  foreach ( const Item &item, items ) {
    if ( d->deleteAlreadyCalled( item.id() ) ) {
      // IncidenceChanger::deleteIncidence() called twice, ignore this one.
      kDebug() << "Item " << item.id() << " already deleted or being deleted, skipping";
    } else {
      itemsToDelete.append( item );
    }
  }

  const Change change( ++d->mLatestChangeId, atomicOperationId, recordToHistory, parent );

  if ( atomicOperationId != 0 && d->mAtomicOperations[atomicOperationId]->rollbackInProgress ) {
    // rollback is in progress, no more changes allowed.
    // TODO: better message, and i18n
    const QString errorMessage = "One change belonging to a group of changes failed."
                                 "Undoing in progress.";

    d->emitDeleteFinished( change.changeId, QVector<Akonadi::Item::Id>(),
                           ResultCodeRollback, errorMessage );
    return change.changeId;
  }

  if ( itemsToDelete.isEmpty() ) {
    QVector<Akonadi::Item::Id> itemIdList;
    itemIdList.append( Item().id() );
    kDebug() << "Items already deleted or being deleted, skipping";
    // Queued emit because return must be executed first, otherwise caller won't know this workId
    d->emitDeleteFinished( change.changeId, itemIdList, ResultCodeAlreadyDeleted,
                           i18n( "That calendar item was already deleted, "
                                 "or currently being deleted." ) );

    return change.changeId;
  }

  ItemDeleteJob *deleteJob = new ItemDeleteJob( itemsToDelete );
  d->mChangeForJob.insert( deleteJob, change );

  if ( atomicOperationId != 0 ) {
    d->mAtomicOperations[atomicOperationId]->numChanges++;
  }

  foreach ( const Item &item, itemsToDelete ) {
    d->mDeletedItemIds.insert( item.id() );
  }

  // QueuedConnection because of possible sync exec calls.
  connect( deleteJob, SIGNAL(result(KJob*)),
           d, SLOT(handleDeleteJobResult(KJob*)), Qt::QueuedConnection );

  return change.changeId;
}

int IncidenceChanger2::modifyIncidence( const Item &changedItem,
                                        const Item &originalItem,
                                        uint atomicOperationId,
                                        bool recordToHistory,
                                        QWidget *parent )
{
  if ( !changedItem.isValid() || !changedItem.hasPayload<Incidence::Ptr>() ) {
    kWarning() << "An invalid item or payload is not allowed.";
    return -1;
  }

  if ( originalItem.isValid() && !originalItem.hasPayload<Incidence::Ptr>() ) {
    kWarning() << "The original item is valid, but doesn't have a valid payload.";
    return -1;
  }

  if ( atomicOperationId != 0 && !d->atomicOperationIsValid( atomicOperationId ) ) {
    Q_ASSERT_X( false, "modifyIncidence()",
                       "endAtomicOperation() already called for that atomicOperationId"
                       " or startAtomicOperation() not even called" );
    return -1;
  }

  if ( !d->hasRights( changedItem.parentCollection(), ChangeTypeModify ) ) {
    kWarning() << "Item " << changedItem.id() << " can't be deleted due to ACL restrictions";
    return -2;
  }

  Change change( ++d->mLatestChangeId, atomicOperationId, recordToHistory, parent );
  change.originalItem = originalItem;
  change.newItem = changedItem;

  d->performModification( change );
  return change.changeId;
}

void IncidenceChanger2::Private::performModification( Change change )
{
  const Item::Id id = change.newItem.id();
  Q_ASSERT( change.newItem.isValid() );
  Q_ASSERT( change.newItem.hasPayload<Incidence::Ptr>() );

  if ( deleteAlreadyCalled( id ) ) {
    // IncidenceChanger::deleteIncidence() called twice, ignore this one.
    kDebug() << "Item " << id << " already deleted or being deleted, skipping";

    // Queued emit because return must be executed first, otherwise caller won't know this workId
    emitModifyFinished( change.changeId, change.newItem, ResultCodeAlreadyDeleted,
                        i18n( "That calendar item was already deleted, "
                              "or currently being deleted." ) );
    return;
  }

  if ( change.atomicOperationId != 0 &&
       mAtomicOperations[change.atomicOperationId]->rollbackInProgress ) {
    // rollback is in progress, no more changes allowed.
    // TODO: better message, and i18n
    const QString errorMessage = "One change belonging to a group of changes failed."
                                 "Undoing in progress.";

    emitModifyFinished( change.changeId, change.newItem, ResultCodeRollback, errorMessage );
    return;
  }

  if ( mLatestRevisionByItemId.contains( id ) &&
       mLatestRevisionByItemId[id] > change.newItem.revision() ) {
    /* When a ItemModifyJob ends, the application can still modify the old items if the user
     * is quick because the ETM wasn't updated yet, and we'll get a STORE error, because
     * we are not modifying the latest revision.
     *
     * When a job ends, we keep the new revision in m_latestVersionByItemId
     * so we can update the item's revision
     */
    change.newItem.setRevision( mLatestRevisionByItemId[id] );
  }

  { // increment revision ( KCalCore revision, not akonadi )
    Incidence::Ptr incidence = change.newItem.payload<Incidence::Ptr>();
    const int revision = incidence->revision();
    incidence->setRevision( revision + 1 );
  }

  // Dav Fix.
  // Don't write back remote revision since we can't make sure it is the current one
  change.newItem.setRemoteRevision( QString() );

  if ( mModificationsInProgress.contains( change.newItem.id() ) ) {
    // There's already a ItemModifyJob running for this item ID
    // Let's wait for it to end.
    queueModification( change );
  } else {
    ItemModifyJob *modifyJob = new ItemModifyJob( change.newItem );
    mChangeForJob.insert( modifyJob, change );

    if ( change.atomicOperationId != 0 ) {
      mAtomicOperations[change.atomicOperationId]->numChanges++;
    }

    mModificationsInProgress[change.newItem.id()] = change;
    // QueuedConnection because of possible sync exec calls.
    connect( modifyJob, SIGNAL(result(KJob*)),
             SLOT(handleModifyJobResult(KJob*)), Qt::QueuedConnection );
  }
}

uint IncidenceChanger2::startAtomicOperation()
{
  static uint latestAtomicOperationId = 0;
  ++latestAtomicOperationId;

  AtomicOperation *atomicOperation = new AtomicOperation( latestAtomicOperationId );
  atomicOperation->history = new History( this );
  d->mAtomicOperations[latestAtomicOperationId] = atomicOperation;

  return latestAtomicOperationId;
}

void IncidenceChanger2::endAtomicOperation( uint atomicOperationId )
{
  Q_ASSERT_X( d->mAtomicOperations.contains( atomicOperationId ),
              "endAtomicOperation()",
              "Unknown atomic operation id, only use ids returned by startAtomicOperation()." );

  AtomicOperation *atomicOperation = d->mAtomicOperations[atomicOperationId];

  if ( atomicOperation->numChanges == atomicOperation->numCompletedChanges ) {
    // All jobs already ended, free stuff.
    delete d->mAtomicOperations.take( atomicOperationId );
  } else {
    atomicOperation->endCalled = true;
    // The gate closed. We will Q_ASSERT that create|modify|deleteIncidence
    // aren't called with this atomicOperationId.
  }
}

void IncidenceChanger2::setShowDialogsOnError( bool enable )
{
  d->mShowDialogsOnError = enable;
}

bool IncidenceChanger2::showDialogsOnError() const
{
  return d->mShowDialogsOnError;
}

void IncidenceChanger2::setRespectsCollectionRights( bool respects )
{
  d->mRespectsCollectionRights = respects;
}

bool IncidenceChanger2::respectsCollectionRights() const
{
  return d->mRespectsCollectionRights;
}

void IncidenceChanger2::setDestinationPolicy(
  IncidenceChanger2::DestinationPolicy destinationPolicy )
{
  d->mDestinationPolicy = destinationPolicy;
}

IncidenceChanger2::DestinationPolicy IncidenceChanger2::destinationPolicy() const
{
  return d->mDestinationPolicy;
}

void IncidenceChanger2::setDefaultCollection( const Akonadi::Collection &collection )
{
  d->mDefaultCollection = collection;
}

Collection IncidenceChanger2::defaultCollection() const
{
  return d->mDefaultCollection;
}

History * IncidenceChanger2::history() const
{
  return d->mHistory;
}
