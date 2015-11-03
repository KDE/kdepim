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

#include "filteractionsendfakedispositiontest.h"
#include "../filteractions/filteractionsendfakedisposition.h"
#include <qtest.h>
#include <pimcommon/minimumcombobox.h>

FilterActionSendFakeDispositionTest::FilterActionSendFakeDispositionTest(QObject *parent)
    : QObject(parent)
{

}

FilterActionSendFakeDispositionTest::~FilterActionSendFakeDispositionTest()
{

}

void FilterActionSendFakeDispositionTest::shouldHaveDefaultValue()
{
    MailCommon::FilterActionSendFakeDisposition filter;
    QWidget *w = filter.createParamWidget(0);
    QCOMPARE(w->objectName(), QStringLiteral("combobox"));

    PimCommon::MinimumComboBox *comboBox = dynamic_cast<PimCommon::MinimumComboBox *>(w);
    QVERIFY(comboBox);
    QVERIFY(!comboBox->isEditable());
    QVERIFY(comboBox->count() > 0);
}

void FilterActionSendFakeDispositionTest::shouldBeEmpty()
{
    MailCommon::FilterActionSendFakeDisposition filter;
    QVERIFY(filter.isEmpty());
}

void FilterActionSendFakeDispositionTest::shouldHaveRequiredPart()
{
    MailCommon::FilterActionSendFakeDisposition filter;
    QCOMPARE(filter.requiredPart(), MailCommon::SearchRule::CompleteMessage);
}

QTEST_MAIN(FilterActionSendFakeDispositionTest)
