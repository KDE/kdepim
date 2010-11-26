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

#ifndef MAILTHREADGROUPERCOMPARATOR_H
#define MAILTHREADGROUPERCOMPARATOR_H

#include "threadgroupermodel.h"

#include <kmime/kmime_message.h>

class MailThreadGrouperComparator : public ThreadGrouperComparator
{
  public:
    enum SortingOption
    {
      SortByDateTime,
      SortByDateTimeMostRecent,
      SortBySenderReceiver,
      SortBySubject,
      SortBySize
    };

    /**
     * Creates a new mail thread grouper comparator.
     */
    MailThreadGrouperComparator();

    /**
     * Destroys the mail thread grouper comparator.
     */
    ~MailThreadGrouperComparator();

    /**
     * Returns the unique identifier for the given message @p item.
     */
    QByteArray identifierForItem( const Akonadi::Item &item ) const;

    /**
     * Returns the parent identifier for the given message @p item.
     */
    QByteArray parentIdentifierForItem( const Akonadi::Item &item ) const;

    /**
     * Returns if the @p left message item is smaller than the @p right message item.
     */
    bool lessThan( const Akonadi::Item &left, const Akonadi::Item &right ) const;

    void setSortingOption( SortingOption option );
    SortingOption sortingOption() const;

    /**
     * Sets whether the currently compared items come from an outbound mail collection
     * (e.g. outbox, sent or drafts).
     */
    void setIsOutboundCollection( bool outbound );

  private:
    QByteArray identifierForMessage( const KMime::Message::Ptr&, Akonadi::Item::Id ) const;
    KDateTime mostRecentUpdate( const KMime::Message::Ptr&, Akonadi::Item::Id ) const;
    SortingOption mSortingOption;
    bool mIsOutboundCollection;
};

#endif
