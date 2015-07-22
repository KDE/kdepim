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

#include "sievescriptdebuggerwidgettest.h"
#include "../sievescriptdebuggerwidget.h"
#include <QTest>
#include <QStackedWidget>

SieveScriptDebuggerWidgetTest::SieveScriptDebuggerWidgetTest(QObject *parent)
    : QObject(parent)
{

}

SieveScriptDebuggerWidgetTest::~SieveScriptDebuggerWidgetTest()
{

}

void SieveScriptDebuggerWidgetTest::shouldHaveDefaultValue()
{
    KSieveUi::SieveScriptDebuggerWidget w;
    QStackedWidget *stackedwidget = w.findChild<QStackedWidget *>(QStringLiteral("stackedwidget"));
    QVERIFY(stackedwidget);

    QCOMPARE(stackedwidget->count(), 2);
    for (int i = 0; i < stackedwidget->count(); ++i) {
        QWidget *w = stackedwidget->widget(i);
        QVERIFY((w->objectName() == QStringLiteral("sievescriptfrontend")) || (w->objectName() == QStringLiteral("sievenoexistingfrontend")));
    }
}

QTEST_MAIN(SieveScriptDebuggerWidgetTest)
