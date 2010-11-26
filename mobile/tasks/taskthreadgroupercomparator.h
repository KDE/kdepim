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

#ifndef TASKTHREADGROUPERCOMPARATOR_H
#define TASKTHREADGROUPERCOMPARATOR_H

#include "threadgroupermodel.h"

class TaskThreadGrouperComparator : public ThreadGrouperComparator
{
  public:
    /**
     * Creates a new task thread grouper comparator.
     */
    TaskThreadGrouperComparator();

    /**
     * Destroys the task thread grouper comparator.
     */
    ~TaskThreadGrouperComparator();

    /**
     * Returns the unique identifier for the given task @p item.
     */
    QByteArray identifierForItem( const Akonadi::Item &item ) const;

    /**
     * Returns the parent identifier for the given task @p item.
     */
    QByteArray parentIdentifierForItem( const Akonadi::Item &item ) const;

    /**
     * Returns if the @p left task item is smaller than the @p right task item.
     */
    bool lessThan( const Akonadi::Item &left, const Akonadi::Item &right ) const;
};

#endif
