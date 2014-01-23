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

#include "ubuntuonestorageservice.h"
#include "storageservice/widgets/storageservicetreewidget.h"
#include "storageservice/storageservicemanager.h"
#include "ubuntuonejob.h"

#include <qjson/parser.h>

#include <KLocalizedString>
#include <KConfig>
#include <KGlobal>
#include <KConfigGroup>

#include <QDebug>


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
    KConfig config(StorageServiceManager::kconfigName());
    KConfigGroup grp(&config, "Ubuntu One Settings");

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

    KConfig config(StorageServiceManager::kconfigName());
    KConfigGroup grp(&config, "Ubuntu One Settings");
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
    KConfig config(StorageServiceManager::kconfigName());
    KConfigGroup grp(&config, "Ubuntu One Settings");
    grp.deleteGroup();
    grp.sync();
}

void UbuntuoneStorageService::storageServiceauthentication()
{
    UbuntuOneJob *job = new UbuntuOneJob(this);
    connect(job, SIGNAL(authorizationDone(QString,QString,QString,QString)), this, SLOT(slotAuthorizationDone(QString,QString,QString,QString)));
    connect(job, SIGNAL(authorizationFailed(QString)), this, SLOT(slotAuthorizationFailed(QString)));
    job->requestTokenAccess();
}

