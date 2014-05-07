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
#include "storageservice/widgets/storageservicetreewidget.h"
#include "storageservice/storageservicemanager.h"
#include "storageservice/settings/storageservicesettings.h"
#include "pimcommon/storageservice/storageservicejobconfig.h"
#include "pimcommon/storageservice/utils/storageserviceutils.h"

#include "yousenditutil.h"
#include "yousenditjob.h"

#include <kwallet.h>

#include <KLocalizedString>
#include <KGlobal>
#include <KLocale>

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

void YouSendItStorageService::shutdownService()
{
    mToken.clear();
    mUsername.clear();
    mPassword.clear();
}

bool YouSendItStorageService::hasValidSettings() const
{
    return !PimCommon::StorageServiceJobConfig::self()->youSendItApiKey().isEmpty();
}

bool YouSendItStorageService::needAuthenticate()
{
    if (mNeedToReadConfigFirst)
        readConfig();
    return (mToken.isEmpty() || mUsername.isEmpty() || mPassword.isEmpty());
}

void YouSendItStorageService::readConfig()
{
    if (StorageServiceSettings::self()->createDefaultFolder()) {
        KWallet::Wallet *wallet = StorageServiceSettings::self()->wallet();
        if (wallet) {
            QStringList lst = wallet->entryList();
            if (lst.contains(storageServiceName())) {
                QMap<QString, QString> map;
                wallet->readMap( storageServiceName(), map );
                if (map.contains(QLatin1String("Username"))) {
                    mUsername = map.value(QLatin1String("Username"));
                }
                if (map.contains(QLatin1String("Token"))) {
                    mToken = map.value(QLatin1String("Token"));
                }
                if (map.contains(QLatin1String("Password"))) {
                    mPassword = map.value(QLatin1String("Password"));
                }
            }
            mNeedToReadConfigFirst = false;
        }
    }
}

void YouSendItStorageService::removeConfig()
{
    if (StorageServiceSettings::self()->createDefaultFolder()) {
        const QString walletEntry = storageServiceName();
        KWallet::Wallet *wallet = StorageServiceSettings::self()->wallet();
        if (wallet)
            wallet->removeEntry(walletEntry);
    }
}

void YouSendItStorageService::storageServiceauthentication()
{
    YouSendItJob *job = new YouSendItJob(this);
    connect(job, SIGNAL(authorizationDone(QString,QString,QString)), this, SLOT(slotAuthorizationDone(QString,QString,QString)));
    connect(job, SIGNAL(authorizationFailed(QString)), this, SLOT(slotAuthorizationFailed(QString)));
    connect(job, SIGNAL(actionFailed(QString)), this, SLOT(slotActionFailed(QString)));
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
    if (StorageServiceSettings::self()->createDefaultFolder()) {
        const QString walletEntry = storageServiceName();
        KWallet::Wallet *wallet = StorageServiceSettings::self()->wallet();
        if (wallet) {
            QMap<QString, QString> map;
            map[QLatin1String( "Username" )] = username;
            map[QLatin1String( "Token" )] = token;
            map[QLatin1String( "Password" )] = mPassword;
            wallet->writeMap( walletEntry, map);
        }
    }
    emitAuthentificationDone();
}

