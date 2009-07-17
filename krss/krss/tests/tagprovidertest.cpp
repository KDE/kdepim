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

#include "tagprovidertest.h"
#include "tagprovider.h"
#include "tagjobs.h"
#include "resourcemanager.h"
#include "item.h"
#include "itemjobs.h"
#include "feedcollection.h"

#include <akonadi/itemfetchjob.h>
#include <akonadi/collectionfetchjob.h>
#include <akonadi/collectionmodifyjob.h>
#include <akonadi/qtest_akonadi.h>

#include <QtCore/QMetaType>

using namespace KRss;

QTEST_AKONADIMAIN( TagProviderTest, NoGUI )

void TagProviderTest::initTestCase()
{
    ResourceManager::registerAttributes();
    qRegisterMetaType<KRss::Tag>();
    qRegisterMetaType<KRss::TagId>( "KRss::TagId" ); // otherwise it would be registered as KUrl

    TagProviderRetrieveJob *job = new TagProviderRetrieveJob();
    QVERIFY( job->exec() );
    m_tagProvider = job->tagProvider();
}

void TagProviderTest::createTagTest()
{
    Tag tag;
    tag.setLabel( "Test tag" );
    tag.setDescription( "This is a test tag" );

    QSignalSpy spyCreateTag( m_tagProvider.get(), SIGNAL( tagCreated( const KRss::Tag& ) ) );
    QVERIFY( spyCreateTag.isValid() );

    TagCreateJob *job = m_tagProvider->tagCreateJob();
    job->setTag( tag );
    QVERIFY( job->exec() );
    const Tag returnedTag = job->tag();
    QTest::qWait( 1000 );
    QCOMPARE( spyCreateTag.count(), 1 );
    const Tag spiedTag = spyCreateTag.takeFirst().at( 0 ).value<Tag>();

    // compare the two tags
    QVERIFY( !spiedTag.isNull() );
    QCOMPARE( spiedTag.id(), returnedTag.id() );
    QCOMPARE( spiedTag.label(), returnedTag.label() );
    QCOMPARE( spiedTag.description(), returnedTag.description() );

    // compare it with the original tag
    QCOMPARE( spiedTag.label(), tag.label() );
    QCOMPARE( spiedTag.description(), tag.description() );

    // check whether the tag provider holds the created tag
    m_tag = returnedTag;
    QVERIFY( m_tagProvider->tags().keys().contains( m_tag.id() ) );
}

void TagProviderTest::createExistingTag()
{
    Tag tag;
    tag.setLabel( "Test tag" );
    tag.setDescription( "This is a test tag" );

    QSignalSpy spyCreateTag( m_tagProvider.get(), SIGNAL( tagCreated( const KRss::Tag& ) ) );
    QVERIFY( spyCreateTag.isValid() );

    TagCreateJob *job = m_tagProvider->tagCreateJob();
    job->setTag( tag );
    QVERIFY( job->exec() );
    const Tag returnedTag = job->tag();
    QTest::qWait( 1000 );

    // signal shouldn't be emitted since this tag already exists
    QCOMPARE( spyCreateTag.count(), 0 );
    QVERIFY( m_tag.id() == returnedTag.id() );
}

void TagProviderTest::modifyTagTest()
{
    m_tag.setLabel( "Modified test tag" );
    m_tag.setDescription( "This was a test tag" );

    QSignalSpy spyModifyTag( m_tagProvider.get(), SIGNAL( tagModified( const KRss::Tag& ) ) );
    QVERIFY( spyModifyTag.isValid() );

    TagModifyJob *job = m_tagProvider->tagModifyJob();
    job->setTag( m_tag );
    QVERIFY( job->exec() );
    QTest::qWait( 1000 );
    QCOMPARE( spyModifyTag.count(), 1 );
    const Tag spiedTag = spyModifyTag.takeFirst().at( 0 ).value<Tag>();

    // compare it with the original tag
    QVERIFY( !spiedTag.isNull() );
    QCOMPARE( spiedTag.id(), m_tag.id() );
    QCOMPARE( spiedTag.label(), m_tag.label() );
    QCOMPARE( spiedTag.description(), m_tag.description() );

    // check whether the tag provider holds the modified tag
    QVERIFY( m_tagProvider->tags().keys().contains( m_tag.id() ) );
}

