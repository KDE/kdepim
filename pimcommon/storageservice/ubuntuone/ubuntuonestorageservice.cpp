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

#include "ubuntuonestorageservice.h"
#include "ubuntuonejob.h"

#include <KLocale>
#include <KConfig>
#include <KGlobal>
#include <KConfigGroup>


using namespace PimCommon;

UbuntuoneStorageService::UbuntuoneStorageService(QObject *parent)
    : PimCommon::StorageServiceAbstract(parent)
{
    readConfig();
}

UbuntuoneStorageService::~UbuntuoneStorageService()
{
}

void UbuntuoneStorageService::readConfig()
{
    KConfigGroup grp(KGlobal::config(), "Ubuntu One Settings");

}

void UbuntuoneStorageService::removeConfig()
{
    KConfigGroup grp(KGlobal::config(), "Ubuntu One Settings");
    grp.deleteGroup();
    KGlobal::config()->sync();
}

void UbuntuoneStorageService::authentification()
{
    UbuntuOneJob *job = new UbuntuOneJob(this);
    job->requestTokenAccess();
    //TODO connect
}

void UbuntuoneStorageService::listFolder()
{

}

void UbuntuoneStorageService::createFolder(const QString &folder)
{

}

void UbuntuoneStorageService::accountInfo()
{

}

QString UbuntuoneStorageService::name()
{
    return i18n("Ubuntu One");
}

void UbuntuoneStorageService::uploadFile(const QString &filename)
{
    //TODO
}

QString UbuntuoneStorageService::description()
{
    return QString();
}

QUrl UbuntuoneStorageService::serviceUrl()
{
    return QUrl(QLatin1String("https://one.ubuntu.com/"));
}

QString UbuntuoneStorageService::serviceName()
{
    return QLatin1String("ubuntuone");
}


