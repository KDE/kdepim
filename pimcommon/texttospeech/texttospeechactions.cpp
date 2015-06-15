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

#include "texttospeechactions.h"
#include <KLocalizedString>
#include <QAction>

using namespace PimCommon;

TextToSpeechActions::TextToSpeechActions(QObject *parent)
    : QObject(parent),
      mState(TextToSpeechWidget::Stop),
      mStopAction(Q_NULLPTR),
      mPlayPauseAction(Q_NULLPTR)
{
    mStopAction = new QAction(i18n("Stop"), this);
    mStopAction->setObjectName(QStringLiteral("stopbutton"));
    mStopAction->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-stop")));
    mStopAction->setToolTip(i18n("Stop"));
    //connect(mStopAction, &QToolButton::clicked, this, &TextToSpeechWidget::slotStop);

    mPlayPauseAction = new QAction(this);
    mPlayPauseAction->setObjectName(QStringLiteral("playpausebutton"));
    mPlayPauseAction->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-start")));
    //connect(mPlayPauseAction, &QToolButton::clicked, this, &TextToSpeechWidget::slotPlayPause);
}

TextToSpeechActions::~TextToSpeechActions()
{

}

QAction *TextToSpeechActions::stopAction() const
{
    return mStopAction;
}

QAction *TextToSpeechActions::playPauseAction() const
{
    return mPlayPauseAction;
}

TextToSpeechWidget::State TextToSpeechActions::state() const
{
    return mState;
}

void TextToSpeechActions::setState(const TextToSpeechWidget::State &state)
{
    if (mState != state) {
        mState = state;
        updateButtonState();
    }
}

void TextToSpeechActions::updateButtonState()
{
    mPlayPauseAction->setIcon(QIcon::fromTheme((mState == TextToSpeechWidget::Stop) ? QStringLiteral("media-playback-start") : QStringLiteral("media-playback-stop")));
    mPlayPauseAction->setEnabled((mState != TextToSpeechWidget::Stop));
    const QString text = (mState != TextToSpeechWidget::Play) ? i18n("Pause") : i18n("Play");
    mPlayPauseAction->setToolTip(text);
    mPlayPauseAction->setText(text);
}

void TextToSpeechActions::slotPlayPause()
{
    if (mState == PimCommon::TextToSpeechWidget::Pause) {
        mState = PimCommon::TextToSpeechWidget::Play;
    } else if (mState == PimCommon::TextToSpeechWidget::Play) {
        mState = PimCommon::TextToSpeechWidget::Pause;
    } else if (mState == PimCommon::TextToSpeechWidget::Stop) {
        mState = PimCommon::TextToSpeechWidget::Play;
    } else {
        return;
    }
    updateButtonState();
    Q_EMIT stateChanged(mState);
}

void TextToSpeechActions::slotStop()
{
    if (mState != PimCommon::TextToSpeechWidget::Stop) {
        mState = PimCommon::TextToSpeechWidget::Stop;
        updateButtonState();
        Q_EMIT stateChanged(mState);
    }
}
