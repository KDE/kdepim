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

#include "texttospeechwidget.h"
#include "texttospeechinterface.h"
#include <KLocalizedString>
#include <QHBoxLayout>
#include <QToolButton>
#include <QDebug>

using namespace PimCommon;

TextToSpeechWidget::TextToSpeechWidget(QWidget *parent)
    : QWidget(parent),
      mState(Stop)
{
    QHBoxLayout *hbox = new QHBoxLayout;
    setLayout(hbox);
    //TODO add icons
    mStopButton = new QToolButton;
    mStopButton->setObjectName(QLatin1String("stopbutton"));
    connect(mStopButton, &QToolButton::clicked, this, &TextToSpeechWidget::slotStop);
    hbox->addWidget(mStopButton);
    mPlayPauseButton = new QToolButton;
    mPlayPauseButton->setObjectName(QLatin1String("playpausebutton"));
    connect(mPlayPauseButton, &QToolButton::clicked, this, &TextToSpeechWidget::slotPlayPause);
    hbox->addWidget(mPlayPauseButton);
    updateButtonState();
    mTextToSpeechInterface = new TextToSpeechInterface(this, this);
}

TextToSpeechWidget::~TextToSpeechWidget()
{
}

void TextToSpeechWidget::slotPlayPause()
{
    if (mState == Pause) {
        mState = Play;
    } else if (mState == Play) {
        mState = Pause;
    } else if (mState == Stop) {
        mState = Play;
    } else {
        return;
    }
    //TODO update icons
    updateButtonState();
    Q_EMIT stateChanged(mState);
}

void TextToSpeechWidget::slotStop()
{
    //TODO update icons.
    if (mState != Stop) {
        mState = Stop;
        updateButtonState();
        Q_EMIT stateChanged(mState);
    }
}

TextToSpeechWidget::State TextToSpeechWidget::state() const
{
    return mState;
}

void TextToSpeechWidget::setState(TextToSpeechWidget::State state)
{
    if (mState != state) {
        mState = state;
        updateButtonState();
    }
}

void TextToSpeechWidget::updateButtonState()
{
    //TODO update icons.
    mPlayPauseButton->setEnabled((mState != Stop));
}

void TextToSpeechWidget::setTextToSpeechInterface(AbstractTextToSpeechInterface *interface)
{
    delete mTextToSpeechInterface;
    mTextToSpeechInterface = interface;
}
