/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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
#include "storageservice/widgets/storageservicetreewidgetitem.h"
#include "storageservice/storageservicemanager.h"
#include "storageservice/settings/storageservicesettings.h"
#include "storageservice/storageservicejobconfig.h"
#include "storageservice/utils/storageserviceutils.h"

#include "yousenditutil.h"
#include "yousenditjob.h"

#include <kwallet.h>

#include <KLocalizedString>

#include "pimcommon_debug.h"
#include <KFormat>
#include <QLocale>

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
    if (mNeedToReadConfigFirst) {
        readConfig();
    }
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
                wallet->readMap(storageServiceName(), map);
                if (map.contains(QStringLiteral("Username"))) {
                    mUsername = map.value(QStringLiteral("Username"));
                }
                if (map.contains(QStringLiteral("Token"))) {
                    mToken = map.value(QStringLiteral("Token"));
                }
                if (map.contains(QStringLiteral("Password"))) {
                    mPassword = map.value(QStringLiteral("Password"));
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
        if (wallet) {
            wallet->removeEntry(walletEntry);
        }
    }
}

void YouSendItStorageService::storageServiceauthentication()
{
    YouSendItJob *job = new YouSendItJob(this);
    connect(job, &YouSendItJob::authorizationDone, this, &YouSendItStorageService::slotAuthorizationDone);
    connect(job, &YouSendItJob::authorizationFailed, this, &YouSendItStorageService::slotAuthorizationFailed);
    connect(job, &YouSendItJob::actionFailed, this, &YouSendItStorageService::slotActionFailed);
    job->requestTokenAccess();
}

void YouSendItStorageService::slotAuthorizationFailed(const QString &errorMessage)
{
    mUsername.clear();
    mPassword.clear();
    mToken.clear();
    emitAuthentificationFailed(errorMessage);
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
            map[QStringLiteral("Username")] = username;
            map[QStringLiteral("Token")] = token;
            map[QStringLiteral("Password")] = mPassword;
            wallet->writeMap(walletEntry, map);
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
        connect(job, &YouSendItJob::listFolderDone, this, &YouSendItStorageService::slotListFolderDone);
        connect(job, &YouSendItJob::actionFailed, this, &YouSendItStorageService::slotActionFailed);
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
        connect(job, &YouSendItJob::createFolderDone, this, &YouSendItStorageService::slotCreateFolderDone);
        connect(job, &YouSendItJob::actionFailed, this, &YouSendItStorageService::slotActionFailed);
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
        connect(job, &YouSendItJob::accountInfoDone, this, &YouSendItStorageService::slotAccountInfoDone);
        connect(job, &YouSendItJob::actionFailed, this, &YouSendItStorageService::slotActionFailed);
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
        connect(job, &YouSendItJob::uploadFileDone, this, &YouSendItStorageService::slotUploadFileDone);
        connect(job, &YouSendItJob::actionFailed, this, &YouSendItStorageService::slotActionFailed);
        connect(job, &YouSendItJob::shareLinkDone, this, &YouSendItStorageService::slotShareLinkDone);
        connect(job, &YouSendItJob::uploadDownloadFileProgress, this, &YouSendItStorageService::slotuploadDownloadFileProgress);
        connect(job, &YouSendItJob::uploadFileFailed, this, &YouSendItStorageService::slotUploadFileFailed);
        mUploadReply = job->uploadFile(filename, uploadAsName, destination);
    }
}

QString YouSendItStorageService::description()
{
    return i18n("YouSendIt is a file hosting that offers cloud storage, file synchronization, and client software.");
}

QUrl YouSendItStorageService::serviceUrl()
{
    return QUrl(QStringLiteral("https://www.yousendit.com/"));
}

QString YouSendItStorageService::serviceName()
{
    return QStringLiteral("yousendit");
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
        connect(job, &YouSendItJob::shareLinkDone, this, &YouSendItStorageService::slotShareLinkDone);
        connect(job, &YouSendItJob::actionFailed, this, &YouSendItStorageService::slotActionFailed);
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
        connect(job, &YouSendItJob::downLoadFileDone, this, &YouSendItStorageService::slotDownLoadFileDone);
        connect(job, &YouSendItJob::actionFailed, this, &YouSendItStorageService::slotActionFailed);
        connect(job, &YouSendItJob::downLoadFileFailed, this, &YouSendItStorageService::slotDownLoadFileFailed);
        connect(job, &YouSendItJob::uploadDownloadFileProgress, this, &YouSendItStorageService::slotuploadDownloadFileProgress);
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
        connect(job, &YouSendItJob::createFolderDone, this, &YouSendItStorageService::slotShareLinkDone);
        connect(job, &YouSendItJob::actionFailed, this, &YouSendItStorageService::slotActionFailed);
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
        connect(job, &YouSendItJob::deleteFileDone, this, &YouSendItStorageService::slotDeleteFileDone);
        connect(job, &YouSendItJob::actionFailed, this, &YouSendItStorageService::slotActionFailed);
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
        connect(job, &YouSendItJob::deleteFolderDone, this, &YouSendItStorageService::slotDeleteFolderDone);
        connect(job, &YouSendItJob::actionFailed, this, &YouSendItStorageService::slotActionFailed);
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
        connect(job, &YouSendItJob::renameFolderDone, this, &YouSendItStorageService::slotRenameFolderDone);
        connect(job, &YouSendItJob::actionFailed, this, &YouSendItStorageService::slotActionFailed);
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
        connect(job, &YouSendItJob::renameFileDone, this, &YouSendItStorageService::slotRenameFileDone);
        connect(job, &YouSendItJob::actionFailed, this, &YouSendItStorageService::slotActionFailed);
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
        connect(job, &YouSendItJob::moveFolderDone, this, &YouSendItStorageService::slotMoveFolderDone);
        connect(job, &YouSendItJob::actionFailed, this, &YouSendItStorageService::slotActionFailed);
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
        connect(job, &YouSendItJob::moveFileDone, this, &YouSendItStorageService::slotMoveFileDone);
        connect(job, &YouSendItJob::actionFailed, this, &YouSendItStorageService::slotActionFailed);
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
        connect(job, &YouSendItJob::copyFileDone, this, &YouSendItStorageService::slotCopyFileDone);
        connect(job, &YouSendItJob::actionFailed, this, &YouSendItStorageService::slotActionFailed);
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
        connect(job, &YouSendItJob::copyFolderDone, this, &YouSendItStorageService::slotCopyFolderDone);
        connect(job, &YouSendItJob::actionFailed, this, &YouSendItStorageService::slotActionFailed);
        job->copyFolder(source, destination);
    }
}

