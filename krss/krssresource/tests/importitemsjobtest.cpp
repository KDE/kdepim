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

#include "importitemsjobtest.h"
#include "importitemsjob.h"

#include <krss/rssitem.h>
#include <krss/item.h>
#include <krss/resourcemanager.h>
#include <krss/feedcollection.h>

#include <Akonadi/CollectionFetchJob>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <akonadi/qtest_akonadi.h>

using Akonadi::Collection;
using Akonadi::CollectionFetchJob;
using Akonadi::ItemFetchJob;

QTEST_AKONADIMAIN( ImportItemsJobTest, NoGUI )

void ImportItemsJobTest::initTestCase()
{
    KRss::ResourceManager::registerAttributes();
}

void ImportItemsJobTest::testCorrectness()
{
    const QString kdehome = qgetenv( "KDEHOME" );
    KRssResource::ImportItemsJob *ijob = new KRssResource::ImportItemsJob( "http://planetkde.org/rss20.xml", this );
    ijob->setResourceId( "akonadi_opml_rss_resource_0" );
    ijob->setFlagsSynchronizable( false );
    ijob->setSourceFile( kdehome + "/planetkde-correctness.xml" );
    QVERIFY( ijob->exec() );

    CollectionFetchJob *cjob = new CollectionFetchJob( Collection::root(), CollectionFetchJob::Recursive, this );
    cjob->setResource( "akonadi_opml_rss_resource_0" );
    QVERIFY( cjob->exec() );
    const QList<Collection> feeds = cjob->collections();
    QVERIFY( feeds.count() == 2 );  // root + planetkde

    Collection planetkde;
    if ( static_cast<KRss::FeedCollection>( feeds.at( 0 ) ).xmlUrl() == "http://planetkde.org/rss20.xml" )
        planetkde = feeds.at( 0 );
    else if ( static_cast<KRss::FeedCollection>( feeds.at( 1 ) ).xmlUrl() == "http://planetkde.org/rss20.xml" )
        planetkde = feeds.at( 1 );
    else
        QFAIL( "planetkde.org feed not found" );

    ItemFetchJob *fjob = new ItemFetchJob( planetkde, this );
    fjob->fetchScope().fetchAllAttributes();
    fjob->fetchScope().fetchFullPayload();
    fjob->fetchScope().setCacheOnly( true );
    QVERIFY( fjob->exec() );

    const QList<Akonadi::Item> items = fjob->items();
    QVERIFY( items.count() == 7 );

    Q_FOREACH( const Akonadi::Item& item, items ) {
        if ( item.remoteId() == "tag:blogger.com,1999:blog-6431293039768060590.post-962616740612527640" ) {
            // new + unread
            QVERIFY( item.flags().count() == 2 );
            QVERIFY( item.hasFlag( KRss::RssItem::flagNew() ) );
            QVERIFY( !item.hasFlag( KRss::RssItem::flagRead() ) );
            QVERIFY( item.hasPayload<KRss::RssItem>() );
            const KRss::RssItem rssItem = item.payload<KRss::RssItem>();

            QVERIFY( rssItem.hash() == 44457 );
            QVERIFY( rssItem.guidIsHash() == false );
            QVERIFY( rssItem.title() == "Sam Duff (Socceroos): Odd, but cool" );
            QVERIFY( rssItem.link() == "http://socceroosd.blogspot.com/2009/03/odd-but-cool.html");
            QVERIFY( rssItem.description() == "Content 1" );
            QVERIFY( rssItem.content().isEmpty() );
            QVERIFY( rssItem.guid() == "tag:blogger.com,1999:blog-6431293039768060590.post-962616740612527640" );
            //TODO: test the remaining properties
    }
    else if ( item.remoteId() == "http://labs.trolltech.com/blogs/2009/03/18/maccocoa-binary-package-available-for-testing/" ) {
            // unread
            QVERIFY( item.flags().count() == 1 );
            QVERIFY( !item.hasFlag( KRss::RssItem::flagRead() ) );
        }
        else if ( item.remoteId() == "http://wm161.net/?p=467" ) {
            // deleted
            QVERIFY( item.flags().count() == 1 );
            QVERIFY( item.hasFlag( KRss::RssItem::flagDeleted() ) );
            QVERIFY( item.hasPayload<KRss::RssItem>() );
            const KRss::RssItem rssItem = item.payload<KRss::RssItem>();

            QVERIFY( rssItem.description().isEmpty() );
            QVERIFY( rssItem.content().isEmpty() );
        }
        else if ( item.remoteId() == "http://blusrcu.ba/nookie/?p=17" ) {
            // read
            QVERIFY( item.flags().count() == 0 );
        }
        else if ( item.remoteId() == "tag:blogger.com,1999:blog-2333161201961104586.post-3320000861926986867" ) {
            // read + important
            QVERIFY( item.flags().count() == 1 );
            QVERIFY( item.hasFlag( KRss::RssItem::flagImportant() ) );
        }
        else if ( item.remoteId() == "http://hemswell.lincoln.ac.uk/~padams/index.php?entry=entry090317-065020" ) {
            // new (implies unread) + important
            QVERIFY( item.flags().count() == 3 );
            QVERIFY( item.hasFlag( KRss::RssItem::flagNew() ) );
            QVERIFY( !item.hasFlag( KRss::RssItem::flagRead() ) );
            QVERIFY( item.hasFlag( KRss::RssItem::flagImportant() ) );
        }
        else if ( item.remoteId() == "http://www.purinchu.net/wp/?p=414" ) {
            // unread + important
            QVERIFY( item.flags().count() == 2 );
            QVERIFY( !item.hasFlag( KRss::RssItem::flagRead() ) );
            QVERIFY( item.hasFlag( KRss::RssItem::flagImportant() ) );
        }
    }
}

void ImportItemsJobTest::testPerformance()
{
    const QString kdehome = qgetenv( "KDEHOME" );
    KRssResource::ImportItemsJob *ijob = new KRssResource::ImportItemsJob( "http://planetkde.org/rss20.xml", this );
    ijob->setResourceId( "akonadi_opml_rss_resource_0" );
    ijob->setFlagsSynchronizable( false );
    if ( !QFile::exists( kdehome + "/planetkde-performance.xml" ) ) {
        qDebug() << "Skipping the performance test";
        return;
    }

    ijob->setSourceFile( kdehome + "/planetkde-performance.xml" );
    QBENCHMARK {
        QVERIFY( ijob->exec() );
    }
}

#include "importitemsjobtest.moc"
