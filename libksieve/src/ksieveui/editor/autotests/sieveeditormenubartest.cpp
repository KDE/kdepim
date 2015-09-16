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
#include <qtest.h>
#include "../sieveeditormenubar.h"
#include <QAction>
#include <qtestmouse.h>
#include <QSignalSpy>

SieveEditorMenuBarTest::SieveEditorMenuBarTest(QObject *parent)
    : QObject(parent)
{

}

SieveEditorMenuBarTest::~SieveEditorMenuBarTest()
{

}

void SieveEditorMenuBarTest::shouldDisableActions()
{
    KSieveUi::SieveEditorMenuBar bar;
    QVERIFY(bar.goToLineAction()->isEnabled());
    QVERIFY(bar.findAction()->isEnabled());
    QVERIFY(bar.replaceAction()->isEnabled());
    QVERIFY(bar.pasteAction()->isEnabled());
    QVERIFY(bar.zoomInAction()->isEnabled());
    QVERIFY(bar.zoomOutAction()->isEnabled());
    QVERIFY(bar.zoomResetAction()->isEnabled());
    QVERIFY(bar.editorMenu()->isEnabled());
    QVERIFY(bar.toolsMenu()->isEnabled());
    QVERIFY(bar.fileMenu()->isEnabled());
    QVERIFY(bar.commentCodeAction()->isEnabled());
    QVERIFY(bar.uncommentCodeAction()->isEnabled());
    QVERIFY(bar.debugSieveScriptAction()->isEnabled());
    QVERIFY(!bar.undoAction()->isEnabled());
    QVERIFY(!bar.redoAction()->isEnabled());
    QVERIFY(!bar.copyAction()->isEnabled());
    QVERIFY(bar.pasteAction()->isEnabled());
    QVERIFY(!bar.cutAction()->isEnabled());
    QVERIFY(bar.selectAllAction()->isEnabled());
    QVERIFY(bar.wordWrapAction()->isEnabled());

    bar.setEditorMode(false);
    QVERIFY(!bar.goToLineAction()->isEnabled());
    QVERIFY(!bar.findAction()->isEnabled());
    QVERIFY(!bar.replaceAction()->isEnabled());
    QVERIFY(!bar.pasteAction()->isEnabled());
    QVERIFY(!bar.zoomInAction()->isEnabled());
    QVERIFY(!bar.zoomOutAction()->isEnabled());
    QVERIFY(!bar.zoomResetAction()->isEnabled());
    QVERIFY(!bar.selectAllAction()->isEnabled());
    QVERIFY(bar.editorMenu()->isEnabled());
    QVERIFY(bar.toolsMenu()->isEnabled());
    QVERIFY(bar.fileMenu()->isEnabled());
    QVERIFY(!bar.commentCodeAction()->isEnabled());
    QVERIFY(!bar.uncommentCodeAction()->isEnabled());
    QVERIFY(!bar.debugSieveScriptAction()->isEnabled());
    QVERIFY(!bar.undoAction()->isEnabled());
    QVERIFY(!bar.redoAction()->isEnabled());
    QVERIFY(!bar.copyAction()->isEnabled());
    QVERIFY(!bar.pasteAction()->isEnabled());
    QVERIFY(!bar.cutAction()->isEnabled());
    QVERIFY(!bar.selectAllAction()->isEnabled());
    QVERIFY(!bar.wordWrapAction()->isEnabled());

    bar.setEditorMode(true);
    QVERIFY(bar.goToLineAction()->isEnabled());
    QVERIFY(bar.findAction()->isEnabled());
    QVERIFY(bar.replaceAction()->isEnabled());
    QVERIFY(bar.pasteAction()->isEnabled());
    QVERIFY(bar.zoomInAction()->isEnabled());
    QVERIFY(bar.zoomOutAction()->isEnabled());
    QVERIFY(bar.zoomResetAction()->isEnabled());
    QVERIFY(bar.selectAllAction()->isEnabled());
    QVERIFY(bar.editorMenu()->isEnabled());
    QVERIFY(bar.toolsMenu()->isEnabled());
    QVERIFY(bar.fileMenu()->isEnabled());
    QVERIFY(bar.commentCodeAction()->isEnabled());
    QVERIFY(bar.uncommentCodeAction()->isEnabled());
    QVERIFY(bar.debugSieveScriptAction()->isEnabled());
    QVERIFY(bar.undoAction()->isEnabled());
    QVERIFY(bar.redoAction()->isEnabled());
    QVERIFY(bar.copyAction()->isEnabled());
    QVERIFY(bar.pasteAction()->isEnabled());
    QVERIFY(bar.cutAction()->isEnabled());
    QVERIFY(bar.selectAllAction()->isEnabled());
    QVERIFY(bar.wordWrapAction()->isEnabled());
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
    QVERIFY(bar.zoomInAction());
    QVERIFY(bar.zoomOutAction());
    QVERIFY(bar.zoomResetAction());
    QVERIFY(bar.selectAllAction());
    QVERIFY(bar.editorMenu());
    QVERIFY(bar.toolsMenu());
    QVERIFY(bar.fileMenu());
    QVERIFY(bar.commentCodeAction());
    QVERIFY(bar.uncommentCodeAction());
    QVERIFY(bar.debugSieveScriptAction());
    QVERIFY(bar.wordWrapAction());
    QCOMPARE(bar.actions().count(), 3);
    QCOMPARE(bar.editorMenu()->actions().count(), 19);
    QCOMPARE(bar.fileMenu()->actions().count(), 0);
    QCOMPARE(bar.toolsMenu()->actions().count(), 4);

    QVERIFY(bar.findAction()->isEnabled());
    QVERIFY(bar.replaceAction()->isEnabled());
    QVERIFY(!bar.undoAction()->isEnabled());
    QVERIFY(!bar.redoAction()->isEnabled());
    QVERIFY(!bar.copyAction()->isEnabled());
    QVERIFY(bar.pasteAction()->isEnabled());
    QVERIFY(!bar.cutAction()->isEnabled());
    QVERIFY(bar.selectAllAction()->isEnabled());
}

