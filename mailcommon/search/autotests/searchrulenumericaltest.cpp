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
#include "../searchrule/searchrulenumerical.h"
#include <qtest_kde.h>
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
    MailCommon::SearchRuleNumerical ruleStatus(QByteArray(), MailCommon::SearchRule::FuncContains, QLatin1String("foo"));
    QVERIFY(ruleStatus.isEmpty());
    ruleStatus =MailCommon::SearchRuleNumerical(QByteArray(), MailCommon::SearchRule::FuncContains, QLatin1String("0"));
    QVERIFY(!ruleStatus.isEmpty());
}

void SearchRuleNumericalTest::shouldHaveRequirePart()
{
    MailCommon::SearchRuleNumerical ruleStatus;
    QCOMPARE(ruleStatus.requiredPart(), MailCommon::SearchRule::Envelope);
}

void SearchRuleNumericalTest::shouldMatchNumerical()
{

}

void SearchRuleNumericalTest::shouldMatchNumerical_data()
{

}

QTEST_KDEMAIN(SearchRuleNumericalTest, GUI)