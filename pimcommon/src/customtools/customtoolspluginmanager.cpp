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

#include "customtoolsplugin.h"
#include "customtoolspluginmanager.h"
#include <KPluginFactory>
#include <KPluginLoader>
#include <kpluginmetadata.h>
#include <QFileInfo>
#include <QSet>

using namespace PimCommon;

class CustomToolsPluginManagerInstancePrivate
{
public:
    CustomToolsPluginManagerInstancePrivate()
        : customToolsPluginManager(new CustomToolsPluginManager)
    {
    }

    ~CustomToolsPluginManagerInstancePrivate()
    {
        delete customToolsPluginManager;
    }

    CustomToolsPluginManager *customToolsPluginManager;
};

Q_GLOBAL_STATIC(CustomToolsPluginManagerInstancePrivate, sInstance)

class CustomToolsPluginInfo
{
public:
    CustomToolsPluginInfo()
        : plugin(Q_NULLPTR)
    {

    }
    QString saveName() const;

    KPluginMetaData metaData;
    PimCommon::CustomToolsPlugin *plugin;
};

QString CustomToolsPluginInfo::saveName() const
{
    return QFileInfo(metaData.fileName()).baseName();
}

class PimCommon::CustomToolsPluginManagerPrivate
{
public:
    CustomToolsPluginManagerPrivate(CustomToolsPluginManager *qq)
        : q(qq)
    {

    }
    QVector<PimCommon::CustomToolsPlugin *> pluginsList() const;
    void initializePluginList();
    void loadPlugin(CustomToolsPluginInfo *item);
    QVector<CustomToolsPluginInfo> mPluginList;
    CustomToolsPluginManager *q;
};

void CustomToolsPluginManagerPrivate::initializePluginList()
{
    const QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(QStringLiteral("pimcommon"), [](const KPluginMetaData & md) {
        return md.serviceTypes().contains(QStringLiteral("PimCommonCustomTools/Plugin"));
    });

    QVectorIterator<KPluginMetaData> i(plugins);
    i.toBack();
    QSet<QString> unique;
    while (i.hasPrevious()) {
        CustomToolsPluginInfo info;
        info.metaData = i.previous();

        // only load plugins once, even if found multiple times!
        if (unique.contains(info.saveName())) {
            continue;
        }
        info.plugin = Q_NULLPTR;
        mPluginList.push_back(info);
        unique.insert(info.saveName());
    }
    QVector<CustomToolsPluginInfo>::iterator end(mPluginList.end());
    for (QVector<CustomToolsPluginInfo>::iterator it = mPluginList.begin(); it != end; ++it) {
        loadPlugin(&(*it));
    }
}

QVector<PimCommon::CustomToolsPlugin *> CustomToolsPluginManagerPrivate::pluginsList() const
{
    QVector<PimCommon::CustomToolsPlugin *> lst;
    QVector<CustomToolsPluginInfo>::ConstIterator end(mPluginList.constEnd());
    for (QVector<CustomToolsPluginInfo>::ConstIterator it = mPluginList.constBegin(); it != end; ++it) {
        if ((*it).plugin) {
            lst << (*it).plugin;
        }
    }
    return lst;
}

void CustomToolsPluginManagerPrivate::loadPlugin(CustomToolsPluginInfo *item)
{
    item->plugin = KPluginLoader(item->metaData.fileName()).factory()->create<PimCommon::CustomToolsPlugin>(q, QVariantList() << item->saveName());
}

CustomToolsPluginManager *CustomToolsPluginManager::self()
{
    return sInstance->customToolsPluginManager;
}

CustomToolsPluginManager::CustomToolsPluginManager(QObject *parent)
    : QObject(parent),
      d(new PimCommon::CustomToolsPluginManagerPrivate(this))
{
    d->initializePluginList();
}

CustomToolsPluginManager::~CustomToolsPluginManager()
{
    delete d;
}

QVector<PimCommon::CustomToolsPlugin *> CustomToolsPluginManager::pluginsList() const
{
    return d->pluginsList();
}
