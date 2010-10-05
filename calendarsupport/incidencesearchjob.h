/*
    This file is part of CalendarSupport.

    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

    Copyright (c) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    Author: Sérgio Martins <sergio.martins@kdab.com>

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

#ifndef AKONADI_CALENDARSEARCHJOB_H
#define AKONADI_CALENDARSEARCHJOB_H

#include "calendarsupport_export.h"

#include <akonadi/item.h>
#include <akonadi/itemsearchjob.h>

#include <KCalCore/Incidence>

namespace CalendarSupport {

/**
 * @short Job that searches for calendar incidences in the Akonadi storage.
 *
 * This job searches for calendar incidences (events, to-dos and journals) that match given search
 * criteria and return the list of incidences.
 *
 * Examples:
 *
 * @code
 *
 * // Search all incidences with uid 1234
 * CalendarSupport::IncidenceSearchJob *job = new Akonadi::IncidenceSearchJob();
 * job->setQuery( CalendarSupport::IncidenceSearchJob::IncidenceUid, "1234", CalendarSupport::IncidenceSearchJob::ExactMatch );
 * connect( job, SIGNAL( result( KJob* ) ), this, SLOT( searchResult( KJob* ) ) );
 *
 * ...
 *
 * MyClass::searchResult( KJob *job )
 * {
 *   CalendarSupport::IncidenceSearchJob *searchJob = qobject_cast<CalendarSupport::IncidenceSearchJob*>( job );
 *   const KCalCore::Incidence::List incidences = searchJob->incidences();
 *   // do something with the incidences
 * }
 *
 * @endcode
 *
 * @code
 *
 * // Search for all existing incidences
 * CalendarSupport::IncidenceSearchJob *job = new CalendarSupport::IncidenceSearchJob();
 * connect( job, SIGNAL( result( KJob* ) ), this, SLOT( searchResult( KJob* ) ) );
 *
 * ...
 *
 * MyClass::searchResult( KJob *job )
 * {
 *   CalendarSupport::IncidenceSearchJob *searchJob = qobject_cast<CalendarSupport::IncidenceSearchJob*>( job );
 *   const KCalCore::Incidence::List incidences = searchJob->incidences();
 *   // do something with the incidences
 * }
 *
 * @endcode
 *
 * @author Tobias Koenig <tokoe@kde.org>
 * @author Sérgio Martins <iamsergio@gmail.com>
 * @since 4.6
 */
class CALENDARSUPPORT_EXPORT IncidenceSearchJob : public Akonadi::ItemSearchJob
{
  Q_OBJECT

  public:
    /**
     * Creates a new incidence search job.
     *
     * @param parent The parent object.
     */
    explicit IncidenceSearchJob( QObject *parent = 0 );

    /**
     * Destroys the incidence search job.
     */
    ~IncidenceSearchJob();

    /**
     * Describes the criteria that can be searched for.
     */
    enum Criterion
    {
      IncidenceUid   ///< The global unique identifier of the incidence. @since 4.6
    };

    /**
     * Describes the type of pattern matching that shall be used.
     *
     * @since 4.6
     */
    enum Match
    {
      ExactMatch,      ///< The result must match exactly the pattern (case sensitive).
      StartsWithMatch, ///< The result must start with the pattern (case insensitive).
      ContainsMatch    ///< The result must contain the pattern (case insensitive).
    };

    /**
     * Sets the @p criterion and @p value for the search with @p match.
     *
     * @since 4.6
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
     * Returns the incidences that matched the search criteria.
     */
    KCalCore::Incidence::List incidences() const;

  private:
    //@cond PRIVATE
    class Private;
    Private* const d;
    //@endcond
};

}

#endif
