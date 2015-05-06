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
#include "abstracttexttospeechconfiginterface.h"
#include "texttospeechconfiginterface.h"
#include "texttospeechlanguagecombobox.h"
#include <KLocalizedString>

#include <QFormLayout>
#include <QSlider>
#include <QComboBox>
#include <QTimer>

using namespace PimCommon;
TextToSpeechConfigWidget::TextToSpeechConfigWidget(QWidget *parent)
    : QWidget(parent),
      mAbstractTextToSpeechConfigInterface(new TextToSpeechConfigInterface(this))
{
    QFormLayout *layout = new QFormLayout;
    setLayout(layout);
    mVolume = new QSlider;
    mVolume->setObjectName(QStringLiteral("volume"));
    mVolume->setRange(0, 100);
    mVolume->setOrientation(Qt::Horizontal);
    connect(mVolume, &QSlider::valueChanged, this, &TextToSpeechConfigWidget::valueChanged);

    layout->addRow(i18n("Volume:"), mVolume);

    mRate = new QSlider;
    mRate->setObjectName(QStringLiteral("rate"));
    mRate->setRange(-100, 100);
    mRate->setOrientation(Qt::Horizontal);
    layout->addRow(i18n("Rate:"), mRate);
    connect(mRate, &QSlider::valueChanged, this, &TextToSpeechConfigWidget::valueChanged);

    mPitch = new QSlider;
    mPitch->setRange(-100, 100);
    connect(mPitch, &QSlider::valueChanged, this, &TextToSpeechConfigWidget::valueChanged);
    mPitch->setObjectName(QStringLiteral("pitch"));
    mPitch->setOrientation(Qt::Horizontal);
    layout->addRow(i18n("Pitch:"), mPitch);

    mLanguage = new PimCommon::TextToSpeechLanguageComboBox;
    mLanguage->setObjectName(QStringLiteral("language"));
    layout->addRow(i18n("Language:"), mLanguage);
    connect(mLanguage, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &TextToSpeechConfigWidget::valueChanged);

    QTimer::singleShot(0, this, SLOT(slotUpdateAvailableLocales()));
}

TextToSpeechConfigWidget::~TextToSpeechConfigWidget()
{

}

void TextToSpeechConfigWidget::valueChanged()
{
    Q_EMIT configChanged(true);
}

void TextToSpeechConfigWidget::updateLocale()
{
    const QString localeName = PimCommon::PimCommonSettings::self()->localeName();
    if (localeName.isEmpty()) {
        return;
    }
    mLanguage->selectLocaleName(localeName);
}

void TextToSpeechConfigWidget::readConfig()
{
    mVolume->setValue(static_cast<int>(PimCommon::PimCommonSettings::self()->volume()));
    mRate->setValue(static_cast<int>(PimCommon::PimCommonSettings::self()->rate() * 100));
    mPitch->setValue(static_cast<int>(PimCommon::PimCommonSettings::self()->pitch() * 100));
    updateLocale();
}

void TextToSpeechConfigWidget::writeConfig()
{
    PimCommon::PimCommonSettings::self()->setVolume(mVolume->value());
    PimCommon::PimCommonSettings::self()->setRate(static_cast<double>(mRate->value() / 100));
    PimCommon::PimCommonSettings::self()->setPitch(static_cast<double>(mPitch->value() / 100));
    PimCommon::PimCommonSettings::self()->setLocaleName(mLanguage->currentData().value<QLocale>().name());
}

void TextToSpeechConfigWidget::setTextToSpeechConfigInterface(AbstractTextToSpeechConfigInterface *interface)
{
    delete mAbstractTextToSpeechConfigInterface;
    mAbstractTextToSpeechConfigInterface = interface;
    slotUpdateAvailableLocales();
}

void TextToSpeechConfigWidget::slotUpdateAvailableLocales()
{
    mLanguage->clear();
    const QVector<QLocale> locales = mAbstractTextToSpeechConfigInterface->availableLocales();
    QLocale current = mAbstractTextToSpeechConfigInterface->locale();
    mLanguage->updateAvailableLocales(locales, current);
    updateLocale();
}
