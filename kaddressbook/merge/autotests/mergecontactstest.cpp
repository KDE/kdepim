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
#include <qtest.h>
#include "../job/mergecontacts.h"

using namespace KABMergeContacts;
using namespace KContacts;
MergeContactsTest::MergeContactsTest()
{
}

void MergeContactsTest::shouldReturnDefaultAddressWhenNoListItem()
{
    MergeContacts contacts;
    KContacts::Addressee result = contacts.mergedContact();
    QCOMPARE(result.isEmpty(), true);
}

void MergeContactsTest::shouldReturnDefaultAddressWhenOneItem()
{
    Akonadi::Item::List lst;
    Akonadi::Item item;
    KContacts::Addressee address;
    address.setName(QStringLiteral("foo1"));
    item.setPayload<KContacts::Addressee>(address);
    lst << item;

    MergeContacts contacts(lst);
    KContacts::Addressee result = contacts.mergedContact();
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
    KContacts::Addressee address;
    Akonadi::Item item;
    address.setName(QStringLiteral("foo1"));
    item.setPayload<KContacts::Addressee>(address);
    lst << item;
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
    QTest::newRow("noWithOneNameConflict") <<  QString() << QString() << QStringLiteral("foo") << false;
    QTest::newRow("noWithOneNameConflict1") <<  QString() << QStringLiteral("foo") << QString()  << false;
    QTest::newRow("noWithOneNameConflict2") <<  QStringLiteral("foo") << QString() << QString() << false;
    QTest::newRow("noConflictWithSameName") <<  QStringLiteral("foo") << QStringLiteral("foo") << QString() << false;
    QTest::newRow("noConflictWithSameName2") <<  QStringLiteral("foo") << QStringLiteral("foo") << QStringLiteral("foo") << false;
    QTest::newRow("conflict") <<  QStringLiteral("foo") << QStringLiteral("foo1") << QStringLiteral("foo") << true;
    QTest::newRow("conflict1") <<  QString() << QStringLiteral("foo1") << QStringLiteral("foo") << true;
}

void MergeContactsTest::checkNeedManualSelectionWithName()
{
    QFETCH(QString, nameItemA);
    QFETCH(QString, nameItemB);
    QFETCH(QString, nameItemC);
    QFETCH(bool, needManualCheck);

    Akonadi::Item::List lst;
    Addressee addressA;
    Akonadi::Item itemA;
    addressA.setName(nameItemA);
    itemA.setPayload<Addressee>(addressA);
    lst << itemA;

    Addressee addressB;
    Akonadi::Item itemB;
    addressB.setName(nameItemB);
    itemB.setPayload<Addressee>(addressB);
    lst << itemB;

    Addressee addressC;
    Akonadi::Item itemC;
    addressC.setName(nameItemC);
    itemC.setPayload<Addressee>(addressC);
    lst << itemC;

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
    QTest::newRow("noWithOneNameConflict") <<  QString() << QString() << QStringLiteral("foo") << false;
    QTest::newRow("noWithOneNameConflict1") <<  QString() << QStringLiteral("foo") << QString()  << false;
    QTest::newRow("noWithOneNameConflict2") <<  QStringLiteral("foo") << QString() << QString() << false;
    QTest::newRow("noConflictWithSameName") <<  QStringLiteral("foo") << QStringLiteral("foo") << QString() << false;
    QTest::newRow("noConflictWithSameName2") <<  QStringLiteral("foo") << QStringLiteral("foo") << QStringLiteral("foo") << false;
    QTest::newRow("conflict") <<  QStringLiteral("foo") << QStringLiteral("foo1") << QStringLiteral("foo") << true;
    QTest::newRow("conflict1") <<  QString() << QStringLiteral("foo1") << QStringLiteral("foo") << true;
}

void MergeContactsTest::checkNeedManualSelectionWithNickName()
{
    QFETCH(QString, nameItemA);
    QFETCH(QString, nameItemB);
    QFETCH(QString, nameItemC);
    QFETCH(bool, needManualCheck);

    Akonadi::Item::List lst;
    Addressee addressA;
    Akonadi::Item itemA;
    addressA.setNickName(nameItemA);
    itemA.setPayload<Addressee>(addressA);
    lst << itemA;

    Addressee addressB;
    Akonadi::Item itemB;
    addressB.setNickName(nameItemB);
    itemB.setPayload<Addressee>(addressB);
    lst << itemB;

    Addressee addressC;
    Akonadi::Item itemC;
    addressC.setNickName(nameItemC);
    itemC.setPayload<Addressee>(addressC);
    lst << itemC;

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
    QTest::newRow("noWithOneNameConflict") <<  QString() << QString() << QStringLiteral("foo") << false;
    QTest::newRow("noWithOneNameConflict1") <<  QString() << QStringLiteral("foo") << QString()  << false;
    QTest::newRow("noWithOneNameConflict2") <<  QStringLiteral("foo") << QString() << QString() << false;
    QTest::newRow("noConflictWithSameName") <<  QStringLiteral("foo") << QStringLiteral("foo") << QString() << false;
    QTest::newRow("noConflictWithSameName2") <<  QStringLiteral("foo") << QStringLiteral("foo") << QStringLiteral("foo") << false;
    QTest::newRow("conflict") <<  QStringLiteral("foo") << QStringLiteral("foo1") << QStringLiteral("foo") << true;
    QTest::newRow("conflict1") <<  QString() << QStringLiteral("foo1") << QStringLiteral("foo") << true;
}

void MergeContactsTest::checkNeedManualSelectionWithOrganization()
{
    QFETCH(QString, nameItemA);
    QFETCH(QString, nameItemB);
    QFETCH(QString, nameItemC);
    QFETCH(bool, needManualCheck);

    Akonadi::Item::List lst;
    Addressee addressA;
    Akonadi::Item itemA;
    addressA.setOrganization(nameItemA);
    itemA.setPayload<Addressee>(addressA);
    lst << itemA;

    Addressee addressB;
    Akonadi::Item itemB;
    addressB.setOrganization(nameItemB);
    itemB.setPayload<Addressee>(addressB);
    lst << itemB;

    Addressee addressC;
    Akonadi::Item itemC;
    addressC.setOrganization(nameItemC);
    itemC.setPayload<Addressee>(addressC);
    lst << itemC;

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
    QTest::newRow("noWithOneNameConflict") <<  QString() << QString() << QStringLiteral("foo") << false;
    QTest::newRow("noWithOneNameConflict1") <<  QString() << QStringLiteral("foo") << QString()  << false;
    QTest::newRow("noWithOneNameConflict2") <<  QStringLiteral("foo") << QString() << QString() << false;
    QTest::newRow("noConflictWithSameName") <<  QStringLiteral("foo") << QStringLiteral("foo") << QString() << false;
    QTest::newRow("noConflictWithSameName2") <<  QStringLiteral("foo") << QStringLiteral("foo") << QStringLiteral("foo") << false;
    QTest::newRow("conflict") <<  QStringLiteral("foo") << QStringLiteral("foo1") << QStringLiteral("foo") << true;
    QTest::newRow("conflict1") <<  QString() << QStringLiteral("foo1") << QStringLiteral("foo") << true;
}

