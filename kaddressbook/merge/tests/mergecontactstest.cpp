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


#include "mergecontactstest.h"
#include "../job/mergecontacts.h"
#include <qtest_kde.h>

using namespace KABMergeContacts;
using namespace KABC;
MergeContactsTest::MergeContactsTest()
{
}

void MergeContactsTest::shouldReturnDefaultAddressWhenNoListItem()
{
    MergeContacts contacts;
    KABC::Addressee result = contacts.mergedContact();
    QCOMPARE(result.isEmpty(), true);
}

void MergeContactsTest::shouldReturnDefaultAddressWhenOneItem()
{
    Akonadi::Item::List lst;
    Akonadi::Item item;
    KABC::Addressee address;
    address.setName(QLatin1String("foo1"));
    item.setPayload<KABC::Addressee>( address );
    lst<<item;

    MergeContacts contacts(lst);
    KABC::Addressee result = contacts.mergedContact();
    QCOMPARE(result.isEmpty(), true);
}

void MergeContactsTest::noNeedManualSelectionCheckWhenEmptyList()
{
    MergeContacts contacts;
    const bool result = contacts.needManualSelectInformations();
    QCOMPARE(result, false);
}

void MergeContactsTest::noNeedManualSelectionCheckWhenOneItem()
{
    Akonadi::Item::List lst;
    KABC::Addressee address;
    Akonadi::Item item;
    address.setName(QLatin1String("foo1"));
    item.setPayload<KABC::Addressee>( address );
    lst<<item;
    MergeContacts contacts(lst);
    const bool result = contacts.needManualSelectInformations();
    QCOMPARE(result, false);
}

void MergeContactsTest::shouldMergeNotes_data()
{
    QTest::addColumn<QString>("noteItemA");
    QTest::addColumn<QString>("noteItemB");
    QTest::addColumn<QString>("noteItemC");
    QTest::addColumn<QString>("note");
    QTest::newRow("noNotes") <<  QString() << QString() << QString() << QString();
    QTest::newRow("oneNotes") <<  QString(QLatin1String("one")) << QString() << QString() << QString(QLatin1String("one"));
    QTest::newRow("twoNotes") <<  QString() << QString(QLatin1String("one")) << QString(QLatin1String("one")) << QString(QLatin1String("one\none"));
    QTest::newRow("threeNotes") <<  QString(QLatin1String("one")) << QString(QLatin1String("one")) << QString(QLatin1String("one")) << QString(QLatin1String("one\none\none"));
}

void MergeContactsTest::shouldMergeNotes()
{
    QFETCH( QString, noteItemA );
    QFETCH( QString, noteItemB );
    QFETCH( QString, noteItemC );
    QFETCH( QString, note );

    Akonadi::Item::List lst;
    Addressee addressA;
    Akonadi::Item itemA;
    addressA.setName(QLatin1String("foo1"));
    addressA.setNote(noteItemA);
    itemA.setPayload<KABC::Addressee>( addressA );
    lst<<itemA;

    Addressee addressB;
    Akonadi::Item itemB;
    addressB.setName(QLatin1String("foo1"));
    addressB.setNote(noteItemB);
    itemB.setPayload<KABC::Addressee>( addressB );
    lst<<itemB;

    Addressee addressC;
    Akonadi::Item itemC;
    addressC.setName(QLatin1String("foo1"));
    addressC.setNote(noteItemC);
    itemC.setPayload<KABC::Addressee>( addressC );
    lst<<itemC;


    MergeContacts contacts(lst);

    const Addressee result = contacts.mergedContact();
    QCOMPARE(result.note(), note);

}


QTEST_KDEMAIN(MergeContactsTest, NoGUI)
