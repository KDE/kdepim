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

#ifndef KRSS_NETFEEDCREATEJOB_H
#define KRSS_NETFEEDCREATEJOB_H

#include "krss_export.h"
#include "feed.h"

#include <KJob>

#include <QVariantMap>

namespace boost {
template <typename T> class weak_ptr;
}

namespace KRss {

class FeedList;
class NetResource;
class NetFeedCreateJobPrivate;

class KRSS_EXPORT NetFeedCreateJob : public KJob
{
    Q_OBJECT
    friend class ::KRss::NetResource;
    friend class ::KRss::NetFeedCreateJobPrivate;

public:
    enum Error {
        CouldNotCreateFeed = KJob::UserDefinedError,
        CouldNotLoadFeed,
        UserDefinedError = CouldNotCreateFeed + 100
    };

    ~NetFeedCreateJob();

    /** If \a feedList is set when the job will ensure that the actual feed object
     *  is available in \a feedList after creation (which you can get later by its id).
     *  Otherwise you have to listen to FeedList::feedAdded( const KRss::Feed::Id& id ).
     */
    void setFeedList( const boost::weak_ptr<FeedList>& feedList );
    void setSubscriptionLabel( const QString& subscriptionLabel );

    void start();
    QString errorString() const;
    Feed::Id feedId() const;

private:
    NetFeedCreateJob( const QString& xmlUrl, const boost::weak_ptr<NetResource>& resource,
                      QObject* parent = 0 );

private:
    NetFeedCreateJobPrivate* const d;
    Q_PRIVATE_SLOT( d, void doStart() )
    Q_PRIVATE_SLOT( d, void slotCallFinished( const QVariantMap& ) )
    Q_PRIVATE_SLOT( d, void slotCollectionLoaded( KJob* job ) )
};

} // namespace KRss

#endif // KRSS_NETFEEDCREATEJOB_H
