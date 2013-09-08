/*
  This file is part of KOrganizer.

  Copyright (c) 2008 Thomas Thrainer <tom_t@gmx.at>

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
  51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "todoviewsortfilterproxymodel.h"
#include "todomodel.h"

#include <calendarsupport/utils.h>

#include <KLocale>

TodoViewSortFilterProxyModel::TodoViewSortFilterProxyModel( const EventViews::PrefsPtr &prefs,
                                                            QObject *parent )
  : QSortFilterProxyModel( parent )
  , mPreferences( prefs )
{
}

void TodoViewSortFilterProxyModel::sort( int column, Qt::SortOrder order )
{
  mSortOrder = order;
  QSortFilterProxyModel::sort( column, order );
}

bool TodoViewSortFilterProxyModel::filterAcceptsRow( int source_row,
                                                     const QModelIndex &source_parent ) const
{
  bool ret = QSortFilterProxyModel::filterAcceptsRow( source_row, source_parent );

  bool returnValue = true;
  if ( ret && !mPriorities.isEmpty() ) {
    QString priorityValue =
      sourceModel()->index( source_row, TodoModel::PriorityColumn, source_parent ).
      data( Qt::EditRole ).toString();
    returnValue = mPriorities.contains( priorityValue );
  }
  if ( ret && !mCategories.isEmpty() ) {
    QStringList categories =
      sourceModel()->index( source_row, TodoModel::CategoriesColumn, source_parent ).
      data( Qt::EditRole ).toStringList();

    foreach ( const QString &category, categories ) {
      if ( mCategories.contains( category ) ) {
        return returnValue && true;
      }
    }
    ret = false;
  }

  // check if one of the children is accepted, and accept this node too if so
  QModelIndex cur = sourceModel()->index( source_row, TodoModel::SummaryColumn, source_parent );
  if ( cur.isValid() ) {
    for ( int r = 0; r < cur.model()->rowCount( cur ); ++r ) {
      if ( filterAcceptsRow( r, cur ) ) {
        return true;
      }
    }
  }

  return ret && returnValue;
}

bool TodoViewSortFilterProxyModel::lessThan( const QModelIndex &left,
                                             const QModelIndex &right ) const
{
  if ( mPreferences->sortCompletedTodosSeparately() &&
       left.column() != TodoModel::PercentColumn ) {
    QModelIndex cLeft = left.sibling( left.row(), TodoModel::PercentColumn );
    QModelIndex cRight = right.sibling( right.row(), TodoModel::PercentColumn );

    if ( cRight.data( Qt::EditRole ).toInt() == 100 &&
         cLeft.data( Qt::EditRole ).toInt() != 100 ) {
      return mSortOrder == Qt::AscendingOrder ? true : false;
    } else if ( cRight.data( Qt::EditRole ).toInt() != 100 &&
                cLeft.data( Qt::EditRole ).toInt() == 100 ) {
      return mSortOrder == Qt::AscendingOrder ? false : true;
    }
  }

  // To-dos without due date should appear last when sorting ascending,
  // so you can see the most urgent tasks first. (bug #174763)
  if ( right.column() == TodoModel::DueDateColumn ) {
    const int comparison = compareDueDates( left, right );

    if ( comparison != 0 ) {
      return comparison == -1;
    } else {
      // Due dates are equal, but the user still expects sorting by importance
      // Fallback to the PriorityColumn
      QModelIndex leftPriorityIndex = left.sibling( left.row(), TodoModel::PriorityColumn );
      QModelIndex rightPriorityIndex = right.sibling( right.row(), TodoModel::PriorityColumn );
      const int fallbackComparison = comparePriorities( leftPriorityIndex, rightPriorityIndex );

      if ( fallbackComparison != 0 ) {
        return fallbackComparison == 1;
      }
    }

  } else if ( right.column() == TodoModel::StartDateColumn ) {
    return compareStartDates( left, right ) == -1;
  } else if ( right.column() == TodoModel::PriorityColumn ) {
    const int comparison = comparePriorities( left, right );

    if ( comparison != 0 ) {
      return comparison == -1;
    } else {
      // Priorities are equal, but the user still expects sorting by importance
      // Fallback to the DueDateColumn
      QModelIndex leftDueDateIndex = left.sibling( left.row(), TodoModel::DueDateColumn );
      QModelIndex rightDueDateIndex = right.sibling( right.row(), TodoModel::DueDateColumn );
      const int fallbackComparison = compareDueDates( leftDueDateIndex, rightDueDateIndex );

      if ( fallbackComparison != 0 ) {
        return fallbackComparison == 1;
      }
    }
  } else if ( right.column() == TodoModel::PercentColumn ) {
    const int comparison = compareCompletion( left, right );
    if ( comparison != 0 ) {
      return comparison == -1;
    }
  }

  if ( left.data() == right.data() ) {
    // If both are equal, lets choose an order, otherwise Qt will display them randomly.
    // Fixes to-dos jumping around when you have calendar A selected, and then check/uncheck
    // a calendar B with no to-dos. No to-do is added/removed because calendar B is empty,
    // but you see the existing to-dos switching places.
    QModelIndex leftSummaryIndex = left.sibling( left.row(), TodoModel::SummaryColumn );
    QModelIndex rightSummaryIndex = right.sibling( right.row(), TodoModel::SummaryColumn );

    // This patch is not about fallingback to the SummaryColumn for sorting.
    // It's about avoiding jumping due to random reasons.
    // That's why we ignore the sort direction...
    return mSortOrder == Qt::AscendingOrder ?
      QSortFilterProxyModel::lessThan( leftSummaryIndex, rightSummaryIndex ) :
      QSortFilterProxyModel::lessThan( rightSummaryIndex, leftSummaryIndex );

    // ...so, if you have 4 to-dos, all with CompletionColumn = "55%",
    // and click the header multiple times, nothing will happen because
    // it is already sorted by Completion.
  } else {
    return QSortFilterProxyModel::lessThan( left, right );
  }
}

void TodoViewSortFilterProxyModel::setPriorityFilter( const QStringList &priorities )
{
  // preparing priority list for comparison
  mPriorities.clear();
  foreach ( const QString &eachPriority, priorities ){
    if ( eachPriority == i18nc( "priority is unspecified", "unspecified" ) ){
      mPriorities.append( i18n( "%1", 0 ) );
    } else if ( eachPriority == i18nc( "highest priority", "%1 (highest)", 1 ) ) {
      mPriorities.append( i18n( "%1", 1 ) );
    } else if ( eachPriority == i18nc( "medium priority", "%1 (medium)", 5 ) ) {
      mPriorities.append( i18n( "%1", 5 ) );
    } else if ( eachPriority == i18nc( "lowest priority", "%1 (lowest)", 9 ) ) {
      mPriorities.append( i18n( "%1", 9 ) );
    } else {
      mPriorities.append( eachPriority );
    }
  }
  invalidateFilter();
}

int TodoViewSortFilterProxyModel::compareStartDates(const QModelIndex &left,
                                                    const QModelIndex &right) const
{
    Q_ASSERT( left.column() == TodoModel::StartDateColumn );
    Q_ASSERT( right.column() == TodoModel::StartDateColumn );

    // The due date column is a QString so fetch the akonadi item
    // We can't compare QStrings because it won't work if the format is MM/DD/YYYY
    const KCalCore::Todo::Ptr leftTodo =
      CalendarSupport::todo( left.data( TodoModel::TodoRole ).value<Akonadi::Item>() );
    const KCalCore::Todo::Ptr rightTodo =
      CalendarSupport::todo( right.data( TodoModel::TodoRole ). value<Akonadi::Item>() );

    if ( !leftTodo || !rightTodo ) {
      return false;
    }

    const bool leftIsEmpty  = !leftTodo->hasStartDate();
    const bool rightIsEmpty = !rightTodo->hasStartDate();

    if ( leftIsEmpty != rightIsEmpty ) { // One of them doesn't have a start date
      // For sorting, no date is considered a very big date
      return rightIsEmpty ? -1 : 1;
    } else if ( !leftIsEmpty ) { // Both have start dates
      const KDateTime leftDateTime = leftTodo->dtStart();
      const KDateTime rightDateTime = rightTodo->dtStart();

      if ( leftDateTime == rightDateTime ) {
        return 0;
      } else {
        return leftDateTime < rightDateTime ? -1 : 1;
      }
    } else { // Neither has a start date
      return 0;
    }
}

void TodoViewSortFilterProxyModel::setCategoryFilter( const QStringList &categories )
{
  mCategories = categories;
  invalidateFilter();
}

/* -1 - less than
 *  0 - equal
 *  1 - bigger than
 */
