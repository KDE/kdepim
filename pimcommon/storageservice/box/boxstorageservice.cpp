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

#include "boxstorageservice.h"
#include "storageservice/widgets/storageservicetreewidget.h"
#include "storageservice/storageservicemanager.h"
#include "boxjob.h"

#include <qjson/parser.h>

#include <KLocalizedString>
#include <KConfig>
#include <KGlobal>
#include <KConfigGroup>

#include <QPointer>
#include <QDebug>

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
    KConfig config(StorageServiceManager::kconfigName());
    KConfigGroup grp(&config, "Box Settings");
    mRefreshToken = grp.readEntry("Refresh Token");
    mToken = grp.readEntry("Token");
    if (grp.hasKey("Expire Time"))
        mExpireDateTime = grp.readEntry("Expire Time", QDateTime::currentDateTime());
    else
        mExpireDateTime = QDateTime::currentDateTime();
}

void BoxStorageService::removeConfig()
{
    KConfig config(StorageServiceManager::kconfigName());
    KConfigGroup grp(&config, "Box Settings");
    grp.deleteGroup();
    grp.sync();
}

void BoxStorageService::storageServiceauthentication()
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
    emitAuthentificationFailder(errorMessage);
}

void BoxStorageService::slotAuthorizationDone(const QString &refreshToken, const QString &token, qint64 expireTime)
{
    mRefreshToken = refreshToken;
    mToken = token;
    KConfig config(StorageServiceManager::kconfigName());
    KConfigGroup grp(&config, "Box Settings");
    grp.writeEntry("Refresh Token", mRefreshToken);
    grp.writeEntry("Token", mToken);
    grp.writeEntry("Expire Time", QDateTime::currentDateTime().addSecs(expireTime));
    grp.sync();
    emitAuthentificationDone();
}

bool BoxStorageService::needToRefreshToken() const
{
    if (mExpireDateTime < QDateTime::currentDateTime())
        return true;
    else
        return false;
}

void BoxStorageService::storageServiceShareLink(const QString &root, const QString &path)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(ShareLink);
        mNextAction->setPath(path);
        mNextAction->setRootPath(root);
        storageServiceauthentication();
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
        connect(job, SIGNAL(shareLinkDone(QString)), this, SLOT(slotShareLinkDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->shareLink(root, path);
    }
}

void BoxStorageService::storageServicedownloadFile(const QString &name, const QString &fileId, const QString &destination)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(DownLoadFile);
        mNextAction->setNextActionName(name);
        mNextAction->setDownloadDestination(destination);
        storageServiceauthentication();
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
        connect(job, SIGNAL(downLoadFileDone(QString)), this, SLOT(slotDownLoadFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        mDownloadReply = job->downloadFile(name, fileId, destination);
    }
}

void BoxStorageService::storageServicedeleteFile(const QString &filename)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(DeleteFile);
        mNextAction->setNextActionName(filename);
        storageServiceauthentication();
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
        connect(job, SIGNAL(deleteFileDone(QString)), SLOT(slotDeleteFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->deleteFile(filename);
    }
}

void BoxStorageService::storageServicedeleteFolder(const QString &foldername)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(DeleteFolder);
        mNextAction->setNextActionFolder(foldername);
        storageServiceauthentication();
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
        connect(job, SIGNAL(deleteFolderDone(QString)), SLOT(slotDeleteFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->deleteFolder(foldername);
    }
}

void BoxStorageService::storageServiceRenameFolder(const QString &source, const QString &destination)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(RenameFolder);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
        connect(job, SIGNAL(renameFolderDone(QString)), SLOT(slotRenameFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->renameFolder(source, destination);
    }
}

void BoxStorageService::storageServiceRenameFile(const QString &source, const QString &destination)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(RenameFile);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
        connect(job, SIGNAL(renameFileDone(QString)), SLOT(slotRenameFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->renameFile(source, destination);
    }
}

void BoxStorageService::storageServiceMoveFolder(const QString &source, const QString &destination)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(MoveFolder);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
        connect(job, SIGNAL(moveFolderDone(QString)), SLOT(slotRenameFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->moveFolder(source, destination);
    }
}

