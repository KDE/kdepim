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

#ifndef KRSS_SUBSCRIPTIONSMODEL_H
#define KRSS_SUBSCRIPTIONSMODEL_H

#include "krss_export.h"
#include "feedlistmodel.h"

namespace KRss {

class PersistentFeed;
class SubscriptionsModelPrivate;

class KRSS_EXPORT SubscriptionsModel : public FeedListModel
{
    Q_OBJECT

public:

    explicit SubscriptionsModel( const QString &subscriptionLabel, const TagProvider *provider,
                                 QObject *parent = 0 );
    ~SubscriptionsModel();

    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );
    Qt::ItemFlags flags( const QModelIndex &index ) const;

    QSet<PersistentFeed*> subscribed() const;
    QSet<PersistentFeed*> unsubscribed() const;

private:

    Q_DISABLE_COPY( SubscriptionsModel )
    SubscriptionsModelPrivate * const d;
};

} // namespace KRss

#endif // KRSS_SUBSCRIPTIONSMODEL_H
