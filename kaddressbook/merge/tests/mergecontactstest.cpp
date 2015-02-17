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
    const MergeContacts::ConflictInformations result = contacts.requiresManualSelectionOfInformation();
    QCOMPARE(result, MergeContacts::None);
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
    const MergeContacts::ConflictInformations result = contacts.requiresManualSelectionOfInformation();
    QCOMPARE(result, MergeContacts::None);
}

void MergeContactsTest::checkNeedManualSelectionWithName_data()
{
    QTest::addColumn<QString>("nameItemA");
    QTest::addColumn<QString>("nameItemB");
    QTest::addColumn<QString>("nameItemC");
    QTest::addColumn<bool>("needManualCheck");
    QTest::newRow("noConflict") <<  QString() << QString() << QString() << false;
    QTest::newRow("noWithOneNameConflict") <<  QString() << QString() << QString(QLatin1String("foo")) << false;
    QTest::newRow("noWithOneNameConflict1") <<  QString() << QString(QLatin1String("foo")) << QString()  << false;
    QTest::newRow("noWithOneNameConflict2") <<  QString(QLatin1String("foo")) << QString() << QString() << false;
    QTest::newRow("noConflictWithSameName") <<  QString(QLatin1String("foo")) << QString(QLatin1String("foo")) << QString() << false;
    QTest::newRow("noConflictWithSameName2") <<  QString(QLatin1String("foo")) << QString(QLatin1String("foo")) << QString(QLatin1String("foo")) << false;
    QTest::newRow("conflict") <<  QString(QLatin1String("foo")) << QString(QLatin1String("foo1")) << QString(QLatin1String("foo")) << true;
    QTest::newRow("conflict1") <<  QString() << QString(QLatin1String("foo1")) << QString(QLatin1String("foo")) << true;
}

void MergeContactsTest::checkNeedManualSelectionWithName()
{
    QFETCH( QString, nameItemA );
    QFETCH( QString, nameItemB );
    QFETCH( QString, nameItemC );
    QFETCH( bool, needManualCheck );

    Akonadi::Item::List lst;
    Addressee addressA;
    Akonadi::Item itemA;
    addressA.setName(nameItemA);
    itemA.setPayload<KABC::Addressee>( addressA );
    lst<<itemA;

    Addressee addressB;
    Akonadi::Item itemB;
    addressB.setName(nameItemB);
    itemB.setPayload<KABC::Addressee>( addressB );
    lst<<itemB;

    Addressee addressC;
    Akonadi::Item itemC;
    addressC.setName(nameItemC);
    itemC.setPayload<KABC::Addressee>( addressC );
    lst<<itemC;

    MergeContacts contacts(lst);
    const MergeContacts::ConflictInformations result = contacts.requiresManualSelectionOfInformation();
    const bool bResult = (result == MergeContacts::Name);
    QCOMPARE(bResult, needManualCheck);
}

void MergeContactsTest::checkNeedManualSelectionWithNickName_data()
{
    QTest::addColumn<QString>("nameItemA");
    QTest::addColumn<QString>("nameItemB");
    QTest::addColumn<QString>("nameItemC");
    QTest::addColumn<bool>("needManualCheck");
    QTest::newRow("noConflict") <<  QString() << QString() << QString() << false;
    QTest::newRow("noWithOneNameConflict") <<  QString() << QString() << QString(QLatin1String("foo")) << false;
    QTest::newRow("noWithOneNameConflict1") <<  QString() << QString(QLatin1String("foo")) << QString()  << false;
    QTest::newRow("noWithOneNameConflict2") <<  QString(QLatin1String("foo")) << QString() << QString() << false;
    QTest::newRow("noConflictWithSameName") <<  QString(QLatin1String("foo")) << QString(QLatin1String("foo")) << QString() << false;
    QTest::newRow("noConflictWithSameName2") <<  QString(QLatin1String("foo")) << QString(QLatin1String("foo")) << QString(QLatin1String("foo")) << false;
    QTest::newRow("conflict") <<  QString(QLatin1String("foo")) << QString(QLatin1String("foo1")) << QString(QLatin1String("foo")) << true;
    QTest::newRow("conflict1") <<  QString() << QString(QLatin1String("foo1")) << QString(QLatin1String("foo")) << true;
}

