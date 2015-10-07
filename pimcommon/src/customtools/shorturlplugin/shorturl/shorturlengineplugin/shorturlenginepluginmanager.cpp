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

#include "shorturlengineplugin.h"
#include "shorturlenginepluginmanager.h"

#include <QFileInfo>
#include <QVector>
#include <kpluginmetadata.h>
#include <KPluginLoader>
#include <KPluginFactory>
#include <QSet>

using namespace PimCommon;

class ShortUrlEnginePluginManagerInstancePrivate
{
public:
    ShortUrlEnginePluginManagerInstancePrivate()
        : shortUrlEnginePluginManager(new ShortUrlEnginePluginManager)
    {
    }

    ~ShortUrlEnginePluginManagerInstancePrivate()
    {
        delete shortUrlEnginePluginManager;
    }

    ShortUrlEnginePluginManager *shortUrlEnginePluginManager;
};

Q_GLOBAL_STATIC(ShortUrlEnginePluginManagerInstancePrivate, sInstance)

class ShortUrlEnginePluginInfo
{
public:
    ShortUrlEnginePluginInfo()
        : plugin(Q_NULLPTR)
    {

    }
    QString saveName() const;

    KPluginMetaData metaData;
    PimCommon::ShortUrlEnginePlugin *plugin;
};

QString ShortUrlEnginePluginInfo::saveName() const
{
    return QFileInfo(metaData.fileName()).baseName();
}

class PimCommon::ShortUrlEnginePluginManagerPrivate
{
public:
    ShortUrlEnginePluginManagerPrivate(ShortUrlEnginePluginManager *qq)
        : q(qq)
    {

    }
    void initializePlugins();
    void loadPlugin(ShortUrlEnginePluginInfo *item);
    QVector<PimCommon::ShortUrlEnginePlugin *> pluginsList() const;
    QVector<ShortUrlEnginePluginInfo> mPluginList;
    ShortUrlEnginePluginManager *q;
};

void ShortUrlEnginePluginManagerPrivate::initializePlugins()
{
    const QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(QStringLiteral("pimcommon"), [](const KPluginMetaData & md) {
        return md.serviceTypes().contains(QStringLiteral("PimCommonShortUrlEngine/Plugin"));
    });

    QVectorIterator<KPluginMetaData> i(plugins);
    i.toBack();
    QSet<QString> unique;
    while (i.hasPrevious()) {
        ShortUrlEnginePluginInfo info;
        info.metaData = i.previous();

        // only load plugins once, even if found multiple times!
        if (unique.contains(info.saveName())) {
            continue;
        }
        info.plugin = Q_NULLPTR;
        mPluginList.push_back(info);
        unique.insert(info.saveName());
    }
    QVector<ShortUrlEnginePluginInfo>::iterator end(mPluginList.end());
    for (QVector<ShortUrlEnginePluginInfo>::iterator it = mPluginList.begin(); it != end; ++it) {
        loadPlugin(&(*it));
    }
}

void ShortUrlEnginePluginManagerPrivate::loadPlugin(ShortUrlEnginePluginInfo *item)
{
    item->plugin = KPluginLoader(item->metaData.fileName()).factory()->create<PimCommon::ShortUrlEnginePlugin>(q, QVariantList() << item->saveName());
    item->plugin->setPluginName(item->metaData.name());
}

QVector<ShortUrlEnginePlugin *> ShortUrlEnginePluginManagerPrivate::pluginsList() const
{
    QVector<PimCommon::ShortUrlEnginePlugin *> lst;
    QVector<ShortUrlEnginePluginInfo>::ConstIterator end(mPluginList.constEnd());
    for (QVector<ShortUrlEnginePluginInfo>::ConstIterator it = mPluginList.constBegin(); it != end; ++it) {
        if ((*it).plugin) {
            lst << (*it).plugin;
        }
    }
    return lst;
}

ShortUrlEnginePluginManager::ShortUrlEnginePluginManager(QObject *parent)
    : QObject(parent),
      d(new PimCommon::ShortUrlEnginePluginManagerPrivate(this))
{
    d->initializePlugins();
}

ShortUrlEnginePluginManager::~ShortUrlEnginePluginManager()
{
    delete d;
}

ShortUrlEnginePluginManager *ShortUrlEnginePluginManager::self()
{
    return sInstance->shortUrlEnginePluginManager;
}

QVector<ShortUrlEnginePlugin *> ShortUrlEnginePluginManager::pluginsList() const
{
    return d->pluginsList();
}

