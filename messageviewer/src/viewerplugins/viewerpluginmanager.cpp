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

#include "viewerpluginmanager.h"
#include "viewerplugin.h"

#include <kpluginmetadata.h>
#include <KPluginLoader>
#include <QFileInfo>

using namespace MessageViewer;

class ViewerPluginManagerPrivateInstancePrivate
{
public:
    ViewerPluginManagerPrivateInstancePrivate()
        : viewerPluginManager(new ViewerPluginManager)
    {
    }

    ~ViewerPluginManagerPrivateInstancePrivate()
    {
        delete viewerPluginManager;
    }

    ViewerPluginManager *viewerPluginManager;
};

Q_GLOBAL_STATIC(ViewerPluginManagerPrivateInstancePrivate, sInstance)

class ViewerPluginInfo
{
public:
    ViewerPluginInfo()
        : plugin(Q_NULLPTR)
    {

    }
    QString saveName() const;

    KPluginMetaData metaData;
    MessageViewer::ViewerPlugin *plugin;
};

QString ViewerPluginInfo::saveName() const
{
    return QFileInfo(metaData.fileName()).baseName();
}


class MessageViewer::ViewerPluginManagerPrivate
{
public:
    ViewerPluginManagerPrivate()
    {

    }
    void initializePluginList();
};

void ViewerPluginManagerPrivate::initializePluginList()
{
    const QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(QStringLiteral("messageviewer"), [](const KPluginMetaData & md) {
        return md.serviceTypes().contains(QStringLiteral("MessageViewerHeaderStyle/ViewerPlugin"));
    });

}


ViewerPluginManager::ViewerPluginManager(QObject *parent)
    : QObject(parent),
      d(new MessageViewer::ViewerPluginManagerPrivate)
{
    d->initializePluginList();
}


MessageViewer::ViewerPluginManager::~ViewerPluginManager()
{
    delete d;
}

ViewerPluginManager *ViewerPluginManager::self()
{
    return sInstance->viewerPluginManager;
}
