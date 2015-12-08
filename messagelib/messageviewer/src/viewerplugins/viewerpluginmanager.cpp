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
#include <KPluginFactory>
#include <QFileInfo>
#include <QSet>

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
    ViewerPluginManagerPrivate(ViewerPluginManager *qq)
        : q(qq)
    {

    }
    void initializePluginList();
    void loadPlugin(ViewerPluginInfo *item);
    QVector<MessageViewer::ViewerPlugin *> pluginsList() const;
    QVector<ViewerPluginInfo> mPluginList;
    ViewerPluginManager *q;
};

namespace {
QString pluginVersion() {
    return QStringLiteral("1.0");
}
}
void ViewerPluginManagerPrivate::initializePluginList()
{
    const QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(QStringLiteral("messageviewer"), [](const KPluginMetaData & md) {
        return md.serviceTypes().contains(QStringLiteral("MessageViewer/ViewerPlugin"));
    });

    QVectorIterator<KPluginMetaData> i(plugins);
    i.toBack();
    QSet<QString> unique;
    while (i.hasPrevious()) {
        ViewerPluginInfo info;
        info.metaData = i.previous();

        const QString version = info.metaData.version();
        if (pluginVersion() == version) {
            // only load plugins once, even if found multiple times!
            if (unique.contains(info.saveName())) {
                continue;
            }
            info.plugin = Q_NULLPTR;
            mPluginList.push_back(info);
            unique.insert(info.saveName());
        }
    }
    QVector<ViewerPluginInfo>::iterator end(mPluginList.end());
    for (QVector<ViewerPluginInfo>::iterator it = mPluginList.begin(); it != end; ++it) {
        loadPlugin(&(*it));
    }

}

void ViewerPluginManagerPrivate::loadPlugin(ViewerPluginInfo *item)
{
    item->plugin = KPluginLoader(item->metaData.fileName()).factory()->create<MessageViewer::ViewerPlugin>(q, QVariantList() << item->saveName());
}

QVector<ViewerPlugin *> ViewerPluginManagerPrivate::pluginsList() const
{
    QVector<MessageViewer::ViewerPlugin *> lst;
    QVector<ViewerPluginInfo>::ConstIterator end(mPluginList.constEnd());
    for (QVector<ViewerPluginInfo>::ConstIterator it = mPluginList.constBegin(); it != end; ++it) {
        if ((*it).plugin) {
            lst << (*it).plugin;
        }
    }
    return lst;
}

ViewerPluginManager::ViewerPluginManager(QObject *parent)
    : QObject(parent),
      d(new MessageViewer::ViewerPluginManagerPrivate(this))
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

QVector<MessageViewer::ViewerPlugin *> ViewerPluginManager::pluginsList() const
{
    return d->pluginsList();
}
