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

#include "texttospeechactionstest.h"
#include "../texttospeechactions.h"
#include <qtest.h>
#include <QAction>
#include <QSignalSpy>

Q_DECLARE_METATYPE(PimCommon::TextToSpeechWidget::State)

TextToSpeechActionsTest::TextToSpeechActionsTest(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<PimCommon::TextToSpeechWidget::State>();
}

TextToSpeechActionsTest::~TextToSpeechActionsTest()
{

}

void TextToSpeechActionsTest::shouldHaveDefaultValue()
{
    PimCommon::TextToSpeechActions act;
    QVERIFY(act.stopAction());
    QVERIFY(act.playPauseAction());
    QCOMPARE(act.state(), PimCommon::TextToSpeechWidget::Stop);

    QVERIFY(act.stopAction()->isEnabled());
    QVERIFY(!act.stopAction()->icon().isNull());

    QVERIFY(!act.playPauseAction()->isEnabled());
    QVERIFY(!act.playPauseAction()->icon().isNull());
}

void TextToSpeechActionsTest::shouldChangeButtonEnableStateWhenChangeState()
{
    PimCommon::TextToSpeechActions act;
    act.setState(PimCommon::TextToSpeechWidget::Play);

    QVERIFY(act.stopAction()->isEnabled());
    QVERIFY(act.playPauseAction()->isEnabled());

    act.setState(PimCommon::TextToSpeechWidget::Pause);
    QVERIFY(act.stopAction()->isEnabled());
    QVERIFY(act.playPauseAction()->isEnabled());

    act.setState(PimCommon::TextToSpeechWidget::Stop);

    QVERIFY(act.stopAction()->isEnabled());
    QVERIFY(!act.playPauseAction()->isEnabled());
}

void TextToSpeechActionsTest::shouldChangeStateWhenClickOnPlayPause()
{
    PimCommon::TextToSpeechActions act;
    act.setState(PimCommon::TextToSpeechWidget::Play);
    QCOMPARE(act.state(), PimCommon::TextToSpeechWidget::Play);

    act.playPauseAction()->trigger();
    QCOMPARE(act.state(), PimCommon::TextToSpeechWidget::Pause);

    act.playPauseAction()->trigger();
    QCOMPARE(act.state(), PimCommon::TextToSpeechWidget::Play);
}

void TextToSpeechActionsTest::shouldChangeStateWhenClickOnStop()
{
    PimCommon::TextToSpeechActions act;
    act.setState(PimCommon::TextToSpeechWidget::Play);

    act.stopAction()->trigger();
    QCOMPARE(act.state(), PimCommon::TextToSpeechWidget::Stop);
}

void TextToSpeechActionsTest::shouldEmitStateChanged()
{
    PimCommon::TextToSpeechActions act;
    act.setState(PimCommon::TextToSpeechWidget::Play);
    QSignalSpy spy(&act, SIGNAL(stateChanged(PimCommon::TextToSpeechWidget::State)));
    act.setState(PimCommon::TextToSpeechWidget::Play);
    QCOMPARE(spy.count(), 0);


    act.playPauseAction()->trigger();
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).value<PimCommon::TextToSpeechWidget::State>(), PimCommon::TextToSpeechWidget::Pause);
    act.playPauseAction()->trigger();
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(1).at(0).value<PimCommon::TextToSpeechWidget::State>(), PimCommon::TextToSpeechWidget::Play);
    act.playPauseAction()->trigger();
    QCOMPARE(spy.count(), 3);
    QCOMPARE(spy.at(2).at(0).value<PimCommon::TextToSpeechWidget::State>(), PimCommon::TextToSpeechWidget::Pause);
    act.stopAction()->trigger();
    QCOMPARE(spy.count(), 4);
    QCOMPARE(spy.at(3).at(0).value<PimCommon::TextToSpeechWidget::State>(), PimCommon::TextToSpeechWidget::Stop);
}


QTEST_MAIN(TextToSpeechActionsTest)
