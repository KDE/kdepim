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
#include "webdavsettingsdialog.h"
#include "webdavjob.h"

#include <KLocale>
#include <KConfig>
#include <KGlobal>
#include <KConfigGroup>

#include <QPointer>

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
    QPointer<WebDavSettingsDialog> dlg = new WebDavSettingsDialog;
    if (dlg->exec()) {
        WebDavJob *job = new WebDavJob(this);
        mServiceLocation = dlg->serviceLocation();
        mPublicLocation = dlg->publicLocation();
        job->requestTokenAccess();
    }
    delete dlg;
}

void WebDavStorageService::shareLink(const QString &root, const QString &path)
{
    if (mServiceLocation.isEmpty()) {
        authentification();
    } else {
        WebDavJob *job = new WebDavJob(this);
        connect(job, SIGNAL(shareLinkDone(QString)), this, SLOT(slotShareLinkDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->shareLink(root, path);
    }
}

void WebDavStorageService::listFolder()
{
    if (mServiceLocation.isEmpty()) {
        authentification();
    } else {
        WebDavJob *job = new WebDavJob(this);
        connect(job, SIGNAL(listFolderDone()), this, SLOT(slotListFolderDone()));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->listFolder();
    }
}

void WebDavStorageService::createFolder(const QString &folder)
{
    if (mServiceLocation.isEmpty()) {
        authentification();
    } else {
        WebDavJob *job = new WebDavJob(this);
        connect(job, SIGNAL(createFolderDone()), this, SLOT(slotCreateFolderDone()));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->createFolder(folder);
    }
}

void WebDavStorageService::accountInfo()
{
    if (mServiceLocation.isEmpty()) {
        authentification();
    } else {
        WebDavJob *job = new WebDavJob(this);
        connect(job,SIGNAL(accountInfoDone(PimCommon::AccountInfo)), this, SLOT(slotAccountInfoDone(PimCommon::AccountInfo)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->accountInfo();
    }
}

QString WebDavStorageService::name()
{
    return i18n("Webdav");
}

void WebDavStorageService::uploadFile(const QString &filename)
{
    if (mServiceLocation.isEmpty()) {
        authentification();
    } else {
        WebDavJob *job = new WebDavJob(this);
        connect(job, SIGNAL(uploadFileDone()), this, SLOT(slotUploadFileDone()));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        connect(job, SIGNAL(uploadFileProgress(qint64,qint64)), SLOT(slotUploadFileProgress(qint64,qint64)));
        job->uploadFile(filename);
    }
}

QString WebDavStorageService::description()
{
    //TODO
    return QString(); // i18n("");
}

QUrl WebDavStorageService::serviceUrl()
{
    return QUrl(QLatin1String(""));
}

QString WebDavStorageService::serviceName()
{
    return QLatin1String("webdav");
}

QString WebDavStorageService::storageServiceName() const
{
    return serviceName();
}

KIcon WebDavStorageService::icon() const
{
    return KIcon();
}
