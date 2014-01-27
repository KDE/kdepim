/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#include "gdrivestorageservice.h"
#include "storageservice/widgets/storageservicetreewidget.h"
#include "storageservice/storageservicemanager.h"
#include "gdrivejob.h"

#include <qjson/parser.h>

#include <KLocalizedString>
#include <KConfig>
#include <KGlobal>
#include <KConfigGroup>

#include <QPointer>
#include <QDebug>

using namespace PimCommon;

GDriveStorageService::GDriveStorageService(QObject *parent)
    : PimCommon::StorageServiceAbstract(parent)
{
    readConfig();
}

GDriveStorageService::~GDriveStorageService()
{
}

void GDriveStorageService::readConfig()
{
    KConfig config(StorageServiceManager::kconfigName());
    KConfigGroup grp(&config, "GoogleDrive Settings");
    mRefreshToken = grp.readEntry("Refresh Token");
    mToken = grp.readEntry("Token");
    if (grp.hasKey("Expire Time"))
        mExpireDateTime = grp.readEntry("Expire Time", QDateTime::currentDateTime());
    else
        mExpireDateTime = QDateTime::currentDateTime();
}

void GDriveStorageService::removeConfig()
{
    KConfig config(StorageServiceManager::kconfigName());
    KConfigGroup grp(&config, "GoogleDrive Settings");
    grp.deleteGroup();
    config.sync();
}

void GDriveStorageService::storageServiceauthentication()
{
    GDriveJob *job = new GDriveJob(this);
    connect(job, SIGNAL(authorizationDone(QString,QString,qint64)), this, SLOT(slotAuthorizationDone(QString,QString,qint64)));
    connect(job, SIGNAL(authorizationFailed(QString)), this, SLOT(slotAuthorizationFailed(QString)));
    job->requestTokenAccess();
}

void GDriveStorageService::slotAuthorizationFailed(const QString &errorMessage)
{
    mRefreshToken.clear();
    mToken.clear();
    emitAuthentificationFailder(errorMessage);
}

void GDriveStorageService::slotAuthorizationDone(const QString &refreshToken, const QString &token, qint64 expireTime)
{
    mRefreshToken = refreshToken;
    mToken = token;
    mExpireDateTime = QDateTime::currentDateTime().addSecs(expireTime);
    KConfig config(StorageServiceManager::kconfigName());
    KConfigGroup grp(&config, "GoogleDrive Settings");
    grp.writeEntry("Refresh Token", mRefreshToken);
    grp.writeEntry("Token", mToken);
    grp.writeEntry("Expire Time", mExpireDateTime);
    grp.sync();
    emitAuthentificationDone();
}

bool GDriveStorageService::needToRefreshToken() const
{
    if (mExpireDateTime < QDateTime::currentDateTime())
        return true;
    else
        return false;
}

void GDriveStorageService::storageServiceShareLink(const QString &root, const QString &path)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(ShareLink);
        mNextAction->setPath(path);
        mNextAction->setRootPath(root);
        storageServiceauthentication();
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
        connect(job, SIGNAL(shareLinkDone(QString)), this, SLOT(slotShareLinkDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->shareLink(root, path);
    }
}

void GDriveStorageService::storageServicedownloadFile(const QString &name, const QString &fileId, const QString &destination)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(DownLoadFile);
        mNextAction->setNextActionName(name);
        mNextAction->setDownloadDestination(destination);
        mNextAction->setFileId(fileId);
        storageServiceauthentication();
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
        connect(job, SIGNAL(downLoadFileDone(QString)), this, SLOT(slotDownLoadFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        connect(job, SIGNAL(downLoadFileFailed(QString)), this, SLOT(slotDownLoadFileFailed(QString)));
        connect(job, SIGNAL(uploadDownloadFileProgress(qint64,qint64)), SLOT(slotuploadDownloadFileProgress(qint64,qint64)));
        mDownloadReply = job->downloadFile(name, fileId, destination);
    }
}

void GDriveStorageService::storageServicedeleteFile(const QString &filename)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(DeleteFile);
        mNextAction->setNextActionName(filename);
        storageServiceauthentication();
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
        connect(job, SIGNAL(deleteFileDone(QString)), SLOT(slotDeleteFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->deleteFile(filename);
    }
}

void GDriveStorageService::storageServicedeleteFolder(const QString &foldername)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(DeleteFolder);
        mNextAction->setNextActionFolder(foldername);
        storageServiceauthentication();
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
        connect(job, SIGNAL(deleteFolderDone(QString)), SLOT(slotDeleteFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->deleteFolder(foldername);
    }
}

void GDriveStorageService::storageServiceRenameFolder(const QString &source, const QString &destination)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(RenameFolder);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
        connect(job, SIGNAL(renameFolderDone(QString)), SLOT(slotRenameFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->renameFolder(source, destination);
    }
}

void GDriveStorageService::storageServiceRenameFile(const QString &source, const QString &destination)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(RenameFile);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
        connect(job, SIGNAL(renameFileDone(QString)), SLOT(slotRenameFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->renameFile(source, destination);
    }
}

void GDriveStorageService::storageServiceMoveFolder(const QString &source, const QString &destination)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(MoveFolder);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
        connect(job, SIGNAL(moveFolderDone(QString)), SLOT(slotRenameFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->moveFolder(source, destination);
    }
}

void GDriveStorageService::storageServiceMoveFile(const QString &source, const QString &destination)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(MoveFile);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
        connect(job, SIGNAL(moveFileDone(QString)), SLOT(slotRenameFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->moveFile(source, destination);
    }
}

void GDriveStorageService::storageServiceCopyFile(const QString &source, const QString &destination)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(CopyFile);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
        connect(job, SIGNAL(copyFileDone(QString)), SLOT(slotCopyFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->copyFile(source, destination);
    }
}