void YouSendItStorageService::storageServicelistFolder(const QString &folder)
{
    if (needAuthenticate()) {
        mNextAction->setNextActionType(ListFolderAction);
        mNextAction->setNextActionFolder(folder);
        storageServiceauthentication();
    } else {
        YouSendItJob *job = new YouSendItJob(this);
        job->initializeToken(mPassword, mUsername, mToken);
        connect(job, SIGNAL(listFolderDone(QVariant)), this, SLOT(slotListFolderDone(QVariant)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->listFolder(folder);
    }
}

void YouSendItStorageService::storageServicecreateFolder(const QString &name, const QString &destination)
{
    if (needAuthenticate()) {
        mNextAction->setNextActionName(name);
        mNextAction->setNextActionFolder(destination);
        mNextAction->setNextActionType(CreateFolderAction);
        storageServiceauthentication();
    } else {
        YouSendItJob *job = new YouSendItJob(this);
        job->initializeToken(mPassword, mUsername, mToken);
        connect(job, SIGNAL(createFolderDone(QString)), this, SLOT(slotCreateFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->createFolder(name, destination);
    }
}

void YouSendItStorageService::storageServiceaccountInfo()
{
    if (needAuthenticate()) {
        mNextAction->setNextActionType(AccountInfoAction);
        storageServiceauthentication();
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

void YouSendItStorageService::storageServiceuploadFile(const QString &filename, const QString &uploadAsName, const QString &destination)
{
    if (needAuthenticate()) {
        mNextAction->setNextActionName(filename);
        mNextAction->setNextActionFolder(destination);
        mNextAction->setUploadAsName(uploadAsName);
        mNextAction->setNextActionType(UploadFileAction);
        storageServiceauthentication();
    } else {
        YouSendItJob *job = new YouSendItJob(this);
        job->initializeToken(mPassword, mUsername, mToken);
        connect(job, SIGNAL(uploadFileDone(QString)), this, SLOT(slotUploadFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        connect(job, SIGNAL(shareLinkDone(QString)), this, SLOT(slotShareLinkDone(QString)));
        connect(job, SIGNAL(uploadDownloadFileProgress(qint64,qint64)), SLOT(slotuploadDownloadFileProgress(qint64,qint64)));
        connect(job, SIGNAL(uploadFileFailed(QString)), this, SLOT(slotUploadFileFailed(QString)));
        mUploadReply = job->uploadFile(filename, uploadAsName, destination);
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
    cap |= RenameFileCapabilitity;
    cap |= MoveFileCapability;
    cap |= MoveFolderCapability;

    //Can not be implemented.
    //cap |= CopyFileCapability;
    //cap |= CopyFolderCapability;


    return cap;
}

void YouSendItStorageService::storageServiceShareLink(const QString &root, const QString &path)
{
    if (needAuthenticate()) {
        mNextAction->setRootPath(root);
        mNextAction->setPath(path);
        mNextAction->setNextActionType(ShareLinkAction);
        storageServiceauthentication();
    } else {
        YouSendItJob *job = new YouSendItJob(this);
        job->initializeToken(mPassword, mUsername, mToken);
        connect(job, SIGNAL(shareLinkDone(QString)), this, SLOT(slotShareLinkDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->shareLink(root, path);
    }
}

void YouSendItStorageService::storageServicedownloadFile(const QString &name, const QString &fileId, const QString &destination)
{
    if (needAuthenticate()) {
        mNextAction->setNextActionName(name);
        mNextAction->setNextActionType(DownLoadFileAction);
        mNextAction->setDownloadDestination(destination);
        mNextAction->setFileId(fileId);
        storageServiceauthentication();
    } else {
        YouSendItJob *job = new YouSendItJob(this);
        job->initializeToken(mPassword, mUsername, mToken);
        connect(job, SIGNAL(downLoadFileDone(QString)), this, SLOT(slotDownLoadFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        connect(job, SIGNAL(downLoadFileFailed(QString)), this, SLOT(slotDownLoadFileFailed(QString)));
        connect(job, SIGNAL(uploadDownloadFileProgress(qint64,qint64)), SLOT(slotuploadDownloadFileProgress(qint64,qint64)));
        mDownloadReply = job->downloadFile(name, fileId, destination);
    }
}

void YouSendItStorageService::storageServicecreateServiceFolder()
{
    if (needAuthenticate()) {
        mNextAction->setNextActionType(CreateServiceFolderAction);
        storageServiceauthentication();
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
    if (needAuthenticate()) {
        mNextAction->setNextActionName(filename);
        mNextAction->setNextActionType(DeleteFileAction);
        storageServiceauthentication();
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
    if (needAuthenticate()) {
        mNextAction->setNextActionName(foldername);
        mNextAction->setNextActionType(DeleteFolderAction);
        storageServiceauthentication();
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
    if (needAuthenticate()) {
        mNextAction->setNextActionType(RenameFolderAction);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
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
    if (needAuthenticate()) {
        mNextAction->setNextActionType(RenameFileAction);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
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
    if (needAuthenticate()) {
        mNextAction->setNextActionType(MoveFolderAction);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
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
    if (needAuthenticate()) {
        mNextAction->setNextActionType(MoveFileAction);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
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
    if (needAuthenticate()) {
        mNextAction->setNextActionType(CopyFileAction);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        YouSendItJob *job = new YouSendItJob(this);
        job->initializeToken(mPassword, mUsername, mToken);
        connect(job, SIGNAL(copyFileDone(QString)), SLOT(slotCopyFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->copyFile(source, destination);
    }
}

void YouSendItStorageService::storageServiceCopyFolder(const QString &source, const QString &destination)
{
    if (needAuthenticate()) {
        mNextAction->setNextActionType(CopyFolderAction);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        YouSendItJob *job = new YouSendItJob(this);
        job->initializeToken(mPassword, mUsername, mToken);
        connect(job, SIGNAL(copyFolderDone(QString)), SLOT(slotCopyFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->copyFolder(source, destination);
    }
}

QMap<QString, QString> YouSendItStorageService::itemInformation(const QVariantMap &variantMap)
{
    QMap<QString, QString> information;
    qDebug()<<" variantMap "<<variantMap;
    bool folder = false;
    if (variantMap.contains(QLatin1String("name"))) {
        information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::Name), variantMap.value(QLatin1String("name")).toString());
    }
    if (variantMap.contains(QLatin1String("updatedOn"))) {
        const QString t = variantMap.value(QLatin1String("updatedOn")).toString();
        information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::LastModified), KGlobal::locale()->formatDateTime(YouSendItUtil::convertToDateTime(t,true)));
        folder = true;
    }
    information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::Type), folder ? i18n("Directory") : i18n("File"));
    if (variantMap.contains(QLatin1String("createdOn"))) {
        const QString t = variantMap.value(QLatin1String("createdOn")).toString();
        information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::Created), KGlobal::locale()->formatDateTime(YouSendItUtil::convertToDateTime(t,folder ? true : false)));
    }
    if (variantMap.contains(QLatin1String("lastUpdatedOn"))) {
        const QString t = variantMap.value(QLatin1String("lastUpdatedOn")).toString();
        information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::LastModified), KGlobal::locale()->formatDateTime(YouSendItUtil::convertToDateTime(t)));
    }
    if (!folder && variantMap.contains(QLatin1String("size"))) {
        information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::Size), KGlobal::locale()->formatByteSize(variantMap.value(QLatin1String("size")).toULongLong()));
    }
    if (variantMap.contains(QLatin1String("writeable"))) {
        information.insert(i18n("writable:"), (variantMap.value(QLatin1String("writeable")).toString() == QLatin1String("true")) ? i18n("Yes") : i18n("No"));
    }
    return information;
}

QString YouSendItStorageService::fileIdentifier(const QVariantMap &variantMap)
{
    //TODO
    return QString();
}

QString YouSendItStorageService::fileShareRoot(const QVariantMap &variantMap)
{
    return QString();
}

StorageServiceAbstract::Capabilities YouSendItStorageService::capabilities() const
{
    return serviceCapabilities();
}

QString YouSendItStorageService::fillListWidget(StorageServiceTreeWidget *listWidget, const QVariant &data, const QString &currentFolder)
{
    Q_UNUSED(currentFolder);
    listWidget->clear();
    listWidget->createMoveUpItem();
    QJson::Parser parser;
    bool ok;
    const QMap<QString, QVariant> info = parser.parse(data.toString().toUtf8(), &ok).toMap();
    //qDebug()<<" INFO "<<info;
    if (info.contains(QLatin1String("folders"))) {
        const QVariantMap mapFolder = info.value(QLatin1String("folders")).toMap();
        const QVariantList folders = mapFolder.value(QLatin1String("folder")).toList();
        Q_FOREACH (const QVariant &v, folders) {
            const QVariantMap map = v.toMap();
            //qDebug()<<" folder map"<<map;
            if (map.contains(QLatin1String("name"))) {
                const QString name = map.value(QLatin1String("name")).toString();
                const QString folderId = map.value(QLatin1String("id")).toString();
                StorageServiceTreeWidgetItem *item = listWidget->addFolder(name, folderId);
                if (map.contains(QLatin1String("createdOn"))) {
                    const QString t = map.value(QLatin1String("createdOn")).toString();
                    item->setDateCreated(YouSendItUtil::convertToDateTime(t,true));
                }
                if (map.contains(QLatin1String("updatedOn"))) {
                    const QString t = map.value(QLatin1String("updatedOn")).toString();
                    item->setLastModification(YouSendItUtil::convertToDateTime(t,true));
                }
                item->setStoreInfo(map);
            }
        }
        const QVariantMap mapFiles = info.value(QLatin1String("files")).toMap();
        const QVariantList files = mapFiles.value(QLatin1String("file")).toList();
        Q_FOREACH (const QVariant &v, files) {
            const QVariantMap map = v.toMap();
            //qDebug()<<" file map !"<<map;
            if (map.contains(QLatin1String("name"))) {
                const QString name = map.value(QLatin1String("name")).toString();
                qDebug()<<" name !"<<name;
                const QString fileId = map.value(QLatin1String("id")).toString();
                StorageServiceTreeWidgetItem *item = listWidget->addFile(name, fileId);
                if (map.contains(QLatin1String("size"))) {
                    //qDebug()<<" size "<<map.value(QLatin1String("size"));
                    const qulonglong size = map.value(QLatin1String("size")).toULongLong();
                    item->setSize(size);
                }
                if (map.contains(QLatin1String("createdOn"))) {
                    const QString t = map.value(QLatin1String("createdOn")).toString();
                    item->setDateCreated(YouSendItUtil::convertToDateTime(t));
                }
                if (map.contains(QLatin1String("lastUpdatedOn"))) {
                    const QString t = map.value(QLatin1String("lastUpdatedOn")).toString();
                    item->setLastModification(YouSendItUtil::convertToDateTime(t));
                }
                item->setStoreInfo(map);
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

#include "moc_yousenditstorageservice.cpp"