int TodoViewSortFilterProxyModel::compareDueDates( const QModelIndex &left,
                                                   const QModelIndex &right ) const
{
  Q_ASSERT( left.column() == TodoModel::DueDateColumn );
  Q_ASSERT( right.column() == TodoModel::DueDateColumn );

  // The due date column is a QString so fetch the akonadi item
  // We can't compare QStrings because it won't work if the format is MM/DD/YYYY
  const KCalCore::Todo::Ptr leftTodo =
    CalendarSupport::todo( left.data( TodoModel::TodoRole ).value<Akonadi::Item>() );
  const KCalCore::Todo::Ptr rightTodo =
    CalendarSupport::todo( right.data( TodoModel::TodoRole ). value<Akonadi::Item>() );

  if ( !leftTodo || !rightTodo ) {
    return false;
  }

  const bool leftIsEmpty  = !leftTodo->hasDueDate();
  const bool rightIsEmpty = !rightTodo->hasDueDate();

  if ( leftIsEmpty != rightIsEmpty ) { // One of them doesn't have a due date
    // For sorting, no date is considered a very big date
    return rightIsEmpty ? -1 : 1;
  } else if ( !leftIsEmpty ) { // Both have due dates
    const KDateTime leftDateTime = leftTodo->dtDue();
    const KDateTime rightDateTime = rightTodo->dtDue();

    if ( leftDateTime == rightDateTime ) {
      return 0;
    } else {
      return leftDateTime < rightDateTime ? -1 : 1;
    }
  } else { // Neither has a due date
    return 0;
  }
}

