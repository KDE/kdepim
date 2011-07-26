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

#include "feedcollectiontest.h"
#include "feedcollection.h"

#include <qtest_kde.h>

using namespace KRss;

QTEST_KDEMAIN( FeedCollectionTest, NoGUI )

// list compare which ignores the order (taken from kdepimlibs/akonadi/tests)
template <class T> static void compareLists( const QList<T> &l1, const QList<T> &l2 )
{
    QCOMPARE( l1.count(), l2.count() );
    Q_FOREACH ( const T entry, l1 ) {
        QVERIFY( l2.contains( entry ) );
    }
}

void FeedCollectionTest::testSimpleProperties()
{
    FeedCollection feed;

    feed.setTitle( QLatin1String("Planet KDE") );
    QCOMPARE( feed.title(), QLatin1String( "Planet KDE" ) );
    feed.setXmlUrl( QLatin1String("http://planetkde.org/rss20.xml") );
    QCOMPARE( feed.xmlUrl(), QLatin1String( "http://planetkde.org/rss20.xml" ) );
    feed.setHtmlUrl( QLatin1String("http://planetkde.org") );
    QCOMPARE( feed.htmlUrl(), QLatin1String( "http://planetkde.org" ) );
    feed.setDescription( QLatin1String("Planet KDE blogs") );
    QCOMPARE( feed.description(), QLatin1String( "Planet KDE blogs" ) );
    feed.setFeedType( QLatin1String("RSS") );
    QCOMPARE( feed.feedType(), QLatin1String( "RSS" ) );
}

// TODO(dmitry): proper tag handling
/*void FeedCollectionTest::testTags()
{
    FeedCollection feed;

    feed.setTags( QStringList() << "Planets" << "Linux" );
    compareLists( feed.tags(), QStringList() << "Planets" << "Linux" );
    feed.addTag( "KDE" );
    compareLists( feed.tags(), QStringList() << "Planets" << "Linux" << "KDE" );
    feed.removeTag( "Planets" );
    compareLists( feed.tags(), QStringList() << "Linux" << "KDE" );

    // check that FeedCollection doesn't produce dupes
    feed.addTag( "Linux" );
    compareLists( feed.tags(), QStringList() << "Linux" << "KDE" );

    // check that FeedCollection doesn't do weird things when
    // removing non-existing tag
    feed.removeTag( "Empty tag" );
    compareLists( feed.tags(), QStringList() << "Linux" << "KDE" );
}*/

void FeedCollectionTest::testSubscriptionLabels()
{
    FeedCollection feed;

    feed.setSubscriptionLabels( QStringList() << QLatin1String("Label 1") << QLatin1String("Label 2") );
    compareLists( feed.subscriptionLabels(), QStringList() << QLatin1String("Label 1") << QLatin1String("Label 2") );
    feed.addSubscriptionLabel( QLatin1String("Label 3") );
    compareLists( feed.subscriptionLabels(), QStringList() << QLatin1String("Label 1") << QLatin1String("Label 2") << QLatin1String("Label 3") );
    feed.removeSubscriptionLabel( QLatin1String("Label 3") );
    compareLists( feed.subscriptionLabels(), QStringList() << QLatin1String("Label 1") << QLatin1String("Label 2") );

    // check that FeedCollection doesn't produce dupes
    feed.addSubscriptionLabel( QLatin1String("Label 1") );
    compareLists( feed.subscriptionLabels(), QStringList() << QLatin1String("Label 1") << QLatin1String("Label 2") );

    // check that FeedCollection doesn't do weird things when
    // removing non-existing label
    feed.removeSubscriptionLabel( QLatin1String("Empty label") );
    compareLists( feed.subscriptionLabels(), QStringList() << QLatin1String("Label 1") << QLatin1String("Label 2") );
}

void FeedCollectionTest::testCachePolicy()
{
    FeedCollection feed;

    feed.setFetchInterval( 5 );
    QCOMPARE( feed.fetchInterval(), 5 );

    feed.setFetchInterval( 0 );
    QCOMPARE( feed.fetchInterval(), 0 );

    feed.setFetchInterval( -1 );
    QCOMPARE( feed.fetchInterval(), -1 );
}

#include "feedcollectiontest.moc"
