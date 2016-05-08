/*
  Copyright (c) 2016 Rebois Guillaume <guillaume.rebois@orange.fr>

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
#include "pimcommon/lineeditwithkeywords.h"
#include "qtest.h"
#include "klineedit.h"
#include <QObject>

using namespace PimCommon;

class LineEditWithKeywordsTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void shouldHaveDefaultValue()
    {
        LineEditWithKeywords lineEdit;
        QVERIFY(lineEdit.text().isEmpty());
        KCompletion *comp = lineEdit.completionObject();
        QVERIFY(!comp->items().isEmpty());
    }

    void shouldBeAlwaysSameList()
    {
        LineEditWithKeywords lineEdit;
        QStringList list1;
        list1 << lineEdit.completionObject()->items();

        LineEditWithKeywords lineEdit2;
        QStringList list2;
        list2 << lineEdit2.completionObject()->items();
        QCOMPARE(list1,list2);
    }

    void shouldAddNewWord()
    {
        LineEditWithKeywords lineEdit;
        KCompletion *comp = lineEdit.completionObject();

        QVERIFY(!comp->items().contains(QStringLiteral("word1")));
        int nb = comp->items().size();
        comp->addItem(QStringLiteral("word1"));
        QCOMPARE(comp->items().size(),nb+1);
        QVERIFY(comp->items().contains(QStringLiteral("word1")));
    }

    void shouldHaveDefaultListAfterClearHistory()
    {
        LineEditWithKeywords lineEdit;
        KCompletion *comp = lineEdit.completionObject();
        QMetaObject::invokeMethod(&lineEdit, "slotClearHistory");

        LineEditWithKeywords lineEdit2;
        KCompletion *comp2 = lineEdit2.completionObject();
        QVERIFY(!comp->items().isEmpty());
        QCOMPARE(comp->items(),comp2->items());
    }
};

QTEST_MAIN(LineEditWithKeywordsTest)
#include "lineeditwithkeywordstest.moc"