void TagProviderTest::deleteEmptyTagTest()
{
    QSignalSpy spyDeleteTag( m_tagProvider.get(), SIGNAL( tagDeleted( const KRss::TagId& ) ) );
    QVERIFY( spyDeleteTag.isValid() );

    TagDeleteJob *job = m_tagProvider->tagDeleteJob();
    job->setTag( m_tag );
    QVERIFY( job->exec() );
    QTest::qWait( 1000 );
    QCOMPARE( spyDeleteTag.count(), 1 );
    const TagId spiedTagId = spyDeleteTag.takeFirst().at( 0 ).value<KRss::TagId>();

    // compare the id with the original tag
    QEXPECT_FAIL( "", "The id of the spied tag is empty, this has something to do with Qt's metatype system",
                  Continue );
    QCOMPARE( spiedTagId, m_tag.id() );

    // verify that the tag provider doesn't hold the deleted tag any more
    QVERIFY( !m_tagProvider->tags().keys().contains( m_tag.id() ) );
}

/*void TagProviderTest::deleteNonEmptyTagTest()
{
    // create a new tag
    Tag tag;
    tag.setLabel( "Non empty tag" );
    TagCreateJob *cjob = m_tagProvider->tagCreateJob();
    cjob->setTag( tag );
    QVERIFY( cjob->exec() );
    tag = cjob->tag();
    QTest::qWait( 1000 );

    // attach this tag to some items
    for ( int i = 1; i <= 3; ++i ) {
        Akonadi::ItemFetchJob *fjob = new Akonadi::ItemFetchJob( Akonadi::Item( i ) );
        QVERIFY( fjob->exec() );
        Item item = Item( fjob->items().first() );
        item.setTags( QList<TagId>() << tag.id() );
        ItemModifyJob *mjob = new ItemModifyJob( item );
        QVERIFY( mjob->exec() );
    }
    QTest::qWait( 1000 );

    // attach this tag to the only feed we have
    Akonadi::CollectionFetchJob *cfjob = new Akonadi::CollectionFetchJob( Akonadi::Collection( 3 ),
                                                                          Akonadi::CollectionFetchJob::Base );
    QVERIFY( cfjob->exec() );
    FeedCollection feed = cfjob->collections().first();
    feed.setTags( QList<TagId>() << tag.id() );
    Akonadi::CollectionModifyJob *cmjob = new Akonadi::CollectionModifyJob( feed );
    QVERIFY( cmjob->exec() );
    QTest::qWait( 1000 );

    // spy upon tagDeleted signal
    // TODO: maybe we should spy upon 'item modified' and 'feed modified' signals as well?
    // but this requires instantiating a FeedList and doing ItemListJob to get an ItemCollection
    QSignalSpy spyDeleteTag( m_tagProvider, SIGNAL( tagDeleted( const KRss::TagId& ) ) );
    QVERIFY( spyDeleteTag.isValid() );

    // go delete the tag
    TagDeleteJob *djob = m_tagProvider->tagDeleteJob();
    djob->setTag( tag );
    QVERIFY( djob->exec() );
    QTest::qWait( 1000 );
    QCOMPARE( spyDeleteTag.count(), 1 );
    const TagId spiedTagId = spyDeleteTag.takeFirst().at( 0 ).value<KRss::TagId>();

    // compare the id with the original tag
    QEXPECT_FAIL( "", "The id of the spied tag is empty, this has something to do with Qt's metatype system",
                  Continue );
    QCOMPARE( spiedTagId, tag.id() );

    // verify that the tag provider doesn't hold the deleted tag any more
    QVERIFY( !m_tagProvider->tags().keys().contains( tag.id() ) );

    // check whether the tag ids from the items and feeds have been removed
    for ( int i = 1; i <= 3; ++i ) {
        Akonadi::ItemFetchJob *fjob = new Akonadi::ItemFetchJob( Akonadi::Item( i ) );
        QVERIFY( fjob->exec() );
        const Item item = Item( fjob->items().first() );
        QVERIFY( item.tags().empty() );
    }

    Akonadi::CollectionFetchJob *tcfjob = new Akonadi::CollectionFetchJob( Akonadi::Collection( 3 ),
                                                                          Akonadi::CollectionFetchJob::Base );
    QVERIFY( tcfjob->exec() );
    const FeedCollection tfeed = tcfjob->collections().first();
    QVERIFY( tfeed.tags().isEmpty() );
}*/

#include "tagprovidertest.moc"
