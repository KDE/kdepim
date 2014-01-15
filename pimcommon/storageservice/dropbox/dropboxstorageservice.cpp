/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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
#include "storageservice/storageservicetreewidget.h"
#include "dropboxjob.h"

#include <qjson/parser.h>

#include <KLocalizedString>
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

void DropBoxStorageService::removeConfig()
{
    KConfigGroup grp(KGlobal::config(), "Dropbox Settings");
    grp.deleteGroup();
    KGlobal::config()->sync();
}

void DropBoxStorageService::readConfig()
{
    KConfigGroup grp(KGlobal::config(), "Dropbox Settings");
    mAccessToken = grp.readEntry("Access Token");
    mAccessTokenSecret = grp.readEntry("Access Token Secret");
    mAccessOauthSignature = grp.readEntry("Access Oauth Signature");
}

void DropBoxStorageService::storageServiceauthentication()
{
    DropBoxJob *job = new DropBoxJob(this);
    connect(job, SIGNAL(authorizationDone(QString,QString,QString)), this, SLOT(slotAuthorizationDone(QString,QString,QString)));
    connect(job, SIGNAL(authorizationFailed(QString)), this, SLOT(slotAuthorizationFailed(QString)));
    job->requestTokenAccess();
}

void DropBoxStorageService::storageServiceShareLink(const QString &root, const QString &path)
{
    if (mAccessToken.isEmpty()) {
        mNextAction->setNextActionType(ShareLink);
        mNextAction->setRootPath(root);
        mNextAction->setPath(path);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken,mAccessTokenSecret,mAccessOauthSignature);
        connect(job, SIGNAL(shareLinkDone(QString)), this, SLOT(slotShareLinkDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->shareLink(root, path);
    }
}

void DropBoxStorageService::storageServicecreateServiceFolder()
{
    if (mAccessToken.isEmpty()) {
        mNextAction->setNextActionType(CreateServiceFolder);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken,mAccessTokenSecret,mAccessOauthSignature);
        connect(job, SIGNAL(createFolderDone(QString)), this, SLOT(slotCreateFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->createServiceFolder();
    }
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
    emitAuthentificationDone();
}

void DropBoxStorageService::storageServicelistFolder(const QString &folder)
{
    if (mAccessToken.isEmpty()) {        
        mNextAction->setNextActionType(ListFolder);
        mNextAction->setNextActionFolder(folder);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken,mAccessTokenSecret,mAccessOauthSignature);
        connect(job, SIGNAL(listFolderDone(QString)), this, SLOT(slotListFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->listFolder(folder);
    }
}

void DropBoxStorageService::storageServiceaccountInfo()
{
    if (mAccessToken.isEmpty()) {
        mNextAction->setNextActionType(AccountInfo);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken,mAccessTokenSecret,mAccessOauthSignature);
        connect(job, SIGNAL(accountInfoDone(PimCommon::AccountInfo)), this, SLOT(slotAccountInfoDone(PimCommon::AccountInfo)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->accountInfo();
    }
}

void DropBoxStorageService::storageServicecreateFolder(const QString &folder)
{
    if (mAccessToken.isEmpty()) {
        mNextAction->setNextActionType(CreateFolder);
        mNextAction->setNextActionFolder(folder);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken,mAccessTokenSecret,mAccessOauthSignature);
        connect(job, SIGNAL(createFolderDone(QString)), this, SLOT(slotCreateFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->createFolder(folder);
    }
}

void DropBoxStorageService::storageServiceuploadFile(const QString &filename)
{
    if (mAccessToken.isEmpty()) {
        mNextAction->setNextActionType(UploadFile);
        mNextAction->setNextActionFileName(filename);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken,mAccessTokenSecret,mAccessOauthSignature);
        connect(job, SIGNAL(uploadFileDone(QString)), this, SLOT(slotUploadFileDone(QString)));
        connect(job, SIGNAL(shareLinkDone(QString)), this, SLOT(slotShareLinkDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        connect(job, SIGNAL(uploadFileProgress(qint64,qint64)), SLOT(slotUploadFileProgress(qint64,qint64)));
        job->uploadFile(filename);
    }
}

void DropBoxStorageService::slotAuthorizationFailed(const QString &errorMessage)
{
    mAccessToken.clear();
    mAccessTokenSecret.clear();
    mAccessOauthSignature.clear();
    emitAuthentificationFailder(errorMessage);
}

QString DropBoxStorageService::name()
{
    return i18n("Dropbox");
}

QString DropBoxStorageService::description()
{
    return i18n("Dropbox is a file hosting service operated by Dropbox, Inc. that offers cloud storage, file synchronization, and client software.");
}

QUrl DropBoxStorageService::serviceUrl()
{
    return QUrl(QLatin1String("https://www.dropbox.com/"));
}

QString DropBoxStorageService::serviceName()
{
    return QLatin1String("dropbox");
}

QString DropBoxStorageService::iconName()
{
    return QString();
}

StorageServiceAbstract::Capabilities DropBoxStorageService::serviceCapabilities()
{
    StorageServiceAbstract::Capabilities cap;
    cap |= AccountInfoCapability;
    cap |= UploadFileCapability;
    //cap |= DownloadFileCapability;
    cap |= CreateFolderCapability;
    cap |= DeleteFolderCapability;
    cap |= DeleteFileCapability;
    cap |= ListFolderCapability;
    cap |= ShareLinkCapability;
    cap |= RenameFolderCapability;
    cap |= RenameFileCapabilitity;
    //cap |= MoveFileCapability;
    //cap |= MoveFolderCapability;

    return cap;
}

QString DropBoxStorageService::storageServiceName() const
{
    return serviceName();
}

void DropBoxStorageService::storageServicedownloadFile(const QString &filename)
{
    if (mAccessToken.isEmpty()) {
        mNextAction->setNextActionType(DownLoadFile);
        mNextAction->setNextActionFileName(filename);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken,mAccessTokenSecret,mAccessOauthSignature);
        connect(job, SIGNAL(downLoadFileDone(QString)), this, SLOT(slotDownLoadFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->downloadFile(filename);
    }
}

void DropBoxStorageService::storageServicedeleteFile(const QString &filename)
{
    if (mAccessToken.isEmpty()) {
        mNextAction->setNextActionType(DeleteFile);
        mNextAction->setNextActionFileName(filename);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken,mAccessTokenSecret,mAccessOauthSignature);
        connect(job, SIGNAL(deleteFileDone(QString)), SLOT(slotDeleteFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->deleteFile(filename);
    }
}

void DropBoxStorageService::storageServicedeleteFolder(const QString &foldername)
{
    if (mAccessToken.isEmpty()) {
        mNextAction->setNextActionType(DeleteFolder);
        mNextAction->setNextActionFolder(foldername);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken,mAccessTokenSecret,mAccessOauthSignature);
        connect(job, SIGNAL(deleteFolderDone(QString)), SLOT(slotDeleteFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->deleteFolder(foldername);
    }
}

void DropBoxStorageService::storageServiceRenameFolder(const QString &source, const QString &destination)
{
    if (mAccessToken.isEmpty()) {
        mNextAction->setNextActionType(RenameFolder);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken,mAccessTokenSecret,mAccessOauthSignature);
        connect(job, SIGNAL(renameFolderDone(QString)), SLOT(slotRenameFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->renameFolder(source, destination);
    }
}

void DropBoxStorageService::storageServiceRenameFile(const QString &source, const QString &destination)
{
    if (mAccessToken.isEmpty()) {
        mNextAction->setNextActionType(RenameFile);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken,mAccessTokenSecret,mAccessOauthSignature);
        connect(job, SIGNAL(renameFileDone(QString)), SLOT(slotRenameFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->renameFile(source, destination);
    }
}

void DropBoxStorageService::storageServiceMoveFolder(const QString &source, const QString &destination)
{
    if (mAccessToken.isEmpty()) {
        mNextAction->setNextActionType(MoveFolder);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken,mAccessTokenSecret,mAccessOauthSignature);
        connect(job, SIGNAL(moveFolderDone(QString)), SLOT(slotMoveFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->moveFolder(source, destination);
    }
}

void DropBoxStorageService::storageServiceMoveFile(const QString &source, const QString &destination)
{
    if (mAccessToken.isEmpty()) {
        mNextAction->setNextActionType(RenameFolder);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken,mAccessTokenSecret,mAccessOauthSignature);
        connect(job, SIGNAL(moveFileDone(QString)), SLOT(slotMoveFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->moveFile(source, destination);
    }
}

StorageServiceAbstract::Capabilities DropBoxStorageService::capabilities() const
{
    return serviceCapabilities();
}

void DropBoxStorageService::fillListWidget(StorageServiceTreeWidget *listWidget, const QString &data)
{
    listWidget->clear();
    QJson::Parser parser;
    bool ok;
    QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    qDebug()<<" info "<<info;
    if (info.contains(QLatin1String("contents"))) {
        const QVariantList lst = info.value(QLatin1String("contents")).toList();
        Q_FOREACH (const QVariant &variant, lst) {
            const QVariantMap qwer = variant.toMap();
            qDebug()<<" qwer "<<qwer;
            if (qwer.contains(QLatin1String("is_dir"))) {
                bool value = qwer.value(QLatin1String("is_dir")).toBool();
                const QString name = qwer.value(QLatin1String("path")).toString();
                if (value) {
                    listWidget->addFolder(name, name);
                } else {
                    QString mimetype;
                    if (qwer.contains(QLatin1String("mime_type"))) {
                        mimetype = qwer.value(QLatin1String("mime_type")).toString();
                        qDebug()<<" mimetype"<<mimetype;
                    }
                    StorageServiceListItem *item = listWidget->addFile(name, name, mimetype);
                    if (qwer.contains(QLatin1String("bytes"))) {
                        item->setSize(qwer.value(QLatin1String("bytes")).toULongLong());
                    }
                }
            }
        }
    }
}

bool DropBoxStorageService::hasProgressIndicatorSupport() const
{
    return true;
}

KIcon DropBoxStorageService::icon() const
{
    return KIcon();
}