void MergeContactsTest::checkNeedManualSelectionWithNickName()
{
    QFETCH( QString, nameItemA );
    QFETCH( QString, nameItemB );
    QFETCH( QString, nameItemC );
    QFETCH( bool, needManualCheck );

    Akonadi::Item::List lst;
    Addressee addressA;
    Akonadi::Item itemA;
    addressA.setNickName(nameItemA);
    itemA.setPayload<KABC::Addressee>( addressA );
    lst<<itemA;

    Addressee addressB;
    Akonadi::Item itemB;
    addressB.setNickName(nameItemB);
    itemB.setPayload<KABC::Addressee>( addressB );
    lst<<itemB;

    Addressee addressC;
    Akonadi::Item itemC;
    addressC.setNickName(nameItemC);
    itemC.setPayload<KABC::Addressee>( addressC );
    lst<<itemC;

    MergeContacts contacts(lst);
    const MergeContacts::ConflictInformations result = contacts.requiresManualSelectionOfInformation();
    const bool bResult = (result == MergeContacts::NickName);
    QCOMPARE(bResult, needManualCheck);
}

void MergeContactsTest::checkNeedManualSelectionWithOrganization_data()
{
    QTest::addColumn<QString>("nameItemA");
    QTest::addColumn<QString>("nameItemB");
    QTest::addColumn<QString>("nameItemC");
    QTest::addColumn<bool>("needManualCheck");
    QTest::newRow("noConflict") <<  QString() << QString() << QString() << false;
    QTest::newRow("noWithOneNameConflict") <<  QString() << QString() << QString(QLatin1String("foo")) << false;
    QTest::newRow("noWithOneNameConflict1") <<  QString() << QString(QLatin1String("foo")) << QString()  << false;
    QTest::newRow("noWithOneNameConflict2") <<  QString(QLatin1String("foo")) << QString() << QString() << false;
    QTest::newRow("noConflictWithSameName") <<  QString(QLatin1String("foo")) << QString(QLatin1String("foo")) << QString() << false;
    QTest::newRow("noConflictWithSameName2") <<  QString(QLatin1String("foo")) << QString(QLatin1String("foo")) << QString(QLatin1String("foo")) << false;
    QTest::newRow("conflict") <<  QString(QLatin1String("foo")) << QString(QLatin1String("foo1")) << QString(QLatin1String("foo")) << true;
    QTest::newRow("conflict1") <<  QString() << QString(QLatin1String("foo1")) << QString(QLatin1String("foo")) << true;
}

void MergeContactsTest::checkNeedManualSelectionWithOrganization()
{
    QFETCH( QString, nameItemA );
    QFETCH( QString, nameItemB );
    QFETCH( QString, nameItemC );
    QFETCH( bool, needManualCheck );

    Akonadi::Item::List lst;
    Addressee addressA;
    Akonadi::Item itemA;
    addressA.setOrganization(nameItemA);
    itemA.setPayload<KABC::Addressee>( addressA );
    lst<<itemA;

    Addressee addressB;
    Akonadi::Item itemB;
    addressB.setOrganization(nameItemB);
    itemB.setPayload<KABC::Addressee>( addressB );
    lst<<itemB;

    Addressee addressC;
    Akonadi::Item itemC;
    addressC.setOrganization(nameItemC);
    itemC.setPayload<KABC::Addressee>( addressC );
    lst<<itemC;

    MergeContacts contacts(lst);
    const MergeContacts::ConflictInformations result = contacts.requiresManualSelectionOfInformation();
    const bool bResult = (result == MergeContacts::Organization);
    QCOMPARE(bResult, needManualCheck);
}

void MergeContactsTest::checkNeedManualSelectionWithTitle_data()
{
    QTest::addColumn<QString>("nameItemA");
    QTest::addColumn<QString>("nameItemB");
    QTest::addColumn<QString>("nameItemC");
    QTest::addColumn<bool>("needManualCheck");
    QTest::newRow("noConflict") <<  QString() << QString() << QString() << false;
    QTest::newRow("noWithOneNameConflict") <<  QString() << QString() << QString(QLatin1String("foo")) << false;
    QTest::newRow("noWithOneNameConflict1") <<  QString() << QString(QLatin1String("foo")) << QString()  << false;
    QTest::newRow("noWithOneNameConflict2") <<  QString(QLatin1String("foo")) << QString() << QString() << false;
    QTest::newRow("noConflictWithSameName") <<  QString(QLatin1String("foo")) << QString(QLatin1String("foo")) << QString() << false;
    QTest::newRow("noConflictWithSameName2") <<  QString(QLatin1String("foo")) << QString(QLatin1String("foo")) << QString(QLatin1String("foo")) << false;
    QTest::newRow("conflict") <<  QString(QLatin1String("foo")) << QString(QLatin1String("foo1")) << QString(QLatin1String("foo")) << true;
    QTest::newRow("conflict1") <<  QString() << QString(QLatin1String("foo1")) << QString(QLatin1String("foo")) << true;
}

