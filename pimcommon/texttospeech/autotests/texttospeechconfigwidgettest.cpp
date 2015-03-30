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
#include "pimcommon/texttospeech/abstracttexttospeechconfiginterface.h"

#include <qtest.h>
#include <QSlider>
#include <QSignalSpy>
#include <QComboBox>

TextToSpeechConfigWidgetTest::TextToSpeechConfigWidgetTest(QObject *parent)
    : QObject(parent)
{
}

TextToSpeechConfigWidgetTest::~TextToSpeechConfigWidgetTest()
{
}

void TextToSpeechConfigWidgetTest::addInterface(PimCommon::TextToSpeechConfigWidget *widget)
{
    PimCommon::AbstractTextToSpeechConfigInterface *interface = new PimCommon::AbstractTextToSpeechConfigInterface(this);
    widget->setTextToSpeechConfigInterface(interface);
}

void TextToSpeechConfigWidgetTest::shouldHaveDefaultValue()
{
    PimCommon::TextToSpeechConfigWidget textToSpeechConfigWidget;
    addInterface(&textToSpeechConfigWidget);
    QSlider *volume = textToSpeechConfigWidget.findChild<QSlider *>(QStringLiteral("volume"));
    QVERIFY(volume);

    QSlider *rate = textToSpeechConfigWidget.findChild<QSlider *>(QStringLiteral("rate"));
    QVERIFY(rate);

    QSlider *pitch = textToSpeechConfigWidget.findChild<QSlider *>(QStringLiteral("pitch"));
    QVERIFY(pitch);

    QComboBox *language = textToSpeechConfigWidget.findChild<QComboBox *>(QStringLiteral("language"));
    QVERIFY(language);
    //FIXME
    //QVERIFY(language->count()>0);
}

void TextToSpeechConfigWidgetTest::shouldEmitConfigChangedWhenChangeConfigValue()
{
    PimCommon::TextToSpeechConfigWidget textToSpeechConfigWidget;
    addInterface(&textToSpeechConfigWidget);
    QSignalSpy spy(&textToSpeechConfigWidget, SIGNAL(configChanged(bool)));
    QSlider *volume = textToSpeechConfigWidget.findChild<QSlider *>(QStringLiteral("volume"));
    volume->setValue(5);
    QCOMPARE(spy.count(), 1);

    QSlider *rate = textToSpeechConfigWidget.findChild<QSlider *>(QStringLiteral("rate"));
    rate->setValue(5);
    QCOMPARE(spy.count(), 2);

    QSlider *pitch = textToSpeechConfigWidget.findChild<QSlider *>(QStringLiteral("pitch"));
    pitch->setValue(5);
    QCOMPARE(spy.count(), 3);

    QComboBox *language = textToSpeechConfigWidget.findChild<QComboBox *>(QStringLiteral("language"));
    language->blockSignals(true);
    QStringList lst;
    lst << QStringLiteral("foo");
    lst << QStringLiteral("foo");
    lst << QStringLiteral("foo");
    lst << QStringLiteral("foo");
    language->addItems(lst);
    language->blockSignals(false);
    language->setCurrentIndex(3);
    QCOMPARE(spy.count(), 4);
}

QTEST_MAIN(TextToSpeechConfigWidgetTest)
