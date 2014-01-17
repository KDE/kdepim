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

#include "yousenditstorageservice.h"
#include "storageservice/storageservicetreewidget.h"
#include "yousenditjob.h"

#include <KLocalizedString>
#include <KConfig>
#include <KGlobal>
#include <KConfigGroup>

#include <qjson/parser.h>

#include <QDebug>

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

void YouSendItStorageService::storageServiceauthentication()
{
    YouSendItJob *job = new YouSendItJob(this);
    connect(job, SIGNAL(authorizationDone(QString,QString,QString)), this, SLOT(slotAuthorizationDone(QString,QString,QString)));
    connect(job, SIGNAL(authorizationFailed(QString)), this, SLOT(slotAuthorizationFailed(QString)));
    job->requestTokenAccess();
}

void YouSendItStorageService::slotAuthorizationFailed(const QString &errorMessage)
{
    mUsername.clear();
    mPassword.clear();
    mToken.clear();
    emitAuthentificationFailder(errorMessage);
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
    emitAuthentificationDone();
}

void YouSendItStorageService::storageServicelistFolder(const QString &folder)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(ListFolder);
        mNextAction->setNextActionFolder(folder);
        authentication();
    } else {
        YouSendItJob *job = new YouSendItJob(this);
        job->initializeToken(mPassword, mUsername, mToken);
        connect(job, SIGNAL(listFolderDone(QString)), this, SLOT(slotListFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->listFolder(folder);
    }
}

void YouSendItStorageService::storageServicecreateFolder(const QString &folder)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionFolder(folder);
        mNextAction->setNextActionType(CreateFolder);
        authentication();
    } else {
        YouSendItJob *job = new YouSendItJob(this);
        job->initializeToken(mPassword, mUsername, mToken);
        connect(job, SIGNAL(createFolderDone(QString)), this, SLOT(slotCreateFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->createFolder(folder);
    }
}

void YouSendItStorageService::storageServiceaccountInfo()
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(AccountInfo);
        authentication();
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

void YouSendItStorageService::storageServiceuploadFile(const QString &filename)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionFileName(filename);
        mNextAction->setNextActionType(UploadFile);
        authentication();
    } else {
        YouSendItJob *job = new YouSendItJob(this);
        job->initializeToken(mPassword, mUsername, mToken);
        connect(job, SIGNAL(uploadFileDone(QString)), this, SLOT(slotUploadFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        connect(job, SIGNAL(shareLinkDone(QString)), this, SLOT(slotShareLinkDone(QString)));
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

QString YouSendItStorageService::iconName()
{
    return QString();
}

StorageServiceAbstract::Capabilities YouSendItStorageService::serviceCapabilities()
{
    StorageServiceAbstract::Capabilities cap;
    cap |= AccountInfoCapability;
    cap |= UploadFileCapability;
    //cap |= DownloadFileCapability;
    cap |= CreateFolderCapability;
    cap |= DeleteFolderCapability;
    cap |= ListFolderCapability;
    //cap |= ShareLinkCapability;
    cap |= DeleteFileCapability;
    cap |= RenameFolderCapability;
    //cap |= RenameFileCapabilitity;
    //cap |= MoveFileCapability;
    //cap |= MoveFolderCapability;

    return cap;
}

void YouSendItStorageService::storageServiceShareLink(const QString &root, const QString &path)
{
    if (mToken.isEmpty()) {
        mNextAction->setRootPath(root);
        mNextAction->setPath(path);
        mNextAction->setNextActionType(ShareLink);
        authentication();
    } else {
        YouSendItJob *job = new YouSendItJob(this);
        job->initializeToken(mPassword, mUsername, mToken);
        connect(job, SIGNAL(shareLinkDone(QString)), this, SLOT(slotShareLinkDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->shareLink(root, path);
    }
}

void YouSendItStorageService::storageServicedownloadFile(const QString &filename, const QString &destination)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionFileName(filename);
        mNextAction->setNextActionType(DownLoadFile);
        mNextAction->setDownloadDestination(filename);
        authentication();
    } else {
        YouSendItJob *job = new YouSendItJob(this);
        job->initializeToken(mPassword, mUsername, mToken);
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        connect(job, SIGNAL(downLoadFileDone(QString)), this, SLOT(slotDownLoadFileDone(QString)));
        job->downloadFile(filename, destination);
    }
}

void YouSendItStorageService::storageServicecreateServiceFolder()
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(CreateServiceFolder);
        authentication();
    } else {
        YouSendItJob *job = new YouSendItJob(this);
        job->initializeToken(mPassword, mUsername, mToken);
        connect(job, SIGNAL(createFolderDone(QString)), this, SLOT(slotShareLinkDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->createServiceFolder();
    }
}

void YouSendItStorageService::storageServicedeleteFile(const QString &filename)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionFileName(filename);
        mNextAction->setNextActionType(DeleteFile);
        authentication();
    } else {
        YouSendItJob *job = new YouSendItJob(this);
        job->initializeToken(mPassword, mUsername, mToken);
        connect(job, SIGNAL(deleteFileDone(QString)), SLOT(slotDeleteFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->deleteFile(filename);
    }
}

void YouSendItStorageService::storageServicedeleteFolder(const QString &foldername)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionFileName(foldername);
        mNextAction->setNextActionType(DeleteFolder);
        authentication();
    } else {
        YouSendItJob *job = new YouSendItJob(this);
        job->initializeToken(mPassword, mUsername, mToken);
        connect(job, SIGNAL(deleteFolderDone(QString)), SLOT(slotDeleteFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->deleteFolder(foldername);
    }
}

void YouSendItStorageService::storageServiceRenameFolder(const QString &source, const QString &destination)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(RenameFolder);
        mNextAction->setRenameFolder(source, destination);
        authentication();
    } else {
        YouSendItJob *job = new YouSendItJob(this);
        job->initializeToken(mPassword, mUsername, mToken);
        connect(job, SIGNAL(renameFolderDone(QString)), SLOT(slotRenameFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->renameFolder(source, destination);
    }
}

void YouSendItStorageService::storageServiceRenameFile(const QString &source, const QString &destination)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(RenameFile);
        mNextAction->setRenameFolder(source, destination);
        authentication();
    } else {
        YouSendItJob *job = new YouSendItJob(this);
        job->initializeToken(mPassword, mUsername, mToken);
        connect(job, SIGNAL(renameFileDone(QString)), SLOT(slotRenameFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->renameFile(source, destination);
    }
}

void YouSendItStorageService::storageServiceMoveFolder(const QString &source, const QString &destination)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(MoveFolder);
        mNextAction->setRenameFolder(source, destination);
        authentication();
    } else {
        YouSendItJob *job = new YouSendItJob(this);
        job->initializeToken(mPassword, mUsername, mToken);
        connect(job, SIGNAL(moveFolderDone(QString)), SLOT(slotMoveFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->moveFolder(source, destination);
    }
}

void YouSendItStorageService::storageServiceMoveFile(const QString &source, const QString &destination)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(MoveFile);
        mNextAction->setRenameFolder(source, destination);
        authentication();
    } else {
        YouSendItJob *job = new YouSendItJob(this);
        job->initializeToken(mPassword, mUsername, mToken);
        connect(job, SIGNAL(moveFileDone(QString)), SLOT(slotMoveFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->moveFile(source, destination);
    }
}

void YouSendItStorageService::storageServiceCopyFile(const QString &source, const QString &destination)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(CopyFile);
        mNextAction->setRenameFolder(source, destination);
        authentication();
    } else {
        YouSendItJob *job = new YouSendItJob(this);
        job->initializeToken(mPassword, mUsername, mToken);
        connect(job, SIGNAL(copyFileDone(QString)), SLOT(slotCopyFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->copyFile(source, destination);
    }
}

StorageServiceAbstract::Capabilities YouSendItStorageService::capabilities() const
{
    return serviceCapabilities();
}

QString YouSendItStorageService::fillListWidget(StorageServiceTreeWidget *listWidget, const QString &data)
{
    listWidget->clear();
    QJson::Parser parser;
    bool ok;
    const QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    if (info.contains(QLatin1String("folders"))) {
        const QVariantMap mapFolder = info.value(QLatin1String("folders")).toMap();
        const QVariantList folders = mapFolder.value(QLatin1String("folder")).toList();
        Q_FOREACH (const QVariant &v, folders) {
            const QVariantMap map = v.toMap();
            //qDebug()<<" folder map"<<map;
            if (map.contains(QLatin1String("name"))) {
                const QString name = map.value(QLatin1String("name")).toString();
                const QString folderId = map.value(QLatin1String("id")).toString();
                listWidget->addFolder(name, folderId);
            }
        }
        const QVariantMap mapFiles = info.value(QLatin1String("files")).toMap();
        const QVariantList files = mapFiles.value(QLatin1String("file")).toList();
        Q_FOREACH (const QVariant &v, files) {
            const QVariantMap map = v.toMap();
            qDebug()<<" file map !"<<map;
            if (map.contains(QLatin1String("name"))) {
                const QString name = map.value(QLatin1String("name")).toString();
                qDebug()<<" name !"<<name;
                const QString fileId = map.value(QLatin1String("id")).toString();
                StorageServiceListItem *item = listWidget->addFile(name, fileId);
                if (map.contains(QLatin1String("size"))) {
                    //qDebug()<<" size "<<map.value(QLatin1String("size"));
                    const qulonglong size = map.value(QLatin1String("size")).toULongLong();
                    item->setSize(size);
                }

            }
        }
    }
    return QString();
}

QString YouSendItStorageService::storageServiceName() const
{
    return serviceName();
}

KIcon YouSendItStorageService::icon() const
{
    return KIcon();
}
