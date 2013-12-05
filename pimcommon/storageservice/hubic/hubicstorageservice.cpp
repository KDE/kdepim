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
#include "hubicjob.h"

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
    mRefreshToken = grp.readEntry("Refresh Token");
}

void HubicStorageService::removeConfig()
{
    KConfigGroup grp(KGlobal::config(), "Hubic Settings");
    grp.deleteGroup();
    KGlobal::config()->sync();
}

void HubicStorageService::authentification()
{
    HubicJob *job = new HubicJob(this);
    connect(job, SIGNAL(authorizationDone(QString)), this, SLOT(slotAuthorizationDone(QString)));
    connect(job, SIGNAL(authorizationFailed()), this, SLOT(slotAuthorizationFailed()));
    job->requestTokenAccess();
}

void HubicStorageService::slotAuthorizationFailed()
{
    mRefreshToken.clear();
}

void HubicStorageService::slotAuthorizationDone(const QString &refreshToken)
{
    mRefreshToken = refreshToken;
    KConfigGroup grp(KGlobal::config(), "Hubic Settings");
    grp.writeEntry("Refresh Token", mRefreshToken);
    grp.sync();
}

void HubicStorageService::listFolder()
{
    if (mRefreshToken.isEmpty()) {
        authentification();
    } else {
        HubicJob *job = new HubicJob(this);
        //TODO
        job->listFolder();
    }
}

void HubicStorageService::createFolder(const QString &folder)
{
    if (mRefreshToken.isEmpty()) {
        authentification();
    } else {
        HubicJob *job = new HubicJob(this);
        //TODO
        job->createFolder(folder);
    }

}

void HubicStorageService::accountInfo()
{
    if (mRefreshToken.isEmpty()) {
        authentification();
    } else {
        HubicJob *job = new HubicJob(this);
        //TODO
        job->accountInfo();
    }
}

QString HubicStorageService::name()
{
    return i18n("Hubic");
}

void HubicStorageService::uploadFile(const QString &filename)
{
    if (mRefreshToken.isEmpty()) {
        authentification();
    } else {
        HubicJob *job = new HubicJob(this);
        //TODO
        job->uploadFile(filename);
    }
}

QString HubicStorageService::description()
{
    return i18n("Hubic is a file hosting service operated by Ovh, Inc. that offers cloud storage, file synchronization, and client software.");
}

QUrl HubicStorageService::serviceUrl()
{
    return QUrl(QLatin1String("https://hubic.com"));
}

QString HubicStorageService::serviceName()
{
    return QLatin1String("hubic");
}

void PimCommon::HubicStorageService::shareLink(const QString &root, const QString &path)
{
    if (mRefreshToken.isEmpty()) {
        authentification();
    } else {
        HubicJob *job = new HubicJob(this);
        //TODO
        job->shareLink(root, path);
    }
}