void BoxStorageService::storageServiceMoveFile(const QString &source, const QString &destination)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(MoveFile);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
        connect(job, SIGNAL(moveFileDone(QString)), SLOT(slotRenameFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->moveFile(source, destination);
    }
}

void BoxStorageService::storageServiceCopyFile(const QString &source, const QString &destination)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(CopyFile);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
        connect(job, SIGNAL(copyFileDone(QString)), SLOT(slotCopyFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->copyFile(source, destination);
    }
}

void BoxStorageService::storageServiceCopyFolder(const QString &source, const QString &destination)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(CopyFolder);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
        connect(job, SIGNAL(copyFolderDone(QString)), SLOT(slotCopyFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->copyFolder(source, destination);
    }
}

void BoxStorageService::storageServicelistFolder(const QString &folder)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(ListFolder);
        mNextAction->setNextActionFolder(folder);
        storageServiceauthentication();
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
        connect(job, SIGNAL(listFolderDone(QString)), this, SLOT(slotListFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->listFolder(folder);
    }
}

void BoxStorageService::storageServicecreateFolder(const QString &name, const QString &destination)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(CreateFolder);
        mNextAction->setNextActionName(name);
        mNextAction->setNextActionFolder(destination);
        storageServiceauthentication();
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
        connect(job, SIGNAL(createFolderDone(QString)), this, SLOT(slotCreateFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->createFolder(name, destination);
    }
}

void BoxStorageService::storageServiceaccountInfo()
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(AccountInfo);
        storageServiceauthentication();
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

void BoxStorageService::storageServiceuploadFile(const QString &filename, const QString &destination)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(UploadFile);
        mNextAction->setNextActionName(filename);
        mNextAction->setNextActionFolder(destination);
        storageServiceauthentication();
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
        connect(job, SIGNAL(uploadFileDone(QString)), this, SLOT(slotUploadFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        connect(job, SIGNAL(shareLinkDone(QString)), this, SLOT(slotShareLinkDone(QString)));
        connect(job, SIGNAL(uploadFileProgress(qint64,qint64)), SLOT(slotUploadFileProgress(qint64,qint64)));
        mUploadReply = job->uploadFile(filename, destination);
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

StorageServiceAbstract::Capabilities BoxStorageService::serviceCapabilities()
{
    StorageServiceAbstract::Capabilities cap;
    cap |= AccountInfoCapability;
    //cap |= UploadFileCapability;
    //cap |= DownloadFileCapability;
    cap |= CreateFolderCapability;
    cap |= DeleteFolderCapability;
    cap |= ListFolderCapability;
    cap |= DeleteFileCapability;
    //cap |= ShareLinkCapability;
    cap |= RenameFolderCapability;
    cap |= RenameFileCapabilitity;
    cap |= MoveFileCapability;
    cap |= MoveFolderCapability;
    cap |= CopyFileCapability;
    cap |= CopyFolderCapability;
    return cap;
}


QString BoxStorageService::storageServiceName() const
{
    return serviceName();
}

KIcon BoxStorageService::icon() const
{
    return KIcon();
}

StorageServiceAbstract::Capabilities BoxStorageService::capabilities() const
{
    return serviceCapabilities();
}

void BoxStorageService::storageServicecreateServiceFolder()
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(CreateServiceFolder);
        storageServiceauthentication();
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken, mExpireDateTime);
        connect(job, SIGNAL(createFolderDone(QString)), this, SLOT(slotCreateFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->createServiceFolder();
    }
}

QString BoxStorageService::fillListWidget(StorageServiceTreeWidget *listWidget, const QString &data)
{
    listWidget->clear();
    QJson::Parser parser;
    bool ok;

    const QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    //qDebug()<<" info "<<info;
    listWidget->createMoveUpItem();
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
                    StorageServiceTreeWidgetItem *item = 0;
                    if (type == QLatin1String("folder")) {
                        item = listWidget->addFolder(name, id);
                    } else if (type == QLatin1String("file")) {
                        item = listWidget->addFile(name, id);
                    }
                    if (item) {
                        item->setStoreInfo(mapEntries);
                    }
                }
            }
        }
    }
    //qDebug()<<" parentId"<<parentId;
    return parentId;
}

QString BoxStorageService::itemInformation(const QVariantMap &variantMap)
{
    qDebug()<<" variantMap" <<variantMap;
    return QString();
}
