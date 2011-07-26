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

#include "subscriptionsmodel.h"
#include "persistentfeed.h"

#include <KDebug>

#include <QtCore/QStringList>
#include <QtCore/QHashIterator>

using namespace boost;
using namespace KRss;

namespace KRss {

class SubscriptionsModelPrivate {

public:

    explicit SubscriptionsModelPrivate( const QString &subscriptionLabel )
        : m_subscriptionLabel( subscriptionLabel )
    {
    }

    void dump();

public:

    const QString m_subscriptionLabel;
    QHash<PersistentFeed*, bool> m_changes;
};

} // namespace KRss

void SubscriptionsModelPrivate::dump()
{
    QHashIterator<PersistentFeed*, bool> it_change( m_changes );
    while ( it_change.hasNext() ) {
        it_change.next();
        kDebug() << "Feed:" << it_change.key()->id() << ", status:" << it_change.value();
    }
}

SubscriptionsModel::SubscriptionsModel( const QString &subscriptionLabel, const TagProvider *provider,
                                        QObject *parent )
    : FeedListModel( shared_ptr<const FeedList>(), provider, parent ),
      d( new SubscriptionsModelPrivate( subscriptionLabel ) )
{
}

SubscriptionsModel::~SubscriptionsModel()
{
    delete d;
}

QVariant SubscriptionsModel::data( const QModelIndex &index, int role ) const
{
    if ( !index.isValid() )
        return QVariant();

    if ( role == Qt::CheckStateRole && FeedListModel::data( index, FeedListModel::TierRole ) == FeedListModel::FeedTier ) {
        PersistentFeed *feed = static_cast<PersistentFeed *> ( FeedListModel::data( index, FeedListModel::FeedRole).value<Feed*>() );

        // if the feed subscription status has changed, retrieve it from m_changes
        if ( d->m_changes.contains( feed ) ) {
            return ( d->m_changes.value( feed ) ? Qt::Checked : Qt::Unchecked );
        }

        // otherwise, retriever it from the feed
        return ( feed->subscriptionLabels().contains( d->m_subscriptionLabel ) ? Qt::Checked : Qt::Unchecked );
    }

    return FeedListModel::data( index, role );
}

bool SubscriptionsModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
    if ( !index.isValid() )
        return false;

    if ( role == Qt::CheckStateRole && FeedListModel::data( index, FeedListModel::TierRole ) == FeedListModel::FeedTier ) {
        PersistentFeed *feed = qobject_cast<PersistentFeed *> ( FeedListModel::data( index, FeedListModel::FeedRole).value<Feed*>() );

        // if the feed subscription status has changed, revert the change
        // otherwise, put the new subscription status to m_changes
        if ( d->m_changes.contains( feed ) )
            d->m_changes.remove( feed );
        else
            d->m_changes[ feed ] = value.toBool();

        // update all occurrences of this feed
        const QModelIndexList allIndexes = feedIndexes( feed );
        Q_FOREACH( const QModelIndex &index, allIndexes ) {
            emit dataChanged( index, index );
        }
        return true;
    }

    return FeedListModel::setData( index, value, role );
}

Qt::ItemFlags SubscriptionsModel::flags( const QModelIndex &index ) const
{
    Qt::ItemFlags flags = FeedListModel::flags( index );
    if ( FeedListModel::data( index, FeedListModel::TierRole ) == FeedListModel::FeedTier )
        flags = flags | Qt::ItemIsUserCheckable;

    return flags;
}

QSet<PersistentFeed*> SubscriptionsModel::subscribed() const
{
    QSet<PersistentFeed*> subs;
    QHashIterator<PersistentFeed*, bool> it_change( d->m_changes );
    while ( it_change.hasNext() ) {
        it_change.next();
        if ( it_change.value() )
            subs.insert( it_change.key() );
    }

    return subs;
}

QSet<PersistentFeed*> SubscriptionsModel::unsubscribed() const
{
    QSet<PersistentFeed*> unsubs;
    QHashIterator<PersistentFeed*, bool> it_change( d->m_changes );
    while ( it_change.hasNext() ) {
        it_change.next();
        if ( !it_change.value() )
            unsubs.insert( it_change.key() );
    }

    return unsubs;
}

#include "subscriptionsmodel.moc"
