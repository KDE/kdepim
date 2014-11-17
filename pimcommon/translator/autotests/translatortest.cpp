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

#include "translatortest.h"
#include "translator/translatorwidget.h"
#include "texteditor/plaintexteditor/plaintexteditorwidget.h"

#include <QPushButton>

#include <qtest.h>
#include <qtestkeyboard.h>
#include <qtestmouse.h>
#include <QComboBox>
#include <QSignalSpy>

TranslatorTest::TranslatorTest()
{

}

void TranslatorTest::shouldHaveDefaultValuesOnCreation()
{
    PimCommon::TranslatorWidget edit;
    QComboBox *from = edit.findChild<QComboBox *>(QLatin1String("from"));
    QComboBox *to = edit.findChild<QComboBox *>(QLatin1String("to"));
    PimCommon::TranslatorTextEdit *inputtext = edit.findChild<PimCommon::TranslatorTextEdit *>(QLatin1String("inputtext"));
    PimCommon::PlainTextEditorWidget *translatedText = edit.findChild<PimCommon::PlainTextEditorWidget *>(QLatin1String("translatedtext"));
    QPushButton *translate = edit.findChild<QPushButton *>(QLatin1String("translate-button"));
    QPushButton *clear = edit.findChild<QPushButton *>(QLatin1String("clear-button"));
    QPushButton *invert = edit.findChild<QPushButton *>(QLatin1String("invert-button"));
    QVERIFY(invert);
    QVERIFY(clear);
    QVERIFY(translate);
    QVERIFY(from);
    QVERIFY(to);
    QCOMPARE(from->count() > 0, true);
    QCOMPARE(to->count() > 0, true);
    QVERIFY(edit.toggleAction());
    QVERIFY(inputtext);
    QVERIFY(translatedText);
    QCOMPARE(inputtext->toPlainText(), QString());
    QCOMPARE(translatedText->toPlainText(), QString());
    QCOMPARE(translate->isEnabled(), false);
}

void TranslatorTest::shouldEnableTranslateButtonWhenTextToTranslateIsNotEmpty()
{
    PimCommon::TranslatorWidget edit;
    PimCommon::TranslatorTextEdit *inputtext = edit.findChild<PimCommon::TranslatorTextEdit *>(QLatin1String("inputtext"));
    QPushButton *translate = edit.findChild<QPushButton *>(QLatin1String("translate-button"));
    inputtext->setPlainText(QStringLiteral("Foo"));
    QCOMPARE(translate->isEnabled(), true);
}

void TranslatorTest::shouldDisableTranslateButtonAndClearTextWhenClickOnClearButton()
{
    PimCommon::TranslatorWidget edit;
    PimCommon::TranslatorTextEdit *inputtext = edit.findChild<PimCommon::TranslatorTextEdit *>(QLatin1String("inputtext"));
    QPushButton *translate = edit.findChild<QPushButton *>(QLatin1String("translate-button"));
    PimCommon::PlainTextEditorWidget *translatedText = edit.findChild<PimCommon::PlainTextEditorWidget *>(QLatin1String("translatedtext"));
    inputtext->setPlainText(QStringLiteral("Foo"));
    QPushButton *clear = edit.findChild<QPushButton *>(QLatin1String("clear-button"));
    QTest::mouseClick(clear, Qt::LeftButton);
    QCOMPARE(inputtext->toPlainText(), QString());
    QCOMPARE(translatedText->toPlainText(), QString());
    QCOMPARE(translate->isEnabled(), false);
}

void TranslatorTest::shouldInvertLanguageWhenClickOnInvertButton()
{
    PimCommon::TranslatorWidget edit;
    QComboBox *from = edit.findChild<QComboBox *>(QLatin1String("from"));
    QComboBox *to = edit.findChild<QComboBox *>(QLatin1String("to"));

    const int fromIndex = 5;
    const int toIndex = 7;
    from->setCurrentIndex(fromIndex);
    to->setCurrentIndex(toIndex);
    QPushButton *invert = edit.findChild<QPushButton *>(QLatin1String("invert-button"));
    QCOMPARE(fromIndex != toIndex, true);
    QTest::mouseClick(invert, Qt::LeftButton);
    const int newFromIndex = from->currentIndex();
    const int newToIndex = to->currentIndex();
    QCOMPARE(fromIndex != newFromIndex, true);
    QCOMPARE(toIndex != newToIndex, true);
}

void TranslatorTest::shouldHideWidgetWhenPressEscape()
{
    PimCommon::TranslatorWidget edit;
    edit.show();
    QTest::qWaitForWindowExposed(&edit);
    QTest::keyPress(&edit, Qt::Key_Escape);
    QCOMPARE(edit.isVisible(), false);
}

void TranslatorTest::shouldEmitTranslatorWasClosedSignalWhenCloseIt()
{
    PimCommon::TranslatorWidget edit;
    edit.show();
    QTest::qWaitForWindowExposed(&edit);
    QSignalSpy spy(&edit, SIGNAL(translatorWasClosed()));
    QTest::keyClick(&edit, Qt::Key_Escape);
    QCOMPARE(spy.count(), 1);
}

QTEST_MAIN(TranslatorTest)
