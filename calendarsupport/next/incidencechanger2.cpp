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
  mLatestOperationId = 0;
  mShowDialogsOnError = true;
  mHistory = new History( q );
  mDestinationPolicy = DestinationPolicyDefault;

  qRegisterMetaType<QVector<Akonadi::Item::Id> >( "QVector<Akonadi::Item::Id>" );
}

IncidenceChanger2::Private::~Private()
{
  delete mHistory;
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

  return result && collection.isValid();
}

void IncidenceChanger2::Private::handleCreateJobResult( KJob *job )
{
  QString errorString;
  ResultCode resultCode = ResultCodeSuccess;

  const Change change = mChangeForJob.take( job );

  const ItemCreateJob *j = qobject_cast<const ItemCreateJob*>( job );
  const Item item = j->item();
  if ( j->error() ) {
    resultCode = ResultCodeJobError;
    errorString = j->errorString();
    kError() << errorString;
    if ( mShowDialogsOnError ) {
      KMessageBox::sorry( change.parent, i18n( "Error while trying to create calendar item. Error was: %1",
                                               errorString ) );
    }
  } else {
    if ( change.recordToHistory ) {
      mHistory->recordCreation( item, change.atomicOperationId );
    }
    resultCode = ResultCodeSuccess;
  }

  emit q->createFinished( change.changeId, item, resultCode, errorString );
}

void IncidenceChanger2::Private::handleDeleteJobResult( KJob *job )
{
  QString errorString;
  ResultCode resultCode;

  const Change change = mChangeForJob.take( job );

  const ItemDeleteJob *j = qobject_cast<const ItemDeleteJob*>( job );
  const Item::List items = j->deletedItems();

  QVector<Item::Id> itemIdList;
  foreach( const Item &item, items ) {
    itemIdList.append( item.id() );
  }

  if ( j->error() ) {
    resultCode = ResultCodeJobError;
    errorString = j->errorString();
    kError() << errorString;
    if ( mShowDialogsOnError ) {
      KMessageBox::sorry( change.parent, i18n( "Error while trying to delete calendar item. Error was: %1",
                                               errorString ) );
    }

    foreach( const Item &item, items ) {
      // Werent deleted due to error
      mDeletedItemIds.remove( item.id() );
    }
  } else {
    if ( change.recordToHistory ) {
      //TODO: check return value
      //TODO: make History support a list of items
      foreach( const Item &item, items ) {
        mHistory->recordDeletion( item, change.atomicOperationId );
      }
    }
    resultCode = ResultCodeSuccess;
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
      KMessageBox::sorry( change.parent, i18n( "Error while trying to modify calendar item. Error was: %1",
                                               errorString ) );
    }
  } else {
    if ( change.recordToHistory ) {
      mHistory->recordModification( change.originalItem, item, change.atomicOperationId );
    }

    resultCode = ResultCodeSuccess;
  }

  emit q->modifyFinished( change.changeId, item, resultCode, errorString );
}

bool IncidenceChanger2::Private::deleteAlreadyCalled( Akonadi::Item::Id id ) const
{
  return mDeletedItemIds.contains( id );
}