void MergeContactsTest::checkNeedManualSelectionWithTitle()
{
    QFETCH(QString, nameItemA);
    QFETCH(QString, nameItemB);
    QFETCH(QString, nameItemC);
    QFETCH(bool, needManualCheck);

    Akonadi::Item::List lst;
    Addressee addressA;
    Akonadi::Item itemA;
    addressA.setTitle(nameItemA);
    itemA.setPayload<Addressee>(addressA);
    lst << itemA;

    Addressee addressB;
    Akonadi::Item itemB;
    addressB.setTitle(nameItemB);
    itemB.setPayload<Addressee>(addressB);
    lst << itemB;

    Addressee addressC;
    Akonadi::Item itemC;
    addressC.setTitle(nameItemC);
    itemC.setPayload<Addressee>(addressC);
    lst << itemC;

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
    QTest::newRow("noWithOneNameConflict") <<  QString() << QString() << QStringLiteral("foo") << false;
    QTest::newRow("noWithOneNameConflict1") <<  QString() << QStringLiteral("foo") << QString()  << false;
    QTest::newRow("noWithOneNameConflict2") <<  QStringLiteral("foo") << QString() << QString() << false;
    QTest::newRow("noConflictWithSameName") <<  QStringLiteral("foo") << QStringLiteral("foo") << QString() << false;
    QTest::newRow("noConflictWithSameName2") <<  QStringLiteral("foo") << QStringLiteral("foo") << QStringLiteral("foo") << false;
    QTest::newRow("conflict") <<  QStringLiteral("foo") << QStringLiteral("foo1") << QStringLiteral("foo") << true;
    QTest::newRow("conflict1") <<  QString() << QStringLiteral("foo1") << QStringLiteral("foo") << true;
}

void MergeContactsTest::checkNeedManualSelectionWithFamilyName()
{
    QFETCH(QString, nameItemA);
    QFETCH(QString, nameItemB);
    QFETCH(QString, nameItemC);
    QFETCH(bool, needManualCheck);

    Akonadi::Item::List lst;
    Addressee addressA;
    Akonadi::Item itemA;
    addressA.setFamilyName(nameItemA);
    itemA.setPayload<Addressee>(addressA);
    lst << itemA;

    Addressee addressB;
    Akonadi::Item itemB;
    addressB.setFamilyName(nameItemB);
    itemB.setPayload<Addressee>(addressB);
    lst << itemB;

    Addressee addressC;
    Akonadi::Item itemC;
    addressC.setFamilyName(nameItemC);
    itemC.setPayload<Addressee>(addressC);
    lst << itemC;

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
    QTest::newRow("noWithOneNameConflict") <<  QString() << QString() << QStringLiteral("foo") << false;
    QTest::newRow("noWithOneNameConflict1") <<  QString() << QStringLiteral("foo") << QString()  << false;
    QTest::newRow("noWithOneNameConflict2") <<  QStringLiteral("foo") << QString() << QString() << false;
    QTest::newRow("noConflictWithSameName") <<  QStringLiteral("foo") << QStringLiteral("foo") << QString() << false;
    QTest::newRow("noConflictWithSameName2") <<  QStringLiteral("foo") << QStringLiteral("foo") << QStringLiteral("foo") << false;
    QTest::newRow("conflict") <<  QStringLiteral("foo") << QStringLiteral("foo1") << QStringLiteral("foo") << true;
    QTest::newRow("conflict1") <<  QString() << QStringLiteral("foo1") << QStringLiteral("foo") << true;
}

void MergeContactsTest::checkNeedManualSelectionWithDepartement()
{
    QFETCH(QString, nameItemA);
    QFETCH(QString, nameItemB);
    QFETCH(QString, nameItemC);
    QFETCH(bool, needManualCheck);

    Akonadi::Item::List lst;
    Addressee addressA;
    Akonadi::Item itemA;
    addressA.setDepartment(nameItemA);
    itemA.setPayload<Addressee>(addressA);
    lst << itemA;

    Addressee addressB;
    Akonadi::Item itemB;
    addressB.setDepartment(nameItemB);
    itemB.setPayload<Addressee>(addressB);
    lst << itemB;

    Addressee addressC;
    Akonadi::Item itemC;
    addressC.setDepartment(nameItemC);
    itemC.setPayload<Addressee>(addressC);
    lst << itemC;

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
    QTest::newRow("noWithOneNameConflict") <<  QString() << QString() << QStringLiteral("foo") << false;
    QTest::newRow("noWithOneNameConflict1") <<  QString() << QStringLiteral("foo") << QString()  << false;
    QTest::newRow("noWithOneNameConflict2") <<  QStringLiteral("foo") << QString() << QString() << false;
    QTest::newRow("noConflictWithSameName") <<  QStringLiteral("foo") << QStringLiteral("foo") << QString() << false;
    QTest::newRow("noConflictWithSameName2") <<  QStringLiteral("foo") << QStringLiteral("foo") << QStringLiteral("foo") << false;
    QTest::newRow("conflict") <<  QStringLiteral("foo") << QStringLiteral("foo1") << QStringLiteral("foo") << true;
    QTest::newRow("conflict1") <<  QString() << QStringLiteral("foo1") << QStringLiteral("foo") << true;
}

