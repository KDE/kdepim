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

#include <KLocalizedString>
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
    mRefreshToken = grp.readEntry("Refresh Token");
    mToken = grp.readEntry("Token");
    if (grp.hasKey("Expire Time"))
        mExpireDateTime = grp.readEntry("Expire Time", QDateTime::currentDateTime());
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
    connect(job, SIGNAL(authorizationDone(QString,QString,qint64)), this, SLOT(slotAuthorizationDone(QString,QString,qint64)));
    connect(job, SIGNAL(authorizationFailed(QString)), this, SLOT(slotAuthorizationFailed(QString)));
    job->requestTokenAccess();
}

void BoxStorageService::slotAuthorizationFailed(const QString &errorMessage)
{
    mRefreshToken.clear();
    mToken.clear();
    Q_EMIT authentificationFailed(serviceName(), errorMessage);
}

void BoxStorageService::slotAuthorizationDone(const QString &refreshToken, const QString &token, qint64 expireTime)
{
    mRefreshToken = refreshToken;
    mToken = token;
    KConfigGroup grp(KGlobal::config(), "Box Settings");
    grp.writeEntry("Refresh Token", mRefreshToken);
    grp.writeEntry("Token", mToken);
    grp.writeEntry("Expire Time", QDateTime::currentDateTime().addSecs(expireTime));
    grp.sync();
    Q_EMIT authentificationDone(serviceName());
}


void BoxStorageService::shareLink(const QString &root, const QString &path)
{
    if (mToken.isEmpty()) {
        authentification();
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
        connect(job, SIGNAL(shareLinkDone(QString)), this, SLOT(slotShareLinkDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->shareLink(root, path);
    }
}

void BoxStorageService::downloadFile()
{

}

void BoxStorageService::listFolder()
{
    if (mToken.isEmpty()) {
        authentification();
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
        connect(job, SIGNAL(listFolderDone(QStringList)), this, SLOT(slotListFolderDone(QStringList)));
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
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
        connect(job, SIGNAL(createFolderDone(QString)), this, SLOT(slotCreateFolderDone(QString)));
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
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
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
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
        connect(job, SIGNAL(uploadFileDone(QString)), this, SLOT(slotUploadFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        connect(job, SIGNAL(uploadFileProgress(qint64,qint64)), SLOT(slotUploadFileProgress(qint64,qint64)));
        job->uploadFile(filename);
    }
}

QString BoxStorageService::description()
{
    return i18n("Box.com is a file hosting that offers cloud storage, file synchronization, and client software.");
}

QUrl BoxStorageService::serviceUrl()
{
    return QUrl(QLatin1String("https://app.box.com/"));
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
