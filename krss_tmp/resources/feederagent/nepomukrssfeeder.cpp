/*
    Copyright (C) 2009    Dmitry Ivanov <vonami@gmail.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "nepomukrssfeeder.h"
#include "rsschannelitem.h"

#include <KDateTime>
#include <krss/rssitem.h>
#include <akonadi/changerecorder.h>
#include <akonadi/item.h>
#include <akonadi/itemfetchscope.h>

using namespace Akonadi;

NepomukRssFeeder::NepomukRssFeeder( const QString &id )
    : NepomukFeederAgent<NepomukFast::RssChannel>( id )
{
    addSupportedMimeType( QLatin1String( "application/rss+xml" ) );
    setNeedsStrigi( false );
    changeRecorder()->itemFetchScope().fetchFullPayload();
    changeRecorder()->itemFetchScope().fetchAllAttributes();
}

void NepomukRssFeeder::updateItem( const Akonadi::Item& item, const QUrl& graphUri )
{
    if ( !item.hasPayload<KRss::RssItem>() ) {
        kDebug() << "No RSS payload";
        return;
    }

    NepomukFast::RssChannelItem rssChannelItem( item.url(), graphUri );
    NepomukFeederAgentBase::setParent( rssChannelItem, item );

    const KRss::RssItem payload = item.payload<KRss::RssItem>();
    rssChannelItem.setTitle( payload.title() );
    rssChannelItem.setPlainTextContent( payload.content().isEmpty() ? payload.description() : payload.content() );
    rssChannelItem.setIsNew( item.hasFlag( KRss::RssItem::flagNew() ) );
    rssChannelItem.setIsRead( item.hasFlag( KRss::RssItem::flagRead() ) );
    rssChannelItem.setIsImportant( item.hasFlag( KRss::RssItem::flagImportant() ) );
    // TODO: virtuoso complains about the date properties
    //rssChannelItem.setDatePublished( payload.datePublished().dateTime() );
    //rssChannelItem.setDateUpdated( payload.dateUpdated().dateTime() );
    //rssChannelItem.setUrl( payload.link() );
    rssChannelItem.setGuid( payload.guid() );
}

AKONADI_AGENT_MAIN( NepomukRssFeeder )

#include "nepomukrssfeeder.moc"
