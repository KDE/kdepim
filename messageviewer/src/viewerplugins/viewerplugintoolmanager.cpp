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

#include "viewerplugintoolmanager.h"
#include "viewerpluginmanager.h"
#include "viewerplugin.h"

#include <QVector>


using namespace MessageViewer;

class MessageViewer::ViewerPluginToolManagerPrivate
{
public:
    ViewerPluginToolManagerPrivate()
        : mActionCollection(Q_NULLPTR)
    {

    }
    void createView();
    void closeAllTools();
    void setActionCollection(KActionCollection *ac);
    KActionCollection *mActionCollection;
};

void ViewerPluginToolManagerPrivate::createView()
{
    QVector<MessageViewer::ViewerPlugin *> listPlugin = MessageViewer::ViewerPluginManager::self()->pluginsList();
    Q_FOREACH(MessageViewer::ViewerPlugin *plugin, listPlugin) {
        //plugin->createView()
    }
}

void ViewerPluginToolManagerPrivate::closeAllTools()
{
    //TODO
}

void ViewerPluginToolManagerPrivate::setActionCollection(KActionCollection *ac)
{
    mActionCollection = ac;
}


ViewerPluginToolManager::ViewerPluginToolManager(QObject *parent)
    : QObject(parent),
      d(new MessageViewer::ViewerPluginToolManagerPrivate)
{
}

ViewerPluginToolManager::~ViewerPluginToolManager()
{
    delete d;
}

void ViewerPluginToolManager::closeAllTools()
{
    d->closeAllTools();
}

void ViewerPluginToolManager::createView()
{
    d->createView();
}

void ViewerPluginToolManager::setActionCollection(KActionCollection *ac)
{
    d->setActionCollection(ac);
}