void MergeContactsTest::checkNeedManualSelectionWithTitle()
{
    QFETCH( QString, nameItemA );
    QFETCH( QString, nameItemB );
    QFETCH( QString, nameItemC );
    QFETCH( bool, needManualCheck );

    Akonadi::Item::List lst;
    Addressee addressA;
    Akonadi::Item itemA;
    addressA.setTitle(nameItemA);
    itemA.setPayload<KABC::Addressee>( addressA );
    lst<<itemA;

    Addressee addressB;
    Akonadi::Item itemB;
    addressB.setTitle(nameItemB);
    itemB.setPayload<KABC::Addressee>( addressB );
    lst<<itemB;

    Addressee addressC;
    Akonadi::Item itemC;
    addressC.setTitle(nameItemC);
    itemC.setPayload<KABC::Addressee>( addressC );
    lst<<itemC;

    MergeContacts contacts(lst);
    const MergeContacts::ConflictInformations result = contacts.requiresManualSelectionOfInformation();
    const bool bResult = (result == MergeContacts::Title);
    QCOMPARE(bResult, needManualCheck);
}

void MergeContactsTest::checkNeedManualSelectionWithFamilyName_data()
{
    QTest::addColumn<QString>("nameItemA");
    QTest::addColumn<QString>("nameItemB");
    QTest::addColumn<QString>("nameItemC");
    QTest::addColumn<bool>("needManualCheck");
    QTest::newRow("noConflict") <<  QString() << QString() << QString() << false;
    QTest::newRow("noWithOneNameConflict") <<  QString() << QString() << QString(QLatin1String("foo")) << false;
    QTest::newRow("noWithOneNameConflict1") <<  QString() << QString(QLatin1String("foo")) << QString()  << false;
    QTest::newRow("noWithOneNameConflict2") <<  QString(QLatin1String("foo")) << QString() << QString() << false;
    QTest::newRow("noConflictWithSameName") <<  QString(QLatin1String("foo")) << QString(QLatin1String("foo")) << QString() << false;
    QTest::newRow("noConflictWithSameName2") <<  QString(QLatin1String("foo")) << QString(QLatin1String("foo")) << QString(QLatin1String("foo")) << false;
    QTest::newRow("conflict") <<  QString(QLatin1String("foo")) << QString(QLatin1String("foo1")) << QString(QLatin1String("foo")) << true;
    QTest::newRow("conflict1") <<  QString() << QString(QLatin1String("foo1")) << QString(QLatin1String("foo")) << true;
}

void MergeContactsTest::checkNeedManualSelectionWithFamilyName()
{
    QFETCH( QString, nameItemA );
    QFETCH( QString, nameItemB );
    QFETCH( QString, nameItemC );
    QFETCH( bool, needManualCheck );

    Akonadi::Item::List lst;
    Addressee addressA;
    Akonadi::Item itemA;
    addressA.setFamilyName(nameItemA);
    itemA.setPayload<KABC::Addressee>( addressA );
    lst<<itemA;

    Addressee addressB;
    Akonadi::Item itemB;
    addressB.setFamilyName(nameItemB);
    itemB.setPayload<KABC::Addressee>( addressB );
    lst<<itemB;

    Addressee addressC;
    Akonadi::Item itemC;
    addressC.setFamilyName(nameItemC);
    itemC.setPayload<KABC::Addressee>( addressC );
    lst<<itemC;

    MergeContacts contacts(lst);
    const MergeContacts::ConflictInformations result = contacts.requiresManualSelectionOfInformation();
    const bool bResult = (result == MergeContacts::FamilyName);
    QCOMPARE(bResult, needManualCheck);
}


