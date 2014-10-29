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
#include "texttospeechconfigdialog.h"
#include <KLocalizedString>
#include <QHBoxLayout>
#include <QToolButton>
#include <QDebug>
#include <QLabel>
#include <QSlider>
#include <QPointer>

using namespace PimCommon;

TextToSpeechWidget::TextToSpeechWidget(QWidget *parent)
    : QWidget(parent),
      mState(Stop),
      mNeedToHide(false)
{
    QHBoxLayout *hbox = new QHBoxLayout;
    setLayout(hbox);
    //TODO add icons
    hbox->addStretch(0);
    QLabel *volume = new QLabel(i18n("Volume:"));
    hbox->addWidget(volume);
    mVolume = new QSlider;
    mVolume->setMinimumWidth(100);
    mVolume->setOrientation(Qt::Horizontal);
    mVolume->setObjectName(QStringLiteral("volumeslider"));
    mVolume->setRange(0, 100);
    connect(mVolume, &QSlider::valueChanged, this, &TextToSpeechWidget::slotVolumeChanged);
    hbox->addWidget(mVolume);

    mStopButton = new QToolButton;
    mStopButton->setObjectName(QLatin1String("stopbutton"));
    mStopButton->setToolTip(i18n("Stop"));
    connect(mStopButton, &QToolButton::clicked, this, &TextToSpeechWidget::slotStop);
    hbox->addWidget(mStopButton);

    mPlayPauseButton = new QToolButton;
    mPlayPauseButton->setObjectName(QLatin1String("playpausebutton"));
    connect(mPlayPauseButton, &QToolButton::clicked, this, &TextToSpeechWidget::slotPlayPause);
    hbox->addWidget(mPlayPauseButton);

    mConfigureButton = new QToolButton;
    mConfigureButton->setIcon(QIcon::fromTheme(QLatin1String("configure")));
    mConfigureButton->setToolTip(i18n("Configure..."));
    mConfigureButton->setObjectName(QLatin1String("configurebutton"));
    connect(mConfigureButton, &QToolButton::clicked, this, &TextToSpeechWidget::slotConfigure);
    hbox->addWidget(mConfigureButton);


    updateButtonState();
    mTextToSpeechInterface = new TextToSpeechInterface(this, this);
    hide();
}

TextToSpeechWidget::~TextToSpeechWidget()
{
}

void TextToSpeechWidget::slotConfigure()
{
    if (!mConfigDialog.data()) {
        mNeedToHide = false;
        mConfigDialog = new TextToSpeechConfigDialog(this);
        if (mConfigDialog->exec()) {
            //TODO reload config
        }
        delete mConfigDialog;
        if (mNeedToHide) {
            hide();
            mNeedToHide = false;
        }
    }
}

void TextToSpeechWidget::slotVolumeChanged(int value)
{
    mTextToSpeechInterface->setVolume(value);
}

bool TextToSpeechWidget::isReady() const
{
    return mTextToSpeechInterface->isReady();
}

void TextToSpeechWidget::say(const QString &text)
{
    if (mTextToSpeechInterface->isReady()) {
        mTextToSpeechInterface->say(text);
    }
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
    updateButtonState();
    Q_EMIT stateChanged(mState);
}

void TextToSpeechWidget::slotStop()
{
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

void TextToSpeechWidget::slotStateChanged(PimCommon::TextToSpeech::State state)
{
    if (state == PimCommon::TextToSpeech::Ready) {
        if (mConfigDialog) {
            mNeedToHide = true;
        } else {
            hide();
        }
    }
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
    mPlayPauseButton->setToolTip((mState != Play) ? i18n("Pause") : i18n("Play"));
}

void TextToSpeechWidget::setTextToSpeechInterface(AbstractTextToSpeechInterface *interface)
{
    delete mTextToSpeechInterface;
    mTextToSpeechInterface = interface;
}
