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

#include "genericplugin.h"
#include "genericpluginmanager.h"

#include <kpluginmetadata.h>
#include <KPluginLoader>
#include <KPluginFactory>
#include <qfileinfo.h>
#include <QVariant>
#include <QSet>
#include <QVariantList>

using namespace PimCommon;

class GenericPluginManagerInstancePrivate
{
public:
    GenericPluginManagerInstancePrivate()
        : genericPluginManager(new GenericPluginManager)
    {
    }

    ~GenericPluginManagerInstancePrivate()
    {
        delete genericPluginManager;
    }

    GenericPluginManager *genericPluginManager;
};

Q_GLOBAL_STATIC(GenericPluginManagerInstancePrivate, sInstance)

class GenericPluginInfo
{
public:
    GenericPluginInfo()
        : plugin(Q_NULLPTR)
    {

    }
    QString saveName() const;

    KPluginMetaData metaData;
    PimCommon::GenericPlugin *plugin;
};

QString GenericPluginInfo::saveName() const
{
    return QFileInfo(metaData.fileName()).baseName();
}

class PimCommon::GenericPluginManagerPrivate
{
public:
    GenericPluginManagerPrivate(GenericPluginManager *qq)
        : q(qq)
    {

    }
    void loadPlugin(GenericPluginInfo *item);
    QVector<GenericPlugin *> pluginsList() const;
    bool initializePlugins();
    QString serviceTypeName;
    QString pluginName;
    QVector<GenericPluginInfo> mPluginList;
    GenericPluginManager *q;
};

bool GenericPluginManagerPrivate::initializePlugins()
{
    if (serviceTypeName.isEmpty() || pluginName.isEmpty()) {
        return false;
    }
    if (!mPluginList.isEmpty()) {
        return true;
    }
    static const QString s_serviceTypeName = serviceTypeName;
    const QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(pluginName, [](const KPluginMetaData & md) {
        return md.serviceTypes().contains(s_serviceTypeName);
    });

    QVectorIterator<KPluginMetaData> i(plugins);
    i.toBack();
    QSet<QString> unique;
    while (i.hasPrevious()) {
        GenericPluginInfo info;
        info.metaData = i.previous();

        // only load plugins once, even if found multiple times!
        if (unique.contains(info.saveName())) {
            continue;
        }
        info.plugin = Q_NULLPTR;
        mPluginList.push_back(info);
        unique.insert(info.saveName());
    }
    QVector<GenericPluginInfo>::iterator end(mPluginList.end());
    for (QVector<GenericPluginInfo>::iterator it = mPluginList.begin(); it != end; ++it) {
        loadPlugin(&(*it));
    }
    return true;
}

QVector<GenericPlugin *> GenericPluginManagerPrivate::pluginsList() const
{
    QVector<PimCommon::GenericPlugin *> lst;
    QVector<GenericPluginInfo>::ConstIterator end(mPluginList.constEnd());
    for (QVector<GenericPluginInfo>::ConstIterator it = mPluginList.constBegin(); it != end; ++it) {
        if ((*it).plugin) {
            lst << (*it).plugin;
        }
    }
    return lst;
}

void GenericPluginManagerPrivate::loadPlugin(GenericPluginInfo *item)
{
    item->plugin = KPluginLoader(item->metaData.fileName()).factory()->create<PimCommon::GenericPlugin>(q, QVariantList() << item->saveName());
}

GenericPluginManager::GenericPluginManager(QObject *parent)
    : QObject(parent),
      d(new GenericPluginManagerPrivate(this))
{

}

GenericPluginManager::~GenericPluginManager()
{
    delete d;
}

bool GenericPluginManager::initializePlugins()
{
    return d->initializePlugins();
}

void GenericPluginManager::setServiceTypeName(const QString &serviceName)
{
    d->serviceTypeName = serviceName;
}

QString GenericPluginManager::serviceTypeName() const
{
    return d->serviceTypeName;
}

void GenericPluginManager::setPluginName(const QString &pluginName)
{
    d->pluginName = pluginName;
}

QString GenericPluginManager::pluginName() const
{
    return d->pluginName;
}

GenericPluginManager *GenericPluginManager::self()
{
    return sInstance->genericPluginManager;
}

QVector<GenericPlugin *> GenericPluginManager::pluginsList() const
{
    return d->pluginsList();
}

