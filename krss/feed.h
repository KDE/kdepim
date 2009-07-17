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

#ifndef KRSS_FEED_H
#define KRSS_FEED_H

#include "krss_export.h"
#include "tag.h"

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

class QIcon;
class QPixmap;

class KJob;

namespace KRss {

class ConstFeedVisitor;
class Resource;
class FeedCollection;
class FeedListPrivate;
class FeedVisitor;
class ItemListJob;
class StatusModifyJob;
class ItemListJobImpl;
class StatusModifyJobImpl;
class FeedPrivate;
class ItemListing;

class KRSS_EXPORT Feed : public QObject, public boost::enable_shared_from_this<Feed>
{
    Q_OBJECT
public:
    typedef qint64 Id;

    virtual ~Feed();

    virtual void accept( FeedVisitor* ) = 0;
    virtual void accept( ConstFeedVisitor* ) const = 0;

    Feed::Id id() const;
    virtual bool isVirtual() const = 0;

    virtual QString title() const;
    virtual void setTitle( const QString& title );
    virtual QString description() const;
    virtual void setDescription( const QString& description );
    virtual QIcon icon() const;
    virtual void setIcon( const QIcon& icon );
    virtual QList<TagId> tags() const;
    virtual void setTags( const QList<TagId>& tags );
    virtual void addTag( const TagId& tag );
    virtual void removeTag( const TagId& tag );
    virtual QPixmap image() const;
    virtual void setImage( const QPixmap& pixmap );

    virtual QStringList subscriptionLabels() const;
    virtual void setSubscriptionLabels( const QStringList& subscriptionLabels );
    virtual void addSubscriptionLabel( const QString& subscriptionLabel );
    virtual void removeSubscriptionLabel( const QString& subscriptionLabel );

    virtual int total() const;
    virtual int unread() const;

    virtual int fetchInterval() const;
    virtual void setFetchInterval( int minutes );

    virtual ItemListJob* itemListJob();
    virtual StatusModifyJob* statusModifyJob();

    bool isFetching() const;

public Q_SLOTS:
    virtual void fetch() const;
    virtual void abortFetch() const;

Q_SIGNALS:
    void fetchStarted( const KRss::Feed::Id& feedId );
    void fetchPercent( const KRss::Feed::Id& feedId, uint percentage );
    void fetchFinished( const KRss::Feed::Id& feedId );
    void fetchFailed( const KRss::Feed::Id& feedId, const QString &errorMessage );
    void fetchAborted( const KRss::Feed::Id& feedId );

    void changed( const KRss::Feed::Id& feedId );
    void removed( const KRss::Feed::Id& feedId );

    void totalCountChanged( const KRss::Feed::Id& feedId, int count );
    void unreadCountChanged( const KRss::Feed::Id& feedId, int count );

protected:
    explicit Feed( const FeedCollection& feedCollection, const Resource *resource, QObject *parent = 0 );

protected:
    FeedPrivate * const d;
    Q_DISABLE_COPY(Feed)

    friend class ::KRss::FeedListPrivate;
    friend class FeedModifyJobPrivate;
    friend class FeedDeleteJobPrivate;
    friend class ::KRss::FeedPrivate;
    friend class ::KRss::ItemListJobImpl;
    friend class ::KRss::StatusModifyJobImpl;
    friend class ::KRss::ItemListing;

    Q_PRIVATE_SLOT( d, void slotCollectionLoadDone( KJob *job ) )
    Q_PRIVATE_SLOT( d, void slotStatisticsFetchDone( KJob *job ) )
    Q_PRIVATE_SLOT( d, void slotFeedChanged( const KRss::Feed::Id& feedId ) )
    Q_PRIVATE_SLOT( d, void slotFeedRemoved( const KRss::Feed::Id& feedId ) )
    Q_PRIVATE_SLOT( d, void slotFetchStarted( const KRss::Feed::Id& feedId ) )
    Q_PRIVATE_SLOT( d, void slotFetchPercent( const KRss::Feed::Id& feedId, uint percentage ) )
    Q_PRIVATE_SLOT( d, void slotFetchFinished( const KRss::Feed::Id& feedId ) )
    Q_PRIVATE_SLOT( d, void slotFetchFailed( const KRss::Feed::Id& feedId,
                                             const QString &errorMessage ) )
    Q_PRIVATE_SLOT( d, void slotFetchAborted( const KRss::Feed::Id& feedId ) )
    Q_PRIVATE_SLOT( d, void slotStatisticsChanged( const KRss::Feed::Id& feedId,
                                                const Akonadi::CollectionStatistics& stats ) )
};

} // namespace KRss

#endif // KRSS_FEED_H
