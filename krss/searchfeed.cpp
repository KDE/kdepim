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

#include "searchfeed.h"
#include "feed_p.h"
#include "feedcollection.h"
#include "feedvisitor.h"

using namespace KRss;
using namespace boost;

SearchFeed::SearchFeed( const FeedCollection& feedCollection, Resource *resource, QObject *parent )
    : Feed( feedCollection, resource, parent )
{
}

void SearchFeed::accept( FeedVisitor* v )
{
    v->visitSearchFeed( static_pointer_cast<SearchFeed>( shared_from_this() ) );
}

void SearchFeed::accept( ConstFeedVisitor* v ) const
{
    v->visitSearchFeed( static_pointer_cast<const SearchFeed>( shared_from_this() ) );
}

bool SearchFeed::isVirtual() const
{
    return true;
}

QString SearchFeed::query() const
{
    return QString();   // TODO
}

void SearchFeed::setQuery( const QString& query )
{
    // TODO
}

#include "searchfeed.moc"
