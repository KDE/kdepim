/*
    Copyright (C) 2009    Frank Osterfeld <osterfeld@kde.org>

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

#ifndef KRSS_ITEMLISTING_H
#define KRSS_ITEMLISTING_H

#include "krss_export.h"

#include <QObject>

namespace boost {
template <typename T> class shared_ptr;
}

namespace Akonadi {
class ItemFetchScope;
}

namespace KRss {

    class Feed;
    class Item;
    class ItemListingPrivate;

    class KRSS_EXPORT ItemListing : public QObject {
        Q_OBJECT
    public:

        class KRSS_EXPORT Listener {
        public:
            virtual ~Listener();
            virtual void prepareInsert( int ) = 0;
            virtual void finishInsert( int ) = 0;
            virtual void prepareRemove( int ) = 0;
            virtual void finishRemove( int ) = 0;
            virtual void update( int ) = 0;
        };

        explicit ItemListing( const QList<Item>& items, const Akonadi::ItemFetchScope& fetchScope,
                              QObject* parent=0 );
        ~ItemListing();

        const QList<Item>& items() const;

        void connectToFeed( const boost::shared_ptr<const Feed>& feed );

        void addListener( Listener* listener );
        void removeListener( Listener* listener );

    protected:

    private:
        friend class ::KRss::ItemListingPrivate;
        ItemListingPrivate* const d;
        Q_PRIVATE_SLOT( d, void slotItemAdded( const Akonadi::Item&, const Akonadi::Collection& ) )
        Q_PRIVATE_SLOT( d, void slotItemChanged( const Akonadi::Item&, const QSet<QByteArray>& ) )
        Q_PRIVATE_SLOT( d, void slotItemRemoved( const Akonadi::Item& ) )
    };
}

#endif // KRSS_ITEMLISTING_H
