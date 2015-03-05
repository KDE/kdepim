/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "grantleeprinttest.h"
#include "../grantleeprint.h"
#include <qtest.h>
#include <KContacts/Addressee>

GrantleePrintTest::GrantleePrintTest(QObject *parent)
    : QObject(parent)
{

}

GrantleePrintTest::~GrantleePrintTest()
{

}

void GrantleePrintTest::shouldHaveDefaultValue()
{
    KAddressBookGrantlee::GrantleePrint *grantleePrint = new KAddressBookGrantlee::GrantleePrint;
    QVERIFY(grantleePrint);
    grantleePrint->deleteLater();
    grantleePrint = Q_NULLPTR;
}

void GrantleePrintTest::shouldReturnEmptyStringWhenNotContentAndNoContacts()
{
    KAddressBookGrantlee::GrantleePrint *grantleePrint = new KAddressBookGrantlee::GrantleePrint;
    KContacts::Addressee::List lst;
    QVERIFY(grantleePrint->contactsToHtml(lst).isEmpty());
    grantleePrint->deleteLater();
    grantleePrint = Q_NULLPTR;
}

void GrantleePrintTest::shouldReturnEmptyStringWhenAddContentWithoutContacts()
{
    KAddressBookGrantlee::GrantleePrint *grantleePrint = new KAddressBookGrantlee::GrantleePrint;
    grantleePrint->setContent(QStringLiteral("foo"));
    KContacts::Addressee::List lst;

    QVERIFY(grantleePrint->contactsToHtml(lst).isEmpty());
    grantleePrint->deleteLater();
    grantleePrint = Q_NULLPTR;
}

void GrantleePrintTest::shouldReturnStringWhenAddContentAndContacts()
{
    KAddressBookGrantlee::GrantleePrint *grantleePrint = new KAddressBookGrantlee::GrantleePrint;
    grantleePrint->setContent(QStringLiteral("foo"));
    KContacts::Addressee::List lst;
    KContacts::Addressee address;
    address.setName(QStringLiteral("foo1"));
    address.insertEmail(QStringLiteral("foo@kde.org"), true);
    lst << address;

    QCOMPARE(grantleePrint->contactsToHtml(lst), QStringLiteral("foo"));
    grantleePrint->deleteLater();
    grantleePrint = Q_NULLPTR;
}

void GrantleePrintTest::shouldReturnEmails()
{
    KAddressBookGrantlee::GrantleePrint *grantleePrint = new KAddressBookGrantlee::GrantleePrint;
    KContacts::Addressee::List lst;
    KContacts::Addressee address;
    address.setName(QStringLiteral("foo1"));
    address.insertEmail(QStringLiteral("foo@kde.org"), true);
    lst << address;
    grantleePrint->setContent(QStringLiteral("{% if contacts %}{% for contact in contacts %}{% if contact.name %}{{ contact.name }}{% endif %}{% endfor %}{% endif %}"));

    QCOMPARE(grantleePrint->contactsToHtml(lst), QStringLiteral("foo1"));
    grantleePrint->deleteLater();
    grantleePrint = Q_NULLPTR;
}

