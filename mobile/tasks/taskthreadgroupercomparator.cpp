/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Tobias Koenig <tokoe@kdab.com>

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

#include "taskthreadgroupercomparator.h"

#include "settings.h"

#include <calendarsupport/utils.h>
#include <KCalCore/todo.h>

#include <QDebug>

TaskThreadGrouperComparator::TaskThreadGrouperComparator()
{
}

TaskThreadGrouperComparator::~TaskThreadGrouperComparator()
{
}

QByteArray TaskThreadGrouperComparator::identifierForItem( const Akonadi::Item &item ) const
{
  Q_ASSERT( item.hasPayload<KCalCore::Todo::Ptr>() );

  const KCalCore::Todo::Ptr todo = item.payload<KCalCore::Todo::Ptr>();

  QByteArray identifier = todo->uid().toLatin1();

  if ( identifier.isEmpty() )
    identifier = QByteArray::number( item.id() );

  return identifier;
}

QByteArray TaskThreadGrouperComparator::parentIdentifierForItem( const Akonadi::Item &item ) const
{
  Q_ASSERT( item.hasPayload<KCalCore::Todo::Ptr>() );

  const KCalCore::Todo::Ptr todo = item.payload<KCalCore::Todo::Ptr>();

  return todo->relatedTo( KCalCore::Todo::RelTypeParent ).toLatin1();
}

bool TaskThreadGrouperComparator::lessThan( const Akonadi::Item &leftItem, const Akonadi::Item &rightItem ) const
{
  Q_ASSERT( leftItem.isValid() );
  Q_ASSERT( rightItem.isValid() );

  const Akonadi::Item leftThreadRootItem = threadItem( leftItem );
  const Akonadi::Item rightThreadRootItem = threadItem( rightItem );

  Q_ASSERT( rightThreadRootItem.isValid() );
  Q_ASSERT( leftThreadRootItem.isValid() );

  const bool leftItemIsThreadLeader = (leftThreadRootItem == leftItem);
  const bool rightItemIsThreadLeader = (rightThreadRootItem == rightItem);

  if ( leftItemIsThreadLeader && rightItemIsThreadLeader ) {
    const KCalCore::Todo::Ptr leftTodo = CalendarSupport::todo( leftThreadRootItem );
    const KCalCore::Todo::Ptr rightTodo = CalendarSupport::todo( rightThreadRootItem );

    if ( !leftTodo || !rightTodo ) {
      qDebug() << "This shouldn't happen, but i didn't check. Better safe than sorry.";
      return false;
    }

    const bool leftCompleted = leftTodo->isCompleted();
    const bool rightCompleted = rightTodo->isCompleted();
    const int leftPriority = leftTodo->priority();
    const int rightPriority = rightTodo->priority();

    if ( Settings::self()->showCompletedTodosAtBottom() && leftCompleted != rightCompleted ) {
      return rightCompleted;
    }

    if ( leftPriority != rightPriority ) {
      // higher priority first. ( Also note that 9 is low, and 1 is high )
      return leftPriority < rightPriority;
    } else {
      // lower id first
      return leftItem.id() < rightItem.id();
    }

    return leftThreadRootItem.id() < rightThreadRootItem.id();
  } else if ( leftItemIsThreadLeader && !rightItemIsThreadLeader ) {
    if ( leftThreadRootItem == rightThreadRootItem )
      return true; // right item is in thread of left thread leader -> right item located below left item
    else
      return lessThan( leftThreadRootItem, rightThreadRootItem ); // based on thread leaders order
  } else if ( !leftItemIsThreadLeader && rightItemIsThreadLeader ) {
    if ( leftThreadRootItem == rightThreadRootItem )
      return false; // left item is in thread of right thread leader -> left item must be located below right item
    else
      return lessThan( leftThreadRootItem, rightThreadRootItem ); // based on thread leaders order
  } else if ( !leftItemIsThreadLeader && !rightItemIsThreadLeader ) {
    if ( leftThreadRootItem == rightThreadRootItem ) // both in the same thread
      return leftItem.id() < rightItem.id(); // default
    else
      return lessThan( leftThreadRootItem, rightThreadRootItem ); // based on thread leaders order
  }

  return leftItem.id() < rightItem.id(); // default
}

