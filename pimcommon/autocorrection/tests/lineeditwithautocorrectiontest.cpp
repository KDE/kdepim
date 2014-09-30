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

#include "lineeditwithautocorrectiontest.h"
#include "../widgets/lineeditwithautocorrection.h"
#include "../autocorrection.h"
#include <qtest_kde.h>
#include <qtestkeyboard.h>

LineEditWithAutocorrectionTest::LineEditWithAutocorrectionTest()
{
}

void LineEditWithAutocorrectionTest::shouldNotAutocorrectWhenDisabled()
{
    PimCommon::LineEditWithAutoCorrection lineedit(0, QLatin1String("lineeditwithautocorrecttestrc"));
    QHash<QString, QString> entries;
    const QString originalWord = QLatin1String("FOOFOO");
    const QString replaceWord = QLatin1String("BLABLA");
    entries.insert(originalWord, replaceWord);
    lineedit.autocorrection()->setAutocorrectEntries(entries);
    lineedit.show();
    QTest::qWaitForWindowShown(&lineedit);
    QTest::keyClicks(&lineedit, originalWord);
    QTest::keyClick(&lineedit, ' ');
    QCOMPARE(lineedit.toPlainText(), QString(originalWord + QLatin1Char(' ')) );
}

void LineEditWithAutocorrectionTest::shouldReplaceWordWhenExactText()
{
    PimCommon::LineEditWithAutoCorrection lineedit(0, QLatin1String("lineeditwithautocorrecttestrc"));
    const QString originalWord = QLatin1String("FOOFOO");
    const QString replaceWord = QLatin1String("BLABLA");
    QHash<QString, QString> entries;
    entries.insert(originalWord, replaceWord);
    lineedit.autocorrection()->setAutocorrectEntries(entries);
    lineedit.autocorrection()->setEnabledAutoCorrection(true);
    lineedit.autocorrection()->setAdvancedAutocorrect(true);
    lineedit.show();
    QTest::qWaitForWindowShown(&lineedit);
    QTest::keyClicks(&lineedit, originalWord);
    QTest::keyClick(&lineedit, ' ');
    QCOMPARE(lineedit.toPlainText(), QString(replaceWord + QLatin1Char(' ')) );

}

void LineEditWithAutocorrectionTest::shouldNotReplaceWordWhenInexactText()
{
    PimCommon::LineEditWithAutoCorrection lineedit(0, QLatin1String("lineeditwithautocorrecttestrc"));
    const QString originalWord = QLatin1String("FOOFOO");
    const QString replaceWord = QLatin1String("BLABLA");
    QHash<QString, QString> entries;
    entries.insert(originalWord, replaceWord);
    lineedit.autocorrection()->setAutocorrectEntries(entries);
    lineedit.autocorrection()->setEnabledAutoCorrection(true);
    lineedit.autocorrection()->setAdvancedAutocorrect(true);
    lineedit.show();
    const QString nonExactText = QLatin1String("BLIBLI");
    QTest::qWaitForWindowShown(&lineedit);
    QTest::keyClicks(&lineedit, nonExactText);
    QTest::keyClick(&lineedit, ' ');
    QCOMPARE(lineedit.toPlainText(), QString(nonExactText + QLatin1Char(' ')) );
}

void LineEditWithAutocorrectionTest::shouldNotAddTwoSpace()
{
    PimCommon::LineEditWithAutoCorrection lineedit(0, QLatin1String("lineeditwithautocorrecttestrc"));
    const QString originalWord = QLatin1String("FOOFOO ");
    lineedit.autocorrection()->setSingleSpaces(true);
    lineedit.autocorrection()->setEnabledAutoCorrection(true);
    lineedit.show();
    QTest::qWaitForWindowShown(&lineedit);
    QTest::keyClicks(&lineedit, originalWord);
    QTest::keyClick(&lineedit, ' ');
    QCOMPARE(lineedit.toPlainText(), originalWord );
}

QTEST_KDEMAIN(LineEditWithAutocorrectionTest, GUI)
