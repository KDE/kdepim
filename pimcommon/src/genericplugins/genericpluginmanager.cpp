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

class PimCommon::GenericPluginManagerPrivate
{
public:
    GenericPluginManagerPrivate()
    {

    }
    QVector<GenericPlugin *> pluginsList() const;
    bool initializePlugins();
    QString serviceTypeName;
    QString pluginName;
};

bool GenericPluginManagerPrivate::initializePlugins()
{
    if (serviceTypeName.isEmpty() || pluginName.isEmpty()) {
        return false;
    }
    //TODO
    return true;
}

QVector<GenericPlugin *> GenericPluginManagerPrivate::pluginsList() const
{
    //TODO
    return QVector<GenericPlugin *>();
}

GenericPluginManager::GenericPluginManager(QObject *parent)
    : QObject(parent),
      d(new GenericPluginManagerPrivate)
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

