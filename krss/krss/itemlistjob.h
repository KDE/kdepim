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

#ifndef KRSS_ITEMLISTJOB_H
#define KRSS_ITEMLISTJOB_H

#include "krss_export.h"

#include <KCompositeJob>
#include <KJob>

#include <boost/function.hpp>

namespace Akonadi {
    class ItemFetchScope;
}

namespace KRss {

    class Item;

    class KRSS_EXPORT ItemListJob : public KJob {
        Q_OBJECT

    public:
        explicit ItemListJob( QObject* parent = 0 );
        ~ItemListJob();

        virtual void setFetchScope( const Akonadi::ItemFetchScope &fetchScope ) = 0;

        virtual Akonadi::ItemFetchScope &fetchScope() = 0;

        virtual QList<KRss::Item> items() const = 0;
    };

    class KRSS_EXPORT CompositeItemListJob : public ItemListJob {
        Q_OBJECT

    public:
        explicit CompositeItemListJob( QObject * parent = 0 );
        ~CompositeItemListJob();

        void addSubJob( ItemListJob* job );
        void removeSubJob( ItemListJob* job );

        void setFetchScope( const Akonadi::ItemFetchScope &fetchScope );

        Akonadi::ItemFetchScope &fetchScope();

        QList<KRss::Item> items() const;

        void start();

        void setFilter( const boost::function1<bool, const Item&>& filter );
        void clearFilter();

    private:
        class Private;
        Private* const d;

        Q_PRIVATE_SLOT( d, void doStart() )
        Q_PRIVATE_SLOT( d, void jobDone(KJob*) )
        Q_PRIVATE_SLOT( d, void slotPercent(KJob*, unsigned long))
    };
}

#endif
