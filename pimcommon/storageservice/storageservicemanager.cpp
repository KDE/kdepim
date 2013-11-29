/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "storageservicemanager.h"

#include "dropbox/dropboxstorageservice.h"
#include "hubic/hubicstorageservice.h"

#include <KLocale>

using namespace PimCommon;

StorageServiceManager::StorageServiceManager(QObject *parent)
    : QObject(parent)
{
}

StorageServiceManager::~StorageServiceManager()
{

}

void StorageServiceManager::readConfig()
{

}

void StorageServiceManager::writeConfig()
{

}


QString StorageServiceManager::serviceName(ServiceType type)
{
    switch(type) {
    case DropBox:
        return QLatin1String("dropbox");
    case Hubic:
        return QLatin1String("hubic");
    default:
        return QString();
    }
}

QString StorageServiceManager::serviceToI18n(ServiceType type)
{
    switch(type) {
    case DropBox:
        return i18n("Dropbox");
    case Hubic:
        return i18n("Hubic");
    default:
        return QString();
    }
}
