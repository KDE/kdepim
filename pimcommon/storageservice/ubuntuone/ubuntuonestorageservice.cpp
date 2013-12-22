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

#include <KLocalizedString>
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

    mCustomerSecret = grp.readEntry("Customer Secret");
    mToken = grp.readEntry("Token");
    mCustomerKey = grp.readEntry("Customer Key");
    mTokenSecret = grp.readEntry("Token Secret");
}

void UbuntuoneStorageService::slotAuthorizationDone(const QString &customerSecret, const QString &token, const QString &customerKey, const QString &tokenSecret)
{
    mCustomerSecret = customerSecret;
    mToken = token;
    mCustomerKey = customerKey;
    mTokenSecret = tokenSecret;

    KConfigGroup grp(KGlobal::config(), "Ubuntu One Settings");
    grp.writeEntry("Customer Secret", mCustomerSecret);
    grp.writeEntry("Token", mToken);
    grp.writeEntry("Customer Key", mCustomerKey);
    grp.writeEntry("Token Secret", mTokenSecret);

    grp.sync();
    KGlobal::config()->sync();
    emitAuthentificationDone();
}

void UbuntuoneStorageService::removeConfig()
{
    KConfigGroup grp(KGlobal::config(), "Ubuntu One Settings");
    grp.deleteGroup();
    KGlobal::config()->sync();
}

void UbuntuoneStorageService::authentication()
{
    UbuntuOneJob *job = new UbuntuOneJob(this);
    connect(job, SIGNAL(authorizationDone(QString,QString,QString,QString)), this, SLOT(slotAuthorizationDone(QString,QString,QString,QString)));
    connect(job, SIGNAL(authorizationFailed(QString)), this, SLOT(slotAuthorizationFailed(QString)));
    job->requestTokenAccess();
}

void UbuntuoneStorageService::listFolder()
{
    if (mTokenSecret.isEmpty()) {
        mNextAction = ListFolder;
        authentication();
    } else {
        UbuntuOneJob *job = new UbuntuOneJob(this);
        connect(job, SIGNAL(listFolderDone(QStringList)), this, SLOT(slotListFolderDone(QStringList)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->initializeToken(mCustomerSecret, mToken, mCustomerKey, mTokenSecret);
        job->listFolder();
    }
}

void UbuntuoneStorageService::createFolder(const QString &folder)
{
    if (mTokenSecret.isEmpty()) {
        mNextAction = CreateFolder;
        authentication();
    } else {
        UbuntuOneJob *job = new UbuntuOneJob(this);
        job->initializeToken(mCustomerSecret, mToken, mCustomerKey, mTokenSecret);
        connect(job, SIGNAL(createFolderDone(QString)), this, SLOT(slotCreateFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->createFolder(folder);
    }
}

void UbuntuoneStorageService::slotAuthorizationFailed(const QString &errorMessage)
{
    mCustomerSecret.clear();
    mToken.clear();
    mCustomerKey.clear();
    mTokenSecret.clear();
    Q_EMIT authenticationFailed(serviceName(), errorMessage);
}

void UbuntuoneStorageService::accountInfo()
{
    if (mTokenSecret.isEmpty()) {
        mNextAction = AccountInfo;
        authentication();
    } else {
        UbuntuOneJob *job = new UbuntuOneJob(this);
        job->initializeToken(mCustomerSecret, mToken, mCustomerKey, mTokenSecret);
        connect(job,SIGNAL(accountInfoDone(PimCommon::AccountInfo)), this, SLOT(slotAccountInfoDone(PimCommon::AccountInfo)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->accountInfo();
    }
}

QString UbuntuoneStorageService::name()
{
    return i18n("Ubuntu One");
}

void UbuntuoneStorageService::uploadFile(const QString &filename)
{
    if (mTokenSecret.isEmpty()) {
        mNextAction = UploadFile;
        authentication();
    } else {
        UbuntuOneJob *job = new UbuntuOneJob(this);
        job->initializeToken(mCustomerSecret, mToken, mCustomerKey, mTokenSecret);
        connect(job, SIGNAL(uploadFileDone(QString)), this, SLOT(slotUploadFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        connect(job, SIGNAL(uploadFileProgress(qint64,qint64)), SLOT(slotUploadFileProgress(qint64,qint64)));
        job->uploadFile(filename);
    }
}

QString UbuntuoneStorageService::description()
{
    return i18n("UbuntuOne is a file hosting service operated by Canonical. that offers cloud storage, file synchronization, and client software.");
}

QUrl UbuntuoneStorageService::serviceUrl()
{
    return QUrl(QLatin1String("https://one.ubuntu.com/"));
}

QString UbuntuoneStorageService::serviceName()
{
    return QLatin1String("ubuntuone");
}

QString UbuntuoneStorageService::iconName()
{
    return QString();
}

void UbuntuoneStorageService::shareLink(const QString &root, const QString &path)
{    
    if (mTokenSecret.isEmpty()) {
        mNextAction = ShareLink;
        authentication();
    } else {
        UbuntuOneJob *job = new UbuntuOneJob(this);
        job->initializeToken(mCustomerSecret, mToken, mCustomerKey, mTokenSecret);
        connect(job, SIGNAL(shareLinkDone(QString)), this, SLOT(slotShareLinkDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->shareLink(root, path);
    }
}

void UbuntuoneStorageService::downloadFile(const QString &filename)
{
    if (mTokenSecret.isEmpty()) {
        mNextAction = DownLoadFile;
        authentication();
    } else {
        UbuntuOneJob *job = new UbuntuOneJob(this);
        job->initializeToken(mCustomerSecret, mToken, mCustomerKey, mTokenSecret);
        connect(job, SIGNAL(downLoadFileDone(QString)), this, SLOT(slotDownLoadFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->downloadFile(filename);
    }
}

void UbuntuoneStorageService::createServiceFolder()
{
    if (mTokenSecret.isEmpty()) {
        mNextAction = CreateServiceFolder;
        authentication();
    } else {
        UbuntuOneJob *job = new UbuntuOneJob(this);
        job->initializeToken(mCustomerSecret, mToken, mCustomerKey, mTokenSecret);
        connect(job, SIGNAL(createFolderDone(QString)), this, SLOT(slotCreateFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->createServiceFolder();
    }
}

void UbuntuoneStorageService::deleteFile(const QString &filename)
{

}

void UbuntuoneStorageService::deleteFolder(const QString &foldername)
{

}

QString UbuntuoneStorageService::storageServiceName() const
{
    return serviceName();
}

KIcon UbuntuoneStorageService::icon() const
{
    return KIcon();
}

