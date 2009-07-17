/*
    Copyright (C) 2008    Dmitry Ivanov <vonami@gmail.com>

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

#ifndef KRSS_RESOURCE_H
#define KRSS_RESOURCE_H

#include "krss_export.h"
#include "feed.h"

#include <Akonadi/CollectionStatistics>

class KJob;

namespace Akonadi {
class Item;
class Collection;
class AgentInstance;
}

namespace KRss {

class ExportOpmlJob;
class ImportItemsJob;
class ImportOpmlJob;
class ResourcePrivate;

class KRSS_EXPORT Resource : public QObject
{
    Q_OBJECT

public:

    bool isValid() const;

    ImportItemsJob* createImportItemsJob( const QString& xmlUrl, const QString& sourceFile ) const;

    ImportOpmlJob* createImportOpmlJob( const KUrl& url ) const;
    ExportOpmlJob* createExportOpmlJob( const KUrl& url ) const;

Q_SIGNALS:

    void feedAdded( const KRss::Feed::Id& feedId );
    void feedChanged( const KRss::Feed::Id& feedId );
    void feedRemoved( const KRss::Feed::Id& feedId );

    void statisticsChanged( const KRss::Feed::Id& feedId,
                            const Akonadi::CollectionStatistics &statistics );


    void fetchStarted( const KRss::Feed::Id& feedId );
    void fetchPercent( const KRss::Feed::Id& feedId, uint percent );
    void fetchFinished( const KRss::Feed::Id& feedId );
    void fetchFailed( const KRss::Feed::Id& feedId, const QString &errorMessage );
    void fetchAborted( const KRss::Feed::Id& feedId );
    void fetchQueueStarted();
    void fetchQueueFinished();

public Q_SLOTS:

    void addFeed( const QString &xmlUrl, const QString &subscriptionLabel ) const;
    void fetchFeed( const KRss::Feed::Id& feedId ) const;
    void abortFetch( const KRss::Feed::Id& feedId ) const;

private:
    explicit Resource( const Akonadi::AgentInstance &instance, QObject *parent = 0 );
    ~Resource();

private:
    friend class ResourcePrivate;
    friend class ResourceManager;
    ResourcePrivate * const d;
    Q_DISABLE_COPY( Resource )
    Q_PRIVATE_SLOT( d, void slotCollectionAdded( const Akonadi::Collection& col,
                                                 const Akonadi::Collection& parent ) )
    Q_PRIVATE_SLOT( d, void slotCollectionChanged( const Akonadi::Collection& col ) )
    Q_PRIVATE_SLOT( d, void slotCollectionRemoved( const Akonadi::Collection& col ) )
    Q_PRIVATE_SLOT( d, void slotCollectionStatisticsChanged( Akonadi::Collection::Id id,
                                                             const Akonadi::CollectionStatistics& statistics ) )
    Q_PRIVATE_SLOT( d, void slotFetchStarted( qlonglong id ) )
    Q_PRIVATE_SLOT( d, void slotFetchPercent( qlonglong id, uint percent ) )
    Q_PRIVATE_SLOT( d, void slotFetchFinished( qlonglong id ) )
    Q_PRIVATE_SLOT( d, void slotFetchFailed( qlonglong id, const QString& errorMessage ) )
    Q_PRIVATE_SLOT( d, void slotFetchAborted( qlonglong id ) )
    Q_PRIVATE_SLOT( d, void slotRootCollectionFetchFinished( KJob* ) )
};

} //namespace KRss

#endif // KRSS_RESOURCE_H
