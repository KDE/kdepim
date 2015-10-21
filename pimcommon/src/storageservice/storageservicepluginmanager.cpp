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

#include "storageservicepluginmanager.h"
#include "storageserviceplugin.h"
#include <KPluginLoader>
#include <QFileInfo>
#include <QVectorIterator>
#include <kpluginmetadata.h>
#include <KPluginFactory>
#include <QSet>
#include <QVariant>

using namespace PimCommon;

class StorageServicePluginManagerInstancePrivate
{
public:
    StorageServicePluginManagerInstancePrivate()
        : storageServicePluginManager(new StorageServicePluginManager)
    {
    }

    ~StorageServicePluginManagerInstancePrivate()
    {
        delete storageServicePluginManager;
    }

    StorageServicePluginManager *storageServicePluginManager;
};

Q_GLOBAL_STATIC(StorageServicePluginManagerInstancePrivate, sInstance)

class StorageServicePluginInfo
{
public:
    StorageServicePluginInfo()
        : plugin(Q_NULLPTR)
    {

    }
    QString saveName() const;

    KPluginMetaData metaData;
    PimCommon::StorageServicePlugin *plugin;
};

QString StorageServicePluginInfo::saveName() const
{
    return QFileInfo(metaData.fileName()).baseName();
}

class PimCommon::StorageServicePluginManagerPrivate
{
public:
    StorageServicePluginManagerPrivate(StorageServicePluginManager *qq)
        : q(qq)
    {

    }
    QVector<PimCommon::StorageServicePlugin *> pluginsList() const;
    void initializePluginList();
    void loadPlugin(StorageServicePluginInfo *item);
    QVector<StorageServicePluginInfo> mPluginList;
    StorageServicePluginManager *q;
};

void StorageServicePluginManagerPrivate::initializePluginList()
{
    const QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(QStringLiteral("pimcommon"), [](const KPluginMetaData & md) {
        return md.serviceTypes().contains(QStringLiteral("PimCommonStorageService/Plugin"));
    });

    QVectorIterator<KPluginMetaData> i(plugins);
    i.toBack();
    QSet<QString> unique;
    while (i.hasPrevious()) {
        StorageServicePluginInfo info;
        info.metaData = i.previous();

        // only load plugins once, even if found multiple times!
        if (unique.contains(info.saveName())) {
            continue;
        }
        info.plugin = Q_NULLPTR;
        mPluginList.push_back(info);
        unique.insert(info.saveName());
    }
    QVector<StorageServicePluginInfo>::iterator end(mPluginList.end());
    for (QVector<StorageServicePluginInfo>::iterator it = mPluginList.begin(); it != end; ++it) {
        loadPlugin(&(*it));
    }
}

QVector<PimCommon::StorageServicePlugin *> StorageServicePluginManagerPrivate::pluginsList() const
{
    QVector<PimCommon::StorageServicePlugin *> lst;
    QVector<StorageServicePluginInfo>::ConstIterator end(mPluginList.constEnd());
    for (QVector<StorageServicePluginInfo>::ConstIterator it = mPluginList.constBegin(); it != end; ++it) {
        if ((*it).plugin) {
            lst << (*it).plugin;
        }
    }
    return lst;
}

void StorageServicePluginManagerPrivate::loadPlugin(StorageServicePluginInfo *item)
{
    item->plugin = KPluginLoader(item->metaData.fileName()).factory()->create<PimCommon::StorageServicePlugin>(q, QVariantList() << item->saveName());
}

StorageServicePluginManager::StorageServicePluginManager(QObject *parent)
    : QObject(parent),
      d(new PimCommon::StorageServicePluginManagerPrivate(this))
{

}

StorageServicePluginManager::~StorageServicePluginManager()
{
    delete d;
}

QVector<StorageServicePlugin *> StorageServicePluginManager::pluginsList() const
{
    return d->pluginsList();
}

StorageServicePluginManager *StorageServicePluginManager::self()
{
    return sInstance->storageServicePluginManager;
}

