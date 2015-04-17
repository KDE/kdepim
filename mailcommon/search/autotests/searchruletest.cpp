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

#include "searchruletest.h"
#include <qtest.h>
#include "../search/searchrule/searchrule.h"
class TestSearchRule : public MailCommon::SearchRule
{
public:
    TestSearchRule(const QByteArray &field = QByteArray(), Function function = FuncContains,
                   const QString &contents = QString())
        : MailCommon::SearchRule(field, function, contents)
    {

    }

    bool matches(const Akonadi::Item &item) const
    {
        return false;
    }
    bool isEmpty() const
    {
        return false;
    }
    MailCommon::SearchRule::RequiredPart requiredPart() const
    {
        return MailCommon::SearchRule::CompleteMessage;
    }
};

SearchRuleTest::SearchRuleTest(QObject *parent)
    : QObject(parent)
{

}

void SearchRuleTest::shouldHaveDefaultValue()
{
    TestSearchRule searchrule;
    QCOMPARE(searchrule.field(), QByteArray());
    QCOMPARE(searchrule.function(), MailCommon::SearchRule::FuncContains);
    QVERIFY(searchrule.contents().isEmpty());
}

void SearchRuleTest::shouldAssignValue()
{
    TestSearchRule searchrule;
    //TODO
}

QTEST_MAIN(SearchRuleTest)