void GrantleePrintTest::shouldDisplayContactInfo_data()
{
    QTest::addColumn<QString>("variable");
    QTest::addColumn<QString>("result");
    QTest::newRow("name") << QString(QStringLiteral("name")) << QString(QStringLiteral("foo1"));
    QTest::newRow("organization") << QString(QStringLiteral("organization")) << QString(QStringLiteral("kde"));
    QTest::newRow("languages") << QString(QStringLiteral("languages")) << QString(QStringLiteral("fr"));
    QTest::newRow("note") << QString(QStringLiteral("note")) << QString(QStringLiteral("foo-note"));
    QTest::newRow("title") << QString(QStringLiteral("title")) << QString(QStringLiteral("foo-title"));
    QTest::newRow("nickName") << QString(QStringLiteral("nickName")) << QString(QStringLiteral("foo-nickname"));
    QTest::newRow("familyName") << QString(QStringLiteral("familyName")) << QString(QStringLiteral("foo-familyname"));
    QTest::newRow("role") << QString(QStringLiteral("role")) << QString(QStringLiteral("foo-role"));
    QTest::newRow("suffix") << QString(QStringLiteral("suffix")) << QString(QStringLiteral("foo-suffix"));
    QTest::newRow("prefix") << QString(QStringLiteral("prefix")) << QString(QStringLiteral("foo-prefix"));
    QTest::newRow("department") << QString(QStringLiteral("department")) << QString(QStringLiteral("foo-department"));
    QTest::newRow("office") << QString(QStringLiteral("office")) << QString(QStringLiteral("foo-office"));
    QTest::newRow("profesion") << QString(QStringLiteral("profession")) << QString(QStringLiteral("foo-profession"));
    QTest::newRow("manager") << QString(QStringLiteral("manager")) << QString(QStringLiteral("foo-managersname"));
    QTest::newRow("assistant") << QString(QStringLiteral("assistant")) << QString(QStringLiteral("foo-assistantsname"));
    QTest::newRow("spouse") << QString(QStringLiteral("spouse")) << QString(QStringLiteral("foo-spousesname"));
    QTest::newRow("givenname") << QString(QStringLiteral("givenName")) << QString(QStringLiteral("foo-givenname"));
    QTest::newRow("additionalname") << QString(QStringLiteral("additionalName")) << QString(QStringLiteral("foo-additionalname"));
#if 0
    QString realName() const;
    QString formattedName() const;
    QStringList emails() const;
    QString webPage() const;
    QString preferredEmail() const;
    QString birthday() const;
    QVariant addresses() const;
    QVariant phones() const;
    QString addressBookName() const;
    QVariant instantManging() const;
    QVariant geo() const;
    QString photo() const;
    QString logo() const;
    QVariant crypto() const;
#endif
}

void GrantleePrintTest::shouldDisplayContactInfo()
{
    QFETCH(QString, variable);
    QFETCH(QString, result);

    KAddressBookGrantlee::GrantleePrint *grantleePrint = new KAddressBookGrantlee::GrantleePrint;
    KContacts::Addressee::List lst;
    KContacts::Addressee address;
    address.setGivenName(QStringLiteral("foo-givenname"));
    address.setAdditionalName(QStringLiteral("foo-additionalname"));
    address.setName(QStringLiteral("foo1"));
    address.insertEmail(QStringLiteral("foo@kde.org"), true);
    address.setOrganization(QStringLiteral("kde"));
    address.insertLang(KContacts::Lang(QStringLiteral("fr")));
    address.setNote(QStringLiteral("foo-note"));
    address.setTitle(QStringLiteral("foo-title"));
    address.setNickName(QStringLiteral("foo-nickname"));
    address.setFamilyName(QStringLiteral("foo-familyname"));
    address.setRole(QStringLiteral("foo-role"));
    address.setSuffix(QStringLiteral("foo-suffix"));
    address.setPrefix(QStringLiteral("foo-prefix"));
    address.setDepartment(QStringLiteral("foo-department"));
    address.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Office"), QString(QStringLiteral("foo-office")));
    address.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Profession") , QString(QStringLiteral("foo-profession")));
    address.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Office") , QString(QStringLiteral("foo-office")));
    address.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-ManagersName") , QString(QStringLiteral("foo-managersname")));
    address.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-AssistantsName") , QString(QStringLiteral("foo-assistantsname")));
    address.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-SpousesName") , QString(QStringLiteral("foo-spousesname")));

    lst << address;
    grantleePrint->setContent(QString::fromLatin1("{% if contacts %}{% for contact in contacts %}{% if contact.%1 %}{{ contact.%1 }}{% endif %}{% endfor %}{% endif %}").arg(variable));

    QCOMPARE(grantleePrint->contactsToHtml(lst), result);
    grantleePrint->deleteLater();
    grantleePrint = Q_NULLPTR;
}

QTEST_MAIN(GrantleePrintTest)
