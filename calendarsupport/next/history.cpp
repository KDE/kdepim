/*
  This file is part of CalendarSupport

  Copyright (c) 2010 Sérgio Martins <iamsergio@gmail.com>

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

#include "history.h"
#include "history_p.h"
#include "utils.h"

#include <KLocale>

using namespace KCalCore;
using namespace CalendarSupport;

History::History( IncidenceChanger2 *changer ) : QObject(),
                                                 d( new Private( this ) )
{
  Q_ASSERT( changer );

  d->mChanger = changer;
  d->mOperationTypeInProgress = TypeNone;
  d->mUndoAllInProgress = false;

  connect( d->mChanger, SIGNAL(createFinished(int,Akonadi::Item,CalendarSupport::IncidenceChanger2::ResultCode,QString)),
           d, SLOT(createFinished(int,Akonadi::Item,CalendarSupport::IncidenceChanger2::ResultCode,QString)) );

  connect( d->mChanger, SIGNAL(deleteFinished(int,QVector<Akonadi::Item::Id>,CalendarSupport::IncidenceChanger2::ResultCode,QString)),
           d, SLOT(deleteFinished(int,QVector<Akonadi::Item::Id>,CalendarSupport::IncidenceChanger2::ResultCode,QString)) );

  connect( d->mChanger,SIGNAL(modifyFinished(int,Akonadi::Item,CalendarSupport::IncidenceChanger2::ResultCode,QString)),
           d, SLOT(modifyFinished(int,Akonadi::Item,CalendarSupport::IncidenceChanger2::ResultCode,QString)) );
}

History::~History()
{
  delete d;
}

void History::recordCreation( const Akonadi::Item &item,
                              const uint atomicOperationId )
{
  Q_ASSERT_X( item.isValid() && item.hasPayload<Incidence::Ptr>(),
              "recordCreation()", "Item must be valid and have Incidence payload." );

  Entry entry;
  entry.changeType = IncidenceChanger2::ChangeTypeCreate;
  entry.newItems.append( item );
  entry.oldItems.append( Item() );
  entry.atomicOperationId = atomicOperationId;
  d->mLatestRevisionByItemId.insert( item.id(), item.revision() );

  d->mUndoStack.push( entry );
  d->mRedoStack.clear();
  //emit undoAvailable(); // nao gosto, se poder tirar.
  d->updateWidgets();
}

void History::recordModification( const Akonadi::Item &oldItem,
                                  const Akonadi::Item &newItem,
                                  const uint atomicOperationId )
{
  Q_ASSERT_X( oldItem.isValid() && oldItem.hasPayload<Incidence::Ptr>() &&
              newItem.isValid() && newItem.hasPayload<Incidence::Ptr>() &&
              oldItem.id() == newItem.id(),
              "recordChange()", "oldItem and newItem must be valid and have the same id" );
  Entry entry;
  entry.changeType = IncidenceChanger2::ChangeTypeModify;
  entry.atomicOperationId = atomicOperationId;
  entry.oldItems.append( oldItem );
  entry.newItems.append( newItem );
  d->mLatestRevisionByItemId.insert( newItem.id(), newItem.revision() );

  d->mUndoStack.push( entry );
  d->mRedoStack.clear();
  //emit undoAvailable(); // nao gosto, se poder tirar.
  d->updateWidgets();
}

void History::recordDeletion( const Akonadi::Item &item,
                              const uint atomicOperationId )
{
  Item::List list;
  list.append( item );
  recordDeletions( list, atomicOperationId );
}

void History::recordDeletions( const Akonadi::Item::List &items,
                               const uint atomicOperationId )
{
  Entry entry;
  entry.changeType = IncidenceChanger2::ChangeTypeDelete;
  entry.atomicOperationId = atomicOperationId;

  foreach( const Akonadi::Item &item, items ) {
    Q_ASSERT_X( item.isValid() && item.hasPayload<Incidence::Ptr>(),
                "recordDeletion()", "Item must be valid and have an Incidence payload." );
    //cleanup
    d->mLatestRevisionByItemId.remove( item.id() );
    entry.oldItems.append( item );
    entry.newItems.append( Item() );
  }

  d->mUndoStack.push( entry );
  d->mRedoStack.clear();
  //emit undoAvailable(); // nao gosto, se poder tirar.
  d->updateWidgets();
}


void History::registerRedoWidget( QWidget *w )
{
  if ( !d->mRedoWidgets.contains( w ) ) {
    d->mRedoWidgets.append( QPointer<QWidget>( w ) );
    w->setEnabled( d->isRedoAvailable() );
  }
}

void History::registerUndoWidget( QWidget *w )
{
  if ( !d->mUndoWidgets.contains( w ) ) {
    d->mUndoWidgets.append( QPointer<QWidget>( w ) );
    w->setEnabled( d->isUndoAvailable() );
  }
}

bool History::undo( QWidget *parent )
{
  // Don't call undo() without the previous one finishing
  Q_ASSERT( d->mOperationTypeInProgress == TypeNone );

  bool result;

  if ( !d->mUndoStack.isEmpty() ) {
    result = d->doIt( d->mUndoStack.pop(), TypeUndo, parent );
  } else {
    kWarning() << "Don't call undo when the undo stack is empty.";
    result = false;
  }

  return result;
}

bool History::redo( QWidget *parent )
{
  // Don't call redo() without the previous one finishing
  Q_ASSERT( d->mOperationTypeInProgress == TypeNone );

  bool result;

  if ( !d->mRedoStack.isEmpty() ) {
    result = d->doIt( d->mRedoStack.pop(), TypeRedo, parent );
  } else {
    kWarning() << "Don't call redo() when the undo stack is empty.";
    result = false;
  }

  return result;
}

bool History::undoAll()
{
  Q_ASSERT( d->mOperationTypeInProgress == TypeNone );
  d->mUndoAllInProgress = true;
 return undo();
}

bool History::clear()
{
  if ( d->mOperationTypeInProgress == TypeNone ) {
    d->mRedoStack.clear();
    d->mUndoStack.clear();
    d->updateWidgets();
    return true;
  } else {
    return false;
  }
}

QString History::lastErrorString() const
{
  return d->mLastErrorString;
}

bool History::Private::isUndoAvailable() const
{
  return !mUndoStack.isEmpty() && mOperationTypeInProgress == TypeNone;
}

bool History::Private::isRedoAvailable() const
{
  return !mRedoStack.isEmpty() && mOperationTypeInProgress == TypeNone;
}

void History::Private::updateWidgets()
{
  const bool undoAvailable = isUndoAvailable();
  const bool redoAvailable = isRedoAvailable();

  foreach( QPointer<QWidget> w, mUndoWidgets )
    if ( w )
      w->setEnabled( undoAvailable );

  foreach( QPointer<QWidget> w, mRedoWidgets )
    if ( w )
      w->setEnabled( redoAvailable );
}

void History::Private::updateIds( Item::Id oldId, Item::Id newId )
{
  QList<QStack<Entry>*> stacks;
  stacks << &mUndoStack << &mRedoStack;

  for ( int i = 0; i < stacks.count(); ++i ) {
    QStack<Entry>::iterator j;
    for ( j = stacks[i]->begin(); j != stacks[i]->end(); ++j ) {
      Item::List::iterator k;
      for ( k = (*j).oldItems.begin(); k != (*j).oldItems.end(); ++k ) {
        if ( (*k).id() == oldId ) {
          (*k).setId( newId );
        }
      }

      for ( k = (*j).newItems.begin(); k != (*j).newItems.end(); ++k ) {
        if ( (*k).id() == oldId ) {
          (*k).setId( newId );
        }
      }
    }
  }
}

bool History::Private::doIt( const Entry &entry, OperationType type, QWidget *parent )
{
  mOperationTypeInProgress = type;
  mEntryInProgress = entry;
  updateWidgets();

  Entry e = entry;

  if ( type == TypeUndo ) {
    // Invert stuff, that's what undo means.
    switch( e.changeType ) {
      case IncidenceChanger2::ChangeTypeCreate:
        e.changeType = IncidenceChanger2::ChangeTypeDelete;
        break;
      case IncidenceChanger2::ChangeTypeDelete:
        e.changeType = IncidenceChanger2::ChangeTypeCreate;

        if ( e.oldItems.count() > 1 && e.atomicOperationId != 0 ) {
          /** We're undoing a bulk delete.
              ItemDeleteJob supports deleting a list of items,But while undoing that,
              we can't just use one ItemCreateJob, because it's ctor only accepts one item,
              we must create an ItemCreateJob for each.

              So lets group all creations, so, in case one item fails, we can rollback.
          */
          e.atomicOperationId = mChanger->startAtomicOperation();
        }
        break;
      case IncidenceChanger2::ChangeTypeModify:
        break;
      default:
        Q_ASSERT_X( false, "doIt()", "Invalid change type" );
    }

    // Swap old item with new item.
    Akonadi::Item::List oldItems2 = e.oldItems;
    e.oldItems = e.newItems;
    e.newItems = oldItems2;
  }

  int changeId = -1;
  if ( e.changeType == IncidenceChanger2::ChangeTypeCreate ) {
    foreach( const Item &item, e.newItems ) {
      Incidence::Ptr newPayload = CalendarSupport::incidence( item );
      // TODO: don't overwrite result
      const Akonadi::Collection collection = item.parentCollection();
      changeId = mChanger->createIncidence( newPayload, collection, e.atomicOperationId,
                                            false, /* don't record to history, we're on it */
                                            parent );
      mItemIdByChangeId[changeId] = item.id();
    }
    // now wait for mChanger to call our slot.
  } else if ( e.changeType == IncidenceChanger2::ChangeTypeDelete ) {
    changeId = mChanger->deleteIncidences( e.oldItems,
                                           e.atomicOperationId,
                                           false, /* don't record to history, we're on it */
                                           parent );
    // now wait for mChanger to call our slot.
  } else if ( e.changeType == IncidenceChanger2::ChangeTypeModify ) {
    // ItemModifyJob doesn't support a bulk operation, so modify operations will only have one
    // element, therefore first().
    Item newItem = e.newItems.first();

    if ( mLatestRevisionByItemId.contains( newItem.id() ) ) {
      newItem.setRevision( mLatestRevisionByItemId[newItem.id()] );
    }

    changeId = mChanger->modifyIncidence( newItem, Item(), e.atomicOperationId,
                                          false, /* don't record to history, we're on it */
                                          parent );
    // now wait for mChanger to call our slot.
  } else {
    changeId = -1;
    Q_ASSERT_X( false, "History::Private::doIt()", "Must have at least one payload" );
  }

  mPendingChangeIds.insert( changeId );
  mEntryInProgress.changeId = changeId;

  if ( changeId == -1 ) {
    // Don't i18n yet, only after refactoring IncidenceChanger.
    mLastErrorString = "Error in incidence changer, didn't even fire the job";
    mOperationTypeInProgress = TypeNone;
    stack().push( mEntryInProgress ); // Back to the original stack
    updateWidgets();
  }

  return changeId != -1;
}

