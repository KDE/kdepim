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

#ifndef KRSS_SEARCHFEED_H
#define KRSS_SEARCHFEED_H

#include "krss_export.h"
#include "feed.h"

namespace KRss {

class KRSS_EXPORT SearchFeed : public Feed
{
    Q_OBJECT

public:
    /* reimp */ void accept( FeedVisitor* );
    /* reimp */ void accept( ConstFeedVisitor* ) const;

    bool isVirtual() const;
    QString query() const;
    void setQuery( const QString& query );

private:
    explicit SearchFeed( const FeedCollection& feedCollection, Resource *resource, QObject *parent = 0 );

private:
    Q_DISABLE_COPY(SearchFeed)
};

} // namespace KRss

#endif // KRSS_SEARCHFEED_H
