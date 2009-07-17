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

#ifndef KRSS_DEFAULTTAGPROVIDER_H
#define KRSS_DEFAULTTAGPROVIDER_H

#include "krss/tagprovider.h"
#include <akonadi/collection.h>

template<typename T> class QList;

namespace KRss {

/**
 * \a TagProvider implementation that stores tags in Akonadi as
 * virtual search collections.
*/
class DefaultTagProvider : public TagProvider
{
    Q_OBJECT
public:
    explicit DefaultTagProvider( const QList<Akonadi::Collection>& tagCollections, QObject *parent = 0 );

    Tag tag( const TagId& id ) const;
    QHash<TagId, Tag> tags() const;

    TagCreateJob* tagCreateJob() const;
    TagModifyJob* tagModifyJob() const;
    TagDeleteJob* tagDeleteJob() const;

    static const Akonadi::Collection AkonadiSearchCollection;

private:
    TagCreateReferencesJob* tagCreateReferencesJob() const;
    TagModifyReferencesJob* tagModifyReferencesJob() const;
    TagDeleteReferencesJob* tagDeleteReferencesJob() const;

private Q_SLOTS:
    void slotCollectionAdded( const Akonadi::Collection& col, const Akonadi::Collection& parent );
    void slotCollectionChanged( const Akonadi::Collection& col );
    void slotCollectionRemoved( const Akonadi::Collection& col );

private:
    QHash<TagId, Tag> m_tags;
    Q_DISABLE_COPY( DefaultTagProvider )
};

/** Creates and initializes \a DefaultTagProvider.
*/
class DefaultTagProviderLoadJob : public TagProviderLoadJob
{
    Q_OBJECT
public:

    explicit DefaultTagProviderLoadJob( QObject* parent = 0 );
    TagProvider* tagProvider() const;

    QString errorString() const;
    void start();

private Q_SLOTS:
    void slotCollectionsFetched( KJob *job );

private:
    TagProvider *m_provider;
};

} //namespace KRss

#endif // KRSS_DEFAULTTAGPROVIDER_H
