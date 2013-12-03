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

#include "webdavstorageservice.h"
#include "webdavjob.h"

#include <KLocale>
#include <KConfig>
#include <KGlobal>
#include <KConfigGroup>


using namespace PimCommon;

WebDavStorageService::WebDavStorageService(QObject *parent)
    : PimCommon::StorageServiceAbstract(parent)
{
    readConfig();
}

WebDavStorageService::~WebDavStorageService()
{
}

void WebDavStorageService::readConfig()
{
    KConfigGroup grp(KGlobal::config(), "Webdav Settings");

}

void WebDavStorageService::removeConfig()
{
    KConfigGroup grp(KGlobal::config(), "Webdav Settings");
    grp.deleteGroup();
    KGlobal::config()->sync();
}

void WebDavStorageService::authentification()
{
    WebDavJob *job = new WebDavJob(this);
    job->requestTokenAccess();
    //TODO connect
}

void WebDavStorageService::listFolder()
{

}

void WebDavStorageService::createFolder(const QString &folder)
{

}

void WebDavStorageService::accountInfo()
{

}

QString WebDavStorageService::name()
{
    return i18n("Webdav");
}

void WebDavStorageService::uploadFile(const QString &filename)
{
    //TODO
}

QString WebDavStorageService::description()
{
    //TODO
    return i18n("");
}

QUrl WebDavStorageService::serviceUrl()
{
    return QUrl(QLatin1String(""));
}

QString WebDavStorageService::serviceName()
{
    return QLatin1String("webdav");
}


