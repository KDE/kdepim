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

#include "boxstorageservice.h"
#include "boxjob.h"

#include <KLocale>
#include <KConfig>
#include <KGlobal>
#include <KConfigGroup>

#include <QPointer>

using namespace PimCommon;

BoxStorageService::BoxStorageService(QObject *parent)
    : PimCommon::StorageServiceAbstract(parent)
{
    readConfig();
}

BoxStorageService::~BoxStorageService()
{
}

void BoxStorageService::readConfig()
{
    KConfigGroup grp(KGlobal::config(), "Box Settings");

}

void BoxStorageService::removeConfig()
{
    KConfigGroup grp(KGlobal::config(), "Box Settings");
    grp.deleteGroup();
    KGlobal::config()->sync();
}

void BoxStorageService::authentification()
{
    BoxJob *job = new BoxJob(this);
    connect(job, SIGNAL(authorizationDone(QString)), this, SLOT(slotAuthorizationDone(QString)));
    connect(job, SIGNAL(authorizationFailed(QString)), this, SLOT(slotAuthorizationFailed(QString)));
    job->requestTokenAccess();
}

void BoxStorageService::slotAuthorizationFailed(const QString &errorMessage)
{
    mRefreshToken.clear();
    Q_EMIT authentificationFailed(serviceName(), errorMessage);
}

void BoxStorageService::slotAuthorizationDone(const QString &refreshToken)
{
    mRefreshToken = refreshToken;
    KConfigGroup grp(KGlobal::config(), "Box Settings");
    grp.writeEntry("Refresh Token", mRefreshToken);
    grp.sync();
    Q_EMIT authentificationDone(serviceName());
}


void BoxStorageService::shareLink(const QString &root, const QString &path)
{
    if (mToken.isEmpty()) {
        authentification();
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken);
        connect(job, SIGNAL(shareLinkDone(QString)), this, SLOT(slotShareLinkDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->shareLink(root, path);
    }
}

void BoxStorageService::listFolder()
{
    if (mToken.isEmpty()) {
        authentification();
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken);
        connect(job, SIGNAL(listFolderDone()), this, SLOT(slotListFolderDone()));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->listFolder();
    }
}

void BoxStorageService::createFolder(const QString &folder)
{
    if (mToken.isEmpty()) {
        authentification();
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken);
        connect(job, SIGNAL(createFolderDone()), this, SLOT(slotCreateFolderDone()));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->createFolder(folder);
    }
}

void BoxStorageService::accountInfo()
{
    if (mToken.isEmpty()) {
        authentification();
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken);
        connect(job,SIGNAL(accountInfoDone(PimCommon::AccountInfo)), this, SLOT(slotAccountInfoDone(PimCommon::AccountInfo)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->accountInfo();
    }
}

QString BoxStorageService::name()
{
    return i18n("Box");
}

void BoxStorageService::uploadFile(const QString &filename)
{
    if (mToken.isEmpty()) {
        authentification();
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken);
        connect(job, SIGNAL(uploadFileDone()), this, SLOT(slotUploadFileDone()));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        connect(job, SIGNAL(uploadFileProgress(qint64,qint64)), SLOT(slotUploadFileProgress(qint64,qint64)));
        job->uploadFile(filename);
    }
}

QString BoxStorageService::description()
{
    //TODO
    return QString(); // i18n("");
}

QUrl BoxStorageService::serviceUrl()
{
    return QUrl(QLatin1String(""));
}

QString BoxStorageService::serviceName()
{
    return QLatin1String("box");
}

QString BoxStorageService::iconName()
{
    return QString();
}

QString BoxStorageService::storageServiceName() const
{
    return serviceName();
}

KIcon BoxStorageService::icon() const
{
    return KIcon();
}
