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
    grantleePrint = 0;
}

void GrantleePrintTest::shouldReturnEmptyStringWhenNotContentAndNoContacts()
{
    KAddressBookGrantlee::GrantleePrint *grantleePrint = new KAddressBookGrantlee::GrantleePrint;
    KContacts::Addressee::List lst;
    QVERIFY(grantleePrint->contactsToHtml(lst).isEmpty());
    grantleePrint->deleteLater();
    grantleePrint = 0;
}

void GrantleePrintTest::shouldReturnEmptyStringWhenAddContentWithoutContacts()
{
    KAddressBookGrantlee::GrantleePrint *grantleePrint = new KAddressBookGrantlee::GrantleePrint;
    grantleePrint->setContent(QLatin1String("foo"));
    KContacts::Addressee::List lst;

    QVERIFY(grantleePrint->contactsToHtml(lst).isEmpty());
    grantleePrint->deleteLater();
    grantleePrint = 0;
}

void GrantleePrintTest::shouldReturnStringWhenAddContentAndContacts()
{
    KAddressBookGrantlee::GrantleePrint *grantleePrint = new KAddressBookGrantlee::GrantleePrint;
    grantleePrint->setContent(QLatin1String("foo"));
    KContacts::Addressee::List lst;
    KContacts::Addressee address;
    address.setName(QLatin1String("foo1"));
    address.insertEmail(QLatin1String("foo@kde.org"), true);
    lst << address;

    QCOMPARE(grantleePrint->contactsToHtml(lst), QLatin1String("foo"));
    grantleePrint->deleteLater();
    grantleePrint = 0;
}

void GrantleePrintTest::shouldReturnEmails()
{
    KAddressBookGrantlee::GrantleePrint *grantleePrint = new KAddressBookGrantlee::GrantleePrint;
    KContacts::Addressee::List lst;
    KContacts::Addressee address;
    address.setName(QLatin1String("foo1"));
    address.insertEmail(QLatin1String("foo@kde.org"), true);
    lst << address;
    grantleePrint->setContent(QLatin1String("{% if contacts %}{% for contact in contacts %}{% if contact.name %}{{ contact.name }}{% endif %}{% endfor %}{% endif %}"));

    QCOMPARE(grantleePrint->contactsToHtml(lst), QLatin1String("foo1"));
    grantleePrint->deleteLater();
    grantleePrint = 0;
}

void GrantleePrintTest::shouldDisplayContactInfo_data()
{
    QTest::addColumn<QString>("variable");
    QTest::addColumn<QString>("result");
    QTest::newRow("name") << QString(QLatin1String("name")) << QString(QLatin1String("foo1"));
    QTest::newRow("organization") << QString(QLatin1String("organization")) << QString(QLatin1String("kde"));
    QTest::newRow("languages") << QString(QLatin1String("languages")) << QString(QLatin1String("fr"));
    QTest::newRow("note") << QString(QLatin1String("note")) << QString(QLatin1String("foo-note"));
    QTest::newRow("title") << QString(QLatin1String("title")) << QString(QLatin1String("foo-title"));
    QTest::newRow("nickName") << QString(QLatin1String("nickName")) << QString(QLatin1String("foo-nickname"));
    QTest::newRow("familyName") << QString(QLatin1String("familyName")) << QString(QLatin1String("foo-familyname"));
    QTest::newRow("role") << QString(QLatin1String("role")) << QString(QLatin1String("foo-role"));
    QTest::newRow("suffix") << QString(QLatin1String("suffix")) << QString(QLatin1String("foo-suffix"));
    QTest::newRow("prefix") << QString(QLatin1String("prefix")) << QString(QLatin1String("foo-prefix"));
    QTest::newRow("department") << QString(QLatin1String("department")) << QString(QLatin1String("foo-department"));
    QTest::newRow("office") << QString(QLatin1String("office")) << QString(QLatin1String("foo-office"));
    QTest::newRow("profesion") << QString(QLatin1String("profession")) << QString(QLatin1String("foo-profession"));
    QTest::newRow("manager") << QString(QLatin1String("manager")) << QString(QLatin1String("foo-managersname"));
    QTest::newRow("assistant") << QString(QLatin1String("assistant")) << QString(QLatin1String("foo-assistantsname"));
    QTest::newRow("spouse") << QString(QLatin1String("spouse")) << QString(QLatin1String("foo-spousesname"));
#if 0
    QString realName() const;
    QString formattedName() const;
    QString givenName() const;
    QString additionalName() const;
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
    address.setName(QLatin1String("foo1"));
    address.insertEmail(QLatin1String("foo@kde.org"), true);
    address.setOrganization(QLatin1String("kde"));
    address.insertLang(KContacts::Lang(QLatin1String("fr")));
    address.setNote(QLatin1String("foo-note"));
    address.setTitle(QLatin1String("foo-title"));
    address.setNickName(QLatin1String("foo-nickname"));
    address.setFamilyName(QLatin1String("foo-familyname"));
    address.setRole(QLatin1String("foo-role"));
    address.setSuffix(QLatin1String("foo-suffix"));
    address.setPrefix(QLatin1String("foo-prefix"));
    address.setDepartment(QLatin1String("foo-department"));
    address.insertCustom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-Office" ), QString(QLatin1String("foo-office")));
    address.insertCustom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-Profession" ) , QString(QLatin1String("foo-profession")));
    address.insertCustom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-Office" ) , QString(QLatin1String("foo-office")));
    address.insertCustom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-ManagersName" ) , QString(QLatin1String("foo-managersname")));
    address.insertCustom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-AssistantsName" ) , QString(QLatin1String("foo-assistantsname")));
    address.insertCustom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-SpousesName" ) , QString(QLatin1String("foo-spousesname")));

    lst << address;
    grantleePrint->setContent(QString::fromLatin1("{% if contacts %}{% for contact in contacts %}{% if contact.%1 %}{{ contact.%1 }}{% endif %}{% endfor %}{% endif %}").arg(variable));

    QCOMPARE(grantleePrint->contactsToHtml(lst), result);
    grantleePrint->deleteLater();
    grantleePrint = 0;
}

QTEST_MAIN(GrantleePrintTest)
