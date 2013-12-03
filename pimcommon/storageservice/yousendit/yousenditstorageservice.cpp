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

#include "yousenditstorageservice.h"
#include "yousenditjob.h"

#include <KLocale>
#include <KConfig>
#include <KGlobal>
#include <KConfigGroup>


using namespace PimCommon;

YousendItStorageService::YousendItStorageService(QObject *parent)
    : PimCommon::StorageServiceAbstract(parent)
{
    readConfig();
}

YousendItStorageService::~YousendItStorageService()
{
}

void YousendItStorageService::readConfig()
{
    KConfigGroup grp(KGlobal::config(), "YouSendIt Settings");

}

void YousendItStorageService::removeConfig()
{
    KConfigGroup grp(KGlobal::config(), "YouSendIt Settings");
    grp.deleteGroup();
    KGlobal::config()->sync();
}

void YousendItStorageService::authentification()
{
     *job = new HubicJob(this);
    job->requestTokenAccess();
    //TODO connect
}

void YousendItStorageService::listFolder()
{

}

void YousendItStorageService::createFolder(const QString &folder)
{

}

void YousendItStorageService::accountInfo()
{

}

QString YousendItStorageService::name()
{
    return i18n("Hubic");
}

QUrl YousendItStorageService::sharedUrl() const
{
    return QUrl();
}

void YousendItStorageService::uploadFile(const QString &filename)
{
    //TODO
}

QString YousendItStorageService::description()
{
    return i18n("Hubic is a file hosting service operated by Ovh, Inc. that offers cloud storage, file synchronization, and client software.");
}

QUrl YousendItStorageService::serviceUrl()
{
    return QUrl(QLatin1String("https://hubic.com"));
}

QString YousendItStorageService::serviceName()
{
    return QLatin1String("hubic");
}


