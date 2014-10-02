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

#include "texttospeechconfigwidget.h"
#include "settings/pimcommonsettings.h"

#include <KLocalizedString>

#include <QFormLayout>
#include <QSlider>

using namespace PimCommon;
TextToSpeechConfigWidget::TextToSpeechConfigWidget(QWidget *parent)
    : QWidget(parent)
{
    QFormLayout *layout = new QFormLayout;
    setLayout(layout);
    mVolume = new QSlider;
    mVolume->setObjectName(QLatin1String("volume"));
    mVolume->setRange(-100, 100);
    connect(mVolume, &QSlider::valueChanged, this, &TextToSpeechConfigWidget::valueChanged);

    layout->addRow(i18n("Volume:"), mVolume);

    mRate = new QSlider;
    mRate->setObjectName(QLatin1String("rate"));
    mRate->setRange(-100, 100);
    layout->addRow(i18n("Rate:"), mRate);
    connect(mRate, &QSlider::valueChanged, this, &TextToSpeechConfigWidget::valueChanged);

    mPitch = new QSlider;
    mPitch->setRange(-100, 100);
    connect(mPitch, &QSlider::valueChanged, this, &TextToSpeechConfigWidget::valueChanged);
    mPitch->setObjectName(QLatin1String("pitch"));
    layout->addRow(i18n("Pitch:"), mPitch);
}

TextToSpeechConfigWidget::~TextToSpeechConfigWidget()
{

}

void TextToSpeechConfigWidget::valueChanged()
{
    Q_EMIT configChanged(true);
}

void TextToSpeechConfigWidget::readConfig()
{
    PimCommon::PimCommonSettings::self()->setVolume(static_cast<double>(mVolume->value() / 100));
    PimCommon::PimCommonSettings::self()->setRate(static_cast<double>(mRate->value() / 100));
    PimCommon::PimCommonSettings::self()->setPitch(static_cast<double>(mPitch->value() / 100));
}

void TextToSpeechConfigWidget::writeConfig()
{
    mVolume->setValue(static_cast<int>(PimCommon::PimCommonSettings::self()->volume() * 100));
    mRate->setValue(static_cast<int>(PimCommon::PimCommonSettings::self()->rate() * 100));
    mPitch->setValue(static_cast<int>(PimCommon::PimCommonSettings::self()->pitch() * 100));
}
