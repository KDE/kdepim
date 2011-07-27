/*
    Copyright (C) 2009    Dmitry Ivanov <vonami@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KRSS_RETRIEVERESOURCECOLLECTIONSJOB_H
#define KRSS_RETRIEVERESOURCECOLLECTIONSJOB_H

#include "feed.h"

#include <Akonadi/Collection>
#include <KJob>
#include <QtCore/QList>

namespace KRss {

class RetrieveResourceCollectionsJob : public KJob
{
public:
    explicit RetrieveResourceCollectionsJob( QObject* parent = 0 )
        : KJob( parent ) {}

    enum Error {
        CouldNotRetrieveCollections = KJob::UserDefinedError,
        UserDefinedError = CouldNotRetrieveCollections + 100
    };

    virtual QList<Akonadi::Collection> collections() const = 0;
};

class RetrieveNetResourceCollectionsJob : public RetrieveResourceCollectionsJob
{
    Q_OBJECT
public:
    explicit RetrieveNetResourceCollectionsJob( const QString& resourceId, QObject* parent = 0 );

    void start();
    QString errorString() const;
    QList<Akonadi::Collection> collections() const;

private Q_SLOTS:
    void doStart();
    void slotCollectionsRetrieved( KJob* job );

private:
    const QString m_resourceId;
    QList<Akonadi::Collection> m_collections;
};

} // namespace KRss

#endif // KRSS_RETRIEVERESOURCECOLLECTIONSJOB_H
