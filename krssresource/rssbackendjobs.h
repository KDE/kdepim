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

#ifndef KRSSRESOURCE_RSSBACKENDJOBS_H
#define KRSSRESOURCE_RSSBACKENDJOBS_H

#include "krssresource_export.h"

#include <Akonadi/Collection>
#include <Akonadi/Item>
#include <KJob>

namespace KRssResource
{

class KRSSRESOURCE_EXPORT FeedsRetrieveJob : public KJob
{
    Q_OBJECT
public:
    explicit FeedsRetrieveJob( QObject *parent );

    virtual QList<Akonadi::Collection> feeds() const = 0;

    enum Error {
        CouldNotContactBackend = KJob::UserDefinedError,
        CouldNotRetrieveFeeds,
        UserDefinedError
    };
};

class KRSSRESOURCE_EXPORT FeedsImportJob : public KJob
{
    Q_OBJECT
public:
    explicit FeedsImportJob( QObject *parent );

    virtual void setFeeds( const QList<Akonadi::Collection>& feeds ) = 0;
    virtual QList<Akonadi::Collection> feeds() const = 0;

    enum Error {
        CouldNotContactBackend = KJob::UserDefinedError,
        CouldNotImportFeeds,
        UserDefinedError
    };
};

class KRSSRESOURCE_EXPORT FeedCreateJob : public KJob
{
    Q_OBJECT
public:
    explicit FeedCreateJob( QObject *parent );

    virtual Akonadi::Collection feed() const = 0;
    virtual void setXmlUrl( const QString& xmlUrl ) = 0;

    enum Error {
        CouldNotContactBackend = KJob::UserDefinedError,
        CouldNotCreateFeed,
        UserDefinedError
    };
};

class KRSSRESOURCE_EXPORT FeedModifyJob : public KJob
{
    Q_OBJECT
public:
    explicit FeedModifyJob( QObject *parent );

    virtual void setFeed( const Akonadi::Collection& collection ) = 0;
    virtual Akonadi::Collection feed() const = 0;

    enum Error {
        CouldNotContactBackend = KJob::UserDefinedError,
        CouldNotModifyFeed,
        UserDefinedError
    };
};

class KRSSRESOURCE_EXPORT FeedDeleteJob : public KJob
{
    Q_OBJECT
public:
    explicit FeedDeleteJob( QObject *parent );

    virtual void setFeed( const Akonadi::Collection& collection ) = 0;
    virtual Akonadi::Collection feed() const = 0;

    enum Error {
        CouldNotContactBackend = KJob::UserDefinedError,
        CouldNotDeleteFeed,
        UserDefinedError
    };
};

class KRSSRESOURCE_EXPORT FeedFetchJob : public KJob
{
    Q_OBJECT
public:
    explicit FeedFetchJob( QObject *parent );

    virtual void setFeed( const Akonadi::Collection& collection ) = 0;
    virtual Akonadi::Collection feed() const = 0;
    virtual QList<Akonadi::Item> items() const = 0;

    enum Error {
        CouldNotFetchFeed = KJob::UserDefinedError,
        UserDefinedError
    };
};

class KRSSRESOURCE_EXPORT ItemModifyJob : public KJob
{
    Q_OBJECT
public:
    explicit ItemModifyJob( QObject *parent );

    virtual void setItem( const Akonadi::Item& item ) = 0;
    virtual Akonadi::Item item() const = 0;

    enum Error {
        CouldNotContactBackend = KJob::UserDefinedError,
        CouldNotModifyItem,
        UserDefinedError
    };
};

} // namespace KRssResource

#endif // KRSSRESOURCE_RSSBACKENDJOBS_H
