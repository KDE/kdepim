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

#include "searchruledatetest.h"
#include <qtest.h>
#include "../searchrule/searchruledate.h"
#include <KMime/Message>
Q_DECLARE_METATYPE(MailCommon::SearchRule::Function)
SearchRuleDateTest::SearchRuleDateTest(QObject *parent)
    : QObject(parent)
{

}

void SearchRuleDateTest::shouldHaveDefaultValue()
{
    MailCommon::SearchRuleDate searchrule;
    QCOMPARE(searchrule.field(), QByteArray());
    QCOMPARE(searchrule.function(), MailCommon::SearchRule::FuncContains);
    QVERIFY(searchrule.contents().isEmpty());
    QVERIFY(searchrule.isEmpty());
}

void SearchRuleDateTest::shouldRequiresPart()
{
    MailCommon::SearchRuleDate searchrule;
    QCOMPARE(searchrule.requiredPart(), MailCommon::SearchRule::Envelope);
}

void SearchRuleDateTest::shouldBeEmpty()
{
    MailCommon::SearchRuleDate searchrule("<date>", MailCommon::SearchRule::FuncEquals, QString());
    QVERIFY(searchrule.isEmpty());

    MailCommon::SearchRuleDate searchrule2("<date>", MailCommon::SearchRule::FuncEquals, QDate(2015, 5, 5).toString(Qt::ISODate));
    QVERIFY(!searchrule2.isEmpty());
}

void SearchRuleDateTest::shouldMatchDate_data()
{
    QTest::addColumn<MailCommon::SearchRule::Function>("function");
    QTest::addColumn<QDate>("maildate");
    QTest::addColumn<QDate>("matchdate");
    QTest::addColumn<bool>("match");
    QTest::newRow("equaldontmatch") << MailCommon::SearchRule::FuncEquals  << QDate(2015, 5, 5) << QDate(2015, 5, 6) << false;
    QTest::newRow("equalmatch") << MailCommon::SearchRule::FuncEquals  << QDate(2015, 5, 5) << QDate(2015, 5, 5) << true;
    QTest::newRow("notequalnotmatch") << MailCommon::SearchRule::FuncNotEqual  << QDate(2015, 5, 5) << QDate(2015, 5, 5) << false;
    QTest::newRow("notequalmatch") << MailCommon::SearchRule::FuncNotEqual  << QDate(2015, 5, 5) << QDate(2015, 5, 6) << true;

    QTest::newRow("isgreatermatch") << MailCommon::SearchRule::FuncIsGreater  << QDate(2015, 5, 5) << QDate(2015, 5, 4) << true;
    QTest::newRow("isgreaternotmatch") << MailCommon::SearchRule::FuncIsGreater  << QDate(2015, 5, 5) << QDate(2015, 5, 6) << false;
    QTest::newRow("isgreaternotmatchequalvalue") << MailCommon::SearchRule::FuncIsGreater  << QDate(2015, 5, 5) << QDate(2015, 5, 5) << false;

    QTest::newRow("islessorequalmatch") << MailCommon::SearchRule::FuncIsLessOrEqual  << QDate(2015, 5, 5) << QDate(2015, 5, 6) << true;
    QTest::newRow("islessorequalmatch equal") << MailCommon::SearchRule::FuncIsLessOrEqual  << QDate(2015, 5, 5) << QDate(2015, 5, 5) << true;
    QTest::newRow("islessorequalnotmatch") << MailCommon::SearchRule::FuncIsLessOrEqual  << QDate(2015, 5, 7) << QDate(2015, 5, 5) << false;

    QTest::newRow("islessmatch") << MailCommon::SearchRule::FuncIsLess  << QDate(2015, 5, 4) << QDate(2015, 5, 5) << true;
    QTest::newRow("islessnotmatch") << MailCommon::SearchRule::FuncIsLess  << QDate(2015, 5, 5) << QDate(2015, 5, 4) << false;
    QTest::newRow("islessnotmatchequalvalue") << MailCommon::SearchRule::FuncIsLess  << QDate(2015, 5, 5) << QDate(2015, 5, 5) << false;

    QTest::newRow("isgreaterorequalmatch") << MailCommon::SearchRule::FuncIsGreaterOrEqual  << QDate(2015, 5, 6) << QDate(2015, 5, 5) << true;
    QTest::newRow("isgreaterorequalmatch equal") << MailCommon::SearchRule::FuncIsGreaterOrEqual  << QDate(2015, 5, 5) << QDate(2015, 5, 5) << true;
    QTest::newRow("isgreaterorequalnotmatch") << MailCommon::SearchRule::FuncIsGreaterOrEqual  << QDate(2015, 5, 5) << QDate(2015, 5, 7) << false;
}

void SearchRuleDateTest::shouldMatchDate()
{
    QFETCH(MailCommon::SearchRule::Function, function);
    QFETCH(QDate, maildate);
    QFETCH(QDate, matchdate);
    QFETCH(bool, match);
    MailCommon::SearchRuleDate searchrule("<date>", function, matchdate.toString(Qt::ISODate));

    KMime::Message::Ptr msgPtr = KMime::Message::Ptr(new KMime::Message());
    msgPtr->date(true)->setDateTime(QDateTime(maildate));
    Akonadi::Item item;
    item.setPayload<KMime::Message::Ptr>(msgPtr);
    QCOMPARE(searchrule.matches(item), match);
}

QTEST_MAIN(SearchRuleDateTest)
