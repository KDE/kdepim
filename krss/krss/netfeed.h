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

#ifndef KRSS_NETFEED_H
#define KRSS_NETFEED_H

#include "krss_export.h"
#include "feed.h"

namespace KRss {

class KRSS_EXPORT NetFeed : public Feed
{
    Q_OBJECT

public:
    /* reimp */ void accept( FeedVisitor* );
    /* reimp */ void accept( ConstFeedVisitor* ) const;

    /* reimp */  bool isVirtual() const;

    QString xmlUrl() const;
    void setXmlUrl( const QString &xmlUrl );
    QString htmlUrl() const;
    void setHtmlUrl( const QString &htmlUrl );

    bool preferItemLinkForDisplay() const;
    void setPreferItemLinkForDisplay( bool b );

    //TODO: cache policies

private:
    explicit NetFeed( const FeedCollection& feedCollection, const Resource *resource, QObject *parent = 0 );

private:
    friend class FeedListPrivate;
    Q_DISABLE_COPY(NetFeed)
};

} // namespace KRss

#endif // KRSS_NETFEED_H
