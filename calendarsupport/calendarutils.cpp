/*
  Copyright (C) 2010 Bertjan Broeksema <broeksema@kde.org>

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

// NOTE: The code of the following methods is taken from
//       kdepim/korganizer/calendarview.cpp:
//       - makeIndependent (was incidence_unsub)
//       - makeChildrenIndependent

#include "calendarutils.h"

#include "calendar.h"
#include "incidencechanger.h"
#include "utils.h"

#include <KCalCore/Incidence>

#include <KLocalizedString>
#include <KMessageBox>

using namespace Akonadi;
using namespace CalendarSupport;
using namespace KCalCore;

/// CalendarUtilsPrivate

struct MultiChange {
  Item              parent;
  QVector<Item::Id> children;
  bool              success;

  explicit MultiChange( const Item &parent = Item() )
    : parent( parent )
    , success( true )
  {}

  bool inProgress() {
    return parent.isValid() && !children.isEmpty();
  }
};

namespace CalendarSupport {

struct CalendarUtilsPrivate
{
  /// Methods
  CalendarUtilsPrivate( Calendar *calendar, CalendarUtils *qq );
  void handleChangeFinish( const Akonadi::Item &oldInc,
                           const Akonadi::Item &newInc,
                           CalendarSupport::IncidenceChanger::WhatChanged,
                           bool success );

  bool purgeCompletedSubTodos( const Akonadi::Item &todoItem, bool &allPurged );

  /// Members
  Calendar         *mCalendar;
  IncidenceChanger *mChanger;
  MultiChange       mMultiChange;

private:
  CalendarUtils * const q_ptr;
  Q_DECLARE_PUBLIC( CalendarUtils )
};

}

CalendarUtilsPrivate::CalendarUtilsPrivate( Calendar *calendar, CalendarUtils *qq )
  : mCalendar( calendar )
  , mChanger( new IncidenceChanger( calendar, qq ) )
  , q_ptr( qq )
{
  Q_Q( CalendarUtils );
  Q_ASSERT( mCalendar );

  q->connect( mChanger,
             SIGNAL( incidenceChangeFinished( Akonadi::Item,
                                              Akonadi::Item,
                                              CalendarSupport::IncidenceChanger::WhatChanged,
                                              bool ) ),
             SLOT( handleChangeFinish( Akonadi::Item,
                                       Akonadi::Item,
                                       CalendarSupport::IncidenceChanger::WhatChanged,
                                      bool ) ) );
}

void CalendarUtilsPrivate::handleChangeFinish( const Akonadi::Item &oldInc,
                                               const Akonadi::Item &newInc,
                                               CalendarSupport::IncidenceChanger::WhatChanged,
                                               bool success )
{
  Q_Q( CalendarUtils );

  if ( mMultiChange.inProgress() ) {
    mMultiChange.children.remove( mMultiChange.children.indexOf( newInc.id() ) );
    mMultiChange.success = mMultiChange.success && success;

    // Are we still in progress?
    if ( !mMultiChange.inProgress() ) {
      const Akonadi::Item parent = mMultiChange.parent;
      const bool success = mMultiChange.success;

      // Reset the multi change.
      mMultiChange = MultiChange();
      Q_ASSERT( !mMultiChange.inProgress() );

      if ( success ) {
        kDebug() << "MultiChange finished";
        emit q->actionFinished( parent );
      } else {
        kDebug() << "MultiChange failed";
        emit q->actionFailed( parent, QString() );
      }
    }
  } else {
    if ( success ) {
      kDebug() << "Change finished";
      emit q->actionFinished( newInc );
    } else {
      kDebug() << "Change failed";
      // TODO: Let incidence changer return a useful message.
      emit q->actionFailed( oldInc, QString() );
    }
  }
}

bool CalendarUtilsPrivate::purgeCompletedSubTodos( const Akonadi::Item &todoItem, bool &allPurged )
{
  const Todo::Ptr todo = CalendarSupport::todo( todoItem );
  if ( !todo ) {
    return true;
  }

  bool deleteThisTodo = true;
  Akonadi::Item::List subTodos = mCalendar->findChildren( todoItem );
  foreach( const Akonadi::Item &item,  subTodos ) {
    if ( CalendarSupport::hasTodo( item ) ) {
      deleteThisTodo &= purgeCompletedSubTodos( item, allPurged );
    }
  }

  if ( deleteThisTodo ) {
    if ( todo->isCompleted() ) {
      if ( !mChanger->deleteIncidence( todoItem, 0 ) ) {
        allPurged = false;
      }
    } else {
      deleteThisTodo = false;
    }
  } else {
    if ( todo->isCompleted() ) {
      allPurged = false;
    }
  }
  return deleteThisTodo;
}

/// CalendarUtils

CalendarUtils::CalendarUtils( Calendar *calendar, QObject *parent)
  : QObject( parent )
  , d_ptr( new CalendarUtilsPrivate( calendar, this ) )
{
  Q_ASSERT( calendar );
}

CalendarUtils::~CalendarUtils()
{
  delete d_ptr;
}

Calendar *CalendarUtils::calendar() const
{
  Q_D( const CalendarUtils );
  return d->mCalendar;
}

bool CalendarUtils::makeIndependent( const Akonadi::Item &item )
{
  Q_D( CalendarUtils );
  Q_ASSERT( item.isValid() );

  if ( d->mChanger->changeInProgress( item.id() ) ) {
    return false;
  }

  if ( d->mMultiChange.inProgress() && !d->mMultiChange.children.contains( item.id() ) ) {
    return false;
  }

  const Incidence::Ptr inc = CalendarSupport::incidence( item );
  if ( !inc || inc->relatedTo().isEmpty() ) {
    return false;
  }

  Incidence::Ptr oldInc( inc->clone() );
  inc->setRelatedTo( 0 );
  // HACK: This is not a widget, so pass 0 for now
  return d->mChanger->changeIncidence( oldInc, item, CalendarSupport::IncidenceChanger::RELATION_MODIFIED, 0 );
}

bool CalendarUtils::makeChildrenIndependent( const Akonadi::Item &item )
{
  Q_D( CalendarUtils );
  Q_ASSERT( item.isValid() );

  // Is the current item being changed atm?
  if ( d->mChanger->changeInProgress( item.id() ) ) {
    return false;
  }

  if ( d->mMultiChange.inProgress() ) {
    return false;
 }

  const Incidence::Ptr inc = CalendarSupport::incidence( item );
  const Akonadi::Item::List subIncs = d->mCalendar->findChildren( item );

  if ( !inc || subIncs.isEmpty() ) {
    return false;
  }

  // First make sure that no changes are in progress for one of the incs
  foreach ( const Item &subInc, subIncs ) {
    if ( d->mChanger->changeInProgress( subInc.id() ) )
      return false;
  }

  d->mMultiChange = MultiChange( item );
  bool allStarted = true;
  foreach( const Item &subInc, subIncs ) {
    d->mMultiChange.children.append( subInc.id() );
    allStarted = allStarted && makeIndependent( subInc );
  }

  Q_ASSERT( allStarted ); // OKay, maybe we should not assert here, but one or
                          // changes could have been started, so just returning
                          // false isn't suitable either.

  return true;
}

/// Todo specific methods.

void CalendarUtils::purgeCompletedTodos()
{
  Q_D( CalendarUtils );
  bool allDeleted = true;
//  startMultiModify( i18n( "Purging completed to-dos" ) );
  Akonadi::Item::List todos = calendar()->rawTodos();
  Akonadi::Item::List rootTodos;
  Akonadi::Item::List::ConstIterator it;
  for ( it = todos.constBegin(); it != todos.constEnd(); ++it ) {
    Todo::Ptr aTodo = CalendarSupport::todo( *it );
    if ( aTodo && aTodo->relatedTo().isEmpty() ) { // top level todo //REVIEW(AKONADI_PORT)
      rootTodos.append( *it );
    }
  }
  // now that we have a list of all root todos, check them and their children
  for ( it = rootTodos.constBegin(); it != rootTodos.constEnd(); ++it ) {
    d->purgeCompletedSubTodos( *it, allDeleted );
  }

//  endMultiModify();
  if ( !allDeleted ) {
    KMessageBox::information(
      0,
      i18nc( "@info",
             "Unable to purge to-dos with uncompleted children." ),
      i18n( "Delete To-do" ),
      "UncompletedChildrenPurgeTodos" );
  }
}

#include "calendarutils.moc"