void MergeContactsTest::checkNeedManualSelectionWithProfession()
{
    QFETCH(QString, nameItemA);
    QFETCH(QString, nameItemB);
    QFETCH(QString, nameItemC);
    QFETCH(bool, needManualCheck);

    Akonadi::Item::List lst;
    Addressee addressA;
    Akonadi::Item itemA;
    addressA.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Profession"), nameItemA);
    itemA.setPayload<KContacts::Addressee>(addressA);
    lst << itemA;

    Addressee addressB;
    Akonadi::Item itemB;
    addressB.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Profession"), nameItemB);
    itemB.setPayload<KContacts::Addressee>(addressB);
    lst << itemB;

    Addressee addressC;
    Akonadi::Item itemC;
    addressC.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Profession"), nameItemC);
    itemC.setPayload<KContacts::Addressee>(addressC);
    lst << itemC;

    MergeContacts contacts(lst);
    const MergeContacts::ConflictInformations result = contacts.requiresManualSelectionOfInformation();
    const bool bResult = (result == MergeContacts::Profession);
    QCOMPARE(bResult, needManualCheck);
}

void MergeContactsTest::checkNeedManualSelectionWithOffice_data()
{
    QTest::addColumn<QString>("nameItemA");
    QTest::addColumn<QString>("nameItemB");
    QTest::addColumn<QString>("nameItemC");
    QTest::addColumn<bool>("needManualCheck");
    QTest::newRow("noConflict") <<  QString() << QString() << QString() << false;
    QTest::newRow("noWithOneNameConflict") <<  QString() << QString() << QStringLiteral("foo") << false;
    QTest::newRow("noWithOneNameConflict1") <<  QString() << QStringLiteral("foo") << QString()  << false;
    QTest::newRow("noWithOneNameConflict2") <<  QStringLiteral("foo") << QString() << QString() << false;
    QTest::newRow("noConflictWithSameName") <<  QStringLiteral("foo") << QStringLiteral("foo") << QString() << false;
    QTest::newRow("noConflictWithSameName2") <<  QStringLiteral("foo") << QStringLiteral("foo") << QStringLiteral("foo") << false;
    QTest::newRow("conflict") <<  QStringLiteral("foo") << QStringLiteral("foo1") << QStringLiteral("foo") << true;
    QTest::newRow("conflict1") <<  QString() << QStringLiteral("foo1") << QStringLiteral("foo") << true;
}

void MergeContactsTest::checkNeedManualSelectionWithOffice()
{
    QFETCH(QString, nameItemA);
    QFETCH(QString, nameItemB);
    QFETCH(QString, nameItemC);
    QFETCH(bool, needManualCheck);

    Akonadi::Item::List lst;
    Addressee addressA;
    Akonadi::Item itemA;
    addressA.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Office"), nameItemA);
    itemA.setPayload<KContacts::Addressee>(addressA);
    lst << itemA;

    Addressee addressB;
    Akonadi::Item itemB;
    addressB.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Office"), nameItemB);
    itemB.setPayload<KContacts::Addressee>(addressB);
    lst << itemB;

    Addressee addressC;
    Akonadi::Item itemC;
    addressC.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Office"), nameItemC);
    itemC.setPayload<KContacts::Addressee>(addressC);
    lst << itemC;

    MergeContacts contacts(lst);
    const MergeContacts::ConflictInformations result = contacts.requiresManualSelectionOfInformation();
    const bool bResult = (result == MergeContacts::Office);
    QCOMPARE(bResult, needManualCheck);
}

void MergeContactsTest::checkNeedManualSelectionWithManagerName_data()
{
    QTest::addColumn<QString>("nameItemA");
    QTest::addColumn<QString>("nameItemB");
    QTest::addColumn<QString>("nameItemC");
    QTest::addColumn<bool>("needManualCheck");
    QTest::newRow("noConflict") <<  QString() << QString() << QString() << false;
    QTest::newRow("noWithOneNameConflict") <<  QString() << QString() << QStringLiteral("foo") << false;
    QTest::newRow("noWithOneNameConflict1") <<  QString() << QStringLiteral("foo") << QString()  << false;
    QTest::newRow("noWithOneNameConflict2") <<  QStringLiteral("foo") << QString() << QString() << false;
    QTest::newRow("noConflictWithSameName") <<  QStringLiteral("foo") << QStringLiteral("foo") << QString() << false;
    QTest::newRow("noConflictWithSameName2") <<  QStringLiteral("foo") << QStringLiteral("foo") << QStringLiteral("foo") << false;
    QTest::newRow("conflict") <<  QStringLiteral("foo") << QStringLiteral("foo1") << QStringLiteral("foo") << true;
    QTest::newRow("conflict1") <<  QString() << QStringLiteral("foo1") << QStringLiteral("foo") << true;
}

void MergeContactsTest::checkNeedManualSelectionWithManagerName()
{
    QFETCH(QString, nameItemA);
    QFETCH(QString, nameItemB);
    QFETCH(QString, nameItemC);
    QFETCH(bool, needManualCheck);

    Akonadi::Item::List lst;
    Addressee addressA;
    Akonadi::Item itemA;
    addressA.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-ManagersName"), nameItemA);
    itemA.setPayload<KContacts::Addressee>(addressA);
    lst << itemA;

    Addressee addressB;
    Akonadi::Item itemB;
    addressB.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-ManagersName"), nameItemB);
    itemB.setPayload<KContacts::Addressee>(addressB);
    lst << itemB;

    Addressee addressC;
    Akonadi::Item itemC;
    addressC.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-ManagersName"), nameItemC);
    itemC.setPayload<KContacts::Addressee>(addressC);
    lst << itemC;

    MergeContacts contacts(lst);
    const MergeContacts::ConflictInformations result = contacts.requiresManualSelectionOfInformation();
    const bool bResult = (result == MergeContacts::ManagerName);
    QCOMPARE(bResult, needManualCheck);
}

void MergeContactsTest::checkNeedManualSelectionWithAssistantName_data()
{
    QTest::addColumn<QString>("nameItemA");
    QTest::addColumn<QString>("nameItemB");
    QTest::addColumn<QString>("nameItemC");
    QTest::addColumn<bool>("needManualCheck");
    QTest::newRow("noConflict") <<  QString() << QString() << QString() << false;
    QTest::newRow("noWithOneNameConflict") <<  QString() << QString() << QStringLiteral("foo") << false;
    QTest::newRow("noWithOneNameConflict1") <<  QString() << QStringLiteral("foo") << QString()  << false;
    QTest::newRow("noWithOneNameConflict2") <<  QStringLiteral("foo") << QString() << QString() << false;
    QTest::newRow("noConflictWithSameName") <<  QStringLiteral("foo") << QStringLiteral("foo") << QString() << false;
    QTest::newRow("noConflictWithSameName2") <<  QStringLiteral("foo") << QStringLiteral("foo") << QStringLiteral("foo") << false;
    QTest::newRow("conflict") <<  QStringLiteral("foo") << QStringLiteral("foo1") << QStringLiteral("foo") << true;
    QTest::newRow("conflict1") <<  QString() << QStringLiteral("foo1") << QStringLiteral("foo") << true;
}

