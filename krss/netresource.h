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

#ifndef KRSS_NETRESOURCE_H
#define KRSS_NETRESOURCE_H

#include "krss_export.h"
#include "resource.h"

class KJob;

namespace Akonadi {
class Collection;
class CollectionStatistics;
}

namespace KRss {

class ExportOpmlJob;
class ImportItemsJob;
class ImportOpmlJob;
class NetFeedCreateJob;
class RetrieveResourceCollectionsJob;
class FeedList;
class NetResourcePrivate;

class KRSS_EXPORT NetResource : public Resource, public boost::enable_shared_from_this<NetResource>
{
    Q_OBJECT
public:
    ImportItemsJob* createImportItemsJob( const QString& xmlUrl, const QString& sourceFile ) const;
    ImportOpmlJob* createImportOpmlJob( const KUrl& url ) const;
    ExportOpmlJob* createExportOpmlJob( const KUrl& url ) const;
    NetFeedCreateJob* netFeedCreateJob( const QString& xmlUrl, const QString& subscriptionLabel = QString() );

public Q_SLOTS:
    void addFeed( const QString& xmlUrl, const QString& subscriptionLabel ) const;
    void fetchFeed( const KRss::Feed::Id& feedId ) const;
    void abortFetch( const KRss::Feed::Id& feedId ) const;

protected:
    NetResource( const QString& resourceId, const QString& name, QObject* parent = 0 );

private:
    RetrieveResourceCollectionsJob* retrieveResourceCollectionsJob() const;

private:
    friend class ResourceManager;
    Q_DECLARE_PRIVATE( NetResource )
    Q_DISABLE_COPY( NetResource )
    Q_PRIVATE_SLOT( d_func(), void slotCollectionAdded( const Akonadi::Collection& col,
                                                        const Akonadi::Collection& parent ) )
    Q_PRIVATE_SLOT( d_func(), void slotCollectionChanged( const Akonadi::Collection& col ) )
    Q_PRIVATE_SLOT( d_func(), void slotCollectionRemoved( const Akonadi::Collection& col ) )
    Q_PRIVATE_SLOT( d_func(), void slotCollectionStatisticsChanged( Akonadi::Collection::Id id,
                                                            const Akonadi::CollectionStatistics& statistics ) )
    Q_PRIVATE_SLOT( d_func(), void slotFetchStarted( qlonglong id ) )
    Q_PRIVATE_SLOT( d_func(), void slotFetchPercent( qlonglong id, uint percent ) )
    Q_PRIVATE_SLOT( d_func(), void slotFetchFinished( qlonglong id ) )
    Q_PRIVATE_SLOT( d_func(), void slotFetchFailed( qlonglong id, const QString& errorMessage ) )
    Q_PRIVATE_SLOT( d_func(), void slotFetchAborted( qlonglong id ) )
    Q_PRIVATE_SLOT( d_func(), void slotRootCollectionRetrieved( KJob* ) )
};

} // namespace KRss

#endif // KRSS_NETRESOURCE_H
