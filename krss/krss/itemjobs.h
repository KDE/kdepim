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

#ifndef KRSS_ITEMJOBS_H
#define KRSS_ITEMJOBS_H

#include "krss_export.h"
#include "item.h"

#include <KJob>

namespace Akonadi {
class ItemFetchScope;
}

namespace KRss {

class ItemFetchJobPrivate;
class KRSS_EXPORT ItemFetchJob : public KJob
{
    Q_OBJECT
public:
    explicit ItemFetchJob( QObject* parent = 0 );
    ~ItemFetchJob();

    void setItem( const KRss::Item& item );
    KRss::Item item() const;

    void setFetchScope( const Akonadi::ItemFetchScope& fetchScope );
    Akonadi::ItemFetchScope& fetchScope() const;

    void start();
    QString errorString() const;

    enum Error {
        CouldNotFetchItem = KJob::UserDefinedError,
        UserDefinedError
    };

private:
    friend class ::KRss::ItemFetchJobPrivate;
    ItemFetchJobPrivate * const d;
    Q_DISABLE_COPY( ItemFetchJob )
    Q_PRIVATE_SLOT( d, void doStart() )
    Q_PRIVATE_SLOT( d, void slotItemFetched( KJob* ) )
};

class ItemModifyJobPrivate;
class KRSS_EXPORT ItemModifyJob : public KJob
{
    Q_OBJECT
public:
    explicit ItemModifyJob( QObject *parent = 0 );
    ~ItemModifyJob();

    void setIgnorePayload( bool ignorePayload );
    bool ignorePayload() const;
    void setItem( const KRss::Item& item );
    KRss::Item item() const;

    void start();
    QString errorString() const;

    enum Error {
        CouldNotFetchOldItem = KJob::UserDefinedError,
        CouldNotModifyItem,
        CouldNotRetrieveTagProvider,
        CouldNotModifyTagReferences,
        UserDefinedError
    };

private:
    friend class ::KRss::ItemModifyJobPrivate;
    ItemModifyJobPrivate * const d;
    Q_DISABLE_COPY( ItemModifyJob )
    Q_PRIVATE_SLOT( d, void doStart() )
    Q_PRIVATE_SLOT( d, void slotOldItemFetched( KJob* ) )
    Q_PRIVATE_SLOT( d, void slotItemModified( KJob* ) )
    Q_PRIVATE_SLOT( d, void slotTagProviderRetrieved( KJob* ) )
    Q_PRIVATE_SLOT( d, void slotReferencesModified( KJob* ) )
};


class ItemDeleteJobPrivate;
class KRSS_EXPORT ItemDeleteJob : public KJob
{
    Q_OBJECT
public:
    explicit ItemDeleteJob( QObject *parent = 0 );
    ~ItemDeleteJob();

    void setItem( const KRss::Item& item );
    KRss::ItemId item() const;

    void start();
    QString errorString() const;

    enum Error {
        CouldNotRetrieveTagProvider = KJob::UserDefinedError,
        CouldNotDeleteTagReferences,
        CouldNotDeleteItem,
        UserDefinedError
    };

private:
    friend class ::KRss::ItemDeleteJobPrivate;
    ItemDeleteJobPrivate * const d;
    Q_DISABLE_COPY( ItemDeleteJob )
    Q_PRIVATE_SLOT( d, void doStart() )
    Q_PRIVATE_SLOT( d, void slotTagProviderRetrieved( KJob* ) )
    Q_PRIVATE_SLOT( d, void slotReferencesDeleted( KJob* ) )
    Q_PRIVATE_SLOT( d, void slotItemDeleted( KJob* ) )
};

} // namespace KRss

#endif // KRSS_ITEMJOBS_H