void MergeContactsTest::checkNeedManualSelectionWithAssistantName()
{
    QFETCH(QString, nameItemA);
    QFETCH(QString, nameItemB);
    QFETCH(QString, nameItemC);
    QFETCH(bool, needManualCheck);

    Akonadi::Item::List lst;
    Addressee addressA;
    Akonadi::Item itemA;
    addressA.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-AssistantsName"), nameItemA);
    itemA.setPayload<KContacts::Addressee>(addressA);
    lst << itemA;

    Addressee addressB;
    Akonadi::Item itemB;
    addressB.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-AssistantsName"), nameItemB);
    itemB.setPayload<KContacts::Addressee>(addressB);
    lst << itemB;

    Addressee addressC;
    Akonadi::Item itemC;
    addressC.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-AssistantsName"), nameItemC);
    itemC.setPayload<KContacts::Addressee>(addressC);
    lst << itemC;

    MergeContacts contacts(lst);
    const MergeContacts::ConflictInformations result = contacts.requiresManualSelectionOfInformation();
    const bool bResult = (result == MergeContacts::Assistant);
    QCOMPARE(bResult, needManualCheck);
}

void MergeContactsTest::checkNeedManualSelectionWithAnniversary_data()
{
    QTest::addColumn<QString>("nameItemA");
    QTest::addColumn<QString>("nameItemB");
    QTest::addColumn<QString>("nameItemC");
    QTest::addColumn<bool>("needManualCheck");
    QTest::newRow("noConflict") <<  QString() << QString() << QString() << false;
    QTest::newRow("noWithOneNameConflict") <<  QString() << QString() << QStringLiteral("foo") << false;
    QTest::newRow("noWithOneNameConflict1") <<  QString() << QStringLiteral("foo") << QString()  << false;
    QTest::newRow("noWithOneNameConflict2") <<  QStringLiteral("foo") << QString() << QString() << false;
    QTest::newRow("noConflictWithSameName") <<  QStringLiteral("foo") << QStringLiteral("foo") << QString() << false;
    QTest::newRow("noConflictWithSameName2") <<  QStringLiteral("foo") << QStringLiteral("foo") << QStringLiteral("foo") << false;
    QTest::newRow("conflict") <<  QStringLiteral("foo") << QStringLiteral("foo1") << QStringLiteral("foo") << true;
    QTest::newRow("conflict1") <<  QString() << QStringLiteral("foo1") << QStringLiteral("foo") << true;
}

void MergeContactsTest::checkNeedManualSelectionWithAnniversary()
{
    QFETCH(QString, nameItemA);
    QFETCH(QString, nameItemB);
    QFETCH(QString, nameItemC);
    QFETCH(bool, needManualCheck);

    Akonadi::Item::List lst;
    Addressee addressA;
    Akonadi::Item itemA;
    addressA.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Anniversary"), nameItemA);
    itemA.setPayload<KContacts::Addressee>(addressA);
    lst << itemA;

    Addressee addressB;
    Akonadi::Item itemB;
    addressB.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Anniversary"), nameItemB);
    itemB.setPayload<KContacts::Addressee>(addressB);
    lst << itemB;

    Addressee addressC;
    Akonadi::Item itemC;
    addressC.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Anniversary"), nameItemC);
    itemC.setPayload<KContacts::Addressee>(addressC);
    lst << itemC;

    MergeContacts contacts(lst);
    const MergeContacts::ConflictInformations result = contacts.requiresManualSelectionOfInformation();
    const bool bResult = (result == MergeContacts::Anniversary);
    QCOMPARE(bResult, needManualCheck);
}

void MergeContactsTest::shouldMergeTitle_data()
{
    QTest::addColumn<QString>("nameItemA");
    QTest::addColumn<QString>("nameItemB");
    QTest::addColumn<QString>("nameItemC");
    QTest::addColumn<bool>("isEmpty");
    QTest::addColumn<QString>("result");
    QTest::newRow("empty") <<  QString() << QString() << QString() << true << QString();
    QTest::newRow("noWithOneNameConflict") <<  QString() << QString() << QStringLiteral("foo") << false << QStringLiteral("foo");
    QTest::newRow("noWithOneNameConflict1") <<  QString() << QStringLiteral("foo") << QString()  << false << QStringLiteral("foo");
    QTest::newRow("noWithOneNameConflict2") <<  QStringLiteral("foo") << QString() << QString() << false << QStringLiteral("foo");
    QTest::newRow("noConflictWithSameName") <<  QStringLiteral("foo") << QStringLiteral("foo") << QString() << false << QStringLiteral("foo");
    QTest::newRow("noConflictWithSameName2") <<  QStringLiteral("foo") << QStringLiteral("foo") << QStringLiteral("foo") << false << QStringLiteral("foo");
    QTest::newRow("conflict") <<  QStringLiteral("foo") << QStringLiteral("foo1") << QStringLiteral("foo") << false << QStringLiteral("foo");
    QTest::newRow("conflict1") <<  QString() << QStringLiteral("foo1") << QStringLiteral("foo") << false << QStringLiteral("foo1");
}

void MergeContactsTest::shouldMergeTitle()
{
    QFETCH(QString, nameItemA);
    QFETCH(QString, nameItemB);
    QFETCH(QString, nameItemC);
    QFETCH(bool, isEmpty);
    QFETCH(QString, result);

    Akonadi::Item::List lst;
    Addressee addressA;
    addressA.setTitle(nameItemA);
    Akonadi::Item itemA;
    itemA.setPayload<Addressee>(addressA);
    lst << itemA;

    Addressee addressB;
    Akonadi::Item itemB;
    addressB.setTitle(nameItemB);
    itemB.setPayload<Addressee>(addressB);
    lst << itemB;

    Addressee addressC;
    Akonadi::Item itemC;
    addressC.setTitle(nameItemC);
    itemC.setPayload<Addressee>(addressC);
    lst << itemC;

    MergeContacts contacts(lst);
    KContacts::Addressee resultAddr = contacts.mergedContact();
    QCOMPARE(resultAddr.isEmpty(), isEmpty);
    QCOMPARE(resultAddr.title(), result);
}

