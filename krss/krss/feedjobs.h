/*
    Copyright (C) 2008, 2009    Dmitry Ivanov <vonami@gmail.com>

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

#ifndef KRSS_FEEDJOBS_H
#define KRSS_FEEDJOBS_H

#include "krss_export.h"
#include <KJob>

namespace boost {
template <typename T> class shared_ptr;
}

namespace KRss {

class Feed;
class FeedModifyJobPrivate;
class FeedDeleteJobPrivate;

class KRSS_EXPORT FeedModifyJob : public KJob
{
    Q_OBJECT
public:
    enum Error {
        CouldNotModifyFeed = KJob::UserDefinedError,
        CouldNotLoadOriginalFeed,
        CouldNotRetrieveTagProvider,
        CouldNotModifyReferences
    };

public:
    explicit FeedModifyJob( const boost::shared_ptr<const Feed>& feed, QObject *parent = 0 );
    ~FeedModifyJob();

    void start();
    QString errorString() const;

private:
    FeedModifyJobPrivate * const d;
    Q_DISABLE_COPY( FeedModifyJob )

    friend class FeedModifyJobPrivate;

    Q_PRIVATE_SLOT( d, void doStart() )
    Q_PRIVATE_SLOT( d, void slotOriginalFeedLoaded( KJob *job ) )
    Q_PRIVATE_SLOT( d, void slotFeedModified( KJob *job ) )
    Q_PRIVATE_SLOT( d, void slotTagProviderRetrieved( KJob *job ) )
    Q_PRIVATE_SLOT( d, void slotReferencesModified( KJob * job ) )
};

class KRSS_EXPORT FeedDeleteJob : public KJob
{
    Q_OBJECT
public:
    enum Error {
        CouldNotDeleteFeed = KJob::UserDefinedError,
        CouldNotRetrieveTagProvider,
        CouldNotDeleteReferences
    };

public:
    explicit FeedDeleteJob( const boost::shared_ptr<const Feed>& feed, QObject *parent = 0 );
    ~FeedDeleteJob();

    void start();
    QString errorString() const;

private:
    FeedDeleteJobPrivate * const d;

    friend class FeedDeleteJobPrivate;

    Q_DISABLE_COPY( FeedDeleteJob )
    Q_PRIVATE_SLOT( d, void doStart() )
    Q_PRIVATE_SLOT( d, void slotTagProviderRetrieved( KJob *job ) )
    Q_PRIVATE_SLOT( d, void slotReferencesDeleted( KJob *job ) )
    Q_PRIVATE_SLOT( d, void slotFeedDeleted( KJob *job ) )
};

} // namespace KRss

#endif // KRSS_FEEDJOBS_H
