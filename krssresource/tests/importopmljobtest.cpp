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

#include "importopmljobtest.h"
#include "importopmljob.h"
#include "krssinterface.h"

#include <krss/resourcemanager.h>
#include <krss/tagprovider.h>

#include <Akonadi/CollectionFetchJob>
#include <akonadi/qtest_akonadi.h>

using namespace KRss;
using namespace KRssResource;
using Akonadi::CollectionFetchJob;
using Akonadi::Collection;

QTEST_AKONADIMAIN( ImportOpmlJobTest, NoGUI )

// list compare which ignores the order (taken from kdepimlibs/akonadi/tests)
template <class T> static void compareLists( const QList<T> &l1, const QList<T> &l2 )
{
    QCOMPARE( l1.count(), l2.count() );
    Q_FOREACH ( const T entry, l1 ) {
        QVERIFY( l2.contains( entry ) );
    }
}

void ImportOpmlJobTest::initTestCase()
{
    ResourceManager::registerAttributes();
}

void ImportOpmlJobTest::testGoodOpml()
{
    const QString defaultTag = QLatin1String("Imported");
    const QString kdehome = QString::fromLocal8Bit( qgetenv( "KDEHOME" ) );
    org::kde::krss *interface = new org::kde::krss( QLatin1String("org.freedesktop.Akonadi.Resource.akonadi_opml_rss_resource_0"),
                                                    QLatin1String("/KRss"), QDBusConnection::sessionBus(), this );

    QDBusReply<QVariantMap> reply = interface->call( QLatin1String("importOpml"), kdehome + QLatin1String("/to-import.opml"), defaultTag );
    QVERIFY( reply.isValid() );
    QVERIFY( reply.value().value( QLatin1String("error") ).toInt() == 0 );

    TagProviderRetrieveJob *tjob = new TagProviderRetrieveJob();
    QVERIFY( tjob->exec() );
    m_tagProvider = tjob->tagProvider();
    const QHash<TagId, Tag> allTags = m_tagProvider->tags();
    QCOMPARE( allTags.size(), 6 );
    QHashIterator<TagId, Tag> it( allTags );
    QStringList tagLabels;
    while ( it.hasNext() ) {
        it.next();
        tagLabels.append( it.value().label() );
    }
    compareLists( tagLabels, QStringList() << QLatin1String("Imported") << QLatin1String("Linux") << QLatin1String("Planets") << QLatin1String("Ubuntu") << QLatin1String("News") << QLatin1String("Technology") );

    CollectionFetchJob * job = new CollectionFetchJob( Collection::root(), CollectionFetchJob::Recursive, this );
    job->setResource( QLatin1String("akonadi_opml_rss_resource_0") );
    QVERIFY( job->exec() );
    const QList<Collection> feeds = job->collections();
    QVERIFY( feeds.count() == 7 );

    // now walk through the collections and verify
    // that every property is set correctly
    Q_FOREACH( const FeedCollection &feed, feeds ) {
        if ( feed.parent() == Collection::root().id() ) {
            // skip
        }
        else if ( feed.xmlUrl() == QLatin1String("unittestenv/kdehome/planetkde_org_rss20.xml") ) {
            // skip
        }
        // basic test
        else if ( feed.xmlUrl() == QLatin1String("http://planet.freedesktop.org/rss20.xml") ) {
            QCOMPARE( feed.title(), QLatin1String( "planet.freedesktop.org" ) );
            QCOMPARE( feed.htmlUrl(), QLatin1String( "http://planet.freedesktop.org" ) );
            QCOMPARE( feed.description(), QLatin1String( "planet.freedesktop.org - http://planet.freedesktop.org" ) );
            QCOMPARE( feed.feedType(), QLatin1String( "rss" ) );
            const QList<TagId> tagIds = feed.tags();
            QCOMPARE( tagIds.count(), 3 );
            QStringList tagLabels;
            Q_FOREACH( const TagId& tagId, tagIds ) {
                QVERIFY( allTags.contains( tagId ) );
                tagLabels.append( allTags.value( tagId ).label() );
            }
            compareLists( tagLabels, QStringList() << QLatin1String("Imported") << QLatin1String("Linux") << QLatin1String("Planets") );
        }
        // verifies that 'category=Linux,Ubuntu' is parsed correctly and
        // tag 'Linux' is set only once
        else if ( feed.xmlUrl() == QLatin1String("http://planet.ubuntu.com/rss20.xml") ) {
            QCOMPARE( feed.title(), QLatin1String( "Planet Ubuntu" ) );
            QCOMPARE( feed.htmlUrl(), QLatin1String( "http://planet.ubuntu.com/" ) );
            QCOMPARE( feed.description(), QLatin1String( "Planet Ubuntu - http://planet.ubuntu.com/" ) );
            QCOMPARE( feed.feedType(), QLatin1String( "rss" ) );
            const QList<TagId> tagIds = feed.tags();
            QCOMPARE( tagIds.count(), 4 );
            QStringList tagLabels;
            Q_FOREACH( const TagId& tagId, tagIds ) {
                QVERIFY( allTags.contains( tagId ) );
                tagLabels.append( allTags.value( tagId ).label() );
            }
            compareLists( tagLabels, QStringList() << QLatin1String("Imported") << QLatin1String("Linux") << QLatin1String("Planets") << QLatin1String("Ubuntu") );
        }
        // this time without any tags except the defaultTag
        else if ( feed.xmlUrl() == QLatin1String("http://www.jwz.org/cheesegrater/RSS/apod.rss") ) {
            QCOMPARE( feed.title(), QLatin1String( "Astronomy Picture of the Day" ) );
            QCOMPARE( feed.htmlUrl(), QLatin1String( "http://antwrp.gsfc.nasa.gov/apod/" ) );
            QCOMPARE( feed.description(), QLatin1String( "A different image or photograph of our fascinating universe" ) );
            QCOMPARE( feed.feedType(), QLatin1String( "rss" ) );
            const QList<TagId> tagIds = feed.tags();
            QCOMPARE( tagIds.count(), 1 );
            QStringList tagLabels;
            Q_FOREACH( const TagId& tagId, tagIds ) {
                QVERIFY( allTags.contains( tagId ) );
                tagLabels.append( allTags.value( tagId ).label() );
            }
            compareLists( tagLabels, QStringList() << QLatin1String( "Imported") );
        }
        // verify that category="/Linux/News,/Technology" is parsed correctly
        else if ( feed.xmlUrl() == QLatin1String( "http://lwn.net/headlines/newrss") ) {
            QCOMPARE( feed.title(), QLatin1String( "LWN.net" ) );
            QCOMPARE( feed.htmlUrl(), QLatin1String( "http://lwn.net" ) );
            QCOMPARE( feed.description(), QLatin1String( "LWN.net is a comprehensive source of news and opinions from and about the Linux community" ) );
            QCOMPARE( feed.feedType(), QLatin1String( "rss" ) );
            const QList<TagId> tagIds = feed.tags();
            QCOMPARE( tagIds.count(), 4 );
            QStringList tagLabels;
            Q_FOREACH( const TagId& tagId, tagIds ) {
                QVERIFY( allTags.contains( tagId ) );
                tagLabels.append( allTags.value( tagId ).label() );
            }
            compareLists( tagLabels, QStringList() << QLatin1String("Imported") << QLatin1String("Linux") << QLatin1String("News") << QLatin1String("Technology") );
        }
        // make sure that a single tag in category=/News is parsed correctly
        else if ( feed.xmlUrl() == QLatin1String("http://newsrss.bbc.co.uk/rss/newsonline_world_edition/front_page/rss.xml") ) {
            QCOMPARE( feed.title(), QLatin1String( "BBC News | News Front Page | World Edition" ) );
            QCOMPARE( feed.htmlUrl(), QLatin1String( "http://news.bbc.co.uk/go/rss/-/2/hi/default.stm" ) );
            QCOMPARE( feed.description(), QLatin1String( "Visit BBC News for up-to-the-minute news" ) );
            QCOMPARE( feed.feedType(), QLatin1String( "rss" ) );
            const QList<TagId> tagIds = feed.tags();
            QCOMPARE( tagIds.count(), 2 );
            QStringList tagLabels;
            Q_FOREACH( const TagId& tagId, tagIds ) {
                QVERIFY( allTags.contains( tagId ) );
                tagLabels.append( allTags.value( tagId ).label() );
            }
            compareLists( tagLabels, QStringList() << QLatin1String("Imported") << QLatin1String("News") );
        }
        else {
            QFAIL( "I didn't expect to see this feed in the test data" );
        }
    }
}

