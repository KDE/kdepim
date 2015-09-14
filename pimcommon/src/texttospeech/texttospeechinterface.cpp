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

#include "texttospeechinterface.h"
#include "texttospeech.h"
#include "pimcommon_debug.h"

using namespace PimCommon;

TextToSpeechInterface::TextToSpeechInterface(TextToSpeechWidget *textToSpeechWidget, QObject *parent)
    : AbstractTextToSpeechInterface(parent),
      mTextToSpeechWidget(textToSpeechWidget)
{
    PimCommon::TextToSpeech::self(); //init
    connect(mTextToSpeechWidget, &PimCommon::TextToSpeechWidget::stateChanged, this, &TextToSpeechInterface::stateChanged);
    connect(PimCommon::TextToSpeech::self(), &PimCommon::TextToSpeech::stateChanged, mTextToSpeechWidget, &PimCommon::TextToSpeechWidget::slotStateChanged);
}

TextToSpeechInterface::~TextToSpeechInterface()
{
}

bool TextToSpeechInterface::isReady() const
{
    return PimCommon::TextToSpeech::self()->isReady();
}

void TextToSpeechInterface::say(const QString &text)
{
    mTextToSpeechWidget->setState(PimCommon::TextToSpeechWidget::Play);
    mTextToSpeechWidget->show();
    PimCommon::TextToSpeech::self()->say(text);
}

int TextToSpeechInterface::volume() const
{
    return PimCommon::TextToSpeech::self()->volume();
}

void TextToSpeechInterface::setVolume(int value)
{
    PimCommon::TextToSpeech::self()->setVolume(value);
}

void TextToSpeechInterface::reloadSettings()
{
    PimCommon::TextToSpeech::self()->reloadSettings();
}

void TextToSpeechInterface::stateChanged(TextToSpeechWidget::State state)
{
    switch (state) {
    case TextToSpeechWidget::Stop:
        PimCommon::TextToSpeech::self()->stop();
        break;
    case TextToSpeechWidget::Play:
        PimCommon::TextToSpeech::self()->resume();
        break;
    case TextToSpeechWidget::Pause:
        PimCommon::TextToSpeech::self()->pause();
        break;
    }
}