QMap<QString, QString> YouSendItStorageService::itemInformation(const QVariantMap &variantMap)
{
    QMap<QString, QString> information;
    qCDebug(PIMCOMMON_LOG) << " variantMap " << variantMap;
    bool folder = false;
    if (variantMap.contains(QStringLiteral("name"))) {
        information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::Name), variantMap.value(QStringLiteral("name")).toString());
    }
    if (variantMap.contains(QStringLiteral("updatedOn"))) {
        const QString t = variantMap.value(QStringLiteral("updatedOn")).toString();
        information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::LastModified), QLocale().toString(YouSendItUtil::convertToDateTime(t, true)));
        folder = true;
    }
    information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::Type), folder ? i18n("Directory") : i18n("File"));
    if (variantMap.contains(QStringLiteral("createdOn"))) {
        const QString t = variantMap.value(QStringLiteral("createdOn")).toString();
        information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::Created), QLocale().toString(YouSendItUtil::convertToDateTime(t, folder ? true : false)));
    }
    if (variantMap.contains(QStringLiteral("lastUpdatedOn"))) {
        const QString t = variantMap.value(QStringLiteral("lastUpdatedOn")).toString();
        information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::LastModified), QLocale().toString((YouSendItUtil::convertToDateTime(t)), QLocale::ShortFormat));
    }
    if (!folder && variantMap.contains(QStringLiteral("size"))) {
        information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::Size), KFormat().formatByteSize(variantMap.value(QStringLiteral("size")).toULongLong()));
    }
    if (variantMap.contains(QStringLiteral("writeable"))) {
        information.insert(i18n("writable:"), (variantMap.value(QStringLiteral("writeable")).toString() == QLatin1String("true")) ? i18n("Yes") : i18n("No"));
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
#if 0
    QJson::Parser parser;
    bool ok;
    const QMap<QString, QVariant> info = parser.parse(data.toString().toUtf8(), &ok).toMap();
    //qCDebug(PIMCOMMON_LOG)<<" INFO "<<info;
    if (info.contains(QStringLiteral("folders"))) {
        const QVariantMap mapFolder = info.value(QStringLiteral("folders")).toMap();
        const QVariantList folders = mapFolder.value(QStringLiteral("folder")).toList();
        Q_FOREACH (const QVariant &v, folders) {
            const QVariantMap map = v.toMap();
            //qCDebug(PIMCOMMON_LOG)<<" folder map"<<map;
            if (map.contains(QStringLiteral("name"))) {
                const QString name = map.value(QStringLiteral("name")).toString();
                const QString folderId = map.value(QStringLiteral("id")).toString();
                StorageServiceTreeWidgetItem *item = listWidget->addFolder(name, folderId);
                if (map.contains(QStringLiteral("createdOn"))) {
                    const QString t = map.value(QStringLiteral("createdOn")).toString();
                    item->setDateCreated(YouSendItUtil::convertToDateTime(t, true));
                }
                if (map.contains(QStringLiteral("updatedOn"))) {
                    const QString t = map.value(QStringLiteral("updatedOn")).toString();
                    item->setLastModification(YouSendItUtil::convertToDateTime(t, true));
                }
                item->setStoreInfo(map);
            }
        }
        const QVariantMap mapFiles = info.value(QStringLiteral("files")).toMap();
        const QVariantList files = mapFiles.value(QStringLiteral("file")).toList();
        Q_FOREACH (const QVariant &v, files) {
            const QVariantMap map = v.toMap();
            //qCDebug(PIMCOMMON_LOG)<<" file map !"<<map;
            if (map.contains(QStringLiteral("name"))) {
                const QString name = map.value(QStringLiteral("name")).toString();
                qCDebug(PIMCOMMON_LOG) << " name !" << name;
                const QString fileId = map.value(QStringLiteral("id")).toString();
                StorageServiceTreeWidgetItem *item = listWidget->addFile(name, fileId);
                if (map.contains(QStringLiteral("size"))) {
                    //qCDebug(PIMCOMMON_LOG)<<" size "<<map.value(QStringLiteral("size"));
                    const qulonglong size = map.value(QStringLiteral("size")).toULongLong();
                    item->setSize(size);
                }
                if (map.contains(QStringLiteral("createdOn"))) {
                    const QString t = map.value(QStringLiteral("createdOn")).toString();
                    item->setDateCreated(YouSendItUtil::convertToDateTime(t));
                }
                if (map.contains(QStringLiteral("lastUpdatedOn"))) {
                    const QString t = map.value(QStringLiteral("lastUpdatedOn")).toString();
                    item->setLastModification(YouSendItUtil::convertToDateTime(t));
                }
                item->setStoreInfo(map);
            }
        }
    }
#endif
    return QString();
}

QString YouSendItStorageService::storageServiceName() const
{
    return serviceName();
}

QIcon YouSendItStorageService::icon() const
{
    return QIcon();
}

