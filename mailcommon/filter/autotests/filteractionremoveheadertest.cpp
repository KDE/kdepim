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

#include "filteractionremoveheadertest.h"
#include "../filteractions/filteractionremoveheader.h"
#include <qtest.h>
#include <widgets/minimumcombobox.h>

FilterActionRemoveHeaderTest::FilterActionRemoveHeaderTest(QObject *parent)
    : QObject(parent)
{

}

FilterActionRemoveHeaderTest::~FilterActionRemoveHeaderTest()
{

}

void FilterActionRemoveHeaderTest::shouldHaveDefaultValue()
{
    MailCommon::FilterActionRemoveHeader filter;
    QWidget *w = filter.createParamWidget(0);
    PimCommon::MinimumComboBox *comboBox = dynamic_cast<PimCommon::MinimumComboBox *>(w);
    QVERIFY(comboBox);
}

void FilterActionRemoveHeaderTest::shouldHaveSieveRequires()
{
    MailCommon::FilterActionRemoveHeader filter;
    QCOMPARE(filter.sieveRequires(), QStringList() << QStringLiteral("editheader"));
}

QTEST_MAIN(FilterActionRemoveHeaderTest)
