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

#include "headerstylepluginmanager.h"
#include "headerstyleplugin.h"
#include <KPluginFactory>
#include <KPluginLoader>
#include <kpluginmetadata.h>
#include <QFileInfo>
#include <QSet>
#include <QDebug>

using namespace MessageViewer;

class HeaderStylePluginManagerInstancePrivate
{
public:
    HeaderStylePluginManagerInstancePrivate()
        : headerStylePluginManager(new HeaderStylePluginManager)
    {
    }

    ~HeaderStylePluginManagerInstancePrivate()
    {
        delete headerStylePluginManager;
    }

    HeaderStylePluginManager *headerStylePluginManager;
};

Q_GLOBAL_STATIC(HeaderStylePluginManagerInstancePrivate, sInstance)

class HeaderStylePluginInfo
{
public:
    HeaderStylePluginInfo()
        : plugin(Q_NULLPTR)
    {

    }
    QString saveName() const;

    KPluginMetaData metaData;
    MessageViewer::HeaderStylePlugin *plugin;
};

QString HeaderStylePluginInfo::saveName() const
{
    return QFileInfo(metaData.fileName()).baseName();
}

class MessageViewer::HeaderStylePluginManagerPrivate
{
public:
    HeaderStylePluginManagerPrivate(HeaderStylePluginManager *qq)
        : q(qq)
    {

    }
    QVector<MessageViewer::HeaderStylePlugin *> pluginsList() const;
    void initializePluginList();
    void loadPlugin(HeaderStylePluginInfo *item);
    QVector<HeaderStylePluginInfo> mPluginList;
    HeaderStylePluginManager *q;
};

namespace {
QString pluginVersion() {
    return QStringLiteral("1.0");
}
}

void HeaderStylePluginManagerPrivate::initializePluginList()
{
    const QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(QStringLiteral("messageviewer"), [](const KPluginMetaData & md) {
        return md.serviceTypes().contains(QStringLiteral("MessageViewerHeaderStyle/Plugin"));
    });

    QVectorIterator<KPluginMetaData> i(plugins);
    i.toBack();
    QSet<QString> unique;
    QList<int> listOrder;
    while (i.hasPrevious()) {
        HeaderStylePluginInfo info;
        info.metaData = i.previous();

        // only load plugins once, even if found multiple times!
        if (unique.contains(info.saveName())) {
            continue;
        }
        const QVariant p = info.metaData.rawData().value(QStringLiteral("X-KDE-MessageViewer-Header-Order")).toVariant();
        int order = -1;
        if (p.isValid()) {
            order = p.toInt();
        }
        int pos = 0;
        for (; pos < listOrder.count(); ++pos) {
            if (listOrder.at(pos) > order) {
                pos--;
                break;
            }
        }
        pos = qMax(0, pos);
        listOrder.insert(pos, order);
        info.plugin = Q_NULLPTR;
        mPluginList.insert(pos, info);
        unique.insert(info.saveName());
    }
    QVector<HeaderStylePluginInfo>::iterator end(mPluginList.end());
    for (QVector<HeaderStylePluginInfo>::iterator it = mPluginList.begin(); it != end; ++it) {
        loadPlugin(&(*it));
    }
}

QVector<MessageViewer::HeaderStylePlugin *> HeaderStylePluginManagerPrivate::pluginsList() const
{
    QVector<MessageViewer::HeaderStylePlugin *> lst;
    QVector<HeaderStylePluginInfo>::ConstIterator end(mPluginList.constEnd());
    for (QVector<HeaderStylePluginInfo>::ConstIterator it = mPluginList.constBegin(); it != end; ++it) {
        if ((*it).plugin) {
            lst << (*it).plugin;
        }
    }
    return lst;
}

void HeaderStylePluginManagerPrivate::loadPlugin(HeaderStylePluginInfo *item)
{
    item->plugin = KPluginLoader(item->metaData.fileName()).factory()->create<MessageViewer::HeaderStylePlugin>(q, QVariantList() << item->saveName());
}

HeaderStylePluginManager *HeaderStylePluginManager::self()
{
    return sInstance->headerStylePluginManager;
}

HeaderStylePluginManager::HeaderStylePluginManager(QObject *parent)
    : QObject(parent),
      d(new MessageViewer::HeaderStylePluginManagerPrivate(this))
{
    d->initializePluginList();
}

HeaderStylePluginManager::~HeaderStylePluginManager()
{
    delete d;
}

QVector<MessageViewer::HeaderStylePlugin *> HeaderStylePluginManager::pluginsList() const
{
    return d->pluginsList();
}

QStringList HeaderStylePluginManager::pluginListName() const
{
    QStringList lst;
    Q_FOREACH (MessageViewer::HeaderStylePlugin *plugin, d->pluginsList()) {
        lst << plugin->name();
    }
    return lst;
}
