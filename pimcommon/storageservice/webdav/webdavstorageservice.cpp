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

#include "webdavstorageservice.h"
#include "storageservice/storageservicetreewidget.h"
#include "webdavsettingsdialog.h"
#include "webdavjob.h"

#include <KLocalizedString>
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

void WebDavStorageService::storageServiceauthentication()
{
    WebDavJob *job = new WebDavJob(this);
    //connect(job, SIGNAL(authorizationDone(QString,QString,QString)), this, SLOT(slotAuthorizationDone(QString,QString,QString)));
    connect(job, SIGNAL(authorizationFailed(QString)), this, SLOT(slotAuthorizationFailed(QString)));
    job->requestTokenAccess();
}

void WebDavStorageService::storageServiceShareLink(const QString &root, const QString &path)
{
    if (mServiceLocation.isEmpty()) {
        mNextAction->setRootPath(root);
        mNextAction->setPath(path);
        mNextAction->setNextActionType(ShareLink);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        connect(job, SIGNAL(shareLinkDone(QString)), this, SLOT(slotShareLinkDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->shareLink(root, path);
    }
}

void WebDavStorageService::storageServicedownloadFile(const QString &filename, const QString &destination)
{
    if (mServiceLocation.isEmpty()) {
        mNextAction->setNextActionType(DownLoadFile);
        mNextAction->setNextActionFileName(filename);
        mNextAction->setDownloadDestination(filename);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        connect(job, SIGNAL(downLoadFileDone(QString)), this, SLOT(slotDownLoadFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->downloadFile(filename, destination);
    }
}

void WebDavStorageService::storageServicecreateServiceFolder()
{
    if (mServiceLocation.isEmpty()) {
        mNextAction->setNextActionType(CreateServiceFolder);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        connect(job, SIGNAL(createFolderDone(QString)), this, SLOT(slotCreateFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->createServiceFolder();
    }
}

void WebDavStorageService::storageServicedeleteFile(const QString &filename)
{
    if (mServiceLocation.isEmpty()) {
        mNextAction->setNextActionType(DeleteFile);
        mNextAction->setNextActionFileName(filename);
        authentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        connect(job, SIGNAL(deleteFileDone(QString)), SLOT(slotDeleteFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->deleteFile(filename);
    }
}

void WebDavStorageService::storageServicedeleteFolder(const QString &foldername)
{
    if (mServiceLocation.isEmpty()) {
        mNextAction->setNextActionType(DeleteFolder);
        mNextAction->setNextActionFileName(foldername);
        authentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        connect(job, SIGNAL(deleteFolderDone(QString)), SLOT(slotDeleteFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->deleteFolder(foldername);
    }
}

void WebDavStorageService::storageServiceRenameFolder(const QString &source, const QString &destination)
{
    if (mServiceLocation.isEmpty()) {
        mNextAction->setNextActionType(RenameFolder);
        mNextAction->setRenameFolder(source, destination);
        authentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        connect(job, SIGNAL(renameFolderDone(QString)), SLOT(slotRenameFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->renameFolder(source, destination);
    }
}

void WebDavStorageService::storageServiceRenameFile(const QString &source, const QString &destination)
{
    if (mServiceLocation.isEmpty()) {
        mNextAction->setNextActionType(RenameFile);
        mNextAction->setRenameFolder(source, destination);
        authentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        connect(job, SIGNAL(renameFileDone(QString)), SLOT(slotRenameFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->renameFile(source, destination);
    }
}

void WebDavStorageService::storageServiceMoveFolder(const QString &source, const QString &destination)
{
    if (mServiceLocation.isEmpty()) {
        mNextAction->setNextActionType(MoveFolder);
        mNextAction->setRenameFolder(source, destination);
        authentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        connect(job, SIGNAL(moveFolderDone(QString)), SLOT(slotRenameFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->moveFolder(source, destination);
    }
}

void WebDavStorageService::storageServiceMoveFile(const QString &source, const QString &destination)
{
    if (mServiceLocation.isEmpty()) {
        mNextAction->setNextActionType(MoveFile);
        mNextAction->setRenameFolder(source, destination);
        authentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        connect(job, SIGNAL(moveFileDone(QString)), SLOT(slotRenameFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->moveFile(source, destination);
    }
}

void WebDavStorageService::storageServiceCopyFile(const QString &source, const QString &destination)
{
    if (mServiceLocation.isEmpty()) {
        mNextAction->setNextActionType(CopyFile);
        mNextAction->setRenameFolder(source, destination);
        authentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        connect(job, SIGNAL(copyFileDone(QString)), SLOT(slotCopyFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->copyFile(source, destination);
    }
}

StorageServiceAbstract::Capabilities WebDavStorageService::capabilities() const
{
    return serviceCapabilities();
}

QString WebDavStorageService::fillListWidget(StorageServiceTreeWidget *listWidget, const QString &data)
{
    listWidget->clear();
    return QString();
}

void WebDavStorageService::storageServicelistFolder(const QString &folder)
{
    if (mServiceLocation.isEmpty()) {
        mNextAction->setNextActionType(ListFolder);
        mNextAction->setNextActionFolder(folder);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        connect(job, SIGNAL(listFolderDone(QString)), this, SLOT(slotListFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->listFolder(folder);
    }
}

void WebDavStorageService::storageServicecreateFolder(const QString &folder)
{
    if (mServiceLocation.isEmpty()) {
        mNextAction->setNextActionType(CreateFolder);
        mNextAction->setNextActionFolder(folder);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        connect(job, SIGNAL(createFolderDone(QString)), this, SLOT(slotCreateFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->createFolder(folder);
    }
}

void WebDavStorageService::storageServiceaccountInfo()
{
    if (mServiceLocation.isEmpty()) {
        mNextAction->setNextActionType(AccountInfo);
        storageServiceauthentication();
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

void WebDavStorageService::storageServiceuploadFile(const QString &filename)
{
    if (mServiceLocation.isEmpty()) {
        mNextAction->setNextActionType(UploadFile);
        mNextAction->setNextActionFileName(filename);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        connect(job, SIGNAL(uploadFileDone(QString)), this, SLOT(slotUploadFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        connect(job, SIGNAL(shareLinkDone(QString)), this, SLOT(slotShareLinkDone(QString)));
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

QString WebDavStorageService::iconName()
{
    return QString();
}

StorageServiceAbstract::Capabilities WebDavStorageService::serviceCapabilities()
{
    StorageServiceAbstract::Capabilities cap;
    //cap |= AccountInfoCapability;
    //cap |= UploadFileCapability;
    //cap |= DownloadFileCapability;
    //cap |= CreateFolderCapability;
    //cap |= DeleteFolderCapability;
    //cap |= ListFolderCapability;
    //cap |= ShareLinkCapability;
    //cap |= DeleteFileCapability;
    //cap |= RenameFolderCapability;
    //cap |= RenameFileCapabilitity;
    //cap |= MoveFileCapability;
    //cap |= MoveFolderCapability;

    return cap;
}

QString WebDavStorageService::storageServiceName() const
{
    return serviceName();
}

KIcon WebDavStorageService::icon() const
{
    return KIcon();
}
