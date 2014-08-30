/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "shorturlmainwidget.h"
#include "shorturl/shorturlwidget.h"
#include "shorturl/shorturlconfigurewidget.h"

#include <QVBoxLayout>
#include <QPushButton>

ShortUrlMainWidget::ShortUrlMainWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *lay = new QVBoxLayout;
    setLayout(lay);
    mConfigWidget = new PimCommon::ShortUrlConfigureWidget;
    lay->addWidget(mConfigWidget);

    QPushButton *saveConfig = new QPushButton(QLatin1String("Save config"));
    connect(saveConfig, &QPushButton::clicked, this, &ShortUrlMainWidget::slotSaveConfig);
    lay->addWidget(saveConfig);

    mShortUrlWidget = new PimCommon::ShortUrlWidget();
    lay->addWidget(mShortUrlWidget);
    connect(mConfigWidget, &PimCommon::ShortUrlConfigureWidget::settingsChanged, mShortUrlWidget, &PimCommon::ShortUrlWidget::settingsUpdated);
}

void ShortUrlMainWidget::slotSaveConfig()
{
    mConfigWidget->writeConfig();
}