void MergeContactsTest::checkNeedManualSelectionWithDepartement_data()
{
    QTest::addColumn<QString>("nameItemA");
    QTest::addColumn<QString>("nameItemB");
    QTest::addColumn<QString>("nameItemC");
    QTest::addColumn<bool>("needManualCheck");
    QTest::newRow("noConflict") <<  QString() << QString() << QString() << false;
    QTest::newRow("noWithOneNameConflict") <<  QString() << QString() << QString(QLatin1String("foo")) << false;
    QTest::newRow("noWithOneNameConflict1") <<  QString() << QString(QLatin1String("foo")) << QString()  << false;
    QTest::newRow("noWithOneNameConflict2") <<  QString(QLatin1String("foo")) << QString() << QString() << false;
    QTest::newRow("noConflictWithSameName") <<  QString(QLatin1String("foo")) << QString(QLatin1String("foo")) << QString() << false;
    QTest::newRow("noConflictWithSameName2") <<  QString(QLatin1String("foo")) << QString(QLatin1String("foo")) << QString(QLatin1String("foo")) << false;
    QTest::newRow("conflict") <<  QString(QLatin1String("foo")) << QString(QLatin1String("foo1")) << QString(QLatin1String("foo")) << true;
    QTest::newRow("conflict1") <<  QString() << QString(QLatin1String("foo1")) << QString(QLatin1String("foo")) << true;
}

void MergeContactsTest::checkNeedManualSelectionWithDepartement()
{
    QFETCH( QString, nameItemA );
    QFETCH( QString, nameItemB );
    QFETCH( QString, nameItemC );
    QFETCH( bool, needManualCheck );

    Akonadi::Item::List lst;
    Addressee addressA;
    Akonadi::Item itemA;
    addressA.setDepartment(nameItemA);
    itemA.setPayload<KABC::Addressee>( addressA );
    lst<<itemA;

    Addressee addressB;
    Akonadi::Item itemB;
    addressB.setDepartment(nameItemB);
    itemB.setPayload<KABC::Addressee>( addressB );
    lst<<itemB;

    Addressee addressC;
    Akonadi::Item itemC;
    addressC.setDepartment(nameItemC);
    itemC.setPayload<KABC::Addressee>( addressC );
    lst<<itemC;

    MergeContacts contacts(lst);
    const MergeContacts::ConflictInformations result = contacts.requiresManualSelectionOfInformation();
    const bool bResult = (result == MergeContacts::Departement);
    QCOMPARE(bResult, needManualCheck);
}

void MergeContactsTest::checkNeedManualSelectionWithProfession_data()
{
    QTest::addColumn<QString>("nameItemA");
    QTest::addColumn<QString>("nameItemB");
    QTest::addColumn<QString>("nameItemC");
    QTest::addColumn<bool>("needManualCheck");
    QTest::newRow("noConflict") <<  QString() << QString() << QString() << false;
    QTest::newRow("noWithOneNameConflict") <<  QString() << QString() << QString(QLatin1String("foo")) << false;
    QTest::newRow("noWithOneNameConflict1") <<  QString() << QString(QLatin1String("foo")) << QString()  << false;
    QTest::newRow("noWithOneNameConflict2") <<  QString(QLatin1String("foo")) << QString() << QString() << false;
    QTest::newRow("noConflictWithSameName") <<  QString(QLatin1String("foo")) << QString(QLatin1String("foo")) << QString() << false;
    QTest::newRow("noConflictWithSameName2") <<  QString(QLatin1String("foo")) << QString(QLatin1String("foo")) << QString(QLatin1String("foo")) << false;
    QTest::newRow("conflict") <<  QString(QLatin1String("foo")) << QString(QLatin1String("foo1")) << QString(QLatin1String("foo")) << true;
    QTest::newRow("conflict1") <<  QString() << QString(QLatin1String("foo1")) << QString(QLatin1String("foo")) << true;
}

void MergeContactsTest::checkNeedManualSelectionWithProfession()
{
    QFETCH( QString, nameItemA );
    QFETCH( QString, nameItemB );
    QFETCH( QString, nameItemC );
    QFETCH( bool, needManualCheck );

    Akonadi::Item::List lst;
    Addressee addressA;
    Akonadi::Item itemA;
    addressA.insertCustom(QLatin1String("KADDRESSBOOK"), QLatin1String( "X-Profession" ), nameItemA);
    itemA.setPayload<KABC::Addressee>( addressA );
    lst<<itemA;

    Addressee addressB;
    Akonadi::Item itemB;
    addressB.insertCustom(QLatin1String("KADDRESSBOOK"), QLatin1String( "X-Profession" ), nameItemB);
    itemB.setPayload<KABC::Addressee>( addressB );
    lst<<itemB;

    Addressee addressC;
    Akonadi::Item itemC;
    addressC.insertCustom(QLatin1String("KADDRESSBOOK"), QLatin1String( "X-Profession" ), nameItemC);
    itemC.setPayload<KABC::Addressee>( addressC );
    lst<<itemC;

    MergeContacts contacts(lst);
    const MergeContacts::ConflictInformations result = contacts.requiresManualSelectionOfInformation();
    const bool bResult = (result == MergeContacts::Profession);
    QCOMPARE(bResult, needManualCheck);
}


