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

#include "texttospeechwidgettest.h"
#include <qtest.h>
#include "texttospeech/texttospeechwidget.h"
#include <QToolButton>
#include <qtestmouse.h>

TextToSpeechWidgetTest::TextToSpeechWidgetTest(QObject *parent)
    : QObject(parent)
{

}

void TextToSpeechWidgetTest::shouldHaveDefaultValue()
{
    PimCommon::TextToSpeechWidget textToSpeechWidget;
    QCOMPARE( textToSpeechWidget.state(), PimCommon::TextToSpeechWidget::Stop);

    QToolButton *stopButton = qFindChild<QToolButton *>(&textToSpeechWidget, QLatin1String("stopbutton"));
    QVERIFY(stopButton);
    QVERIFY(stopButton->isEnabled());

    QToolButton *playPauseButton = qFindChild<QToolButton *>(&textToSpeechWidget, QLatin1String("playpausebutton"));
    QVERIFY(playPauseButton);
    QVERIFY(!playPauseButton->isEnabled());

}

void TextToSpeechWidgetTest::shouldChangeButtonEnableStateWhenChangeState()
{
    PimCommon::TextToSpeechWidget textToSpeechWidget;
    textToSpeechWidget.setState(PimCommon::TextToSpeechWidget::Play);

    QToolButton *stopButton = qFindChild<QToolButton *>(&textToSpeechWidget, QLatin1String("stopbutton"));
    QVERIFY(stopButton->isEnabled());

    QToolButton *playPauseButton = qFindChild<QToolButton *>(&textToSpeechWidget, QLatin1String("playpausebutton"));
    QVERIFY(playPauseButton->isEnabled());

    textToSpeechWidget.setState(PimCommon::TextToSpeechWidget::Pause);
    QVERIFY(stopButton->isEnabled());
    QVERIFY(playPauseButton->isEnabled());


    textToSpeechWidget.setState(PimCommon::TextToSpeechWidget::Stop);
    QVERIFY(stopButton->isEnabled());
    QVERIFY(!playPauseButton->isEnabled());

}

void TextToSpeechWidgetTest::shouldChangeStateWhenClickOnPlayPause()
{
    PimCommon::TextToSpeechWidget textToSpeechWidget;

    textToSpeechWidget.setState(PimCommon::TextToSpeechWidget::Play);
    QToolButton *playPauseButton = qFindChild<QToolButton *>(&textToSpeechWidget, QLatin1String("playpausebutton"));
    QCOMPARE( textToSpeechWidget.state(), PimCommon::TextToSpeechWidget::Play);

    QTest::mouseClick(playPauseButton, Qt::LeftButton);
    QCOMPARE( textToSpeechWidget.state(), PimCommon::TextToSpeechWidget::Pause);

    QTest::mouseClick(playPauseButton, Qt::LeftButton);
    QCOMPARE( textToSpeechWidget.state(), PimCommon::TextToSpeechWidget::Play);
}

void TextToSpeechWidgetTest::shouldChangeStateWhenClickOnStop()
{
    PimCommon::TextToSpeechWidget textToSpeechWidget;
    textToSpeechWidget.setState(PimCommon::TextToSpeechWidget::Play);

    QToolButton *stopButton = qFindChild<QToolButton *>(&textToSpeechWidget, QLatin1String("stopbutton"));
    QTest::mouseClick(stopButton, Qt::LeftButton);
    QCOMPARE( textToSpeechWidget.state(), PimCommon::TextToSpeechWidget::Stop);

}


QTEST_MAIN(TextToSpeechWidgetTest)
