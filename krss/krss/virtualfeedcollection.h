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

#ifndef KRSS_VIRTUALFEEDCOLLECTION_H
#define KRSS_VIRTUALFEEDCOLLECTION_H

#include "krss_export.h"
#include "feed.h"
#include "tag.h"

#include <akonadi/collection.h>

namespace KRss {

class KRSS_EXPORT VirtualFeedCollection : public Akonadi::Collection
{
public:
    static Feed::Id feedIdFromAkonadi( const Akonadi::Collection::Id& id );
    static Akonadi::Collection::Id feedIdToAkonadi( const Feed::Id& feedId );

    VirtualFeedCollection();
    VirtualFeedCollection( const Akonadi::Collection& collection );

    Feed::Id feedId() const;
    QString title() const;
    void setTitle( const QString& title );
    QString description() const;
    void setDescription( const QString& description );
    QList<TagId> tags() const;
    void setTags( const QList<TagId>& tags );
    void addTag( const TagId& tag );
    void removeTag( const TagId& tag );
    QStringList subscriptionLabels() const;
    void setSubscriptionLabels( const QStringList& subscriptionLabels );
    void addSubscriptionLabel( const QString& subscriptionLabel );
    void removeSubscriptionLabel( const QString& subscriptionLabel );
};

} // namespace KRss

#endif // KRSS_VIRTUALFEEDCOLLECTION_H