void UbuntuoneStorageService::storageServicelistFolder(const QString &folder)
{
    if (mTokenSecret.isEmpty()) {
        mNextAction->setNextActionType(ListFolder);
        mNextAction->setNextActionFolder(folder);
        storageServiceauthentication();
    } else {
        UbuntuOneJob *job = new UbuntuOneJob(this);
        connect(job, SIGNAL(listFolderDone(QString)), this, SLOT(slotListFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->initializeToken(mCustomerSecret, mToken, mCustomerKey, mTokenSecret);
        job->listFolder(folder);
    }
}

void UbuntuoneStorageService::storageServicecreateFolder(const QString &name, const QString &destination)
{
    if (mTokenSecret.isEmpty()) {
        mNextAction->setNextActionType(CreateFolder);
        mNextAction->setNextActionName(name);
        mNextAction->setNextActionFolder(destination);

        storageServiceauthentication();
    } else {
        UbuntuOneJob *job = new UbuntuOneJob(this);
        job->initializeToken(mCustomerSecret, mToken, mCustomerKey, mTokenSecret);
        connect(job, SIGNAL(createFolderDone(QString)), this, SLOT(slotCreateFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->createFolder(name, destination);
    }
}

void UbuntuoneStorageService::slotAuthorizationFailed(const QString &errorMessage)
{
    mCustomerSecret.clear();
    mToken.clear();
    mCustomerKey.clear();
    mTokenSecret.clear();
    emitAuthentificationFailder(errorMessage);
}

void UbuntuoneStorageService::storageServiceaccountInfo()
{
    if (mTokenSecret.isEmpty()) {
        mNextAction->setNextActionType(AccountInfo);
        storageServiceauthentication();
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

void UbuntuoneStorageService::storageServiceuploadFile(const QString &filename, const QString &destination)
{
    if (mTokenSecret.isEmpty()) {
        mNextAction->setNextActionType(UploadFile);
        mNextAction->setNextActionName(filename);
        mNextAction->setNextActionFolder(destination);
        storageServiceauthentication();
    } else {
        UbuntuOneJob *job = new UbuntuOneJob(this);
        job->initializeToken(mCustomerSecret, mToken, mCustomerKey, mTokenSecret);
        connect(job, SIGNAL(uploadFileDone(QString)), this, SLOT(slotUploadFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        connect(job, SIGNAL(shareLinkDone(QString)), this, SLOT(slotShareLinkDone(QString)));
        connect(job, SIGNAL(uploadFileProgress(qint64,qint64)), SLOT(slotUploadFileProgress(qint64,qint64)));
        connect(job, SIGNAL(uploadFileFailed(QString)), this, SLOT(slotUploadFileFailed(QString)));
        mUploadReply = job->uploadFile(filename, destination);
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

StorageServiceAbstract::Capabilities UbuntuoneStorageService::serviceCapabilities()
{
    StorageServiceAbstract::Capabilities cap;
    cap |= AccountInfoCapability;
    //cap |= UploadFileCapability;
    //cap |= DownloadFileCapability;
    cap |= CreateFolderCapability;
    cap |= DeleteFolderCapability;
    cap |= ListFolderCapability;
    //cap |= ShareLinkCapability;
    cap |= DeleteFileCapability;
    cap |= RenameFolderCapability;
    cap |= RenameFileCapabilitity;
    //cap |= MoveFileCapability;
    //cap |= MoveFolderCapability;
    //cap |= CopyFileCapability;
    //cap |= CopyFolderCapability;

    return cap;
}

void UbuntuoneStorageService::storageServiceShareLink(const QString &root, const QString &path)
{    
    if (mTokenSecret.isEmpty()) {
        mNextAction->setNextActionType(ShareLink);
        mNextAction->setRootPath(root);
        mNextAction->setPath(path);
        storageServiceauthentication();
    } else {
        UbuntuOneJob *job = new UbuntuOneJob(this);
        job->initializeToken(mCustomerSecret, mToken, mCustomerKey, mTokenSecret);
        connect(job, SIGNAL(shareLinkDone(QString)), this, SLOT(slotShareLinkDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->shareLink(root, path);
    }
}

void UbuntuoneStorageService::storageServicedownloadFile(const QString &filename, const QString &destination)
{
    if (mTokenSecret.isEmpty()) {
        mNextAction->setNextActionType(DownLoadFile);
        mNextAction->setNextActionName(filename);
        mNextAction->setDownloadDestination(filename);
        storageServiceauthentication();
    } else {
        UbuntuOneJob *job = new UbuntuOneJob(this);
        job->initializeToken(mCustomerSecret, mToken, mCustomerKey, mTokenSecret);
        connect(job, SIGNAL(downLoadFileDone(QString)), this, SLOT(slotDownLoadFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        connect(job, SIGNAL(downLoadFileFailed(QString)), this, SLOT(slotDownLoadFileFailed(QString)));
        mDownloadReply = job->downloadFile(filename, destination);
    }
}

void UbuntuoneStorageService::storageServicecreateServiceFolder()
{
    if (mTokenSecret.isEmpty()) {
        mNextAction->setNextActionType(CreateServiceFolder);
        storageServiceauthentication();
    } else {
        UbuntuOneJob *job = new UbuntuOneJob(this);
        job->initializeToken(mCustomerSecret, mToken, mCustomerKey, mTokenSecret);
        connect(job, SIGNAL(createFolderDone(QString)), this, SLOT(slotCreateFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->createServiceFolder();
    }
}

void UbuntuoneStorageService::storageServicedeleteFile(const QString &filename)
{
    if (mTokenSecret.isEmpty()) {
        mNextAction->setNextActionType(DeleteFile);
        mNextAction->setNextActionName(filename);
        storageServiceauthentication();
    } else {
        UbuntuOneJob *job = new UbuntuOneJob(this);
        job->initializeToken(mCustomerSecret, mToken, mCustomerKey, mTokenSecret);
        connect(job, SIGNAL(deleteFileDone(QString)), SLOT(slotDeleteFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->deleteFile(filename);
    }

}

void UbuntuoneStorageService::storageServicedeleteFolder(const QString &foldername)
{
    if (mTokenSecret.isEmpty()) {
        mNextAction->setNextActionType(DeleteFile);
        mNextAction->setNextActionFolder(foldername);
        storageServiceauthentication();
    } else {
        UbuntuOneJob *job = new UbuntuOneJob(this);
        job->initializeToken(mCustomerSecret, mToken, mCustomerKey, mTokenSecret);
        connect(job, SIGNAL(deleteFolderDone(QString)), SLOT(slotDeleteFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->deleteFolder(foldername);
    }

}

void UbuntuoneStorageService::storageServiceRenameFolder(const QString &source, const QString &destination)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(RenameFolder);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        UbuntuOneJob *job = new UbuntuOneJob(this);
        job->initializeToken(mCustomerSecret, mToken, mCustomerKey, mTokenSecret);
        connect(job, SIGNAL(renameFolderDone(QString)), SLOT(slotRenameFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->renameFolder(source, destination);
    }
}

void UbuntuoneStorageService::storageServiceRenameFile(const QString &source, const QString &destination)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(RenameFile);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        UbuntuOneJob *job = new UbuntuOneJob(this);
        job->initializeToken(mCustomerSecret, mToken, mCustomerKey, mTokenSecret);
        connect(job, SIGNAL(renameFileDone(QString)), SLOT(slotRenameFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->renameFile(source, destination);
    }
}

void UbuntuoneStorageService::storageServiceMoveFolder(const QString &source, const QString &destination)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(MoveFolder);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        UbuntuOneJob *job = new UbuntuOneJob(this);
        job->initializeToken(mCustomerSecret, mToken, mCustomerKey, mTokenSecret);
        connect(job, SIGNAL(moveFolderDone(QString)), SLOT(slotRenameFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->moveFolder(source, destination);
    }
}

void UbuntuoneStorageService::storageServiceMoveFile(const QString &source, const QString &destination)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(MoveFolder);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        UbuntuOneJob *job = new UbuntuOneJob(this);
        job->initializeToken(mCustomerSecret, mToken, mCustomerKey, mTokenSecret);
        connect(job, SIGNAL(moveFileDone(QString)), SLOT(slotRenameFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->moveFile(source, destination);
    }
}

void UbuntuoneStorageService::storageServiceCopyFile(const QString &source, const QString &destination)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(CopyFile);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        UbuntuOneJob *job = new UbuntuOneJob(this);
        job->initializeToken(mCustomerSecret, mToken, mCustomerKey, mTokenSecret);
        connect(job, SIGNAL(copyFileDone(QString)), SLOT(slotCopyFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->copyFile(source, destination);
    }
}

void UbuntuoneStorageService::storageServiceCopyFolder(const QString &source, const QString &destination)
{
    if (mToken.isEmpty()) {
        mNextAction->setNextActionType(CopyFolder);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        UbuntuOneJob *job = new UbuntuOneJob(this);
        job->initializeToken(mCustomerSecret, mToken, mCustomerKey, mTokenSecret);
        connect(job, SIGNAL(copyFolderDone(QString)), SLOT(slotCopyFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->copyFolder(source, destination);
    }
}

StorageServiceAbstract::Capabilities UbuntuoneStorageService::capabilities() const
{
    return serviceCapabilities();
}

QString UbuntuoneStorageService::fillListWidget(StorageServiceTreeWidget *listWidget, const QString &data)
{
    listWidget->clear();
    listWidget->createMoveUpItem();
    QJson::Parser parser;
    bool ok;
    QString parentFolder;
    QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    //qDebug()<<" info "<<info;
    if (info.contains(QLatin1String("children"))) {
        const QVariantList lst = info.value(QLatin1String("children")).toList();
        Q_FOREACH (const QVariant &v, lst) {
            const QVariantMap map = v.toMap();
            if (map.contains(QLatin1String("kind"))) {
                const QString kind = map.value(QLatin1String("kind")).toString();
                StorageServiceTreeWidgetItem *item = 0;
                if (kind == QLatin1String("directory")) {
                    const QString path = map.value(QLatin1String("path")).toString();
                    item = listWidget->addFolder(path, path);
                } else if (kind == QLatin1String("file")) {
                    const QString path = map.value(QLatin1String("path")).toString();
                    item = listWidget->addFile(path, path);
                    if (map.contains(QLatin1String("size"))) {
                        item->setSize(map.value(QLatin1String("size")).toULongLong());
                    }
                    if (map.contains(QLatin1String("when_created"))) {
                        const QDateTime t = QDateTime::fromString(map.value(QLatin1String("when_created")).toString(), QLatin1String("yyyy-MM-ddThh:mm:ssZ"));
                        item->setDateCreated(t);
                    }
                    if (map.contains(QLatin1String("when_changed"))) {
                        const QDateTime t = QDateTime::fromString(map.value(QLatin1String("when_changed")).toString(), QLatin1String("yyyy-MM-ddThh:mm:ssZ"));
                        item->setLastModification(t);
                    }
                } else {
                    qDebug() <<" kind unknown "<<kind;
                }
                if (item) {
                    item->setStoreInfo(map);
                }
            }
        }
    }
    return parentFolder;
}

QString UbuntuoneStorageService::itemInformation(const QVariantMap &variantMap)
{
    qDebug()<<" variantMap "<<variantMap;
    return QString();
}

QString UbuntuoneStorageService::storageServiceName() const
{
    return serviceName();
}

KIcon UbuntuoneStorageService::icon() const
{
    return KIcon();
}