void MergeContactsTest::checkNeedManualSelectionWithBlog_data()
{
    QTest::addColumn<QString>("nameItemA");
    QTest::addColumn<QString>("nameItemB");
    QTest::addColumn<QString>("nameItemC");
    QTest::addColumn<bool>("needManualCheck");
    QTest::newRow("noConflict") <<  QString() << QString() << QString() << false;
    QTest::newRow("noWithOneNameConflict") <<  QString() << QString() << QString(QLatin1String("foo")) << false;
    QTest::newRow("noWithOneNameConflict1") <<  QString() << QString(QLatin1String("foo")) << QString()  << false;
    QTest::newRow("noWithOneNameConflict2") <<  QString(QLatin1String("foo")) << QString() << QString() << false;
    QTest::newRow("noConflictWithSameName") <<  QString(QLatin1String("foo")) << QString(QLatin1String("foo")) << QString() << false;
    QTest::newRow("noConflictWithSameName2") <<  QString(QLatin1String("foo")) << QString(QLatin1String("foo")) << QString(QLatin1String("foo")) << false;
    QTest::newRow("conflict") <<  QString(QLatin1String("foo")) << QString(QLatin1String("foo1")) << QString(QLatin1String("foo")) << true;
    QTest::newRow("conflict1") <<  QString() << QString(QLatin1String("foo1")) << QString(QLatin1String("foo")) << true;
}

void MergeContactsTest::checkNeedManualSelectionWithBlog()
{
    QFETCH( QString, nameItemA );
    QFETCH( QString, nameItemB );
    QFETCH( QString, nameItemC );
    QFETCH( bool, needManualCheck );

    Akonadi::Item::List lst;
    Addressee addressA;
    Akonadi::Item itemA;
    addressA.insertCustom(QLatin1String("KADDRESSBOOK"), QLatin1String( "BlogFeed" ), nameItemA);
    itemA.setPayload<KABC::Addressee>( addressA );
    lst<<itemA;

    Addressee addressB;
    Akonadi::Item itemB;
    addressB.insertCustom(QLatin1String("KADDRESSBOOK"), QLatin1String( "BlogFeed" ), nameItemB);
    itemB.setPayload<KABC::Addressee>( addressB );
    lst<<itemB;

    Addressee addressC;
    Akonadi::Item itemC;
    addressC.insertCustom(QLatin1String("KADDRESSBOOK"), QLatin1String( "BlogFeed" ), nameItemC);
    itemC.setPayload<KABC::Addressee>( addressC );
    lst<<itemC;

    MergeContacts contacts(lst);
    const MergeContacts::ConflictInformations result = contacts.requiresManualSelectionOfInformation();
    const bool bResult = (result == MergeContacts::Blog);
    QCOMPARE(bResult, needManualCheck);
}

void MergeContactsTest::checkNeedManualSelectionWithHomePage_data()
{
    QTest::addColumn<KUrl>("nameItemA");
    QTest::addColumn<KUrl>("nameItemB");
    QTest::addColumn<KUrl>("nameItemC");
    QTest::addColumn<bool>("needManualCheck");
    QTest::newRow("noConflict") <<  KUrl() << KUrl() << KUrl() << false;
    QTest::newRow("noWithOneNameConflict") <<  KUrl() << KUrl() << KUrl(QLatin1String("http://www.kde.org")) << false;
    QTest::newRow("noWithOneNameConflict1") <<  KUrl() << KUrl(QLatin1String("http://www.kde.org")) << KUrl()  << false;
    QTest::newRow("noWithOneNameConflict2") <<  KUrl(QLatin1String("http://www.kde.org")) << KUrl() << KUrl() << false;
    QTest::newRow("noConflictWithSameName") <<  KUrl(QLatin1String("http://www.kde.org")) << KUrl(QLatin1String("http://www.kde.org")) << KUrl() << false;
    QTest::newRow("noConflictWithSameName2") <<  KUrl(QLatin1String("http://www.kde.org")) << KUrl(QLatin1String("http://www.kde.org")) << KUrl(QLatin1String("http://www.kde.org")) << false;
    QTest::newRow("conflictUrl") <<  KUrl(QLatin1String("http://www.kde.org")) << KUrl(QLatin1String("http://www.kde.org1")) << KUrl(QLatin1String("http://www.kde.org")) << true;
    QTest::newRow("conflict1") <<  KUrl() << KUrl(QLatin1String("http://www.kde.org1")) << KUrl(QLatin1String("http://www.kde.org")) << true;
}

