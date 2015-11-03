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

#include "searchrulenumericaltest.h"
#include <qtest.h>
#include "../searchrule/searchrulenumerical.h"
SearchRuleNumericalTest::SearchRuleNumericalTest(QObject *parent)
    : QObject(parent)
{

}

void SearchRuleNumericalTest::shouldHaveDefaultValue()
{
    MailCommon::SearchRuleNumerical ruleStatus;
    QVERIFY(ruleStatus.contents().isEmpty());
    QVERIFY(ruleStatus.field().isEmpty());
    QCOMPARE(ruleStatus.function(), MailCommon::SearchRule::FuncContains);
    QVERIFY(ruleStatus.isEmpty());
}

void SearchRuleNumericalTest::shouldBeEmpty()
{
    MailCommon::SearchRuleNumerical ruleStatus(QByteArray(), MailCommon::SearchRule::FuncContains, QStringLiteral("foo"));
    QVERIFY(ruleStatus.isEmpty());
    ruleStatus = MailCommon::SearchRuleNumerical(QByteArray(), MailCommon::SearchRule::FuncContains, QStringLiteral("0"));
    QVERIFY(!ruleStatus.isEmpty());
}

void SearchRuleNumericalTest::shouldHaveRequirePart()
{
    MailCommon::SearchRuleNumerical ruleStatus;
    QCOMPARE(ruleStatus.requiredPart(), MailCommon::SearchRule::Envelope);
}

void SearchRuleNumericalTest::shouldMatchNumericalsize_data()
{
#if 0
    QTest::addColumn<MailCommon::SearchRule::Function>("function");
    QTest::addColumn<long>("value");
    QTest::addColumn<long>("matchvalue");
    QTest::addColumn<bool>("match");
#endif
}

void SearchRuleNumericalTest::shouldMatchNumericalsize()
{
#if 0
    QFETCH(MailCommon::SearchRule::Function, function);
    QFETCH(long, value);
    QFETCH(long, matchvalue);
    QFETCH(bool, match);
    MailCommon::SearchRuleNumerical searchrule("<size>", function, QString::number(value));
    KMime::Message::Ptr msgPtr = KMime::Message::Ptr(new KMime::Message());
    msgPtr->date(true)->setDateTime(maildate);
    Akonadi::Item item;
    item.setPayload<KMime::Message::Ptr>(msgPtr);
    QCOMPARE(searchrule.matches(item), match);
#endif
}

QTEST_MAIN(SearchRuleNumericalTest)