void SieveEditorMenuBarTest::shouldEmitSignals()
{
    KSieveUi::SieveEditorMenuBar bar;
    QSignalSpy spyComment(&bar, SIGNAL(comment()));
    bar.commentCodeAction()->trigger();

    QSignalSpy spyUnComment(&bar, SIGNAL(uncomment()));
    bar.uncommentCodeAction()->trigger();

    QSignalSpy spyCut(&bar, SIGNAL(cut()));
    bar.cutAction()->trigger();

    QSignalSpy spyGotoLine(&bar, SIGNAL(gotoLine()));
    bar.goToLineAction()->trigger();

    QSignalSpy spyCopy(&bar, SIGNAL(copy()));
    bar.copyAction()->trigger();

    QSignalSpy spyPaste(&bar, SIGNAL(paste()));
    bar.pasteAction()->trigger();

    QSignalSpy spyUndo(&bar, SIGNAL(undo()));
    bar.undoAction()->trigger();

    QSignalSpy spyRedo(&bar, SIGNAL(redo()));
    bar.redoAction()->trigger();

    QSignalSpy spySelectAll(&bar, SIGNAL(selectAll()));
    bar.selectAllAction()->trigger();

    QSignalSpy spyFind(&bar, SIGNAL(find()));
    bar.findAction()->trigger();

    QSignalSpy spyReplace(&bar, SIGNAL(replace()));
    bar.replaceAction()->trigger();

    QSignalSpy spyZoomIn(&bar, SIGNAL(zoomIn()));
    bar.zoomInAction()->trigger();

    QSignalSpy spyZoomOut(&bar, SIGNAL(zoomOut()));
    bar.zoomOutAction()->trigger();

    QSignalSpy spyZoomReset(&bar, SIGNAL(zoomReset()));
    bar.zoomResetAction()->trigger();

    QSignalSpy spyDebugScript(&bar, SIGNAL(debugSieveScript()));
    bar.debugSieveScriptAction()->trigger();

    QSignalSpy spyWordWrapScript(&bar, SIGNAL(wordWrap(bool)));
    bar.wordWrapAction()->trigger();

    QCOMPARE(spyZoomOut.count(), 1);
    QCOMPARE(spyZoomIn.count(), 1);
    QCOMPARE(spyUnComment.count(), 1);
    QCOMPARE(spyComment.count(), 1);
    QCOMPARE(spyGotoLine.count(), 1);
    QCOMPARE(spyCut.count(), 1);
    QCOMPARE(spyCopy.count(), 1);
    QCOMPARE(spyPaste.count(), 1);
    QCOMPARE(spyRedo.count(), 1);
    QCOMPARE(spyUndo.count(), 1);
    QCOMPARE(spySelectAll.count(), 1);
    QCOMPARE(spyFind.count(), 1);
    QCOMPARE(spyReplace.count(), 1);
    QCOMPARE(spyZoomReset.count(), 1);
    QCOMPARE(spyDebugScript.count(), 1);
    QCOMPARE(spyWordWrapScript.count(), 1);
}

QTEST_MAIN(SieveEditorMenuBarTest)
