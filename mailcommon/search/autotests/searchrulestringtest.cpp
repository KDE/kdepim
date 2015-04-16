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

#include "searchrulestringtest.h"
#include <qtest.h>
#include "../searchrule/searchrulestring.h"

SearchRuleStringTest::SearchRuleStringTest(QObject *parent)
    : QObject(parent)
{

}

void SearchRuleStringTest::shouldHaveDefaultValue()
{
    MailCommon::SearchRuleString searchrule;
    QVERIFY(searchrule.contents().isEmpty());
    QVERIFY(searchrule.field().isEmpty());
    QCOMPARE(searchrule.function(), MailCommon::SearchRule::FuncContains);
    QVERIFY(searchrule.isEmpty());
}

void SearchRuleStringTest::shouldHaveRequirePart()
{
    MailCommon::SearchRuleString ruleStatus;
    //Depend. Need to implement test correctly
    //QCOMPARE(ruleStatus.requiredPart(), MailCommon::SearchRule::Envelope);
}

void SearchRuleStringTest::shouldMatchString()
{

}

void SearchRuleStringTest::shouldMatchString_data()
{

}


QTEST_MAIN(SearchRuleStringTest)
