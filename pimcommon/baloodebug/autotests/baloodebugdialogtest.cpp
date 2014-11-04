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


#include "baloodebugdialogtest.h"
#include "../baloodebugdialog.h"
#include "../baloodebugwidget.h"
#include "pimcommon/texteditor/plaintexteditor/plaintexteditorwidget.h"

#include <KLineEdit>
#include <qtest_kde.h>

BalooDebugDialogTest::BalooDebugDialogTest(QObject *parent)
    : QObject(parent)
{

}

BalooDebugDialogTest::~BalooDebugDialogTest()
{

}

void BalooDebugDialogTest::shouldHaveDefaultValue()
{
    PimCommon::BalooDebugDialog dlg;
    PimCommon::BalooDebugWidget *debugWidget = qFindChild<PimCommon::BalooDebugWidget *>(&dlg, QLatin1String("baloodebugwidget"));
    QVERIFY(debugWidget);
    PimCommon::PlainTextEditorWidget *editorWidget = qFindChild<PimCommon::PlainTextEditorWidget *>(debugWidget, QLatin1String("plaintexteditor"));
    QVERIFY(editorWidget);
    KLineEdit *lineEdit = qFindChild<KLineEdit *>(debugWidget, QLatin1String("lineedit"));
    QVERIFY(lineEdit);
    QVERIFY(lineEdit->text().isEmpty());
}

void BalooDebugDialogTest::shouldFillLineEditWhenWeWantToSearchItem()
{
    PimCommon::BalooDebugDialog dlg;
    PimCommon::BalooDebugWidget *debugWidget = qFindChild<PimCommon::BalooDebugWidget *>(&dlg, QLatin1String("baloodebugwidget"));
    QVERIFY(debugWidget);
    KLineEdit *lineEdit = qFindChild<KLineEdit *>(debugWidget, QLatin1String("lineedit"));
    QVERIFY(lineEdit);
    const int value = 42;
    const QString akonadiItem = QString::number(value);
    dlg.setAkonadiId(value);
    QCOMPARE(lineEdit->text(), akonadiItem);
}

QTEST_KDEMAIN(BalooDebugDialogTest, GUI)


