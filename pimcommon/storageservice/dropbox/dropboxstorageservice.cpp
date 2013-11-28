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

#include "dropboxstorageservice.h"
#include "dropboxjob.h"

#include <KLocale>
#include <KConfig>
#include <KGlobal>
#include <KConfigGroup>
#include <KMessageBox>

#include <QDebug>


using namespace PimCommon;

DropBoxStorageService::DropBoxStorageService(QObject *parent)
    : PimCommon::StorageServiceAbstract(parent)
{
    readConfig();
}

DropBoxStorageService::~DropBoxStorageService()
{
}

void DropBoxStorageService::readConfig()
{
    KConfigGroup grp(KGlobal::config(), "Dropbox Settings");
    mAccessToken = grp.readEntry("Access Token");
    mAccessTokenSecret = grp.readEntry("Access Token Secret");
    mAccessOauthSignature = grp.readEntry("Access Oauth Signature");
}

void DropBoxStorageService::slotAuthorizationDone(const QString &accessToken, const QString &accessTokenSecret, const QString &accessOauthSignature)
{
    mAccessToken = accessToken;
    mAccessTokenSecret = accessTokenSecret;
    mAccessOauthSignature = accessOauthSignature;
    KConfigGroup grp(KGlobal::config(), "Dropbox Settings");
    grp.writeEntry("Access Token", mAccessToken);
    grp.writeEntry("Access Token Secret", mAccessTokenSecret);
    grp.writeEntry("Access Oauth Signature", mAccessOauthSignature);
    grp.sync();
    KGlobal::config()->sync();
}

void DropBoxStorageService::listFolder()
{
    DropBoxJob *job = new DropBoxJob(this);
    if (mAccessToken.isEmpty()) {
        connect(job, SIGNAL(authorizationDone(QString,QString,QString)), this, SLOT(slotAuthorizationDone(QString,QString,QString)));
        connect(job, SIGNAL(authorizationFailed()), this, SLOT(slotAuthorizationFailed()));
        job->requestTokenAccess();
    } else {
        job->initializeToken(mAccessToken,mAccessTokenSecret,mAccessOauthSignature);
        connect(job, SIGNAL(listFolderDone()), this, SLOT(slotListFolderDone()));
        connect(job, SIGNAL(listFolderFailed()), this, SLOT(slotListFolderFailed()));
        job->listFolder();
    }
}

void DropBoxStorageService::slotListFolderDone()
{

}

void DropBoxStorageService::slotListFolderFailed()
{
    KMessageBox::error(0, i18n("List Folder Failed"), i18n("List Folder"));
}

void DropBoxStorageService::accountInfo()
{
    DropBoxJob *job = new DropBoxJob(this);
    if (mAccessToken.isEmpty()) {
        connect(job, SIGNAL(authorizationDone(QString,QString,QString)), this, SLOT(slotAuthorizationDone(QString,QString,QString)));
        connect(job, SIGNAL(authorizationFailed()), this, SLOT(slotAuthorizationFailed()));
        job->requestTokenAccess();
    } else {
        job->initializeToken(mAccessToken,mAccessTokenSecret,mAccessOauthSignature);
        connect(job, SIGNAL(accountInfoDone(QString)), this, SLOT(slotAccountInfoDone(QString)));
        connect(job, SIGNAL(accountInfoFailed()), this, SLOT(slotAccountInfoFailed()));
        job->accountInfo();
    }
}

void DropBoxStorageService::slotAccountInfoDone(const QString &info)
{
    qDebug()<<" account info "<<info;
}

void DropBoxStorageService::slotAccountInfoFailed()
{
    KMessageBox::error(0, i18n("Account Info Failed"), i18n("Account Info"));
}

void DropBoxStorageService::createFolder(const QString &folder)
{
    DropBoxJob *job = new DropBoxJob(this);
    if (mAccessToken.isEmpty()) {
        connect(job, SIGNAL(authorizationDone(QString,QString,QString)), this, SLOT(slotAuthorizationDone(QString,QString,QString)));
        connect(job, SIGNAL(authorizationFailed()), this, SLOT(slotAuthorizationFailed()));
        job->requestTokenAccess();
    } else {
        job->initializeToken(mAccessToken,mAccessTokenSecret,mAccessOauthSignature);
        connect(job, SIGNAL(createFolderDone()), this, SLOT(slotCreateFolderDone()));
        connect(job, SIGNAL(createFolderFailed()), this, SLOT(slotCreateFolderFailed()));
        job->createFolder(folder);
    }
}

void DropBoxStorageService::slotCreateFolderDone()
{
    qDebug()<<" folder created";
}

void DropBoxStorageService::slotCreateFolderFailed()
{
    qDebug()<<" folder not created";
    KMessageBox::error(0, i18n("Create folder failed"), i18n("Create Folder"));
}

void DropBoxStorageService::uploadFile(const QString &filename)
{
    DropBoxJob *job = new DropBoxJob(this);
    if (mAccessToken.isEmpty()) {
        connect(job, SIGNAL(authorizationDone(QString,QString,QString)), this, SLOT(slotAuthorizationDone(QString,QString,QString)));
        connect(job, SIGNAL(authorizationFailed()), this, SLOT(slotAuthorizationFailed()));
        job->requestTokenAccess();
    } else {
        job->initializeToken(mAccessToken,mAccessTokenSecret,mAccessOauthSignature);
        connect(job, SIGNAL(uploadFileDone()), this, SLOT(slotUploadFileDone()));
        connect(job, SIGNAL(uploadFileFailed()), this, SLOT(slotUploadFileFailed()));
        job->uploadFile(filename);
    }
}

void DropBoxStorageService::slotUploadFileDone()
{
    qDebug()<<" Upload file done";
}

void DropBoxStorageService::slotUploadFileFailed()
{
    KMessageBox::error(0, i18n("Upload File failed"), i18n("Upload File"));
}

void DropBoxStorageService::slotAuthorizationFailed()
{
    mAccessToken.clear();
    mAccessTokenSecret.clear();
    mAccessOauthSignature.clear();
}

QString DropBoxStorageService::name() const
{
    return i18n("DropBox");
}

QUrl DropBoxStorageService::sharedUrl() const
{
    return QUrl();
}


QString DropBoxStorageService::description() const
{
    return QString();
}

QUrl DropBoxStorageService::serviceUrl() const
{
    return QUrl(QLatin1String("https://www.dropbox.com/"));
}
