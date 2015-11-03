/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#include "contenttypewidgettest.h"
#include "../contenttypewidget.h"
#include <kcombobox.h>
#include <qlabel.h>
#include <qtest.h>
#include <QSignalSpy>
ContentTypeWidgetTest::ContentTypeWidgetTest(QObject *parent)
    : QObject(parent)
{

}

ContentTypeWidgetTest::~ContentTypeWidgetTest()
{

}

void ContentTypeWidgetTest::shouldHaveDefaultValue()
{
    MailCommon::ContentTypeWidget widget;
    QLabel *label = widget.findChild<QLabel *>(QStringLiteral("contenttypewidgetlabel"));
    QVERIFY(label);
    KComboBox *combo = widget.findChild<KComboBox *>(QStringLiteral("contentcombobox"));
    QVERIFY(combo);
    QVERIFY(combo->count() > 0);
    QCOMPARE(widget.currentIndex(), 0);
}

void ContentTypeWidgetTest::shouldChangeComboBoxIndex()
{
    MailCommon::ContentTypeWidget contentType;
    KComboBox *combo = contentType.findChild<KComboBox *>(QStringLiteral("contentcombobox"));
    for (int i = 0; i < combo->count(); ++i) {
        contentType.setCurrentIndex(i);
        QCOMPARE(contentType.currentIndex(), i);
        QCOMPARE(combo->currentIndex(), i);
    }
}

void ContentTypeWidgetTest::shouldEmitSignalWhenIndexChanged()
{
    MailCommon::ContentTypeWidget contentType;
    contentType.show();
    QTest::qWaitForWindowExposed(&contentType);
    QSignalSpy spy(&contentType, SIGNAL(activated(int)));
    contentType.setCurrentIndex(1);
    //FIXME
    //QCOMPARE(spy.at(0).count(), 1);
}

void ContentTypeWidgetTest::shouldChangeItem()
{
    MailCommon::ContentTypeWidget contentType;
    contentType.show();
    QTest::qWaitForWindowExposed(&contentType);
    KComboBox *combo = contentType.findChild<KComboBox *>(QStringLiteral("contentcombobox"));
    for (int i = 0; i < combo->count(); ++i) {
        contentType.setCurrentIndex(i);
        QCOMPARE(contentType.currentText(), combo->currentText());
    }

}

QTEST_MAIN(ContentTypeWidgetTest)
