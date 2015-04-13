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

#include "filteractiondeletetest.h"
#include "../filteractions/filteractiondelete.h"
#include <qtest.h>
#include <QLabel>
FilterActionDeleteTest::FilterActionDeleteTest(QObject *parent)
    : QObject(parent)
{

}

FilterActionDeleteTest::~FilterActionDeleteTest()
{

}

void FilterActionDeleteTest::shouldHaveDefaultValue()
{
    MailCommon::FilterActionDelete filter(0);
    QWidget *w = filter.createParamWidget(0);
    QVERIFY(w);

    QLabel *lab = dynamic_cast<QLabel *>(w);
    QVERIFY(lab);
    QCOMPARE(lab->objectName(), QStringLiteral("label_delete"));
}

void FilterActionDeleteTest::shouldReturnSieveValue()
{
    MailCommon::FilterActionDelete filter(0);
    QCOMPARE(filter.sieveCode(), QStringLiteral("discard;"));
}

void FilterActionDeleteTest::shouldBeNotEmpty()
{
    MailCommon::FilterActionDelete filter;
    QVERIFY(!filter.isEmpty());
}

void FilterActionDeleteTest::shouldRequiresPart()
{
    MailCommon::FilterActionDelete filter;
    QCOMPARE(filter.requiredPart(), MailCommon::SearchRule::Envelope);
}

void FilterActionDeleteTest::shouldDeleteItem()
{
    MailCommon::FilterActionDelete filter(this);
    KMime::Message::Ptr msgPtr = KMime::Message::Ptr(new KMime::Message());
    Akonadi::Item item;
    item.setPayload<KMime::Message::Ptr>(msgPtr);
    MailCommon::ItemContext context(item, false);

    filter.argsFromString("");
    QCOMPARE(filter.process(context, false), MailCommon::FilterAction::GoOn);
    QCOMPARE(context.needsPayloadStore(), false);
    QCOMPARE(context.deleteItem(), true);
    QCOMPARE(context.needsFlagStore(), false);
    QCOMPARE(context.needsFullPayload(), false);
}

QTEST_MAIN(FilterActionDeleteTest)
