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

#include "rssitemsynctest.h"
#include "rssitemsync.h"

#include <krss/rssitem.h>
#include <krss/item.h>

#include <Akonadi/Item>
#include <Akonadi/Collection>
#include <qtest_kde.h>

QTEST_KDEMAIN( RssItemSyncTest, NoGUI )

// from akregator/src/utils.cpp
static inline int calcHash( const QString &str )
{
    const QByteArray array = str.toAscii();
    return qChecksum( array.constData(), array.size() );
}

class RssItemSyncExposed : RssItemSync
{
public:
    RssItemSyncExposed( const Akonadi::Collection &collection )
        : RssItemSync( collection )
    {
        setSynchronizeFlags( false );
    }

    bool updateItemExposed( const Akonadi::Item &storedItem, Akonadi::Item &newItem )
    {
        return updateItem( storedItem, newItem );
    }
};

void RssItemSyncTest::testSyncWithoutFlags()
{
    KRss::RssItem originalArticle;
    originalArticle.setGuidIsHash( false );
    originalArticle.setTitle( "Article which was stored during a previous fetch" );
    originalArticle.setLink( "http://planetkde.org" );
    originalArticle.setDescription( "This description is long enough, right?" );
    originalArticle.setContent( "Holy macaroni!" );
    originalArticle.setHash( calcHash( originalArticle.title() + originalArticle.description() +
                                       originalArticle.content() + originalArticle.link() ) );

    KRss::RssItem updatedArticle;
    updatedArticle.setContent( "Holy macaroni! UPDATE: Doh!" );
    updatedArticle.setHash( calcHash( updatedArticle.title() + updatedArticle.description() +
                                      updatedArticle.content() + updatedArticle.link() ) );


    RssItemSyncExposed *sync = new RssItemSyncExposed( Akonadi::Collection() );

    // verify that itemsync skips unchanged articles
    Akonadi::Item storedItem;
    storedItem.setRemoteId( "remote-id-012345" );
    storedItem.setMimeType( KRss::Item::mimeType() );
    storedItem.setPayload<KRss::RssItem>( originalArticle );
    storedItem.setFlags( Akonadi::Item::Flags() << KRss::RssItem::flagImportant() << KRss::RssItem::flagRead() );

    Akonadi::Item newItemNotChanged;
    newItemNotChanged.setRemoteId( "remote-id-012345" );
    newItemNotChanged.setMimeType( "application/rss+xml" );
    newItemNotChanged.setPayload<KRss::RssItem>( originalArticle );
    newItemNotChanged.setFlags( Akonadi::Item::Flags() << KRss::RssItem::flagNew() );
    QVERIFY( sync->updateItemExposed( storedItem, newItemNotChanged ) == false );

    // verify that itemsync detects updated articles
    // and doesn't override already stored flags
    // and sets 'New' + 'Unread'
    Akonadi::Item newItemChanged;
    newItemChanged.setRemoteId( "remote-id-012345" );
    newItemChanged.setMimeType( "application/rss+xml" );
    newItemChanged.setPayload<KRss::RssItem>( updatedArticle );
    newItemChanged.setFlags( Akonadi::Item::Flags() << KRss::RssItem::flagNew() );
    QVERIFY( sync->updateItemExposed( storedItem, newItemChanged ) == true );
    QCOMPARE( newItemChanged.flags(), Akonadi::Item::Flags() << KRss::RssItem::flagImportant() <<
                                      KRss::RssItem::flagNew() );

    // verify that itemsync skips *updated* articles if their guid is hash
    originalArticle.setGuidIsHash( true );
    storedItem.setPayload<KRss::RssItem>( originalArticle );
    updatedArticle.setGuidIsHash( true );
    Akonadi::Item newItemChanged_Guid;
    newItemChanged_Guid.setRemoteId( "remote-id-012345" );
    newItemChanged_Guid.setMimeType( "application/rss+xml" );
    newItemChanged_Guid.setPayload<KRss::RssItem>( updatedArticle );
    newItemChanged_Guid.setFlags( Akonadi::Item::Flags() << KRss::RssItem::flagNew() );
    QVERIFY( sync->updateItemExposed( storedItem, newItemChanged_Guid ) == false );
}

#include "rssitemsynctest.moc"
