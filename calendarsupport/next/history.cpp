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

  connect( d->mChanger, SIGNAL(createFinished(int,Akonadi::Item,Akonadi::Collection,CalendarSupport::IncidenceChanger2::ResultCode,QString)),
           d, SLOT(createFinished(int,Akonadi::Item,Akonadi::Collection,CalendarSupport::IncidenceChanger2::ResultCode,QString)) );

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
  entry.itemId = item.id();
  entry.changeType = IncidenceChanger2::ChangeTypeCreate;
  entry.newItem = item;
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
  entry.itemId = newItem.id();
  entry.changeType = IncidenceChanger2::ChangeTypeModify;
  entry.atomicOperationId = atomicOperationId;
  entry.oldItem = oldItem;
  entry.newItem = newItem;
  d->mLatestRevisionByItemId.insert( newItem.id(), newItem.revision() );

  d->mUndoStack.push( entry );
  d->mRedoStack.clear();
  //emit undoAvailable(); // nao gosto, se poder tirar.
  d->updateWidgets();
}

void History::recordDeletion( const Akonadi::Item &item,
                              const uint atomicOperationId )
{
  Q_ASSERT_X( item.isValid() && item.hasPayload<Incidence::Ptr>(),
              "recordDeletion()", "Item must be valid and have an Incidence payload." );

  Entry entry;
  entry.itemId = item.id();
  entry.oldItem = item;
  entry.changeType = IncidenceChanger2::ChangeTypeDelete;
  entry.atomicOperationId = atomicOperationId;
  d->mLatestRevisionByItemId.remove( item.id() );

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
  for ( int i = 0; i < mUndoStack.count(); ++i ) {
    if ( mUndoStack[i].itemId == oldId ) {

      mUndoStack[i].itemId = newId;

      if ( mUndoStack[i].oldItem.isValid() )
        mUndoStack[i].oldItem.setId( newId );

      if ( mUndoStack[i].newItem.isValid() )
        mUndoStack[i].newItem.setId( newId );
    }
  }

  for ( int i = 0; i < mRedoStack.count(); ++i ) {
    if ( mRedoStack[i].itemId == oldId ) {

      mRedoStack[i].itemId = newId;

      if ( mRedoStack[i].oldItem.isValid() )
        mRedoStack[i].oldItem.setId( newId );

      if ( mRedoStack[i].newItem.isValid() )
        mRedoStack[i].newItem.setId( newId );
    }
  }

  if ( mEntryInProgress.oldItem.isValid() )
    mEntryInProgress.oldItem.setId( newId );

  if ( mEntryInProgress.newItem.isValid() )
    mEntryInProgress.newItem.setId( newId );

  mEntryInProgress.itemId = newId;
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
        break;
      case IncidenceChanger2::ChangeTypeModify:
        break;
      default:
        Q_ASSERT_X( false, "doIt()", "Invalid change type" );
    }

    // Swap old item with new item.
    Akonadi::Item oldItem2 = e.oldItem;
    e.oldItem = e.newItem;
    e.newItem = oldItem2;
  }

  Incidence::Ptr oldPayload = CalendarSupport::incidence( e.oldItem );
  Incidence::Ptr newPayload = CalendarSupport::incidence( e.newItem );

  bool result;
  if ( e.changeType == IncidenceChanger2::ChangeTypeCreate ) {
    Akonadi::Collection collection = e.newItem.parentCollection();
    result = mChanger->createIncidence( newPayload, collection, e.atomicOperationId,
                                        false, /* don't record to history, we're on it */
                                        parent );
    // now wait for mChanger to call our slot.
  } else if ( e.changeType == IncidenceChanger2::ChangeTypeDelete ) {
    Akonadi::Item item = e.oldItem;
    result = mChanger->deleteIncidence( item, e.atomicOperationId,
                                        false, /* don't record to history, we're on it */
                                        parent );
    // now wait for mChanger to call our slot.
  } else if ( e.changeType == IncidenceChanger2::ChangeTypeModify ) {
    if ( mLatestRevisionByItemId.contains( e.itemId ) ) {
      e.newItem.setRevision( mLatestRevisionByItemId[e.itemId] );
    }

    result = mChanger->modifyIncidence( e.newItem, e.oldItem, e.atomicOperationId,
                                        false, /* don't record to history, we're on it */
                                        parent );
    // now wait for mChanger to call our slot.
  } else {
    result = false;
    Q_ASSERT_X( false, "History::Private::doIt()", "Must have at least one payload" );
  }

  if ( !result ) {
    // Don't i18n yet, only after refactoring IncidenceChanger.
    mLastErrorString = "Error in incidence changer, didn't even fire the job";
    mOperationTypeInProgress = TypeNone;
    stack().push( mEntryInProgress ); // Back to the original stack
    updateWidgets();
  }

  return result;
}

void History::Private::deleteFinished( int changeId,
                                       const QVector<Akonadi::Item::Id> &itemIdList,
                                       IncidenceChanger2::ResultCode changerResultCode,
                                       const QString &errorMessage )
{
  Q_UNUSED( changeId );
  const bool success = ( changerResultCode == IncidenceChanger2::ResultCodeSuccess );
  const History::ResultCode resultCode = success ? History::ResultCodeSuccess :
                                                   History::ResultCodeError;

  // clean up hash
  if ( success ) {
    foreach( Akonadi::Item::Id itemId, itemIdList ) {
      mLatestRevisionByItemId.remove( itemId );
    }
  }

  finishOperation( resultCode, errorMessage );
}

void History::Private::createFinished( int changeId,
                                       const Akonadi::Item &item,
                                       const Collection &usedCollection,
                                       IncidenceChanger2::ResultCode changerResultCode,
                                       const QString &errorMessage )
{
  Q_UNUSED( changeId );
  Q_UNUSED( usedCollection );
  const bool success = ( changerResultCode == IncidenceChanger2::ResultCodeSuccess );
  const History::ResultCode resultCode = success ? History::ResultCodeSuccess :
                                                   History::ResultCodeError;

  if ( success ) {
    // Por comentário.
    updateIds( mEntryInProgress.itemId /*old*/, item.id() /*new*/ );
    mLatestRevisionByItemId.insert( item.id(), item.revision() );
  }

  finishOperation( resultCode, errorMessage );
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

  finishOperation( resultCode, errorMessage );
}

// Just to share code between {add|change|delete}Finished
void History::Private::finishOperation( History::ResultCode resultCode, const QString &errorString )
{
  if ( resultCode == ResultCodeSuccess ) {
    mLastErrorString = QString();
    destinationStack().push( mEntryInProgress );
  } else {
    mLastErrorString = errorString;
    stack().push( mEntryInProgress );
  }

  emitDone( mOperationTypeInProgress, resultCode );

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
