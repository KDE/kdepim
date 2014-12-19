/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "contentstypecomboboxtest.h"
#include "../contentstypecombobox.h"
#include <kcombobox.h>
#include <qlabel.h>
#include <qtest_kde.h>
#include <QSignalSpy>

ContentsTypeComboBoxTest::ContentsTypeComboBoxTest(QObject *parent)
    : QObject(parent)
{

}

ContentsTypeComboBoxTest::~ContentsTypeComboBoxTest()
{

}

void ContentsTypeComboBoxTest::shouldHaveDefaultValue()
{
    MailCommon::ContentsTypeComboBox contentType;
    QLabel *label = qFindChild<QLabel *>(&contentType, QLatin1String("contentstypelabel"));
    QVERIFY(label);
    KComboBox *combo = qFindChild<KComboBox *>(&contentType, QLatin1String("contentstypecombobox"));
    QVERIFY(combo);
    QVERIFY(combo->count()>0);
    QCOMPARE(contentType.currentIndex(), 0);
}

void ContentsTypeComboBoxTest::shouldChangeComboBoxIndex()
{
    MailCommon::ContentsTypeComboBox contentType;
    KComboBox *combo = qFindChild<KComboBox *>(&contentType, QLatin1String("contentstypecombobox"));
    for (int i = 0 ; i < combo->count() ; ++i) {
        contentType.setCurrentIndex(i);
        QCOMPARE(contentType.currentIndex(), i);
        QCOMPARE(combo->currentIndex(), i);
    }
}

void ContentsTypeComboBoxTest::shouldEmitSignalWhenIndexChanged()
{
    MailCommon::ContentsTypeComboBox contentType;
    contentType.show();
    QTest::qWaitForWindowShown(&contentType);
    QSignalSpy spy(&contentType, SIGNAL(indexChanged(int)));
    contentType.setCurrentIndex(1);
    QCOMPARE(spy.at(0).count(), 1);
}

QTEST_KDEMAIN(ContentsTypeComboBoxTest, GUI)