void MergeContactsTest::checkNeedManualSelectionWithHomePage()
{
    QFETCH( KUrl, nameItemA );
    QFETCH( KUrl, nameItemB );
    QFETCH( KUrl, nameItemC );
    QFETCH( bool, needManualCheck );

    Akonadi::Item::List lst;
    Addressee addressA;
    Akonadi::Item itemA;
    addressA.setUrl(nameItemA);
    itemA.setPayload<KABC::Addressee>( addressA );
    lst<<itemA;

    Addressee addressB;
    Akonadi::Item itemB;
    addressB.setUrl(nameItemB);
    itemB.setPayload<KABC::Addressee>( addressB );
    lst<<itemB;

    Addressee addressC;
    Akonadi::Item itemC;
    addressC.setUrl(nameItemC);
    itemC.setPayload<KABC::Addressee>( addressC );
    lst<<itemC;

    MergeContacts contacts(lst);
    const MergeContacts::ConflictInformations result = contacts.requiresManualSelectionOfInformation();
    const bool bResult = (result == MergeContacts::HomePage);
    QCOMPARE(bResult, needManualCheck);
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

void MergeContactsTest::shouldMergeEmails_data()
{
    QTest::addColumn<QStringList>("emailsItemA");
    QTest::addColumn<QStringList>("emailsItemB");
    QTest::addColumn<QStringList>("emailsItemC");
    QTest::addColumn<QStringList>("emails");
    QTest::newRow("noEmails") <<  QStringList() << QStringList() << QStringList() << QStringList();
    QStringList lst;
    lst << QLatin1String("foo");
    lst << QLatin1String("foo1");
    lst << QLatin1String("foo2");
    QTest::newRow("copyFromOnContact") << lst << QStringList() << QStringList() << lst;
    QTest::newRow("copyFromTwoIdenticContact") << lst << lst << QStringList() << lst;
    QStringList lst2;
    lst2 << QLatin1String("foo5");
    lst2 << QLatin1String("foo6");
    lst2 << QLatin1String("foo7");

    QTest::newRow("copyFromTwoDifferentContact") << lst << lst2 << QStringList() << (QStringList() << lst << lst2);
    QStringList lst3;
    lst3 << QLatin1String("foo2");
    lst3 << lst2;
    //Identic => we will return merge between lst and lst2
    QTest::newRow("copyWithSomeIdenticEmail") << lst << lst3 << QStringList() << (QStringList() << lst << lst2);
}

void MergeContactsTest::shouldMergeEmails()
{
    QFETCH( QStringList, emailsItemA );
    QFETCH( QStringList, emailsItemB );
    QFETCH( QStringList, emailsItemC );
    QFETCH( QStringList, emails );

    Akonadi::Item::List lst;
    Addressee addressA;
    Akonadi::Item itemA;
    addressA.setName(QLatin1String("foo1"));
    addressA.setEmails(emailsItemA);
    itemA.setPayload<KABC::Addressee>( addressA );
    lst<<itemA;

    Addressee addressB;
    Akonadi::Item itemB;
    addressB.setName(QLatin1String("foo1"));
    addressB.setEmails(emailsItemB);
    itemB.setPayload<KABC::Addressee>( addressB );
    lst<<itemB;

    Addressee addressC;
    Akonadi::Item itemC;
    addressC.setName(QLatin1String("foo1"));
    addressC.setEmails(emailsItemC);
    itemC.setPayload<KABC::Addressee>( addressC );
    lst<<itemC;


    MergeContacts contacts(lst);

    const Addressee result = contacts.mergedContact();
    QCOMPARE(result.emails(), emails);
}


QTEST_KDEMAIN(MergeContactsTest, NoGUI)
