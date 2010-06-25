/*
    This file is part of Akonadi Contact.

    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

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

#ifndef KPIM_CONTACTSEARCHJOB_P_H
#define KPIM_CONTACTSEARCHJOB_P_H

#include <akonadi/item.h>
#include <akonadi/itemsearchjob.h>
#include <kabc/addressee.h>

namespace KPIM {

class ContactSearchJob : public Akonadi::ItemSearchJob
{
  Q_OBJECT

  public:
    /**
     * Creates a new contact search job.
     *
     * @param parent The parent object.
     */
    explicit ContactSearchJob( QObject *parent = 0 );

    /**
     * Destroys the contact search job.
     */
    ~ContactSearchJob();

    /**
     * Describes the criteria that can be searched for.
     */
    enum Criterion
    {
      Name,       ///< The name of the contact.
      Email,      ///< The email address of the contact.
      NickName,   ///< The nickname of the contact.
      NameOrEmail, ///< The name or email address of the contact.
    };

    /**
     * Describes the type of pattern matching that shall be used.
     */
    enum Match
    {
      ExactMatch,      ///< The result must match exactly the pattern (case sensitive).
      StartsWithMatch, ///< The result must start with the pattern (case insensitive).
      ContainsMatch    ///< The result must contain the pattern (case insensitive).
    };

    /**
     * Sets the @p criterion and @p value for the search.
     *
     * @note ExactMatch is used for the matching.
     * @todo Merge with the method below in KDE5
     */
    void setQuery( Criterion criterion, const QString &value );

    /**
     * Sets the @p criterion and @p value for the search with @p match.
     */
    void setQuery( Criterion criterion, const QString &value, Match match );

    /**
     * Sets a @p limit on how many results will be returned by this search job.
     * This is useful in situation where for example only the first search result is needed anyway,
     * setting a limit of 1 here will greatly reduce the resource usage of Nepomuk during the
     * search.
     *
     * This needs to be called before calling setQuery() to have an effect.
     * By default, the number of results is unlimited.
     */
    void setLimit( int limit );

    /**
     * Returns the contacts that matched the search criteria.
     */
    KABC::Addressee::List contacts() const;

  private:
    //@cond PRIVATE
    class Private;
    Private* const d;
    //@endcond
};

}

#endif
