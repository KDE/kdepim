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
}

QTEST_KDEMAIN(BalooDebugDialogTest, GUI)


