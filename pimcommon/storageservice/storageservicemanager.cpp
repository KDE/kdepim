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

#include "settings/pimcommonsettings.h"

#include "dropbox/dropboxstorageservice.h"
#include "hubic/hubicstorageservice.h"

#include <KLocale>

using namespace PimCommon;

StorageServiceManager::StorageServiceManager(QObject *parent)
    : QObject(parent)
{
    readConfig();
}

StorageServiceManager::~StorageServiceManager()
{
    qDeleteAll(mListService);
}

void StorageServiceManager::readConfig()
{
    const QStringList services = PimCommon::PimCommonSettings::self()->services();
    Q_FOREACH(const QString &service, services) {
        if (service == serviceName(DropBox)) {
            if (!mListService.contains(serviceName(DropBox))) {
                mListService.insert(service, new DropBoxStorageService());
            }
        } else if (service == serviceName(Hubic)) {
            if (!mListService.contains(serviceName(Hubic))) {
                mListService.insert(service, new HubicStorageService());
            }
        }
    }
}

void StorageServiceManager::writeConfig()
{
    PimCommon::PimCommonSettings::self()->setServices(mListService.keys());
    PimCommon::PimCommonSettings::self()->writeConfig();
}


QString StorageServiceManager::description(ServiceType type)
{
    switch(type) {
    case DropBox:
        return PimCommon::DropBoxStorageService::description();
    case Hubic:
        return PimCommon::HubicStorageService::description();
    default:
        return QString();
    }
    return QString();
}

QUrl StorageServiceManager::serviceUrl(ServiceType type)
{
    switch(type) {
    case DropBox:
        return PimCommon::DropBoxStorageService::serviceUrl();
    case Hubic:
        return PimCommon::HubicStorageService::serviceUrl();
    default:
        return QString();
    }
    return QString();
}


QString StorageServiceManager::serviceName(ServiceType type)
{
    switch(type) {
    case DropBox:
        return PimCommon::DropBoxStorageService::serviceName();
    case Hubic:
        return PimCommon::HubicStorageService::serviceName();
    default:
        return QString();
    }
}

QString StorageServiceManager::serviceToI18n(ServiceType type)
{
    switch(type) {
    case DropBox:
        return PimCommon::DropBoxStorageService::name();
    case Hubic:
        return PimCommon::HubicStorageService::name();
    default:
        return QString();
    }
}
