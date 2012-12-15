/*
  Copyright (C) 2011 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    Author: Volker Krause <vkrause@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#ifndef CALENDARSUPPORT_INCIDENCEFETCHJOB_H
#define CALENDARSUPPORT_INCIDENCEFETCHJOB_H

#include "calendarsupport_export.h"

#include <Akonadi/Item>
#include <Akonadi/Job>
#include <Akonadi/MimeTypeChecker>

#include <KCalCore/Incidence>

namespace CalendarSupport {

/**
 * Retrieve all incidences in all calendars.
 * This is a Strigi/Nepomuk-free replacement for an IncidenceSearchJob without a query.
 * @internal
 */
class CALENDARSUPPORT_EXPORT IncidenceFetchJob : public Akonadi::Job
{
  Q_OBJECT
  public:
    explicit IncidenceFetchJob( QObject *parent = 0 );
    Akonadi::Item::List items() const;

  protected:
    void doStart();

  private slots:
    void collectionFetchResult( KJob *job );
    void itemFetchResult( KJob *job );

  private:
    Akonadi::Item::List m_items;
    Akonadi::MimeTypeChecker m_mimeTypeChecker;
    int m_jobCount;
};

}

#endif
