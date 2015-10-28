/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "completionconfiguredialogtest.h"
#include "../completionconfiguredialog/completionconfiguredialog.h"
#include <QTabWidget>
#include <qdialogbuttonbox.h>
#include <qtest.h>
#include <QStandardPaths>

CompletionConfigureDialogTest::CompletionConfigureDialogTest(QObject *parent)
    : QObject(parent)
{

}

CompletionConfigureDialogTest::~CompletionConfigureDialogTest()
{

}

void CompletionConfigureDialogTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void CompletionConfigureDialogTest::shouldHaveDefaultValue()
{
    KPIM::CompletionConfigureDialog dlg;

    QDialogButtonBox *buttonBox = dlg.findChild<QDialogButtonBox *>(QStringLiteral("buttonbox"));
    QVERIFY(buttonBox);

    QTabWidget *tabWidget = dlg.findChild<QTabWidget *>(QStringLiteral("tabwidget"));
    QVERIFY(tabWidget);
    QVERIFY(tabWidget->count() > 0);
    for (int i = 0; i < tabWidget->count(); ++i) {
        const QString objName = tabWidget->widget(i)->objectName();
        const bool hasName = (objName == QLatin1String("completionorder_widget")) ||
                             (objName == QLatin1String("recentaddress_widget"))  ||
                             (objName == QLatin1String("blacklistbaloo_widget"));
        QVERIFY(hasName);
    }
}

QTEST_MAIN(CompletionConfigureDialogTest)
