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
#include "utils.h"

#include <KCalCore/Incidence>
#include <Akonadi/Calendar/ETMCalendar>
#include <Akonadi/Calendar/IncidenceChanger>

#include <KLocalizedString>
#include <KMessageBox>

using namespace CalendarSupport;
using namespace KCalCore;

/// CalendarUtilsPrivate

struct MultiChange {
  Akonadi::Item              parent;
  QVector<Akonadi::Item::Id> children;
  bool              success;

  explicit MultiChange( const Akonadi::Item &parent = Akonadi::Item() )
    : parent( parent ),
      success( true )
  {}

  bool inProgress() const
  {
    return parent.isValid() && !children.isEmpty();
  }
};

namespace CalendarSupport {

class CalendarUtilsPrivate
{
  public:
    /// Methods
    CalendarUtilsPrivate( const Akonadi::ETMCalendar::Ptr &calendar, CalendarUtils *qq );
    void handleChangeFinish( int changeId,
                             const Akonadi::Item &item,
                             Akonadi::IncidenceChanger::ResultCode resultCode,
                             const QString &errorString );

    bool purgeCompletedSubTodos( const KCalCore::Todo::Ptr &todo, bool &allPurged );

    /// Members
    Akonadi::ETMCalendar::Ptr mCalendar;
    Akonadi::IncidenceChanger *mChanger;
    MultiChange       mMultiChange;

  private:
    CalendarUtils *const q_ptr;
    Q_DECLARE_PUBLIC( CalendarUtils )
};

}

CalendarUtilsPrivate::CalendarUtilsPrivate( const Akonadi::ETMCalendar::Ptr &calendar, CalendarUtils *qq )
  : mCalendar( calendar ),
    mChanger( new Akonadi::IncidenceChanger( qq ) ),
    q_ptr( qq )
{
  Q_Q( CalendarUtils );
  Q_ASSERT( mCalendar );

  q->connect( mChanger,
              SIGNAL(modifyFinished(int,Akonadi::Item,Akonadi::IncidenceChanger::ResultCode,QString)),
              SLOT(handleChangeFinish(int,Akonadi::Item,Akonadi::IncidenceChanger::ResultCode,QString)) );
}

void CalendarUtilsPrivate::handleChangeFinish( int,
                                               const Akonadi::Item &item,
                                               Akonadi::IncidenceChanger::ResultCode resultCode,
                                               const QString &errorString )
{
  Q_Q( CalendarUtils );
  const bool success = resultCode == Akonadi::IncidenceChanger::ResultCodeSuccess;
  if ( mMultiChange.inProgress() ) {
    mMultiChange.children.remove( mMultiChange.children.indexOf( item.id() ) );
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
      emit q->actionFinished( item );
    } else {
      kDebug() << "Change failed";
      emit q->actionFailed( Akonadi::Item(), errorString );
    }
  }
}

bool CalendarUtilsPrivate::purgeCompletedSubTodos( const KCalCore::Todo::Ptr &todo, bool &allPurged )
{
  if ( !todo ) {
    return true;
  }

  bool deleteThisTodo = true;
  Akonadi::Item::List subTodos = mCalendar->childItems( todo->uid() );
  foreach ( const Akonadi::Item &item, subTodos ) {
    if ( CalendarSupport::hasTodo( item ) ) {
      deleteThisTodo &= purgeCompletedSubTodos( item.payload<KCalCore::Todo::Ptr>(), allPurged );
    }
  }

  if ( deleteThisTodo ) {
    if ( todo->isCompleted() ) {
      if ( !mChanger->deleteIncidence( mCalendar->item( todo ), 0 ) ) {
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

CalendarUtils::CalendarUtils( const Akonadi::ETMCalendar::Ptr &calendar, QObject *parent )
  : QObject( parent ),
    d_ptr( new CalendarUtilsPrivate( calendar, this ) )
{
  Q_ASSERT( calendar );
}

CalendarUtils::~CalendarUtils()
{
  delete d_ptr;
}

Akonadi::ETMCalendar::Ptr CalendarUtils::calendar() const
{
  Q_D( const CalendarUtils );
  return d->mCalendar;
}

bool CalendarUtils::makeIndependent( const Akonadi::Item &item )
{
  Q_D( CalendarUtils );
  Q_ASSERT( item.isValid() );

  if ( d->mMultiChange.inProgress() && !d->mMultiChange.children.contains( item.id() ) ) {
    return false;
  }

  const Incidence::Ptr inc = CalendarSupport::incidence( item );
  if ( !inc || inc->relatedTo().isEmpty() ) {
    return false;
  }

  Incidence::Ptr oldInc( inc->clone() );
  inc->setRelatedTo( QString() );
  return d->mChanger->modifyIncidence( item, oldInc );
}

bool CalendarUtils::makeChildrenIndependent( const Akonadi::Item &item )
{
  Q_D( CalendarUtils );
  Q_ASSERT( item.isValid() );


  if ( d->mMultiChange.inProgress() ) {
    return false;
  }

  const Incidence::Ptr inc = CalendarSupport::incidence( item );
  const Akonadi::Item::List subIncs = d->mCalendar->childItems( item.id() );

  if ( !inc || subIncs.isEmpty() ) {
    return false;
  }

  d->mMultiChange = MultiChange( item );
  bool allStarted = true;
  foreach ( const Akonadi::Item &subInc, subIncs ) {
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
  KCalCore::Todo::List todos = calendar()->rawTodos();
  KCalCore::Todo::List rootTodos;

  foreach ( const KCalCore::Todo::Ptr &todo, todos ) {
    if ( todo && todo->relatedTo().isEmpty() ) { // top level todo //REVIEW(AKONADI_PORT)
      rootTodos.append( todo );
    }
  }

  // now that we have a list of all root todos, check them and their children
  foreach ( const KCalCore::Todo::Ptr &todo, rootTodos ) {
    d->purgeCompletedSubTodos( todo, allDeleted );
  }

//  endMultiModify();
  if ( !allDeleted ) {
    KMessageBox::information(
      0,
      i18nc( "@info",
             "Unable to purge to-dos with uncompleted children." ),
      i18nc( "@title:window", "Delete To-do" ),
      QLatin1String("UncompletedChildrenPurgeTodos") );
  }
}

#include "moc_calendarutils.cpp"