void MergeContactsTest::shouldMergeDepartement_data()
{
    QTest::addColumn<QString>("nameItemA");
    QTest::addColumn<QString>("nameItemB");
    QTest::addColumn<QString>("nameItemC");
    QTest::addColumn<bool>("isEmpty");
    QTest::addColumn<QString>("result");
    QTest::newRow("empty") <<  QString() << QString() << QString() << true << QString();
    QTest::newRow("noWithOneNameConflict") <<  QString() << QString() << QStringLiteral("foo") << false << QStringLiteral("foo");
    QTest::newRow("noWithOneNameConflict1") <<  QString() << QStringLiteral("foo") << QString()  << false << QStringLiteral("foo");
    QTest::newRow("noWithOneNameConflict2") <<  QStringLiteral("foo") << QString() << QString() << false << QStringLiteral("foo");
    QTest::newRow("noConflictWithSameName") <<  QStringLiteral("foo") << QStringLiteral("foo") << QString() << false << QStringLiteral("foo");
    QTest::newRow("noConflictWithSameName2") <<  QStringLiteral("foo") << QStringLiteral("foo") << QStringLiteral("foo") << false << QStringLiteral("foo");
    QTest::newRow("conflict") <<  QStringLiteral("foo") << QStringLiteral("foo1") << QStringLiteral("foo") << false << QStringLiteral("foo");
    QTest::newRow("conflict1") <<  QString() << QStringLiteral("foo1") << QStringLiteral("foo") << false << QStringLiteral("foo1");
}

void MergeContactsTest::shouldMergeDepartement()
{
    QFETCH(QString, nameItemA);
    QFETCH(QString, nameItemB);
    QFETCH(QString, nameItemC);
    QFETCH(bool, isEmpty);
    QFETCH(QString, result);

    Akonadi::Item::List lst;
    Addressee addressA;
    addressA.setDepartment(nameItemA);
    Akonadi::Item itemA;
    itemA.setPayload<Addressee>(addressA);
    lst << itemA;

    Addressee addressB;
    Akonadi::Item itemB;
    addressB.setDepartment(nameItemB);
    itemB.setPayload<Addressee>(addressB);
    lst << itemB;

    Addressee addressC;
    Akonadi::Item itemC;
    addressC.setDepartment(nameItemC);
    itemC.setPayload<Addressee>(addressC);
    lst << itemC;

    MergeContacts contacts(lst);
    KContacts::Addressee resultAddr = contacts.mergedContact();
    QCOMPARE(resultAddr.isEmpty(), isEmpty);
    QCOMPARE(resultAddr.department(), result);
}

void MergeContactsTest::checkNeedManualSelectionWithPartnersName_data()
{
    QTest::addColumn<QString>("nameItemA");
    QTest::addColumn<QString>("nameItemB");
    QTest::addColumn<QString>("nameItemC");
    QTest::addColumn<bool>("needManualCheck");
    QTest::newRow("noConflict") <<  QString() << QString() << QString() << false;
    QTest::newRow("noWithOneNameConflict") <<  QString() << QString() << QStringLiteral("foo") << false;
    QTest::newRow("noWithOneNameConflict1") <<  QString() << QStringLiteral("foo") << QString()  << false;
    QTest::newRow("noWithOneNameConflict2") <<  QStringLiteral("foo") << QString() << QString() << false;
    QTest::newRow("noConflictWithSameName") <<  QStringLiteral("foo") << QStringLiteral("foo") << QString() << false;
    QTest::newRow("noConflictWithSameName2") <<  QStringLiteral("foo") << QStringLiteral("foo") << QStringLiteral("foo") << false;
    QTest::newRow("conflict") <<  QStringLiteral("foo") << QStringLiteral("foo1") << QStringLiteral("foo") << true;
    QTest::newRow("conflict1") <<  QString() << QStringLiteral("foo1") << QStringLiteral("foo") << true;
}

void MergeContactsTest::checkNeedManualSelectionWithPartnersName()
{
    QFETCH(QString, nameItemA);
    QFETCH(QString, nameItemB);
    QFETCH(QString, nameItemC);
    QFETCH(bool, needManualCheck);

    Akonadi::Item::List lst;
    Addressee addressA;
    Akonadi::Item itemA;
    addressA.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-SpousesName"), nameItemA);
    itemA.setPayload<KContacts::Addressee>(addressA);
    lst << itemA;

    Addressee addressB;
    Akonadi::Item itemB;
    addressB.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-SpousesName"), nameItemB);
    itemB.setPayload<KContacts::Addressee>(addressB);
    lst << itemB;

    Addressee addressC;
    Akonadi::Item itemC;
    addressC.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-SpousesName"), nameItemC);
    itemC.setPayload<KContacts::Addressee>(addressC);
    lst << itemC;

    MergeContacts contacts(lst);
    const MergeContacts::ConflictInformations result = contacts.requiresManualSelectionOfInformation();
    const bool bResult = (result == MergeContacts::PartnerName);
    QCOMPARE(bResult, needManualCheck);
}

void MergeContactsTest::checkNeedManualSelectionWithBlog_data()
{
    QTest::addColumn<QString>("nameItemA");
    QTest::addColumn<QString>("nameItemB");
    QTest::addColumn<QString>("nameItemC");
    QTest::addColumn<bool>("needManualCheck");
    QTest::newRow("noConflict") <<  QString() << QString() << QString() << false;
    QTest::newRow("noWithOneNameConflict") <<  QString() << QString() << QStringLiteral("foo") << false;
    QTest::newRow("noWithOneNameConflict1") <<  QString() << QStringLiteral("foo") << QString()  << false;
    QTest::newRow("noWithOneNameConflict2") <<  QStringLiteral("foo") << QString() << QString() << false;
    QTest::newRow("noConflictWithSameName") <<  QStringLiteral("foo") << QStringLiteral("foo") << QString() << false;
    QTest::newRow("noConflictWithSameName2") <<  QStringLiteral("foo") << QStringLiteral("foo") << QStringLiteral("foo") << false;
    QTest::newRow("conflict") <<  QStringLiteral("foo") << QStringLiteral("foo1") << QStringLiteral("foo") << true;
    QTest::newRow("conflict1") <<  QString() << QStringLiteral("foo1") << QStringLiteral("foo") << true;
}

