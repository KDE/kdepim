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

#include "netfeed.h"
#include "feed_p.h"
#include "feedcollection.h"
#include "feedvisitor.h"

#include <KIcon>

using namespace KRss;
using namespace boost;

NetFeed::NetFeed( const FeedCollection& feedCollection, const shared_ptr<Resource>& resource,
                  QObject* parent )
    : Feed( feedCollection, resource, parent )
{
    setIcon( KIcon("application-rss+xml") );
}

void NetFeed::accept( FeedVisitor* v )
{
    v->visitNetFeed( static_pointer_cast<NetFeed>( shared_from_this() ) );
}

void NetFeed::accept( ConstFeedVisitor* v ) const
{
    v->visitNetFeed( static_pointer_cast<const NetFeed>( shared_from_this() ) );
}

bool NetFeed::isVirtual() const
{
    return false;
}

QString NetFeed::xmlUrl() const
{
    return d->m_feedCollection.xmlUrl();
}

void NetFeed::setXmlUrl( const QString &xmlUrl )
{
    d->m_feedCollection.setXmlUrl( xmlUrl );
}

QString NetFeed::htmlUrl() const
{
    return d->m_feedCollection.htmlUrl();
}

void NetFeed::setHtmlUrl( const QString &htmlUrl )
{
    d->m_feedCollection.setHtmlUrl( htmlUrl );
}

bool NetFeed::preferItemLinkForDisplay() const
{
    return d->m_feedCollection.preferItemLinkForDisplay();
}

void NetFeed::setPreferItemLinkForDisplay( bool b )
{
    d->m_feedCollection.setPreferItemLinkForDisplay( b );
}

#include "netfeed.moc"
