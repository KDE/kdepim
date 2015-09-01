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

class PimCommon::TextToSpeechWidgetPrivate
{
public:
    TextToSpeechWidgetPrivate()
        : mNeedToHide(false),
          mStopButton(Q_NULLPTR),
          mPlayPauseButton(Q_NULLPTR),
          mConfigureButton(Q_NULLPTR),
          mTextToSpeechInterface(Q_NULLPTR),
          mTextToSpeechActions(Q_NULLPTR),
          mVolume(Q_NULLPTR)
    {

    }

    bool mNeedToHide;
    QPointer<PimCommon::TextToSpeechConfigDialog> mConfigDialog;
    QToolButton *mStopButton;
    QToolButton *mPlayPauseButton;
    QToolButton *mConfigureButton;
    AbstractTextToSpeechInterface *mTextToSpeechInterface;
    TextToSpeechActions *mTextToSpeechActions;
    QSlider *mVolume;
};


TextToSpeechWidget::TextToSpeechWidget(QWidget *parent)
    : QWidget(parent),
      d(new PimCommon::TextToSpeechWidgetPrivate)
{
    QHBoxLayout *hbox = new QHBoxLayout;
    setLayout(hbox);

    d->mTextToSpeechActions = new TextToSpeechActions(this);
    connect(d->mTextToSpeechActions, &TextToSpeechActions::stateChanged, this, &TextToSpeechWidget::stateChanged);

    QToolButton *close = new QToolButton(this);
    close->setObjectName(QStringLiteral("close-button"));
    close->setIcon(QIcon::fromTheme(QStringLiteral("dialog-close")));
    close->setToolTip(i18n("Close"));
    connect(close, &QToolButton::clicked, this, &TextToSpeechWidget::hide);
    hbox->addWidget(close);
    hbox->addStretch(0);

    QLabel *volume = new QLabel(i18n("Volume:"));
    hbox->addWidget(volume);
    d->mVolume = new QSlider;
    d->mVolume->setMinimumWidth(100);
    d->mVolume->setOrientation(Qt::Horizontal);
    d->mVolume->setObjectName(QStringLiteral("volumeslider"));
    d->mVolume->setRange(0, 100);
    connect(d->mVolume, &QSlider::valueChanged, this, &TextToSpeechWidget::slotVolumeChanged);
    hbox->addWidget(d->mVolume);

    d->mStopButton = new QToolButton;
    d->mStopButton->setObjectName(QStringLiteral("stopbutton"));
    d->mStopButton->setDefaultAction(d->mTextToSpeechActions->stopAction());
    hbox->addWidget(d->mStopButton);

    d->mPlayPauseButton = new QToolButton;
    d->mPlayPauseButton->setObjectName(QStringLiteral("playpausebutton"));
    d->mPlayPauseButton->setDefaultAction(d->mTextToSpeechActions->playPauseAction());
    hbox->addWidget(d->mPlayPauseButton);

    d->mConfigureButton = new QToolButton;
    d->mConfigureButton->setIcon(QIcon::fromTheme(QStringLiteral("configure")));
    d->mConfigureButton->setToolTip(i18n("Configure..."));
    d->mConfigureButton->setObjectName(QStringLiteral("configurebutton"));
    connect(d->mConfigureButton, &QToolButton::clicked, this, &TextToSpeechWidget::slotConfigure);
    hbox->addWidget(d->mConfigureButton);

    d->mTextToSpeechInterface = new TextToSpeechInterface(this, this);
    hide();
}

TextToSpeechWidget::~TextToSpeechWidget()
{
    delete d;
}

void TextToSpeechWidget::slotConfigure()
{
    if (!d->mConfigDialog.data()) {
        d->mNeedToHide = false;
        d->mConfigDialog = new TextToSpeechConfigDialog(this);
        if (d->mConfigDialog->exec()) {
            d->mTextToSpeechInterface->reloadSettings();
        }
        delete d->mConfigDialog;
        if (d->mNeedToHide) {
            hide();
            d->mNeedToHide = false;
        }
    }
}

void TextToSpeechWidget::slotVolumeChanged(int value)
{
    d->mTextToSpeechInterface->setVolume(value);
}

bool TextToSpeechWidget::isReady() const
{
    return d->mTextToSpeechInterface->isReady();
}

void TextToSpeechWidget::say(const QString &text)
{
    if (d->mTextToSpeechInterface->isReady()) {
        d->mTextToSpeechInterface->say(text);
    }
}

TextToSpeechWidget::State TextToSpeechWidget::state() const
{
    return d->mTextToSpeechActions->state();
}

void TextToSpeechWidget::slotStateChanged(PimCommon::TextToSpeech::State state)
{
    switch (state) {
    case PimCommon::TextToSpeech::Ready: {
        if (state == PimCommon::TextToSpeech::Ready) {
            d->mTextToSpeechActions->setState(TextToSpeechWidget::Stop);
            if (d->mConfigDialog) {
                d->mNeedToHide = true;
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
    d->mTextToSpeechActions->setState(state);
}

void TextToSpeechWidget::setTextToSpeechInterface(AbstractTextToSpeechInterface *interface)
{
    delete d->mTextToSpeechInterface;
    d->mTextToSpeechInterface = interface;
}
