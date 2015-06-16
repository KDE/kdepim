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
#include "texttospeechactions.h"
#include "texttospeechinterface.h"
#include "texttospeechconfigdialog.h"
#include <KLocalizedString>
#include <QHBoxLayout>
#include <QToolButton>
#include "pimcommon_debug.h"
#include <QLabel>
#include <QSlider>
#include <QPointer>

using namespace PimCommon;

TextToSpeechWidget::TextToSpeechWidget(QWidget *parent)
    : QWidget(parent),
      mNeedToHide(false)
{
    QHBoxLayout *hbox = new QHBoxLayout;
    setLayout(hbox);

    mTextToSpeechActions = new TextToSpeechActions(this);
    connect(mTextToSpeechActions, &TextToSpeechActions::stateChanged, this, &TextToSpeechWidget::stateChanged);

    QToolButton *close = new QToolButton(this);
    close->setObjectName(QStringLiteral("close-button"));
    close->setIcon(QIcon::fromTheme(QStringLiteral("dialog-close")));
    close->setToolTip(i18n("Close"));
    connect(close, &QToolButton::clicked, this, &TextToSpeechWidget::hide);
    hbox->addWidget(close);
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
    mStopButton->setObjectName(QStringLiteral("stopbutton"));
    mStopButton->setDefaultAction(mTextToSpeechActions->stopAction());
    hbox->addWidget(mStopButton);

    mPlayPauseButton = new QToolButton;
    mPlayPauseButton->setObjectName(QStringLiteral("playpausebutton"));
    mPlayPauseButton->setDefaultAction(mTextToSpeechActions->playPauseAction());
    hbox->addWidget(mPlayPauseButton);

    mConfigureButton = new QToolButton;
    mConfigureButton->setIcon(QIcon::fromTheme(QStringLiteral("configure")));
    mConfigureButton->setToolTip(i18n("Configure..."));
    mConfigureButton->setObjectName(QStringLiteral("configurebutton"));
    connect(mConfigureButton, &QToolButton::clicked, this, &TextToSpeechWidget::slotConfigure);
    hbox->addWidget(mConfigureButton);

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
            mTextToSpeechInterface->reloadSettings();
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

TextToSpeechWidget::State TextToSpeechWidget::state() const
{
    return mTextToSpeechActions->state();
}

void TextToSpeechWidget::slotStateChanged(PimCommon::TextToSpeech::State state)
{
    switch(state) {
    case PimCommon::TextToSpeech::Ready: {
        if (state == PimCommon::TextToSpeech::Ready) {
            mTextToSpeechActions->setState(TextToSpeechWidget::Stop);
            if (mConfigDialog) {
                mNeedToHide = true;
            } else {
                hide();
            }
        }
        break;
    }
    default:
        //TODO
        break;
    }
}

void TextToSpeechWidget::setState(TextToSpeechWidget::State state)
{
    mTextToSpeechActions->setState(state);
}

void TextToSpeechWidget::setTextToSpeechInterface(AbstractTextToSpeechInterface *interface)
{
    delete mTextToSpeechInterface;
    mTextToSpeechInterface = interface;
}