void MergeContactsTest::checkNeedManualSelectionWithBlog()
{
    QFETCH(QString, nameItemA);
    QFETCH(QString, nameItemB);
    QFETCH(QString, nameItemC);
    QFETCH(bool, needManualCheck);

    Akonadi::Item::List lst;
    Addressee addressA;
    Akonadi::Item itemA;
    addressA.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("BlogFeed"), nameItemA);
    itemA.setPayload<KContacts::Addressee>(addressA);
    lst << itemA;

    Addressee addressB;
    Akonadi::Item itemB;
    addressB.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("BlogFeed"), nameItemB);
    itemB.setPayload<KContacts::Addressee>(addressB);
    lst << itemB;

    Addressee addressC;
    Akonadi::Item itemC;
    addressC.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("BlogFeed"), nameItemC);
    itemC.setPayload<KContacts::Addressee>(addressC);
    lst << itemC;

    MergeContacts contacts(lst);
    const MergeContacts::ConflictInformations result = contacts.requiresManualSelectionOfInformation();
    const bool bResult = (result == MergeContacts::Blog);
    QCOMPARE(bResult, needManualCheck);
}

void MergeContactsTest::checkNeedManualSelectionWithHomePage_data()
{
    QTest::addColumn<QUrl>("nameItemA");
    QTest::addColumn<QUrl>("nameItemB");
    QTest::addColumn<QUrl>("nameItemC");
    QTest::addColumn<bool>("needManualCheck");
    QTest::newRow("noConflict") <<  QUrl() << QUrl() << QUrl() << false;
    QTest::newRow("noWithOneNameConflict") <<  QUrl() << QUrl() << QUrl(QStringLiteral("http://www.kde.org")) << false;
    QTest::newRow("noWithOneNameConflict1") <<  QUrl() << QUrl(QStringLiteral("http://www.kde.org")) << QUrl()  << false;
    QTest::newRow("noWithOneNameConflict2") <<  QUrl(QStringLiteral("http://www.kde.org")) << QUrl() << QUrl() << false;
    QTest::newRow("noConflictWithSameName") <<  QUrl(QStringLiteral("http://www.kde.org")) << QUrl(QStringLiteral("http://www.kde.org")) << QUrl() << false;
    QTest::newRow("noConflictWithSameName2") <<  QUrl(QStringLiteral("http://www.kde.org")) << QUrl(QStringLiteral("http://www.kde.org")) << QUrl(QStringLiteral("http://www.kde.org")) << false;
    QTest::newRow("conflictUrl") <<  QUrl(QStringLiteral("http://www.kde.org")) << QUrl(QStringLiteral("http://www.kde.org1")) << QUrl(QStringLiteral("http://www.kde.org")) << true;
    QTest::newRow("conflict1") <<  QUrl() << QUrl(QStringLiteral("http://www.kde.org1")) << QUrl(QStringLiteral("http://www.kde.org")) << true;
}

void MergeContactsTest::checkNeedManualSelectionWithHomePage()
{
    QFETCH(QUrl, nameItemA);
    QFETCH(QUrl, nameItemB);
    QFETCH(QUrl, nameItemC);
    QFETCH(bool, needManualCheck);

    Akonadi::Item::List lst;
    Addressee addressA;
    Akonadi::Item itemA;
    KContacts::ResourceLocatorUrl url;
    url.setUrl(nameItemA);
    addressA.setUrl(url);
    itemA.setPayload<Addressee>(addressA);
    lst << itemA;

    Addressee addressB;
    Akonadi::Item itemB;
    url.setUrl(nameItemB);
    addressB.setUrl(url);
    itemB.setPayload<Addressee>(addressB);
    lst << itemB;

    Addressee addressC;
    Akonadi::Item itemC;
    url.setUrl(nameItemC);
    addressC.setUrl(url);
    itemC.setPayload<Addressee>(addressC);
    lst << itemC;

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
    QTest::newRow("oneNotes") <<  QStringLiteral("one") << QString() << QString() << QStringLiteral("one");
    QTest::newRow("twoNotes") <<  QString() << QStringLiteral("one") << QStringLiteral("one") << QStringLiteral("one\none");
    QTest::newRow("threeNotes") <<  QStringLiteral("one") << QStringLiteral("one") << QStringLiteral("one") << QStringLiteral("one\none\none");
}

void MergeContactsTest::shouldMergeNotes()
{
    QFETCH(QString, noteItemA);
    QFETCH(QString, noteItemB);
    QFETCH(QString, noteItemC);
    QFETCH(QString, note);

    Akonadi::Item::List lst;
    Addressee addressA;
    Akonadi::Item itemA;
    addressA.setName(QStringLiteral("foo1"));
    addressA.setNote(noteItemA);
    itemA.setPayload<Addressee>(addressA);
    lst << itemA;

    Addressee addressB;
    Akonadi::Item itemB;
    addressB.setName(QStringLiteral("foo1"));
    addressB.setNote(noteItemB);
    itemB.setPayload<Addressee>(addressB);
    lst << itemB;

    Addressee addressC;
    Akonadi::Item itemC;
    addressC.setName(QStringLiteral("foo1"));
    addressC.setNote(noteItemC);
    itemC.setPayload<Addressee>(addressC);
    lst << itemC;

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
    lst << QStringLiteral("foo");
    lst << QStringLiteral("foo1");
    lst << QStringLiteral("foo2");
    QTest::newRow("copyFromOnContact") << lst << QStringList() << QStringList() << lst;
    QTest::newRow("copyFromTwoIdenticContact") << lst << lst << QStringList() << lst;
    QStringList lst2;
    lst2 << QStringLiteral("foo5");
    lst2 << QStringLiteral("foo6");
    lst2 << QStringLiteral("foo7");

    QTest::newRow("copyFromTwoDifferentContact") << lst << lst2 << QStringList() << (QStringList() << lst << lst2);
    QStringList lst3;
    lst3 << QStringLiteral("foo2");
    lst3 << lst2;
    //Identic => we will return merge between lst and lst2
    QTest::newRow("copyWithSomeIdenticEmail") << lst << lst3 << QStringList() << (QStringList() << lst << lst2);
}