/* -1 - less than
 *  0 - equal
 *  1 - bigger than
 */
int TodoViewSortFilterProxyModel::compareCompletion( const QModelIndex &left,
                                                       const QModelIndex &right ) const
{
  Q_ASSERT( left.column() == TodoModel::PercentColumn );
  Q_ASSERT( right.column() == TodoModel::PercentColumn );

  const int leftValue = sourceModel()->data( left ).toInt();
  const int rightValue = sourceModel()->data( right ).toInt();

  if ( leftValue == 100 && rightValue == 100 ) {
    // Untie with the completion date
    const KCalCore::Todo::Ptr leftTodo =
      CalendarSupport::todo( left.data( TodoModel::TodoRole ).value<Akonadi::Item>() );
    const KCalCore::Todo::Ptr rightTodo =
      CalendarSupport::todo( right.data( TodoModel::TodoRole ). value<Akonadi::Item>() );

    if ( !leftTodo || !rightTodo ) {
      return 0;
    } else {
      return ( leftTodo->completed() > rightTodo->completed() ) ? -1 : 1;
    }
  } else {
    return ( leftValue < rightValue ) ? -1 : 1;
  }
}

/* -1 - less than
 *  0 - equal
 *  1 - bigger than
 */
int TodoViewSortFilterProxyModel::comparePriorities( const QModelIndex &left,
                                                       const QModelIndex &right ) const
{
  Q_ASSERT( left.column() == TodoModel::PriorityColumn );
  Q_ASSERT( right.column() == TodoModel::PriorityColumn );

  const QVariant leftValue = sourceModel()->data( left );
  const QVariant rightValue = sourceModel()->data( right );

  const bool leftIsString  = sourceModel()->data( left ).type() == QVariant::String;
  const bool rightIsString = sourceModel()->data( right ).type() == QVariant::String;

  // unspecified priority is a low priority, so, if we don't have two QVariant:Ints
  // we return true ("left is less, i.e. higher prio") if right is a string ("--").
  if ( leftIsString != rightIsString ) {
    return leftIsString ? -1 : 1;
  } else {
    const int leftPriority = leftValue.toInt();
    const int rightPriority = rightValue.toInt();

    if ( leftPriority != rightPriority ) {
      // priority '5' is bigger then priroity '6'
      return leftPriority < rightPriority ? 1 : -1;
    } else {
      return 0;
    }
  }
}
#include "todoviewsortfilterproxymodel.moc"
