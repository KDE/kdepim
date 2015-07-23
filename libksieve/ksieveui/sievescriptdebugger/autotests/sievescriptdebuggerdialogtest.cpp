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

#include "sievescriptdebuggerdialogtest.h"
#include "../sievescriptdebuggerdialog.h"
#include "../sievescriptdebuggerwidget.h"
#include <QDialogButtonBox>
#include <QTest>

SieveScriptDebuggerDialogTest::SieveScriptDebuggerDialogTest(QObject *parent)
    : QObject(parent)
{

}

SieveScriptDebuggerDialogTest::~SieveScriptDebuggerDialogTest()
{

}

void SieveScriptDebuggerDialogTest::shouldHaveDefaultValue()
{
    KSieveUi::SieveScriptDebuggerDialog dlg;
    QDialogButtonBox *buttonBox = dlg.findChild<QDialogButtonBox *>(QStringLiteral("buttonbox"));
    QVERIFY(buttonBox);

    KSieveUi::SieveScriptDebuggerWidget *widget = dlg.findChild<KSieveUi::SieveScriptDebuggerWidget *>(QStringLiteral("sievescriptdebuggerwidget"));
    QVERIFY(widget);

    QVERIFY(dlg.script().isEmpty());
}

QTEST_MAIN(SieveScriptDebuggerDialogTest)