void MergeContactsTest::shouldMergeEmails()
{
    QFETCH(QStringList, emailsItemA);
    QFETCH(QStringList, emailsItemB);
    QFETCH(QStringList, emailsItemC);
    QFETCH(QStringList, emails);

    Akonadi::Item::List lst;
    Addressee addressA;
    Akonadi::Item itemA;
    addressA.setName(QStringLiteral("foo1"));
    addressA.setEmails(emailsItemA);
    itemA.setPayload<Addressee>(addressA);
    lst << itemA;

    Addressee addressB;
    Akonadi::Item itemB;
    addressB.setName(QStringLiteral("foo1"));
    addressB.setEmails(emailsItemB);
    itemB.setPayload<Addressee>(addressB);
    lst << itemB;

    Addressee addressC;
    Akonadi::Item itemC;
    addressC.setName(QStringLiteral("foo1"));
    addressC.setEmails(emailsItemC);
    itemC.setPayload<Addressee>(addressC);
    lst << itemC;

    MergeContacts contacts(lst);

    const Addressee result = contacts.mergedContact();
    QCOMPARE(result.emails(), emails);
}

void MergeContactsTest::shouldMergeFamilyname_data()
{
    QTest::addColumn<QString>("nameItemA");
    QTest::addColumn<QString>("nameItemB");
    QTest::addColumn<QString>("nameItemC");
    QTest::addColumn<bool>("isEmpty");
    QTest::addColumn<QString>("result");
    QTest::newRow("empty") <<  QString() << QString() << QString() << true << QString();
    QTest::newRow("noWithOneNameConflict") <<  QString() << QString() << QStringLiteral("foo") << false << QStringLiteral("foo");
    QTest::newRow("noWithOneNameConflict1") <<  QString() << QStringLiteral("foo") << QString()  << false << QStringLiteral("foo");
    QTest::newRow("noWithOneNameConflict2") <<  QStringLiteral("foo") << QString() << QString() << false << QStringLiteral("foo");
    QTest::newRow("noConflictWithSameName") <<  QStringLiteral("foo") << QStringLiteral("foo") << QString() << false << QStringLiteral("foo");
    QTest::newRow("noConflictWithSameName2") <<  QStringLiteral("foo") << QStringLiteral("foo") << QStringLiteral("foo") << false << QStringLiteral("foo");
    QTest::newRow("conflict") <<  QStringLiteral("foo") << QStringLiteral("foo1") << QStringLiteral("foo") << false << QStringLiteral("foo");
    QTest::newRow("conflict1") <<  QString() << QStringLiteral("foo1") << QStringLiteral("foo") << false << QStringLiteral("foo1");
}

void MergeContactsTest::shouldMergeFamilyname()
{
    QFETCH(QString, nameItemA);
    QFETCH(QString, nameItemB);
    QFETCH(QString, nameItemC);
    QFETCH(bool, isEmpty);
    QFETCH(QString, result);

    Akonadi::Item::List lst;
    Addressee addressA;
    addressA.setFamilyName(nameItemA);
    Akonadi::Item itemA;
    itemA.setPayload<Addressee>(addressA);
    lst << itemA;

    Addressee addressB;
    Akonadi::Item itemB;
    addressB.setFamilyName(nameItemB);
    itemB.setPayload<Addressee>(addressB);
    lst << itemB;

    Addressee addressC;
    Akonadi::Item itemC;
    addressC.setFamilyName(nameItemC);
    itemC.setPayload<Addressee>(addressC);
    lst << itemC;

    MergeContacts contacts(lst);
    Addressee resultAddr = contacts.mergedContact();
    QCOMPARE(resultAddr.isEmpty(), isEmpty);
    QCOMPARE(resultAddr.familyName(), result);
}

void MergeContactsTest::shouldMergeHomePage_data()
{
    QTest::addColumn<QUrl>("nameItemA");
    QTest::addColumn<QUrl>("nameItemB");
    QTest::addColumn<QUrl>("nameItemC");
    QTest::addColumn<bool>("isEmpty");
    QTest::addColumn<QUrl>("result");
    QTest::newRow("noConflict") <<  QUrl() << QUrl() << QUrl() << true << QUrl();
    QTest::newRow("noWithOneNameConflict") <<  QUrl() << QUrl() << QUrl(QStringLiteral("http://www.kde.org")) << false << QUrl(QStringLiteral("http://www.kde.org"));
    QTest::newRow("noWithOneNameConflict1") <<  QUrl() << QUrl(QStringLiteral("http://www.kde.org")) << QUrl() << false << QUrl(QStringLiteral("http://www.kde.org"));
    QTest::newRow("noWithOneNameConflict2") <<  QUrl(QStringLiteral("http://www.kde.org")) << QUrl() << QUrl() << false << QUrl(QStringLiteral("http://www.kde.org"));
    QTest::newRow("noConflictWithSameName") <<  QUrl(QStringLiteral("http://www.kde.org")) << QUrl(QStringLiteral("http://www.kde.org")) << QUrl() << false << QUrl(QStringLiteral("http://www.kde.org"));
    QTest::newRow("noConflictWithSameName2") <<  QUrl(QStringLiteral("http://www.kde.org")) << QUrl(QStringLiteral("http://www.kde.org")) << QUrl(QStringLiteral("http://www.kde.org")) << false << QUrl(QStringLiteral("http://www.kde.org"));
    QTest::newRow("conflictUrl") <<  QUrl(QStringLiteral("http://www.kde.org")) << QUrl(QStringLiteral("http://www.kde.org1")) << QUrl(QStringLiteral("http://www.kde.org")) << false << QUrl(QStringLiteral("http://www.kde.org"));
    QTest::newRow("conflict1") <<  QUrl() << QUrl(QStringLiteral("http://www.kde.org1")) << QUrl(QStringLiteral("http://www.kde.org")) << false << QUrl(QStringLiteral("http://www.kde.org1"));
}

