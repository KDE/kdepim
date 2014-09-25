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
#include "../abstracttexttospeechinterface.h"
#include <qtest.h>
#include "texttospeech/texttospeechwidget.h"
#include <QToolButton>
#include <qtestmouse.h>
#include <QSignalSpy>


//TODO fix me !
//qRegisterMetaType<PimCommon::TextToSpeechWidget::State>();

TextToSpeechWidgetTest::TextToSpeechWidgetTest(QObject *parent)
    : QObject(parent)
{

}

void TextToSpeechWidgetTest::addInterface(PimCommon::TextToSpeechWidget *widget)
{
    PimCommon::AbstractTextToSpeechInterface *interface = new PimCommon::AbstractTextToSpeechInterface(this);
    widget->setTextToSpeechInterface(interface);
}

void TextToSpeechWidgetTest::shouldHaveDefaultValue()
{
    PimCommon::TextToSpeechWidget textToSpeechWidget;
    addInterface(&textToSpeechWidget);
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
    addInterface(&textToSpeechWidget);
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
    addInterface(&textToSpeechWidget);

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
    addInterface(&textToSpeechWidget);
    textToSpeechWidget.setState(PimCommon::TextToSpeechWidget::Play);

    QToolButton *stopButton = qFindChild<QToolButton *>(&textToSpeechWidget, QLatin1String("stopbutton"));
    QTest::mouseClick(stopButton, Qt::LeftButton);
    QCOMPARE( textToSpeechWidget.state(), PimCommon::TextToSpeechWidget::Stop);

}

void TextToSpeechWidgetTest::shouldEmitStateChanged()
{
    PimCommon::TextToSpeechWidget textToSpeechWidget;
    addInterface(&textToSpeechWidget);
    QSignalSpy spy(&textToSpeechWidget, SIGNAL(stateChanged(PimCommon::TextToSpeechWidget::State)));
    textToSpeechWidget.setState(PimCommon::TextToSpeechWidget::Play);
    QCOMPARE(spy.count(), 0);

    QToolButton *stopButton = qFindChild<QToolButton *>(&textToSpeechWidget, QLatin1String("stopbutton"));

    QToolButton *playPauseButton = qFindChild<QToolButton *>(&textToSpeechWidget, QLatin1String("playpausebutton"));

    QTest::mouseClick(playPauseButton, Qt::LeftButton);
    QCOMPARE(spy.count(), 1);
    QTest::mouseClick(playPauseButton, Qt::LeftButton);
    QCOMPARE(spy.count(), 2);
    QTest::mouseClick(playPauseButton, Qt::LeftButton);
    QCOMPARE(spy.count(), 3);
    QTest::mouseClick(stopButton, Qt::LeftButton);
    QCOMPARE(spy.count(), 4);
}



QTEST_MAIN(TextToSpeechWidgetTest)
