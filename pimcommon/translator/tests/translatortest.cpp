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

#include <KPushButton>

#include <qtest_kde.h>
#include <qtestmouse.h>
#include <QComboBox>


TranslatorTest::TranslatorTest()
{

}

void TranslatorTest::shouldHaveDefaultValuesOnCreation()
{
    PimCommon::TranslatorWidget edit;
    QComboBox *from = qFindChild<QComboBox *>(&edit, QLatin1String("from"));
    QComboBox *to = qFindChild<QComboBox *>(&edit, QLatin1String("to"));
    PimCommon::TranslatorTextEdit *inputtext = qFindChild<PimCommon::TranslatorTextEdit *>(&edit, QLatin1String("inputtext"));
    PimCommon::PlainTextEditorWidget *translatedText = qFindChild<PimCommon::PlainTextEditorWidget *>(&edit, QLatin1String("translatedtext"));
    KPushButton *translate = qFindChild<KPushButton *>(&edit, QLatin1String("translate-button"));
    KPushButton *clear = qFindChild<KPushButton *>(&edit, QLatin1String("clear-button"));
    KPushButton *invert = qFindChild<KPushButton *>(&edit, QLatin1String("invert-button"));
    QVERIFY(invert);
    QVERIFY(clear);
    QVERIFY(translate);
    QVERIFY(from);
    QVERIFY(to);
    QCOMPARE(from->count()>0, true);
    QCOMPARE(to->count()>0, true);
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
    PimCommon::TranslatorTextEdit *inputtext = qFindChild<PimCommon::TranslatorTextEdit *>(&edit, QLatin1String("inputtext"));
    KPushButton *translate = qFindChild<KPushButton *>(&edit, QLatin1String("translate-button"));
    inputtext->setPlainText(QString::fromLatin1("Foo"));
    QCOMPARE(translate->isEnabled(), true);
}

void TranslatorTest::shouldDisableTranslateButtonAndClearTextWhenClickOnClearButton()
{
    PimCommon::TranslatorWidget edit;
    PimCommon::TranslatorTextEdit *inputtext = qFindChild<PimCommon::TranslatorTextEdit *>(&edit, QLatin1String("inputtext"));
    KPushButton *translate = qFindChild<KPushButton *>(&edit, QLatin1String("translate-button"));
    PimCommon::PlainTextEditorWidget *translatedText = qFindChild<PimCommon::PlainTextEditorWidget *>(&edit, QLatin1String("translatedtext"));
    inputtext->setPlainText(QString::fromLatin1("Foo"));
    KPushButton *clear = qFindChild<KPushButton *>(&edit, QLatin1String("clear-button"));
    QTest::mouseClick(clear, Qt::LeftButton);
    QCOMPARE(inputtext->toPlainText(), QString());
    QCOMPARE(translatedText->toPlainText(), QString());
    QCOMPARE(translate->isEnabled(), false);
}

void TranslatorTest::shouldInvertLanguageWhenClickOnInvertButton()
{
#if 0 //Adapt it
    PimCommon::TranslatorWidget edit;
    QComboBox *from = qFindChild<QComboBox *>(&edit, QLatin1String("from"));
    QComboBox *to = qFindChild<QComboBox *>(&edit, QLatin1String("to"));
    const int fromIndex = from->currentIndex();
    const int toIndex = to->currentIndex();
    KPushButton *invert = qFindChild<KPushButton *>(&edit, QLatin1String("invert-button"));
    QCOMPARE(fromIndex != toIndex, true);
    QTest::mouseClick(invert, Qt::LeftButton);
    int newFromIndex = from->currentIndex();
    int newToIndex = to->currentIndex();
    QCOMPARE(fromIndex != newFromIndex, true);
    QCOMPARE(toIndex != newToIndex, true);
#endif
}

QTEST_KDEMAIN( TranslatorTest, GUI )
