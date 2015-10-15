/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#include "textgotolinewidgettest.h"
#include "texteditor/commonwidget/textgotolinewidget.h"
#include <qtest.h>
#include <QSignalSpy>
#include <qtestmouse.h>
#include <qtestkeyboard.h>
#include <QSpinBox>
#include <QToolButton>
#include <QPushButton>

TextGoToLineWidgetTest::TextGoToLineWidgetTest()
{
}

void TextGoToLineWidgetTest::shouldHaveDefaultValuesOnCreation()
{
    PimCommon::TextGoToLineWidget edit;
    edit.show();
    QTest::qWaitForWindowExposed(&edit);
    QSpinBox *line = edit.findChild<QSpinBox *>(QStringLiteral("line"));
    QVERIFY(line);
    QCOMPARE(line->minimum(), 1);
    QPushButton *gotolinebutton = edit.findChild<QPushButton *>(QStringLiteral("gotoline"));
    QVERIFY(gotolinebutton);
    QToolButton *closebutton = edit.findChild<QToolButton *>(QStringLiteral("closebutton"));
    QVERIFY(closebutton);
    QVERIFY(line->hasFocus());
}

void TextGoToLineWidgetTest::shouldEmitGoToLineSignalWhenPressOnButton()
{
    PimCommon::TextGoToLineWidget edit;
    QPushButton *gotolinebutton = edit.findChild<QPushButton *>(QStringLiteral("gotoline"));
    QSignalSpy spy(&edit, SIGNAL(moveToLine(int)));
    QTest::mouseClick(gotolinebutton, Qt::LeftButton);
    QCOMPARE(spy.count(), 1);
}

void TextGoToLineWidgetTest::shouldEmitGoToLineSignalCorrectValueWhenPressOnButton()
{
    PimCommon::TextGoToLineWidget edit;
    QPushButton *gotolinebutton = edit.findChild<QPushButton *>(QStringLiteral("gotoline"));
    QSpinBox *line = edit.findChild<QSpinBox *>(QStringLiteral("line"));
    line->setValue(5);
    QCOMPARE(line->value(), 5);
    QSignalSpy spy(&edit, SIGNAL(moveToLine(int)));
    QTest::mouseClick(gotolinebutton, Qt::LeftButton);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toInt(), 5);
}

void TextGoToLineWidgetTest::shouldHideWidgetWhenClickOnCloseButton()
{
    PimCommon::TextGoToLineWidget edit;
    edit.show();
    QTest::qWaitForWindowExposed(&edit);
    QVERIFY(edit.isVisible());
    QToolButton *closebutton = edit.findChild<QToolButton *>(QStringLiteral("closebutton"));
    QTest::mouseClick(closebutton, Qt::LeftButton);
    QVERIFY(!edit.isVisible());
}

void TextGoToLineWidgetTest::shouldHideWidgetWhenPressEscape()
{
    PimCommon::TextGoToLineWidget edit;
    edit.show();
    QTest::qWaitForWindowExposed(&edit);
    QTest::keyPress(&edit, Qt::Key_Escape);
    QCOMPARE(edit.isVisible(), false);
}

void TextGoToLineWidgetTest::shouldEmitGoToLineSignalWhenSpinboxHasFocusAndWePressEnter()
{
    PimCommon::TextGoToLineWidget edit;
    edit.show();
    QTest::qWaitForWindowExposed(&edit);
    QSpinBox *line = edit.findChild<QSpinBox *>(QStringLiteral("line"));
    line->setFocus();
    QVERIFY(line->hasFocus());
    line->setValue(5);
    QSignalSpy spy(&edit, SIGNAL(moveToLine(int)));
    QTest::keyPress(line, Qt::Key_Enter);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toInt(), 5);
}

void TextGoToLineWidgetTest::shouldHasFocusEachTimeThatItShown()
{
    PimCommon::TextGoToLineWidget edit;
    edit.show();
    QTest::qWaitForWindowExposed(&edit);
    QSpinBox *line = edit.findChild<QSpinBox *>(QStringLiteral("line"));
    QVERIFY(line);
    QVERIFY(line->hasFocus());
    edit.hide();
    QVERIFY(!line->hasFocus());
    edit.show();
    QTest::qWaitForWindowExposed(&edit);
    //FIXME QVERIFY(line->hasFocus());
}

void TextGoToLineWidgetTest::shouldSetFocusWhenWeRecallGotToLine()
{
    PimCommon::TextGoToLineWidget edit;
    edit.show();
    QTest::qWaitForWindowExposed(&edit);
    QSpinBox *line = edit.findChild<QSpinBox *>(QStringLiteral("line"));
    QVERIFY(line->hasFocus());
    edit.setFocus();
    QVERIFY(!line->hasFocus());
    edit.goToLine();
    QVERIFY(line->hasFocus());
}

void TextGoToLineWidgetTest::shouldChangeMaximumValue()
{
    PimCommon::TextGoToLineWidget edit;
    edit.show();
    QTest::qWaitForWindowExposed(&edit);
    QSpinBox *line = edit.findChild<QSpinBox *>(QStringLiteral("line"));

    QCOMPARE(line->value(), 1);
    QCOMPARE(line->minimum(), 1);

    edit.setMaximumLineCount(50);
    QCOMPARE(line->value(), 1);
    QCOMPARE(line->minimum(), 1);
    QCOMPARE(line->maximum(), 50);

    edit.setMaximumLineCount(10);
    QCOMPARE(line->value(), 1);
    QCOMPARE(line->minimum(), 1);
    QCOMPARE(line->maximum(), 10);
}

QTEST_MAIN(TextGoToLineWidgetTest)
