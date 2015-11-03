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
#include "filteractionwithaddresstest.h"
#include "../filteractions/filteractionwithaddress.h"
#include <qtest.h>
#include <QWidget>
#include <Akonadi/Contact/EmailAddressRequester>

class TestFilterActionWithAddress : public MailCommon::FilterActionWithAddress
{
public:
    TestFilterActionWithAddress()
        : MailCommon::FilterActionWithAddress(QStringLiteral("foo"), QStringLiteral("bla"))
    {

    }
    FilterAction::ReturnCode process(MailCommon::ItemContext &context, bool) const
    {
        return GoOn;
    }

    MailCommon::SearchRule::RequiredPart requiredPart() const
    {
        return MailCommon::SearchRule::CompleteMessage;
    }

};

FilterActionWithAddressTest::FilterActionWithAddressTest(QObject *parent)
    : QObject(parent)
{

}

FilterActionWithAddressTest::~FilterActionWithAddressTest()
{

}

void FilterActionWithAddressTest::shouldHaveDefaultValue()
{
    TestFilterActionWithAddress filter;
    QWidget *w = filter.createParamWidget(0);
    QCOMPARE(w->objectName(), QStringLiteral("emailaddressrequester"));
    Akonadi::EmailAddressRequester *requester = dynamic_cast<Akonadi::EmailAddressRequester *>(w);
    QVERIFY(requester);
    QVERIFY(filter.isEmpty());
}

void FilterActionWithAddressTest::shouldAssignValue()
{
    TestFilterActionWithAddress filter;
    QWidget *w = filter.createParamWidget(0);
    Akonadi::EmailAddressRequester *requester = dynamic_cast<Akonadi::EmailAddressRequester *>(w);
    filter.argsFromString(QStringLiteral("foo"));
    filter.setParamWidgetValue(w);
    QVERIFY(!filter.isEmpty());
    QVERIFY(!requester->text().isEmpty());
}

void FilterActionWithAddressTest::shouldRequiresPart()
{
    TestFilterActionWithAddress filter;
    QCOMPARE(filter.requiredPart(), MailCommon::SearchRule::CompleteMessage);
}

QTEST_MAIN(FilterActionWithAddressTest)
