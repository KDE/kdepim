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

#include "hubicstorageservice.h"

#include <KLocale>
#include <KConfig>
#include <KGlobal>
#include <KConfigGroup>


using namespace PimCommon;

HubicStorageService::HubicStorageService(QObject *parent)
    : PimCommon::StorageServiceAbstract(parent)
{
    readConfig();
}

HubicStorageService::~HubicStorageService()
{
}

void HubicStorageService::readConfig()
{
    KConfigGroup grp(KGlobal::config(), "Hubic Settings");

}

void HubicStorageService::removeConfig()
{
    KConfigGroup grp(KGlobal::config(), "Hubic Settings");
    grp.deleteGroup();
    KGlobal::config()->sync();
}

void HubicStorageService::authentification()
{

}

void HubicStorageService::listFolder()
{

}

void HubicStorageService::createFolder(const QString &folder)
{

}

void HubicStorageService::accountInfo()
{

}

QString HubicStorageService::name()
{
    return i18n("Hubic");
}

QUrl HubicStorageService::sharedUrl() const
{
    return QUrl();
}

void HubicStorageService::uploadFile(const QString &filename)
{
    //TODO
}

QString HubicStorageService::description()
{
    return QString();
}

QUrl HubicStorageService::serviceUrl()
{
    return QUrl(QLatin1String("https://hubic.com"));
}

QString HubicStorageService::serviceName()
{
    return QLatin1String("hubic");
}


