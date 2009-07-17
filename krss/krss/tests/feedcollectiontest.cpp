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

    feed.setTitle( "Planet KDE" );
    QCOMPARE( feed.title(), QString( "Planet KDE" ) );
    feed.setXmlUrl( "http://planetkde.org/rss20.xml" );
    QCOMPARE( feed.xmlUrl(), QString( "http://planetkde.org/rss20.xml" ) );
    feed.setHtmlUrl( "http://planetkde.org" );
    QCOMPARE( feed.htmlUrl(), QString( "http://planetkde.org" ) );
    feed.setDescription( "Planet KDE blogs" );
    QCOMPARE( feed.description(), QString( "Planet KDE blogs" ) );
    feed.setFeedType( "RSS" );
    QCOMPARE( feed.feedType(), QString( "RSS" ) );
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

    feed.setSubscriptionLabels( QStringList() << "Label 1" << "Label 2" );
    compareLists( feed.subscriptionLabels(), QStringList() << "Label 1" << "Label 2" );
    feed.addSubscriptionLabel( "Label 3" );
    compareLists( feed.subscriptionLabels(), QStringList() << "Label 1" << "Label 2" << "Label 3" );
    feed.removeSubscriptionLabel( "Label 3" );
    compareLists( feed.subscriptionLabels(), QStringList() << "Label 1" << "Label 2" );

    // check that FeedCollection doesn't produce dupes
    feed.addSubscriptionLabel( "Label 1" );
    compareLists( feed.subscriptionLabels(), QStringList() << "Label 1" << "Label 2" );

    // check that FeedCollection doesn't do weird things when
    // removing non-existing label
    feed.removeSubscriptionLabel( "Empty label" );
    compareLists( feed.subscriptionLabels(), QStringList() << "Label 1" << "Label 2" );
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
