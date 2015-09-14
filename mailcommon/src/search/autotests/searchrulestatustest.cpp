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

#include "searchrulestatustest.h"
#include <qtest.h>
#include "../searchrule/searchrulestatus.h"

SearchRuleStatusTest::SearchRuleStatusTest(QObject *parent)
    : QObject(parent)
{

}

void SearchRuleStatusTest::shouldHaveDefaultValue()
{
    MailCommon::SearchRuleStatus ruleStatus;
    QVERIFY(ruleStatus.contents().isEmpty());
    QVERIFY(ruleStatus.field().isEmpty());
    QCOMPARE(ruleStatus.function(), MailCommon::SearchRule::FuncContains);
    QVERIFY(ruleStatus.isEmpty());
}

void SearchRuleStatusTest::shouldHaveRequirePart()
{
    MailCommon::SearchRuleStatus ruleStatus;
    QCOMPARE(ruleStatus.requiredPart(), MailCommon::SearchRule::Envelope);
}

void SearchRuleStatusTest::shouldMatchStatus()
{

}

void SearchRuleStatusTest::shouldMatchStatus_data()
{

}

void SearchRuleStatusTest::shouldBeEmpty()
{
    MailCommon::SearchRuleStatus searchrule;
    QVERIFY(searchrule.isEmpty());
    searchrule = MailCommon::SearchRuleStatus(QByteArray(), MailCommon::SearchRule::FuncContains, QStringLiteral("foo"));
    QVERIFY(searchrule.isEmpty());
    searchrule = MailCommon::SearchRuleStatus(QByteArray("<tag>"), MailCommon::SearchRule::FuncContains, QString());
    QVERIFY(searchrule.isEmpty());

    searchrule = MailCommon::SearchRuleStatus(QByteArray("<tag>"), MailCommon::SearchRule::FuncContains, QStringLiteral("foo"));
    QVERIFY(!searchrule.isEmpty());
}

QTEST_MAIN(SearchRuleStatusTest)
