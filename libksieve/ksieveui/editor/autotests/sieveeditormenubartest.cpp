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

#include "sieveeditormenubartest.h"
#include "../sieveeditormenubar.h"
#include <qtest_kde.h>

SieveEditorMenuBarTest::SieveEditorMenuBarTest(QObject *parent)
    : QObject(parent)
{

}

SieveEditorMenuBarTest::~SieveEditorMenuBarTest()
{

}

void SieveEditorMenuBarTest::shouldHaveDefaultValue()
{
    KSieveUi::SieveEditorMenuBar bar;
    QVERIFY(bar.goToLineAction());
    QVERIFY(bar.findAction());
    QVERIFY(bar.replaceAction());
    QVERIFY(bar.undoAction());
    QVERIFY(bar.redoAction());
    QVERIFY(bar.copyAction());
    QVERIFY(bar.pasteAction());
    QVERIFY(bar.cutAction());
    QVERIFY(bar.selectAllAction());
    QVERIFY(bar.editorMenu());
    QVERIFY(bar.toolsMenu());
    QCOMPARE(bar.actions().count(), 2);
    QCOMPARE(bar.editorMenu()->actions().count(), 12);
}


QTEST_KDEMAIN(SieveEditorMenuBarTest, GUI)
