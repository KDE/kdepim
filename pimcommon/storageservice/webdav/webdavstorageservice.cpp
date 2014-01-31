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
#include "storageservice/widgets/storageservicetreewidget.h"
#include "storageservice/storageservicemanager.h"
#include "storageservice/settings/storageservicesettings.h"
#include "webdavsettingsdialog.h"
#include "webdavjob.h"

#include <kwallet.h>

#include <KLocalizedString>
#include <KConfig>
#include <KGlobal>
#include <KConfigGroup>

#include <QPointer>
#include <QDebug>

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
    //TODO add support for don't save as kwallet ?
    KConfig config(StorageServiceManager::kconfigName());
    KConfigGroup grp(&config, "Webdav Settings");
    mPublicLocation = grp.readEntry(QLatin1String("public location"));
    mServiceLocation = grp.readEntry(QLatin1String("service location"));
    mUsername = grp.readEntry(QLatin1String("username"));
    if (StorageServiceSettings::self()->createDefaultFolder()) {
        const QString walletEntry = StorageServiceManager::kconfigName();
        KWallet::Wallet *wallet = StorageServiceSettings::self()->wallet();
        if (wallet)
            wallet->readPassword(walletEntry, mPassword);
    }
}

bool WebDavStorageService::needInitialized() const
{
    return (mServiceLocation.isEmpty() || mUsername.isEmpty() || mPassword.isEmpty());
}

void WebDavStorageService::removeConfig()
{
    KConfig config(StorageServiceManager::kconfigName());
    KConfigGroup grp(&config, "Webdav Settings");
    grp.deleteGroup();
    if (StorageServiceSettings::self()->createDefaultFolder()) {
        const QString walletEntry = StorageServiceManager::kconfigName();
        KWallet::Wallet *wallet = StorageServiceSettings::self()->wallet();
        if (wallet)
            wallet->removeEntry(walletEntry);
    }

    KGlobal::config()->sync();
}

void WebDavStorageService::storageServiceauthentication()
{
    WebDavJob *job = new WebDavJob(this);
    connect(job, SIGNAL(authorizationDone(QString,QString,QString,QString)), this, SLOT(slotAuthorizationDone(QString,QString,QString,QString)));
    connect(job, SIGNAL(authorizationFailed(QString)), this, SLOT(slotAuthorizationFailed(QString)));
    job->requestTokenAccess();
}

void WebDavStorageService::slotAuthorizationDone(const QString &publicLocation, const QString &serviceLocation, const QString &username, const QString &password)
{
    mPublicLocation = publicLocation;
    mServiceLocation = serviceLocation;
    mUsername = username;
    mPassword = password;

    KConfig config(StorageServiceManager::kconfigName());
    KConfigGroup grp(&config, "Webdav Settings");
    grp.writeEntry(QLatin1String("public location"), publicLocation);
    grp.writeEntry(QLatin1String("service location"), serviceLocation);
    grp.writeEntry(QLatin1String("username"), username);
    if (StorageServiceSettings::self()->createDefaultFolder()) {
        const QString walletEntry = StorageServiceManager::kconfigName();
        KWallet::Wallet *wallet = StorageServiceSettings::self()->wallet();
        if (wallet)
            wallet->writePassword(walletEntry, mPassword);
    }
    grp.sync();
    KGlobal::config()->sync();
    emitAuthentificationDone();
}

void WebDavStorageService::slotAuthorizationFailed(const QString &errorMessage)
{
    mPublicLocation.clear();
    mServiceLocation.clear();
    mUsername.clear();
    mPassword.clear();
    emitAuthentificationFailder(errorMessage);
}