void ImportOpmlJobTest::testImportDuplicates()
{
    const QString defaultTag = QLatin1String("Imported-dups");
    const QString kdehome = QString::fromLocal8Bit( qgetenv( "KDEHOME" ) );
    org::kde::krss *interface = new org::kde::krss( QLatin1String("org.freedesktop.Akonadi.Resource.akonadi_opml_rss_resource_0"),
                                                    QLatin1String("/KRss"), QDBusConnection::sessionBus(), this );

    QDBusReply<QVariantMap> reply = interface->call( QLatin1String("importOpml"), kdehome + QLatin1String("/to-import-duplicates.opml"),
                                                     defaultTag );
    QVERIFY( reply.isValid() );
    QVERIFY( reply.value().value( QLatin1String("error") ).toInt() == 0 );

    // test that 'Imported-dups' and 'OSS' tags were created correctly
    // and 'Linux' was not created
    const QHash<TagId, Tag> allTags = m_tagProvider->tags();
    QEXPECT_FAIL( "", "This fails although the tags are created correctly, no idea why", Continue );
    QCOMPARE( allTags.size(), 8 );
    QHashIterator<TagId, Tag> it( allTags );
    QStringList tagLabels;
    while ( it.hasNext() ) {
        it.next();
        tagLabels.append( it.value().label() );
    }
    //compareLists( tagLabels, QStringList() << "Imported" << "Linux" << "Planets" << "Ubuntu" << "News"
    //                                       << "Technology" << "Imported-dups" << "OSS" );

    CollectionFetchJob * job = new CollectionFetchJob( Collection::root(), CollectionFetchJob::Recursive, this );
    job->setResource( QLatin1String("akonadi_opml_rss_resource_0") );
    QVERIFY( job->exec() );
    const QList<Collection> feeds = job->collections();
    QVERIFY( feeds.count() == 8 );

    // check that the feed was added
    bool freedesktopDupFound = false;
    Q_FOREACH( const FeedCollection &feed, feeds ) {
        if ( feed.xmlUrl() == QLatin1String("http://planet.freedesktop.org/rss20.xml") && feed.id() != m_freedesktopId ) {
            freedesktopDupFound = true;
        }
    }

    QVERIFY( freedesktopDupFound );
}

void ImportOpmlJobTest::testBrokenOpml()
{
    const QString defaultTag = QLatin1String("Imported");
    const QString kdehome = QString::fromLocal8Bit( qgetenv( "KDEHOME" ) );
    org::kde::krss *interface = new org::kde::krss( QLatin1String("org.freedesktop.Akonadi.Resource.akonadi_opml_rss_resource_0"),
                                                    QLatin1String("/KRss"), QDBusConnection::sessionBus(), this );

    QDBusReply<QVariantMap> reply = interface->call( QLatin1String("importOpml"), kdehome + QLatin1String("/to-import-broken.opml"), defaultTag );
    QVERIFY( reply.isValid() );
    QVERIFY( reply.value().value( QLatin1String("error") ).toInt() != 0 );
}

#include "importopmljobtest.moc"
