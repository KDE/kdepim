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

#include "texttospeechinterface.h"
#include "texttospeech.h"
#include <QDebug>

using namespace PimCommon;

TextToSpeechInterface::TextToSpeechInterface(TextToSpeechWidget *textToSpeechWidget, QObject *parent)
    : AbstractTextToSpeechInterface(parent),
      mTextToSpeechWidget(textToSpeechWidget)
{
    connect(PimCommon::TextToSpeech::self(), &PimCommon::TextToSpeech::emitSay, this, &TextToSpeechInterface::say);
    connect(mTextToSpeechWidget, &PimCommon::TextToSpeechWidget::stateChanged, this, &TextToSpeechInterface::stateChanged);
}

TextToSpeechInterface::~TextToSpeechInterface()
{
}

void TextToSpeechInterface::say()
{
    mTextToSpeechWidget->setState(PimCommon::TextToSpeechWidget::Play);
    mTextToSpeechWidget->show();
}

void TextToSpeechInterface::stateChanged(TextToSpeechWidget::State state)
{
    switch(state) {
    case TextToSpeechWidget::Stop:
        PimCommon::TextToSpeech::self()->stop();
        //TODO hide widget
        break;
    case TextToSpeechWidget::Play:
        PimCommon::TextToSpeech::self()->resume();
        break;
    case TextToSpeechWidget::Pause:
        PimCommon::TextToSpeech::self()->pause();
        break;
    }
}