void WebDavStorageService::storageServiceShareLink(const QString &root, const QString &path)
{
    if (needInitialized()) {
        mNextAction->setRootPath(root);
        mNextAction->setPath(path);
        mNextAction->setNextActionType(ShareLink);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        job->initializeToken(mPublicLocation, mServiceLocation, mUsername, mPassword);
        connect(job, SIGNAL(shareLinkDone(QString)), this, SLOT(slotShareLinkDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->shareLink(root, path);
    }
}

void WebDavStorageService::storageServicedownloadFile(const QString &name, const QString &fileId, const QString &destination)
{
    if (needInitialized()) {
        mNextAction->setNextActionType(DownLoadFile);
        mNextAction->setNextActionName(name);
        mNextAction->setDownloadDestination(destination);
        mNextAction->setFileId(fileId);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        job->initializeToken(mPublicLocation, mServiceLocation, mUsername, mPassword);
        connect(job, SIGNAL(downLoadFileDone(QString)), this, SLOT(slotDownLoadFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        connect(job, SIGNAL(downLoadFileFailed(QString)), this, SLOT(slotDownLoadFileFailed(QString)));
        connect(job, SIGNAL(uploadDownloadFileProgress(qint64,qint64)), SLOT(slotuploadDownloadFileProgress(qint64,qint64)));
        mDownloadReply = job->downloadFile(name, fileId, destination);
    }
}

void WebDavStorageService::storageServicecreateServiceFolder()
{
    if (needInitialized()) {
        mNextAction->setNextActionType(CreateServiceFolder);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        job->initializeToken(mPublicLocation, mServiceLocation, mUsername, mPassword);
        connect(job, SIGNAL(createFolderDone(QString)), this, SLOT(slotCreateFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->createServiceFolder();
    }
}

void WebDavStorageService::storageServicedeleteFile(const QString &filename)
{
    if (needInitialized()) {
        mNextAction->setNextActionType(DeleteFile);
        mNextAction->setNextActionName(filename);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        job->initializeToken(mPublicLocation, mServiceLocation, mUsername, mPassword);
        connect(job, SIGNAL(deleteFileDone(QString)), SLOT(slotDeleteFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->deleteFile(filename);
    }
}

void WebDavStorageService::storageServicedeleteFolder(const QString &foldername)
{
    if (needInitialized()) {
        mNextAction->setNextActionType(DeleteFolder);
        mNextAction->setNextActionName(foldername);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        job->initializeToken(mPublicLocation, mServiceLocation, mUsername, mPassword);
        connect(job, SIGNAL(deleteFolderDone(QString)), SLOT(slotDeleteFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->deleteFolder(foldername);
    }
}

void WebDavStorageService::storageServiceRenameFolder(const QString &source, const QString &destination)
{
    if (needInitialized()) {
        mNextAction->setNextActionType(RenameFolder);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        job->initializeToken(mPublicLocation, mServiceLocation, mUsername, mPassword);
        connect(job, SIGNAL(renameFolderDone(QString)), SLOT(slotRenameFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->renameFolder(source, destination);
    }
}

void WebDavStorageService::storageServiceRenameFile(const QString &source, const QString &destination)
{
    if (needInitialized()) {
        mNextAction->setNextActionType(RenameFile);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        job->initializeToken(mPublicLocation, mServiceLocation, mUsername, mPassword);
        connect(job, SIGNAL(renameFileDone(QString)), SLOT(slotRenameFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->renameFile(source, destination);
    }
}

void WebDavStorageService::storageServiceMoveFolder(const QString &source, const QString &destination)
{
    if (needInitialized()) {
        mNextAction->setNextActionType(MoveFolder);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        job->initializeToken(mPublicLocation, mServiceLocation, mUsername, mPassword);
        connect(job, SIGNAL(moveFolderDone(QString)), SLOT(slotMoveFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->moveFolder(source, destination);
    }
}

void WebDavStorageService::storageServiceMoveFile(const QString &source, const QString &destination)
{
    if (needInitialized()) {
        mNextAction->setNextActionType(MoveFile);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        job->initializeToken(mPublicLocation, mServiceLocation, mUsername, mPassword);
        connect(job, SIGNAL(moveFileDone(QString)), SLOT(slotMoveFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->moveFile(source, destination);
    }
}

void WebDavStorageService::storageServiceCopyFile(const QString &source, const QString &destination)
{
    if (needInitialized()) {
        mNextAction->setNextActionType(CopyFile);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        job->initializeToken(mPublicLocation, mServiceLocation, mUsername, mPassword);
        connect(job, SIGNAL(copyFileDone(QString)), SLOT(slotCopyFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->copyFile(source, destination);
    }
}

void WebDavStorageService::storageServiceCopyFolder(const QString &source, const QString &destination)
{
    if (needInitialized()) {
        mNextAction->setNextActionType(CopyFolder);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        job->initializeToken(mPublicLocation, mServiceLocation, mUsername, mPassword);
        connect(job, SIGNAL(copyFolderDone(QString)), SLOT(slotCopyFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->copyFolder(source, destination);
    }
}


StorageServiceAbstract::Capabilities WebDavStorageService::capabilities() const
{
    return serviceCapabilities();
}

QString WebDavStorageService::fillListWidget(StorageServiceTreeWidget *listWidget, const QString &data)
{
    listWidget->clear();
    listWidget->createMoveUpItem();
    return QString();
}

QString WebDavStorageService::itemInformation(const QVariantMap &variantMap)
{
    return QString();
}

QString WebDavStorageService::fileIdentifier(const QVariantMap &variantMap)
{
    return QString();
}

QString WebDavStorageService::fileShareRoot(const QVariantMap &variantMap)
{
    return QString();
}

void WebDavStorageService::storageServicelistFolder(const QString &folder)
{
    if (needInitialized()) {
        mNextAction->setNextActionType(ListFolder);
        mNextAction->setNextActionFolder(folder);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        job->initializeToken(mPublicLocation, mServiceLocation, mUsername, mPassword);
        connect(job, SIGNAL(listFolderDone(QString)), this, SLOT(slotListFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->listFolder(folder);
    }
}

void WebDavStorageService::storageServicecreateFolder(const QString &name, const QString &destination)
{
    if (needInitialized()) {
        mNextAction->setNextActionType(CreateFolder);
        mNextAction->setNextActionName(name);
        mNextAction->setNextActionFolder(destination);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        job->initializeToken(mPublicLocation, mServiceLocation, mUsername, mPassword);
        connect(job, SIGNAL(createFolderDone(QString)), this, SLOT(slotCreateFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->createFolder(name, destination);
    }
}

void WebDavStorageService::storageServiceaccountInfo()
{
    if (needInitialized()) {
        mNextAction->setNextActionType(AccountInfo);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        job->initializeToken(mPublicLocation, mServiceLocation, mUsername, mPassword);
        connect(job,SIGNAL(accountInfoDone(PimCommon::AccountInfo)), this, SLOT(slotAccountInfoDone(PimCommon::AccountInfo)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->accountInfo();
    }
}

QString WebDavStorageService::name()
{
    return i18n("Webdav");
}

void WebDavStorageService::storageServiceuploadFile(const QString &filename, const QString &uploadAsName, const QString &destination)
{
    if (needInitialized()) {
        mNextAction->setNextActionType(UploadFile);
        mNextAction->setNextActionName(filename);
        mNextAction->setNextActionFolder(destination);
        mNextAction->setUploadAsName(uploadAsName);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        job->initializeToken(mPublicLocation, mServiceLocation, mUsername, mPassword);
        connect(job, SIGNAL(uploadFileDone(QString)), this, SLOT(slotUploadFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        connect(job, SIGNAL(shareLinkDone(QString)), this, SLOT(slotShareLinkDone(QString)));
        connect(job, SIGNAL(uploadDownloadFileProgress(qint64,qint64)), SLOT(slotuploadDownloadFileProgress(qint64,qint64)));
        connect(job, SIGNAL(uploadFileFailed(QString)), this, SLOT(slotUploadFileFailed(QString)));
        mUploadReply = job->uploadFile(filename, uploadAsName, destination);
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
    cap |= AccountInfoCapability;
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
    //cap |= CopyFileCapability;
    //cap |= CopyFolderCapability;


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

#include "moc_webdavstorageservice.cpp"
