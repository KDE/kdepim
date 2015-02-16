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

#include "manualmergeresultwidgettest.h"
#include "../manualmerge/manualmergeresultwidget.h"
#include <qtest_kde.h>
#include <QSplitter>
ManualMergeResultWidgetTest::ManualMergeResultWidgetTest(QObject *parent)
    : QObject(parent)
{

}

ManualMergeResultWidgetTest::~ManualMergeResultWidgetTest()
{

}

void ManualMergeResultWidgetTest::shouldHaveDefaultValue()
{
    KABMergeContacts::ManualMergeResultWidget w;
    QSplitter *splitter = qFindChild<QSplitter *>(&w, QLatin1String("splitter"));
    QVERIFY(splitter);
    QVERIFY(!splitter->childrenCollapsible());
    for(int i =0; i < splitter->count(); ++i) {
        const QString objName = splitter->widget(i)->objectName();

        const bool hasName = (objName == QLatin1String("mergecontactwidget")) || (objName == QLatin1String("mergecontactinfowidget"));
        QVERIFY(hasName);
    }
}

QTEST_KDEMAIN(ManualMergeResultWidgetTest, GUI)
