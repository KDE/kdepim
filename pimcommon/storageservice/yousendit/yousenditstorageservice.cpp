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

YouSendItStorageService::YouSendItStorageService(QObject *parent)
    : PimCommon::StorageServiceAbstract(parent)
{
    readConfig();
}

YouSendItStorageService::~YouSendItStorageService()
{
}

void YouSendItStorageService::readConfig()
{
    KConfigGroup grp(KGlobal::config(), "YouSendIt Settings");
    mUsername = grp.readEntry("Username");
    mPassword = grp.readEntry("Password");
    mToken = grp.readEntry("Token");
}

void YouSendItStorageService::removeConfig()
{
    KConfigGroup grp(KGlobal::config(), "YouSendIt Settings");
    grp.deleteGroup();
    KGlobal::config()->sync();
}

void YouSendItStorageService::authentification()
{
    YouSendItJob *job = new YouSendItJob(this);
    connect(job, SIGNAL(authorizationDone(QString,QString,QString)), this, SLOT(slotAuthorizationDone(QString,QString,QString)));
    connect(job, SIGNAL(authorizationFailed()), this, SLOT(slotAuthorizationFailed()));
    job->requestTokenAccess();
}

void YouSendItStorageService::slotAuthorizationFailed()
{
    mUsername.clear();
    mPassword.clear();
    mToken.clear();
    Q_EMIT authentificationFailed(serviceName());
}


void YouSendItStorageService::slotAuthorizationDone(const QString &password, const QString &username, const QString &token)
{
    mUsername = username;
    mPassword = password;
    mToken = token;

    KConfigGroup grp(KGlobal::config(), "YouSendIt Settings");
    grp.readEntry("Username", mUsername);
    //TODO store in kwallet ?
    grp.readEntry("Password", mPassword);
    grp.readEntry("Token", mToken);
    grp.sync();
    KGlobal::config()->sync();
    Q_EMIT authentificationDone(serviceName());
}

void YouSendItStorageService::listFolder()
{
    if (mToken.isEmpty()) {
        authentification();
    } else {
        YouSendItJob *job = new YouSendItJob(this);
        job->initializeToken(mPassword, mUsername, mToken);
        connect(job, SIGNAL(listFolderDone()), this, SLOT(slotListFolderDone()));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->listFolder();
    }
}

void YouSendItStorageService::createFolder(const QString &folder)
{
    if (mToken.isEmpty()) {
        authentification();
    } else {
        YouSendItJob *job = new YouSendItJob(this);
        job->initializeToken(mPassword, mUsername, mToken);
        connect(job, SIGNAL(createFolderDone()), this, SLOT(slotCreateFolderDone()));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->createFolder(folder);
    }
}

void YouSendItStorageService::accountInfo()
{
    if (mToken.isEmpty()) {
        authentification();
    } else {
        YouSendItJob *job = new YouSendItJob(this);
        job->initializeToken(mPassword, mUsername, mToken);
        connect(job,SIGNAL(accountInfoDone(PimCommon::AccountInfo)), this, SLOT(slotAccountInfoDone(PimCommon::AccountInfo)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->accountInfo();
    }
}

QString YouSendItStorageService::name()
{
    return i18n("YouSendIt");
}

void YouSendItStorageService::uploadFile(const QString &filename)
{
    if (mToken.isEmpty()) {
        authentification();
    } else {
        YouSendItJob *job = new YouSendItJob(this);
        job->initializeToken(mPassword, mUsername, mToken);
        connect(job, SIGNAL(uploadFileDone()), this, SLOT(slotUploadFileDone()));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        connect(job, SIGNAL(uploadFileProgress(qint64,qint64)), SLOT(slotUploadFileProgress(qint64,qint64)));
        job->uploadFile(filename);
    }
}

QString YouSendItStorageService::description()
{
    return i18n("YouSendIt is a file hosting that offers cloud storage, file synchronization, and client software.");
}

QUrl YouSendItStorageService::serviceUrl()
{
    return QUrl(QLatin1String("https://www.yousendit.com/"));
}

QString YouSendItStorageService::serviceName()
{
    return QLatin1String("yousendit");
}

void YouSendItStorageService::shareLink(const QString &root, const QString &path)
{
    if (mToken.isEmpty()) {
        authentification();
    } else {
        YouSendItJob *job = new YouSendItJob(this);
        job->initializeToken(mPassword, mUsername, mToken);
        connect(job, SIGNAL(shareLinkDone(QString)), this, SLOT(slotShareLinkDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->shareLink(root, path);
    }
}

QString YouSendItStorageService::storageServiceName() const
{
    return serviceName();
}