// Does a queued emit, with QMetaObject::invokeMethod
void IncidenceChanger2::Private::emitCreateFinished( int changeId,
                                                     const Akonadi::Item &item,
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
void IncidenceChanger2::Private::emitModifyFinished( int changeId,
                                                     const Akonadi::Item &item,
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
void IncidenceChanger2::Private::emitDeleteFinished( int changeId,
                                                     const QVector<Akonadi::Item::Id> &itemIdList,
                                                     CalendarSupport::IncidenceChanger2::ResultCode resultCode,
                                                     const QString &errorString )
{
  QMetaObject::invokeMethod( q, "deleteFinished", Qt::QueuedConnection,
                             Q_ARG( int, changeId ),
                             Q_ARG( QVector<Akonadi::Item::Id>, itemIdList ),
                             Q_ARG( CalendarSupport::IncidenceChanger2::ResultCode, resultCode ),
                             Q_ARG( QString, errorString ) );
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

  const Change change( ++d->mLatestOperationId, atomicOperationId, recordToHistory, parent );
  Collection collectionToUse;

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
                                                             dialogCode /*by-ref*/,
                                                             mimeTypes,
                                                             d->mDefaultCollection );
        if ( dialogCode != QDialog::Accepted ) {
          kDebug() << "No valid collection to use.";
          return -2;
        }

        if ( collectionToUse.isValid() || d->hasRights( collectionToUse, ChangeTypeCreate ) ) {
          kError() << "Invalid collection selected. Can't create incidence.";
          return -2;
        }
      }
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
                                 QLatin1String( "Default collection is invalid or doesn't have "
                                                "rights and DestinationPolicyNeverAsk was used."
                                                "rights = " + rights ) );
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
  ItemCreateJob *createJob = new ItemCreateJob( item, collection );

  d->mChangeForJob.insert( createJob, change );

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

  foreach( const Item &item, items ) {
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

  foreach( const Item &item, items ) {
    if ( d->deleteAlreadyCalled( item.id() ) ) {
      // IncidenceChanger::deleteIncidence() called twice, ignore this one.
      kDebug() << "Item " << item.id() << " already deleted or being deleted, skipping";
    } else {
      itemsToDelete.append( item );
    }
  }

  const Change change( ++d->mLatestOperationId, recordToHistory, atomicOperationId, parent );

  if ( itemsToDelete.isEmpty() ) {
    QVector<Akonadi::Item::Id> itemIdList;
    itemIdList.append( Item().id() );
    kDebug() << "Items already deleted or being deleted, skipping";
    // Queued emit because return must be executed first, otherwise caller won't know this workId
    d->emitDeleteFinished( change.changeId, itemIdList, ResultCodeAlreadyDeleted,
                           i18n( "That calendar item was already deleted, or currently being deleted." ) );

    return change.changeId;
  }

  ItemDeleteJob *deleteJob = new ItemDeleteJob( itemsToDelete );

  d->mChangeForJob.insert( deleteJob, change );

  foreach( const Item &item, itemsToDelete ) {
    d->mDeletedItemIds.insert( item.id() );
  }

  // QueuedConnection because of possible sync exec calls.
  connect( deleteJob, SIGNAL(result(KJob *)),
           d, SLOT(handleDeleteJobResult(KJob *)), Qt::QueuedConnection );

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

  if ( !d->hasRights( changedItem.parentCollection(), ChangeTypeModify ) ) {
    kWarning() << "Item " << changedItem.id() << " can't be deleted due to ACL restrictions";
    return -2;
  }

  Change change( ++d->mLatestOperationId, recordToHistory, atomicOperationId, parent );
  change.originalItem = originalItem;

  if ( d->deleteAlreadyCalled( changedItem.id() ) ) {
    // IncidenceChanger::deleteIncidence() called twice, ignore this one.
    kDebug() << "Item " << changedItem.id() << " already deleted or being deleted, skipping";

    // Queued emit because return must be executed first, otherwise caller won't know this workId
    d->emitModifyFinished( change.changeId, changedItem, ResultCodeAlreadyDeleted,
                           i18n( "That calendar item was already deleted, or currently being deleted." ) );

    return change.changeId;
  }

  { // increment revision
    Incidence::Ptr incidence = changedItem.payload<Incidence::Ptr>();
    const int revision = incidence->revision();
    incidence->setRevision( revision + 1 );
  }

  // Don't write back remote revision since we can't make sure it is the current one
  // fixes problems with DAV resource
  Item item = changedItem;
  item.setRemoteRevision( QString() );

  ItemModifyJob *modifyJob = new ItemModifyJob( item );
  d->mChangeForJob.insert( modifyJob, change );
  // QueuedConnection because of possible sync exec calls.
  connect( modifyJob, SIGNAL(result(KJob *)),
           d, SLOT(handleModifyJobResult(KJob *)), Qt::QueuedConnection );

  return change.changeId;
}

uint IncidenceChanger2::startAtomicOperation()
{
  static uint latestAtomicOperationId = 0;
  return ++latestAtomicOperationId;
}

void IncidenceChanger2::endAtomicOperation( uint atomicOperationId )
{
  Q_UNUSED( atomicOperationId );
  //d->mOperationStatus.remove( atomicOperationId );
}

void IncidenceChanger2::setShowDialogsOnError( bool enable )
{
  d->mShowDialogsOnError = enable;
}

bool IncidenceChanger2::showDialogsOnError() const
{
  return d->mShowDialogsOnError;
}

void IncidenceChanger2::setDestinationPolicy( IncidenceChanger2::DestinationPolicy destinationPolicy )
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
