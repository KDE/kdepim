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

namespace Akonadi {
class CollectionStatistics;
}

namespace KRss {
class FeedModifyJobPrivate;
class FeedDeleteJobPrivate;
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

    enum FetchError {
        NoError=0,
        SomeError //TODO be more specific :)
    };

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

    /**
     * returns a human-readable error message if the last fetch try resulted in an error,
     * or an empty string otherwise.
     */
    QString errorString() const;

    bool hasError() const;
    FetchError error() const;

public Q_SLOTS:
    virtual void fetch() const;
    virtual void abortFetch() const;

Q_SIGNALS:
    void fetchStarted( const KRss::Feed::Id& feedId );
    void fetchPercent( const KRss::Feed::Id& feedId, uint percentage );
    void fetchFinished( const KRss::Feed::Id& feedId );
    void fetchFailed( const KRss::Feed::Id& feedId, const QString& errorMessage );
    void fetchAborted( const KRss::Feed::Id& feedId );

    void changed( const KRss::Feed::Id& feedId );
    void removed( const KRss::Feed::Id& feedId );

    void totalCountChanged( const KRss::Feed::Id& feedId, int count );
    void unreadCountChanged( const KRss::Feed::Id& feedId, int count );

protected:
    explicit Feed( const FeedCollection& feedCollection, const boost::shared_ptr<Resource>& resource,
                   QObject *parent = 0 );

private:
    void triggerChanged();
    void triggerRemoved();
    void triggerStatisticsChanged( const Akonadi::CollectionStatistics& statistics );
    void triggerFetchStarted();
    void triggerFetchPercent( uint percentage );
    void triggerFetchFinished();
    void triggerFetchFailed( FetchError error, const QString& errorMessage );
    void triggerFetchAborted();

protected:
    FeedPrivate * const d;
    Q_DISABLE_COPY(Feed)

    friend class ::KRss::Resource; // for trigger*Something
    friend class ::KRss::FeedListPrivate;
    friend class ::KRss::FeedModifyJobPrivate;
    friend class ::KRss::FeedDeleteJobPrivate;
    friend class ::KRss::FeedPrivate;
    friend class ::KRss::ItemListJobImpl;
    friend class ::KRss::StatusModifyJobImpl;
    friend class ::KRss::ItemListing;

    Q_PRIVATE_SLOT( d, void slotCollectionLoadDone( KJob *job ) )
    Q_PRIVATE_SLOT( d, void slotStatisticsFetchDone( KJob *job ) )
};

} // namespace KRss

#endif // KRSS_FEED_H
