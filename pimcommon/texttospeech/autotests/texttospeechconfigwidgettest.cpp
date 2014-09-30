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

#include "texttospeechconfigwidgettest.h"
#include "pimcommon/texttospeech/texttospeechconfigwidget.h"

#include <qtest.h>
#include <QSlider>
#include <QSignalSpy>

TextToSpeechConfigWidgetTest::TextToSpeechConfigWidgetTest(QObject *parent)
    : QObject(parent)
{
}

TextToSpeechConfigWidgetTest::~TextToSpeechConfigWidgetTest()
{
}

void TextToSpeechConfigWidgetTest::shouldHaveDefaultValue()
{
    PimCommon::TextToSpeechConfigWidget textToSpeechConfigWidget;
    QSlider *volume = qFindChild<QSlider *>(&textToSpeechConfigWidget, QLatin1String("volume"));
    QVERIFY(volume);

    QSlider *rate = qFindChild<QSlider *>(&textToSpeechConfigWidget, QLatin1String("rate"));
    QVERIFY(rate);

    QSlider *pitch = qFindChild<QSlider *>(&textToSpeechConfigWidget, QLatin1String("pitch"));
    QVERIFY(pitch);

}

void TextToSpeechConfigWidgetTest::shouldEmitConfigChangedWhenChangeSliderValue()
{
    PimCommon::TextToSpeechConfigWidget textToSpeechConfigWidget;
    QSignalSpy spy(&textToSpeechConfigWidget, SIGNAL(configChanged(bool)));
    QSlider *volume = qFindChild<QSlider *>(&textToSpeechConfigWidget, QLatin1String("volume"));
    volume->setValue(5);
    QCOMPARE(spy.count(), 1);

    QSlider *rate = qFindChild<QSlider *>(&textToSpeechConfigWidget, QLatin1String("rate"));
    rate->setValue(5);
    QCOMPARE(spy.count(), 2);

    QSlider *pitch = qFindChild<QSlider *>(&textToSpeechConfigWidget, QLatin1String("pitch"));
    pitch->setValue(5);
    QCOMPARE(spy.count(), 3);
}

QTEST_MAIN(TextToSpeechConfigWidgetTest)
