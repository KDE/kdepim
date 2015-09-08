/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include "shorturlconfigurewidget.h"
#include "shorturlutils.h"

#include <KLocalizedString>

#include <QLabel>
#include <QComboBox>
#include <QHBoxLayout>

using namespace PimCommon;

class PimCommon::ShortUrlConfigureWidgetPrivate
{
public:
    ShortUrlConfigureWidgetPrivate()
        : mShortUrlServer(Q_NULLPTR),
          mChanged(false)
    {

    }
    QComboBox *mShortUrlServer;
    bool mChanged;
};

ShortUrlConfigureWidget::ShortUrlConfigureWidget(QWidget *parent)
    : QWidget(parent),
      d(new PimCommon::ShortUrlConfigureWidgetPrivate)
{
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);

    QLabel *lab = new QLabel(i18n("Select Short URL server:"));
    lay->addWidget(lab);

    d->mShortUrlServer = new QComboBox;
    connect(d->mShortUrlServer, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &ShortUrlConfigureWidget::slotChanged);
    lay->addWidget(d->mShortUrlServer);
    setLayout(lay);
    init();
    loadConfig();
}

ShortUrlConfigureWidget::~ShortUrlConfigureWidget()
{
    delete d;
}

void ShortUrlConfigureWidget::slotChanged()
{
    d->mChanged = true;
}

void ShortUrlConfigureWidget::init()
{
    //Google doesn't work now.
    for (int i = PimCommon::ShortUrlUtils::Tinyurl; i < PimCommon::ShortUrlUtils::EndListEngine; ++i) {
        d->mShortUrlServer->addItem(PimCommon::ShortUrlUtils::stringFromEngineType(static_cast<PimCommon::ShortUrlUtils::EngineType>(i)), i);
    }
}

void ShortUrlConfigureWidget::loadConfig()
{
    const int engineType = PimCommon::ShortUrlUtils::readEngineSettings();
    int index = d->mShortUrlServer->findData(engineType);
    if (index < 0) {
        index = 0;
    }
    d->mShortUrlServer->setCurrentIndex(index);
    d->mChanged = false;
}

void ShortUrlConfigureWidget::writeConfig()
{
    if (d->mChanged) {
        PimCommon::ShortUrlUtils::writeEngineSettings(d->mShortUrlServer->itemData(d->mShortUrlServer->currentIndex()).toInt());
        Q_EMIT settingsChanged();
    }
    d->mChanged = false;
}

void ShortUrlConfigureWidget::resetToDefault()
{
    d->mShortUrlServer->setCurrentIndex(0);
    d->mChanged = false;
}

