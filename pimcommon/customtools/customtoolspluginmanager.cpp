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
#include <QDebug>

using namespace PimCommon;

class CustomToolsPluginInfo
{
public:
    KPluginMetaData metaData;
    PimCommon::CustomToolsPlugin *plugin;
};


class PimCommon::CustomToolsPluginManagerPrivate
{
public:
    CustomToolsPluginManagerPrivate()
    {

    }
    void initializePluginList();
};

void CustomToolsPluginManagerPrivate::initializePluginList()
{
    const QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(QStringLiteral("pimcommon"), [](const KPluginMetaData & md) {
        return md.serviceTypes().contains(QStringLiteral("PimCommonCustomTools/Plugin"));
    });
    qDebug()<<" plugins.count() "<<plugins.count();
}

CustomToolsPluginManager::CustomToolsPluginManager(QObject *parent)
    : d(new PimCommon::CustomToolsPluginManagerPrivate)
{
    d->initializePluginList();
}

CustomToolsPluginManager::~CustomToolsPluginManager()
{
    delete d;
}