void History::Private::deleteFinished( int changeId,
                                       const QVector<Akonadi::Item::Id> &itemIdList,
                                       IncidenceChanger2::ResultCode changerResultCode,
                                       const QString &errorMessage )
{
  const bool success = ( changerResultCode == IncidenceChanger2::ResultCodeSuccess );
  const History::ResultCode resultCode = success ? History::ResultCodeSuccess :
                                                   History::ResultCodeError;

  // clean up hash
  if ( success ) {
    foreach( Akonadi::Item::Id itemId, itemIdList ) {
      mLatestRevisionByItemId.remove( itemId );
    }
  }

  finishOperation( changeId, resultCode, errorMessage );
}

void History::Private::createFinished( int changeId,
                                       const Akonadi::Item &item,
                                       IncidenceChanger2::ResultCode changerResultCode,
                                       const QString &errorMessage )
{
  const bool success = ( changerResultCode == IncidenceChanger2::ResultCodeSuccess );
  const History::ResultCode resultCode = success ? History::ResultCodeSuccess :
                                                   History::ResultCodeError;

  finishOperation( changeId, resultCode, errorMessage );

  if ( success ) {
    // TODO: add comentary
    updateIds( mItemIdByChangeId[changeId] /*old*/, item.id() /*new*/ );
    mItemIdByChangeId.remove( changeId );
    mLatestRevisionByItemId.insert( item.id(), item.revision() );
  }
}

