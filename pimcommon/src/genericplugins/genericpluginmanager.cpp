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

#include "genericpluginmanager.h"

using namespace PimCommon;

class PimCommon::GenericPluginManagerPrivate
{
public:
    GenericPluginManagerPrivate()
    {

    }
    QString serviceTypeName;
    QString pluginName;
};

GenericPluginManager::GenericPluginManager(QObject *parent)
    : QObject(parent),
      d(new GenericPluginManagerPrivate)
{

}

GenericPluginManager::~GenericPluginManager()
{
    delete d;
}

void GenericPluginManager::initializePlugins()
{
    //TODO
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