void GDriveStorageService::storageServiceCopyFolder(const QString &source, const QString &destination)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(CopyFolder);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
        connect(job, SIGNAL(copyFolderDone(QString)), SLOT(slotCopyFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->copyFolder(source, destination);
    }
}

QString GDriveStorageService::itemInformation(const QVariantMap &variantMap)
{
    return QString();
}

void GDriveStorageService::storageServicelistFolder(const QString &folder)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(ListFolder);
        mNextAction->setNextActionFolder(folder);
        storageServiceauthentication();
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
        connect(job, SIGNAL(listFolderDone(QString)), this, SLOT(slotListFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->listFolder(folder);
    }
}

void GDriveStorageService::storageServicecreateFolder(const QString &name, const QString &destination)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(CreateFolder);
        mNextAction->setNextActionName(name);
        mNextAction->setNextActionFolder(destination);
        storageServiceauthentication();
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
        connect(job, SIGNAL(createFolderDone(QString)), this, SLOT(slotCreateFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->createFolder(name, destination);
    }
}

void GDriveStorageService::storageServiceaccountInfo()
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(AccountInfo);
        storageServiceauthentication();
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
        connect(job,SIGNAL(accountInfoDone(PimCommon::AccountInfo)), this, SLOT(slotAccountInfoDone(PimCommon::AccountInfo)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->accountInfo();
    }
}

QString GDriveStorageService::name()
{
    return i18n("GoogleDrive");
}

void GDriveStorageService::storageServiceuploadFile(const QString &filename, const QString &destination)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(UploadFile);
        mNextAction->setNextActionName(filename);
        mNextAction->setNextActionFolder(destination);
        storageServiceauthentication();
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
        connect(job, SIGNAL(uploadFileDone(QString)), this, SLOT(slotUploadFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        connect(job, SIGNAL(shareLinkDone(QString)), this, SLOT(slotShareLinkDone(QString)));
        connect(job, SIGNAL(uploadDownloadFileProgress(qint64,qint64)), SLOT(slotuploadDownloadFileProgress(qint64,qint64)));
        connect(job, SIGNAL(uploadFileFailed(QString)), this, SLOT(slotUploadFileFailed(QString)));
        mUploadReply = job->uploadFile(filename, destination);
    }
}

QString GDriveStorageService::description()
{
    return i18n("Googledrive is a file hosting that offers cloud storage, file synchronization, and client software.");
}

QUrl GDriveStorageService::serviceUrl()
{
    return QUrl(QLatin1String("http://www.google.com/drive"));
}

QString GDriveStorageService::serviceName()
{
    return QLatin1String("googledrive");
}

QString GDriveStorageService::iconName()
{
    return QLatin1String("kdepim-googledrive");
}

StorageServiceAbstract::Capabilities GDriveStorageService::serviceCapabilities()
{
    StorageServiceAbstract::Capabilities cap;
    cap |= AccountInfoCapability;
    //cap |= UploadFileCapability;
    //cap |= DownloadFileCapability;
    //cap |= CreateFolderCapability;
    //cap |= DeleteFolderCapability;
    //cap |= ListFolderCapability;
    //cap |= DeleteFileCapability;
    //cap |= ShareLinkCapability;
    //cap |= RenameFolderCapability;
    //cap |= RenameFileCapabilitity;
    //cap |= MoveFileCapability;
    //cap |= MoveFolderCapability;
    //cap |= CopyFileCapability;
    //cap |= CopyFolderCapability;
    return cap;
}


QString GDriveStorageService::storageServiceName() const
{
    return serviceName();
}

QString GDriveStorageService::fileIdentifier(const QVariantMap &variantMap)
{
    //TODO
    return QString();
}

QString GDriveStorageService::fileShareRoot(const QVariantMap &variantMap)
{
    return QString();
}

KIcon GDriveStorageService::icon() const
{
    return KIcon(iconName());
}

StorageServiceAbstract::Capabilities GDriveStorageService::capabilities() const
{
    return serviceCapabilities();
}

void GDriveStorageService::storageServicecreateServiceFolder()
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(CreateServiceFolder);
        storageServiceauthentication();
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
        connect(job, SIGNAL(createFolderDone(QString)), this, SLOT(slotCreateFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->createServiceFolder();
    }
}

QString GDriveStorageService::fillListWidget(StorageServiceTreeWidget *listWidget, const QString &data)
{
    listWidget->clear();
    QJson::Parser parser;
    bool ok;
    listWidget->createMoveUpItem();
    const QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    qDebug()<<" info "<<info;
    QString parentId;
    if (info.contains(QLatin1String("id"))) {
        parentId = info.value(QLatin1String("id")).toString();
    }
    if (info.contains(QLatin1String("item_collection"))) {
        const QVariantMap itemCollection = info.value(QLatin1String("item_collection")).toMap();
        if (itemCollection.contains(QLatin1String("entries"))) {
            const QVariantList entries = itemCollection.value(QLatin1String("entries")).toList();
            Q_FOREACH (const QVariant &v, entries) {
                const QVariantMap mapEntries = v.toMap();
                if (mapEntries.contains(QLatin1String("type"))) {
                    const QString type = mapEntries.value(QLatin1String("type")).toString();
                    const QString name = mapEntries.value(QLatin1String("name")).toString();
                    const QString id = mapEntries.value(QLatin1String("id")).toString();
                    if (type == QLatin1String("folder")) {
                        listWidget->addFolder(name, id);
                    } else if (type == QLatin1String("file")) {
                        listWidget->addFile(name, id);
                    }
                }
                //qDebug()<<" v"<<v;
            }
        }
    }
    qDebug()<<" parentId"<<parentId;
    return parentId;
}