void History::Private::modifyFinished( int changeId,
                                       const Akonadi::Item &item,
                                       IncidenceChanger2::ResultCode changerResultCode,
                                       const QString &errorMessage )
{
  Q_UNUSED( changeId );
  const bool success = ( changerResultCode == IncidenceChanger2::ResultCodeSuccess );
  const History::ResultCode resultCode = success ? History::ResultCodeSuccess :
                                                   History::ResultCodeError;

  if ( success ) {
    mLatestRevisionByItemId[item.id()] = item.revision();
  }

  finishOperation( changeId, resultCode, errorMessage );
}

// Just to share code between {add|change|delete}Finished
void History::Private::finishOperation( int changeId,
                                        History::ResultCode resultCode,
                                        const QString &errorString )
{
  if ( !mPendingChangeIds.contains( changeId ) ) {
    // IncidenceChanger was called by someother class.
    //TODO: what if someother class modifies incidences while we're undoing?
    return;
  }

  mPendingChangeIds.remove( changeId );

  if ( resultCode == ResultCodeSuccess ) {
    mLastErrorString = QString();
    destinationStack().push( mEntryInProgress );
  } else {
    mLastErrorString = errorString;
    stack().push( mEntryInProgress );
  }

  if ( mUndoAllInProgress ) {
    if ( mUndoStack.isEmpty() ) {
      // Everything undone.
      emitDone( mOperationTypeInProgress, resultCode );
      mUndoAllInProgress = false;
    } else {
      // Undo the next one.
      mOperationTypeInProgress = TypeNone;
      q->undo();
    }
  } else {
    //TODO: will need to be queued.
    emitDone ( mOperationTypeInProgress, resultCode );
  }

  mOperationTypeInProgress = TypeNone;
  updateWidgets();
}

QStack<Entry>& History::Private::stack()
{
  // Entries from the undo stack go to the redo stack, and vice-versa
  if ( mOperationTypeInProgress == TypeUndo ) {
    return mUndoStack;
  } else {
    return mRedoStack;
  }
}

QStack<Entry>& History::Private::destinationStack()
{
  // Entries from the undo stack go to the redo stack, and vice-versa
  if ( mOperationTypeInProgress == TypeUndo ) {
    return mRedoStack;
  } else {
    return mUndoStack;
  }
}

void History::Private::emitDone( OperationType type, History::ResultCode resultCode )
{
  if ( type == TypeUndo ) {
    emit q->undone( resultCode );
  } else {
    emit q->redone( resultCode );
  }
}

#include "history.moc"
#include "history_p.moc"
