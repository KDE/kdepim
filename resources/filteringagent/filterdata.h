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

#ifndef RSSFILTERINGAGENT_FILTERDATA_H
#define RSSFILTERINGAGENT_FILTERDATA_H

#include <krss/feed.h>
#include <akonadi/filter/program.h>
#include <akonadi/collection.h>
#include <QtCore/QString>
#include <QtCore/QSharedDataPointer>

class FilterData
{
public:
    FilterData();
    FilterData( const QString& id, const QString& name, const QList<KRss::Feed::Id>& sourceFeeds,
            Akonadi::Filter::Program* program );
    FilterData( const FilterData& other );
    ~FilterData();

    void swap( FilterData& other );
    FilterData& operator=( const FilterData& other );

    bool operator==( const FilterData& other ) const;
    bool operator!=( const FilterData& other ) const;
    bool operator<( const FilterData& other ) const;

    QString id() const;
    void setId( const QString& id );
    QString name() const;
    void setName( const QString& name );
    QList<KRss::Feed::Id> sourceFeeds() const;
    void setSourceFeeds( const QList<KRss::Feed::Id>& sourceFeeds );
    Akonadi::Filter::Program* program() const;
    void setProgram( Akonadi::Filter::Program* program );

private:
    class Private;
    QSharedDataPointer<Private> d;
};

#endif // RSSFILTERINGAGENT_FILTERDATA_H