void MergeContactsTest::shouldMergeBlogFeed_data()
{
    QTest::addColumn<QString>("nameItemA");
    QTest::addColumn<QString>("nameItemB");
    QTest::addColumn<QString>("nameItemC");
    QTest::addColumn<bool>("isEmpty");
    QTest::addColumn<QString>("result");
    QTest::newRow("empty") <<  QString() << QString() << QString() << true << QString();
    QTest::newRow("noWithOneNameConflict") <<  QString() << QString() << QStringLiteral("foo") << false << QStringLiteral("foo");
    QTest::newRow("noWithOneNameConflict1") <<  QString() << QStringLiteral("foo") << QString()  << false << QStringLiteral("foo");
    QTest::newRow("noWithOneNameConflict2") <<  QStringLiteral("foo") << QString() << QString() << false << QStringLiteral("foo");
    QTest::newRow("noConflictWithSameName") <<  QStringLiteral("foo") << QStringLiteral("foo") << QString() << false << QStringLiteral("foo");
    QTest::newRow("noConflictWithSameName2") <<  QStringLiteral("foo") << QStringLiteral("foo") << QStringLiteral("foo") << false << QStringLiteral("foo");
    QTest::newRow("conflict") <<  QStringLiteral("foo") << QStringLiteral("foo1") << QStringLiteral("foo") << false << QStringLiteral("foo");
    QTest::newRow("conflict1") <<  QString() << QStringLiteral("foo1") << QStringLiteral("foo") << false << QStringLiteral("foo1");
}

void MergeContactsTest::shouldMergeBlogFeed()
{
    QFETCH(QString, nameItemA);
    QFETCH(QString, nameItemB);
    QFETCH(QString, nameItemC);
    QFETCH(bool, isEmpty);
    QFETCH(QString, result);

    const QString valueCustomStr = QStringLiteral("BlogFeed");
    Akonadi::Item::List lst;
    Addressee addressA;
    addressA.insertCustom(QStringLiteral("KADDRESSBOOK"), valueCustomStr, nameItemA);
    Akonadi::Item itemA;
    itemA.setPayload<Addressee>(addressA);
    lst << itemA;

    Addressee addressB;
    Akonadi::Item itemB;
    addressB.insertCustom(QStringLiteral("KADDRESSBOOK"), valueCustomStr, nameItemB);
    itemB.setPayload<Addressee>(addressB);
    lst << itemB;

    Addressee addressC;
    Akonadi::Item itemC;
    addressC.insertCustom(QStringLiteral("KADDRESSBOOK"), valueCustomStr, nameItemC);
    itemC.setPayload<Addressee>(addressC);
    lst << itemC;

    MergeContacts contacts(lst);
    Addressee resultAddr = contacts.mergedContact();
    QCOMPARE(resultAddr.isEmpty(), isEmpty);
    const QString resultStr = resultAddr.custom(QStringLiteral("KADDRESSBOOK"), valueCustomStr);
    QCOMPARE(resultStr, result);
}

void MergeContactsTest::shouldMergeHomePage()
{
    QFETCH(QUrl, nameItemA);
    QFETCH(QUrl, nameItemB);
    QFETCH(QUrl, nameItemC);
    QFETCH(bool, isEmpty);
    QFETCH(QUrl, result);
    Akonadi::Item::List lst;
    Addressee addressA;
    Akonadi::Item itemA;
    KContacts::ResourceLocatorUrl url;
    url.setUrl(nameItemA);
    addressA.setUrl(url);
    itemA.setPayload<KContacts::Addressee>(addressA);
    lst << itemA;

    Addressee addressB;
    Akonadi::Item itemB;

    url.setUrl(nameItemB);
    addressB.setUrl(url);
    itemB.setPayload<KContacts::Addressee>(addressB);
    lst << itemB;

    Addressee addressC;
    Akonadi::Item itemC;
    url.setUrl(nameItemC);
    addressC.setUrl(url);
    itemC.setPayload<KContacts::Addressee>(addressC);
    lst << itemC;

    MergeContacts contacts(lst);
    Addressee resultAddr = contacts.mergedContact();
    QCOMPARE(resultAddr.isEmpty(), isEmpty);
    QCOMPARE(resultAddr.url().url(), result);
}

void MergeContactsTest::checkNeedManualSelectionWithBirthday_data()
{
    QTest::addColumn<QString>("nameItemA");
    QTest::addColumn<QString>("nameItemB");
    QTest::addColumn<QString>("nameItemC");
    QTest::addColumn<bool>("needManualCheck");
    QTest::newRow("noConflict") <<  QString() << QString() << QString() << false;
    QTest::newRow("noWithOneNameConflict") <<  QString() << QString() << QStringLiteral("20150606") << false;
    QTest::newRow("noWithOneNameConflict1") <<  QString() << QStringLiteral("20150606") << QString()  << false;
    QTest::newRow("noWithOneNameConflict2") <<  QStringLiteral("20150606") << QString() << QString() << false;
    QTest::newRow("noConflictWithSameName") <<  QStringLiteral("20150606") << QStringLiteral("20150606") << QString() << false;
    QTest::newRow("noConflictWithSameName2") <<  QStringLiteral("20150606") << QStringLiteral("20150606") << QStringLiteral("20150606") << false;
    QTest::newRow("conflict") <<  QStringLiteral("20150606") << QStringLiteral("20150608") << QStringLiteral("20150606") << true;
    QTest::newRow("conflict1") <<  QString() << QStringLiteral("20150606") << QStringLiteral("20150608") << true;
}

void MergeContactsTest::checkNeedManualSelectionWithBirthday()
{
    QFETCH(QString, nameItemA);
    QFETCH(QString, nameItemB);
    QFETCH(QString, nameItemC);
    QFETCH(bool, needManualCheck);

    Akonadi::Item::List lst;
    Addressee addressA;
    Akonadi::Item itemA;
    QDate date = QDate::fromString(nameItemA, QStringLiteral("yyyyMMdd"));
    QDateTime dt(date);
    addressA.setBirthday(dt);
    itemA.setPayload<Addressee>(addressA);
    lst << itemA;

    Addressee addressB;
    Akonadi::Item itemB;
    date = QDate::fromString(nameItemB, QStringLiteral("yyyyMMdd"));
    dt = QDateTime(date);
    addressB.setBirthday(dt);
    itemB.setPayload<Addressee>(addressB);
    lst << itemB;

    Addressee addressC;
    Akonadi::Item itemC;
    date = QDate::fromString(nameItemC, QStringLiteral("yyyyMMdd"));
    dt = QDateTime(date);
    addressC.setBirthday(dt);
    itemC.setPayload<Addressee>(addressC);
    lst << itemC;

    MergeContacts contacts(lst);
    const MergeContacts::ConflictInformations result = contacts.requiresManualSelectionOfInformation();
    const bool bResult = (result == MergeContacts::Birthday);
    QCOMPARE(bResult, needManualCheck);
}

QTEST_MAIN(MergeContactsTest)
