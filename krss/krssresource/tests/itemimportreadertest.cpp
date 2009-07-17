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

#include "itemimportreadertest.h"
#include "itemimportreader.h"

#include <krss/rssitem.h>
#include <krss/item.h>

#include <qtest_kde.h>

using namespace KRss;

QTEST_KDEMAIN( ItemImportReaderTest, NoGUI )

void ItemImportReaderTest::testCorrectness()
{
    QFile file( QString( "%1/unittestenv/kdehome2/planetkde-correctness.xml" ).arg( TESTDATAPATH ) );
    QVERIFY( file.open( QFile::ReadOnly ) );
    KRssResource::ItemImportReader reader( &file );

    int num = 0;
    QHash<QString, Akonadi::Item> items;
    while ( reader.hasNext() ) {
        const Akonadi::Item item = reader.nextItem();
        QVERIFY( !item.remoteId().isEmpty() );
        items.insert( item.remoteId(), item );
        ++num;
    }

    // total == 7
    QCOMPARE( num, 7 );

    // new (implies unread)
    QVERIFY( items.contains( "tag:blogger.com,1999:blog-6431293039768060590.post-962616740612527640" ) );
    const Akonadi::Item item1 = items.value( "tag:blogger.com,1999:blog-6431293039768060590.post-962616740612527640" );
    QVERIFY( item1.flags().count() == 2 );
    QVERIFY( item1.hasFlag( KRss::RssItem::flagNew() ) );
    QVERIFY( !item1.hasFlag( KRss::RssItem::flagRead() ) );

    // new (implies unread) + important
    QVERIFY( items.contains( "http://hemswell.lincoln.ac.uk/~padams/index.php?entry=entry090317-065020" ) );
    const Akonadi::Item item2 = items.value( "http://hemswell.lincoln.ac.uk/~padams/index.php?entry=entry090317-065020" );
    QVERIFY( item2.flags().count() == 3 );
    QVERIFY( item2.hasFlag( KRss::RssItem::flagNew() ) );
    QVERIFY( !item2.hasFlag( KRss::RssItem::flagRead() ) );
    QVERIFY( item2.hasFlag( KRss::RssItem::flagImportant() ) );

    // unread
    QVERIFY( items.contains( "http://labs.trolltech.com/blogs/2009/03/18/maccocoa-binary-package-available-for-testing/" ) );
    const Akonadi::Item item3 = items.value( "http://labs.trolltech.com/blogs/2009/03/18/maccocoa-binary-package-available-for-testing/" );
    QVERIFY( item3.flags().count() == 1 );
    QVERIFY( !item3.hasFlag( KRss::RssItem::flagRead() ) );

    // unread + important
    QVERIFY( items.contains( "http://www.purinchu.net/wp/?p=414" ) );
    const Akonadi::Item item4 = items.value( "http://www.purinchu.net/wp/?p=414" );
    QVERIFY( item4.flags().count() == 2 );
    QVERIFY( !item4.hasFlag( KRss::RssItem::flagRead() ) );
    QVERIFY( item4.hasFlag( KRss::RssItem::flagImportant() ) );

    // read
    QVERIFY( items.contains( "http://blusrcu.ba/nookie/?p=17" ) );
    const Akonadi::Item item5 = items.value( "http://blusrcu.ba/nookie/?p=17" );
    QVERIFY( item5.flags().count() == 0 );

    // read + important
    QVERIFY( items.contains( "tag:blogger.com,1999:blog-2333161201961104586.post-3320000861926986867" ) );
    const Akonadi::Item item6 = items.value( "tag:blogger.com,1999:blog-2333161201961104586.post-3320000861926986867" );
    QVERIFY( item6.flags().count() == 1 );
    QVERIFY( item6.hasFlag( KRss::RssItem::flagImportant() ) );

    // deleted
    QVERIFY( items.contains( "http://wm161.net/?p=467" ) );
    const Akonadi::Item item7 = items.value( "http://wm161.net/?p=467" );
    QVERIFY( item7.flags().count() == 1 );
    QVERIFY( item7.hasFlag( KRss::RssItem::flagDeleted() ) );
    QVERIFY( item7.payload<KRss::RssItem>().content().isEmpty() );

    file.close();
}

void ItemImportReaderTest::testPerformance()
{
    QFile file( QString( "%1/unittestenv/kdehome2/planetkde-performance.xml" ).arg( TESTDATAPATH ) );
    if ( !file.exists() ) {
        qDebug() << "Skipping the performance test";
        return;
    }
    QVERIFY( file.open( QFile::ReadOnly ) );
    KRssResource::ItemImportReader reader( &file );

    int num = 0;
    QHash<QString, Akonadi::Item> items;
    QBENCHMARK {
        while ( reader.hasNext() ) {
            const Akonadi::Item item = reader.nextItem();
            QVERIFY( !item.remoteId().isEmpty() );
            items.insert( item.remoteId(), item );
            ++num;
        }
    }
}

#include "itemimportreadertest.moc"
