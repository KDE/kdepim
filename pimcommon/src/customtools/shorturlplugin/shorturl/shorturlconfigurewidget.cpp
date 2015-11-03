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
#include "shorturlengineplugin/shorturlenginepluginmanager.h"

#include <KLocalizedString>

#include <QLabel>
#include <QComboBox>
#include <QHBoxLayout>
#include <KConfigGroup>
#include <KSharedConfig>

#include <pimcommon/shorturlengineplugin.h>

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
    const QVector<PimCommon::ShortUrlEnginePlugin *>  lstPlugin = PimCommon::ShortUrlEnginePluginManager::self()->pluginsList();
    Q_FOREACH (PimCommon::ShortUrlEnginePlugin *plugin, lstPlugin) {
        d->mShortUrlServer->addItem(plugin->pluginName(), plugin->engineName());
    }
}

void ShortUrlConfigureWidget::loadConfig()
{
    KConfigGroup grp(KSharedConfig::openConfig(), "ShortUrl");
    const QString engineName = grp.readEntry("EngineName");
    int index = d->mShortUrlServer->findData(engineName);
    if (index < 0) {
        index = 0;
    }
    d->mShortUrlServer->setCurrentIndex(index);
    d->mChanged = false;
}

void ShortUrlConfigureWidget::writeConfig()
{
    if (d->mChanged) {
        KConfigGroup grp(KSharedConfig::openConfig(), "ShortUrl");
        const QString engineName = grp.readEntry("EngineName");
        grp.writeEntry("EngineName", d->mShortUrlServer->itemData(d->mShortUrlServer->currentIndex()).toString());
        grp.sync();
        Q_EMIT settingsChanged();
    }
    d->mChanged = false;
}

void ShortUrlConfigureWidget::resetToDefault()
{
    d->mShortUrlServer->setCurrentIndex(0);
    d->mChanged = false;
}

