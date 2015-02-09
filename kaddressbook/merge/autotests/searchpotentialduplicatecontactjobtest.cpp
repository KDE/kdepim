/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "searchpotentialduplicatecontactjobtest.h"
#include "../job/searchpotentialduplicatecontactjob.h"

#include <AkonadiCore/item.h>
#include <kcontacts/addressee.h>
#include <QList>
#include <qtest.h>
#include <QSignalSpy>

using namespace KABMergeContacts;
SearchPotentialDuplicateContactJobTest::SearchPotentialDuplicateContactJobTest()
{
    qRegisterMetaType<QList<Akonadi::Item> >();
    qRegisterMetaType<QList<Akonadi::Item::List> >();
}

void SearchPotentialDuplicateContactJobTest::shouldReturnEmptyListWhenNoItem()
{
    Akonadi::Item::List lst;
    SearchPotentialDuplicateContactJob job(lst);
    QSignalSpy spy(&job, SIGNAL(finished(QList<Akonadi::Item::List>)));
    job.start();
    QCOMPARE(spy.count(), 1);
    QList<Akonadi::Item::List> lstResult = spy.at(0).at(0).value< QList<Akonadi::Item::List> >();
    QCOMPARE(lstResult.count(), 0);
}

void SearchPotentialDuplicateContactJobTest::shouldReturnEmptyListWhenOneItem()
{
    Akonadi::Item::List lst;
    lst << Akonadi::Item(42);
    SearchPotentialDuplicateContactJob job(lst);
    QSignalSpy spy(&job, SIGNAL(finished(QList<Akonadi::Item::List>)));
    job.start();
    QCOMPARE(spy.count(), 1);
    QList<Akonadi::Item::List> lstResult = spy.at(0).at(0).value< QList<Akonadi::Item::List> >();
    QCOMPARE(lstResult.count(), 0);
}

void SearchPotentialDuplicateContactJobTest::shouldReturnListWhenTwoItemsAreDuplicated()
{
    Akonadi::Item::List lst;
    Akonadi::Item itemA;
    KContacts::Addressee address;
    address.setName(QStringLiteral("foo1"));
    itemA.setPayload<KContacts::Addressee>(address);
    itemA.setMimeType(KContacts::Addressee::mimeType());

    lst << itemA << itemA;

    SearchPotentialDuplicateContactJob job(lst);
    QSignalSpy spy(&job, SIGNAL(finished(QList<Akonadi::Item::List>)));
    job.start();
    QCOMPARE(spy.count(), 1);
    QList<Akonadi::Item::List> lstResult = spy.at(0).at(0).value< QList<Akonadi::Item::List> >();
    QCOMPARE(lstResult.count(), 1);
}

void SearchPotentialDuplicateContactJobTest::shouldReturnListWhenThreeItemsAreDuplicated()
{
    Akonadi::Item::List lst;
    Akonadi::Item itemA;
    KContacts::Addressee address;
    address.setName(QLatin1String("foo1"));
    itemA.setPayload<KContacts::Addressee>( address );
    itemA.setMimeType( KContacts::Addressee::mimeType() );

    lst << itemA << itemA << itemA;

    SearchPotentialDuplicateContactJob job(lst);
    QSignalSpy spy(&job, SIGNAL(finished(QList<Akonadi::Item::List>)));
    job.start();
    QCOMPARE(spy.count(), 1);
    QList<Akonadi::Item::List> lstResult = spy.at(0).at(0).value< QList<Akonadi::Item::List> >();
    QCOMPARE(lstResult.count(), 1);
}

void SearchPotentialDuplicateContactJobTest::shouldReturnTwoList()
{
    Akonadi::Item::List lst;
    Akonadi::Item item;
    KContacts::Addressee addressA;
    addressA.setName(QLatin1String("foo1"));
    item.setPayload<KContacts::Addressee>( addressA );
    item.setMimeType( KContacts::Addressee::mimeType() );

    lst << item << item << item;

    KContacts::Addressee addressB;
    addressB.setName(QLatin1String("foo2"));
    item.setPayload<KContacts::Addressee>( addressB );
    item.setMimeType( KContacts::Addressee::mimeType() );

    lst << item << item << item;

    SearchPotentialDuplicateContactJob job(lst);
    QSignalSpy spy(&job, SIGNAL(finished(QList<Akonadi::Item::List>)));
    job.start();
    QCOMPARE(spy.count(), 1);
    QList<Akonadi::Item::List> lstResult = spy.at(0).at(0).value< QList<Akonadi::Item::List> >();
    QCOMPARE(lstResult.count(), 2);
}

void SearchPotentialDuplicateContactJobTest::shouldReturnList_data()
{
    QTest::addColumn<Akonadi::Item::List>("listItem");
    QTest::addColumn<int>("numberOfList");
    QTest::newRow("noList") <<  Akonadi::Item::List() << 0;
    Akonadi::Item itemA;
    KContacts::Addressee addressA;
    addressA.setName(QLatin1String("foo1"));
    itemA.setPayload<KContacts::Addressee>( addressA );
    itemA.setMimeType( KContacts::Addressee::mimeType() );

    Akonadi::Item itemB;

    KContacts::Addressee addressB;
    addressB.setName(QLatin1String("foo2"));
    itemB.setPayload<KContacts::Addressee>( addressB );
    itemB.setMimeType( KContacts::Addressee::mimeType() );

    Akonadi::Item::List lst;
    lst << itemA;
    QTest::newRow("oneItem") <<  lst << 0;

    lst.clear();
    lst << itemA << itemA;
    QTest::newRow("oneDuplicate") <<  lst << 1;

    lst.clear();
    lst << itemB << itemA;
    QTest::newRow("twoDifferentItem") <<  lst << 0;

    lst.clear();
    lst << itemB << itemA << itemA << itemA;
    QTest::newRow("onDuplicate") <<  lst << 1;

    lst.clear();
    lst << itemB << itemA << itemA << itemB;
    QTest::newRow("twoDuplicate") <<  lst << 2;

    Akonadi::Item itemC;

    KContacts::Addressee addressC;
    addressC.setName(QLatin1String("foo3"));
    itemC.setPayload<KContacts::Addressee>( addressC );
    itemC.setMimeType( KContacts::Addressee::mimeType() );

    lst.clear();
    lst << itemB << itemC <<itemA << itemA << itemB << itemC;
    QTest::newRow("threeDuplicate") <<  lst << 3;

    lst.clear();
    lst << itemB << itemC <<itemA;
    QTest::newRow("threeDifferent") <<  lst << 0;

}

void SearchPotentialDuplicateContactJobTest::shouldReturnList()
{
    QFETCH( Akonadi::Item::List, listItem );
    QFETCH( int, numberOfList );

    SearchPotentialDuplicateContactJob job(listItem);
    QSignalSpy spy(&job, SIGNAL(finished(QList<Akonadi::Item::List>)));
    job.start();
    QCOMPARE(spy.count(), 1);
    QList<Akonadi::Item::List> lstResult = spy.at(0).at(0).value< QList<Akonadi::Item::List> >();
    QCOMPARE(lstResult.count(), numberOfList);

}

QTEST_MAIN(SearchPotentialDuplicateContactJobTest)
