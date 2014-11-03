/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#include "baloodebugsearchpathcomboboxtest.h"
#include "../baloodebugsearchpathcombobox.h"
#include <qtest_kde.h>


BalooDebugSearchPathComboBoxTest::BalooDebugSearchPathComboBoxTest(QObject *parent)
    : QObject(parent)
{

}

BalooDebugSearchPathComboBoxTest::~BalooDebugSearchPathComboBoxTest()
{

}

void BalooDebugSearchPathComboBoxTest::shouldHaveDefaultValue()
{
    PimCommon::BalooDebugSearchPathComboBox combox;
    QVERIFY(combox.count()>0);
}

void BalooDebugSearchPathComboBoxTest::shouldReturnPath()
{
    PimCommon::BalooDebugSearchPathComboBox combox;
    QVERIFY(!combox.searchPath().isEmpty());
}

void BalooDebugSearchPathComboBoxTest::shouldReturnCorrectSearchPath()
{
    PimCommon::BalooDebugSearchPathComboBox combox;
    QString path = combox.pathFromEnum(PimCommon::BalooDebugSearchPathComboBox::Contacts);
    QCOMPARE(combox.searchPath(), path);
}

void BalooDebugSearchPathComboBoxTest::shouldSelectCorrectType()
{
    PimCommon::BalooDebugSearchPathComboBox combox;
    QString path = combox.pathFromEnum(PimCommon::BalooDebugSearchPathComboBox::ContactCompleter);
    combox.setSearchType(PimCommon::BalooDebugSearchPathComboBox::ContactCompleter);
    QCOMPARE(combox.searchPath(), path);
    path = combox.pathFromEnum(PimCommon::BalooDebugSearchPathComboBox::Emails);
    combox.setSearchType(PimCommon::BalooDebugSearchPathComboBox::Emails);
    QCOMPARE(combox.searchPath(), path);

}



QTEST_KDEMAIN(BalooDebugSearchPathComboBoxTest, GUI)
