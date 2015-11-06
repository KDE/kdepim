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

#include "boxstorageservice.h"
#include "storageservice/widgets/storageservicetreewidget.h"
#include "storageservice/widgets/storageservicetreewidgetitem.h"
#include "storageservice/storageservicemanager.h"
#include "boxutil.h"
#include "boxjob.h"
#include "storageservice/settings/storageservicesettings.h"
#include "storageservice/storageservicejobconfig.h"
#include "storageservice/utils/storageserviceutils.h"

#include <QJsonDocument>

#include <kwallet.h>

#include <KLocalizedString>

#include "pimcommon_debug.h"
#include <KFormat>
#include <QLocale>

using namespace PimCommon;

BoxStorageService::BoxStorageService(QObject *parent)
    : PimCommon::StorageServiceAbstract(parent)
{
    readConfig();
}

BoxStorageService::~BoxStorageService()
{
}

void BoxStorageService::shutdownService()
{
    mToken.clear();
    mRefreshToken.clear();
    mExpireDateTime = QDateTime();
}

bool BoxStorageService::hasValidSettings() const
{
    return (!PimCommon::StorageServiceJobConfig::self()->oauth2RedirectUrl().isEmpty() &&
            !PimCommon::StorageServiceJobConfig::self()->boxClientId().isEmpty() &&
            !PimCommon::StorageServiceJobConfig::self()->boxClientSecret().isEmpty() &&
            !PimCommon::StorageServiceJobConfig::self()->oauth2RedirectUrl().isEmpty());
}

void BoxStorageService::readConfig()
{
    mExpireDateTime = QDateTime();
    if (StorageServiceSettings::self()->createDefaultFolder()) {
        KWallet::Wallet *wallet = StorageServiceSettings::self()->wallet();
        if (wallet) {
            QStringList lst = wallet->entryList();
            if (lst.contains(storageServiceName())) {
                QMap<QString, QString> map;
                wallet->readMap(storageServiceName(), map);
                if (map.contains(QStringLiteral("Refresh Token"))) {
                    mRefreshToken = map.value(QStringLiteral("Refresh Token"));
                }
                if (map.contains(QStringLiteral("Token"))) {
                    mToken = map.value(QStringLiteral("Token"));
                }
                if (map.contains(QStringLiteral("Expire Time"))) {
                    mExpireDateTime = QDateTime::fromString(map.value(QStringLiteral("Expire Time")));
                }
            }
        }
        mNeedToReadConfigFirst = false;
    }
}

void BoxStorageService::removeConfig()
{
    if (StorageServiceSettings::self()->createDefaultFolder()) {
        const QString walletEntry = storageServiceName();
        KWallet::Wallet *wallet = StorageServiceSettings::self()->wallet();
        if (wallet) {
            wallet->removeEntry(walletEntry);
        }
    }
}

void BoxStorageService::storageServiceauthentication()
{
    BoxJob *job = new BoxJob(this);
    connect(job, &BoxJob::authorizationDone, this, &BoxStorageService::slotAuthorizationDone);
    connect(job, &BoxJob::authorizationFailed, this, &BoxStorageService::slotAuthorizationFailed);
    connect(job, &BoxJob::actionFailed, this, &BoxStorageService::slotActionFailed);
    job->requestTokenAccess();
}

void BoxStorageService::slotAuthorizationFailed(const QString &errorMessage)
{
    mRefreshToken.clear();
    mToken.clear();
    emitAuthentificationFailed(errorMessage);
}

void BoxStorageService::slotAuthorizationDone(const QString &refreshToken, const QString &token, qint64 expireTime)
{
    mRefreshToken = refreshToken;
    mToken = token;
    mExpireDateTime = QDateTime::currentDateTime().addSecs(expireTime);

    if (StorageServiceSettings::self()->createDefaultFolder()) {
        const QString walletEntry = storageServiceName();
        KWallet::Wallet *wallet = StorageServiceSettings::self()->wallet();
        if (wallet) {
            QMap<QString, QString> map;
            map[QStringLiteral("Refresh Token")] = mRefreshToken;
            map[QStringLiteral("Token")] = mToken;
            map[QStringLiteral("Expire Time")] = mExpireDateTime.toString();
            wallet->writeMap(walletEntry, map);
        }
    }
    emitAuthentificationDone();
}

bool BoxStorageService::needToRefreshToken()
{
    if (mNeedToReadConfigFirst) {
        readConfig();
    }
    if (mExpireDateTime < QDateTime::currentDateTime()) {
        return true;
    } else {
        return false;
    }
}

void BoxStorageService::refreshToken()
{
    BoxJob *job = new BoxJob(this);
    job->initializeToken(mRefreshToken, mToken);
    connect(job, &BoxJob::authorizationDone, this, &BoxStorageService::slotAuthorizationDone);
    connect(job, &BoxJob::authorizationFailed, this, &BoxStorageService::slotAuthorizationFailed);
    job->refreshToken();
}

void BoxStorageService::storageServiceShareLink(const QString &root, const QString &path)
{
    const bool needRefresh = needToRefreshToken();
    if (mToken.isEmpty() || needRefresh) {
        mNextAction->setNextActionType(ShareLinkAction);
        mNextAction->setPath(path);
        mNextAction->setRootPath(root);
        if (mToken.isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    }  else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, &BoxJob::shareLinkDone, this, &BoxStorageService::slotShareLinkDone);
        connect(job, &BoxJob::actionFailed, this, &BoxStorageService::slotActionFailed);
        job->shareLink(root, path);
    }
}

void BoxStorageService::storageServicedownloadFile(const QString &name, const QString &fileId, const QString &destination)
{
    const bool needRefresh = needToRefreshToken();
    if (mToken.isEmpty() || needRefresh) {
        mNextAction->setNextActionType(DownLoadFileAction);
        mNextAction->setNextActionName(name);
        mNextAction->setDownloadDestination(destination);
        mNextAction->setFileId(fileId);
        if (mToken.isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, &BoxJob::downLoadFileDone, this, &BoxStorageService::slotDownLoadFileDone);
        connect(job, &BoxJob::actionFailed, this, &BoxStorageService::slotActionFailed);
        connect(job, &BoxJob::downLoadFileFailed, this, &BoxStorageService::slotDownLoadFileFailed);
        connect(job, &BoxJob::uploadDownloadFileProgress, this, &BoxStorageService::slotuploadDownloadFileProgress);
        mDownloadReply = job->downloadFile(name, fileId, destination);
    }
}

void BoxStorageService::storageServicedeleteFile(const QString &filename)
{
    const bool needRefresh = needToRefreshToken();
    if (mToken.isEmpty() || needRefresh) {
        mNextAction->setNextActionType(DeleteFileAction);
        mNextAction->setNextActionName(filename);
        if (mToken.isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, &BoxJob::deleteFileDone, this, &BoxStorageService::slotDeleteFileDone);
        connect(job, &BoxJob::actionFailed, this, &BoxStorageService::slotActionFailed);
        job->deleteFile(filename);
    }
}

void BoxStorageService::storageServicedeleteFolder(const QString &foldername)
{
    const bool needRefresh = needToRefreshToken();
    if (mToken.isEmpty() || needRefresh) {
        mNextAction->setNextActionType(DeleteFolderAction);
        mNextAction->setNextActionFolder(foldername);
        if (mToken.isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, &BoxJob::deleteFolderDone, this, &BoxStorageService::slotDeleteFolderDone);
        connect(job, &BoxJob::actionFailed, this, &BoxStorageService::slotActionFailed);
        job->deleteFolder(foldername);
    }
}

void BoxStorageService::storageServiceRenameFolder(const QString &source, const QString &destination)
{
    const bool needRefresh = needToRefreshToken();
    if (mToken.isEmpty() || needRefresh) {
        mNextAction->setNextActionType(RenameFolderAction);
        mNextAction->setRenameFolder(source, destination);
        if (mToken.isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, &BoxJob::renameFolderDone, this, &BoxStorageService::slotRenameFolderDone);
        connect(job, &BoxJob::actionFailed, this, &BoxStorageService::slotActionFailed);
        job->renameFolder(source, destination);
    }
}

void BoxStorageService::storageServiceRenameFile(const QString &source, const QString &destination)
{
    const bool needRefresh = needToRefreshToken();
    if (mToken.isEmpty() || needRefresh) {
        mNextAction->setNextActionType(RenameFileAction);
        mNextAction->setRenameFolder(source, destination);
        if (mToken.isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, &BoxJob::renameFileDone, this, &BoxStorageService::slotRenameFileDone);
        connect(job, &BoxJob::actionFailed, this, &BoxStorageService::slotActionFailed);
        job->renameFile(source, destination);
    }
}

void BoxStorageService::storageServiceMoveFolder(const QString &source, const QString &destination)
{
    const bool needRefresh = needToRefreshToken();
    if (mToken.isEmpty() || needRefresh) {
        mNextAction->setNextActionType(MoveFolderAction);
        mNextAction->setRenameFolder(source, destination);
        if (mToken.isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, &BoxJob::moveFolderDone, this, &BoxStorageService::slotMoveFolderDone);
        connect(job, &BoxJob::actionFailed, this, &BoxStorageService::slotActionFailed);
        job->moveFolder(source, destination);
    }
}

void BoxStorageService::storageServiceMoveFile(const QString &source, const QString &destination)
{
    const bool needRefresh = needToRefreshToken();
    if (mToken.isEmpty() || needRefresh) {
        mNextAction->setNextActionType(MoveFileAction);
        mNextAction->setRenameFolder(source, destination);
        if (mToken.isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, &BoxJob::moveFileDone, this, &BoxStorageService::slotMoveFileDone);
        connect(job, &BoxJob::actionFailed, this, &BoxStorageService::slotActionFailed);
        job->moveFile(source, destination);
    }
}

void BoxStorageService::storageServiceCopyFile(const QString &source, const QString &destination)
{
    const bool needRefresh = needToRefreshToken();
    if (mToken.isEmpty() || needRefresh) {
        mNextAction->setNextActionType(CopyFileAction);
        mNextAction->setRenameFolder(source, destination);
        if (mToken.isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, &BoxJob::copyFileDone, this, &BoxStorageService::slotCopyFileDone);
        connect(job, &BoxJob::actionFailed, this, &BoxStorageService::slotActionFailed);
        job->copyFile(source, destination);
    }
}

void BoxStorageService::storageServiceCopyFolder(const QString &source, const QString &destination)
{
    const bool needRefresh = needToRefreshToken();
    if (mToken.isEmpty() || needRefresh) {
        mNextAction->setNextActionType(CopyFolderAction);
        mNextAction->setRenameFolder(source, destination);
        if (mToken.isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, &BoxJob::copyFolderDone, this, &BoxStorageService::slotCopyFolderDone);
        connect(job, &BoxJob::actionFailed, this, &BoxStorageService::slotActionFailed);
        job->copyFolder(source, destination);
    }
}

void BoxStorageService::storageServicelistFolder(const QString &folder)
{
    const bool needRefresh = needToRefreshToken();
    if (mToken.isEmpty() || needRefresh) {
        mNextAction->setNextActionType(ListFolderAction);
        mNextAction->setNextActionFolder(folder);
        if (mToken.isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, &BoxJob::listFolderDone, this, &BoxStorageService::slotListFolderDone);
        connect(job, &BoxJob::actionFailed, this, &BoxStorageService::slotActionFailed);
        job->listFolder(folder);
    }
}

void BoxStorageService::storageServicecreateFolder(const QString &name, const QString &destination)
{
    const bool needRefresh = needToRefreshToken();
    if (mToken.isEmpty() || needRefresh) {
        mNextAction->setNextActionType(CreateFolderAction);
        mNextAction->setNextActionName(name);
        mNextAction->setNextActionFolder(destination);
        if (mToken.isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, &BoxJob::createFolderDone, this, &BoxStorageService::slotCreateFolderDone);
        connect(job, &BoxJob::actionFailed, this, &BoxStorageService::slotActionFailed);
        job->createFolder(name, destination);
    }
}

void BoxStorageService::storageServiceaccountInfo()
{
    const bool needRefresh = needToRefreshToken();
    if (mToken.isEmpty() || needRefresh) {
        mNextAction->setNextActionType(AccountInfoAction);
        if (mToken.isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, &BoxJob::accountInfoDone, this, &BoxStorageService::slotAccountInfoDone);
        connect(job, &BoxJob::actionFailed, this, &BoxStorageService::slotActionFailed);
        job->accountInfo();
    }
}

QString BoxStorageService::name()
{
    return i18n("Box");
}

void BoxStorageService::storageServiceuploadFile(const QString &filename, const QString &uploadAsName, const QString &destination)
{
    const bool needRefresh = needToRefreshToken();
    if (mToken.isEmpty() || needRefresh) {
        mNextAction->setNextActionType(UploadFileAction);
        mNextAction->setNextActionName(filename);
        mNextAction->setNextActionFolder(destination);
        mNextAction->setUploadAsName(uploadAsName);
        if (mToken.isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, &BoxJob::uploadFileDone, this, &BoxStorageService::slotUploadFileDone);
        connect(job, &BoxJob::actionFailed, this, &BoxStorageService::slotActionFailed);
        connect(job, &BoxJob::shareLinkDone, this, &BoxStorageService::slotShareLinkDone);
        connect(job, &BoxJob::uploadDownloadFileProgress, this, &BoxStorageService::slotuploadDownloadFileProgress);
        connect(job, &BoxJob::uploadFileFailed, this, &BoxStorageService::slotUploadFileFailed);
        mUploadReply = job->uploadFile(filename, uploadAsName, destination);
    }
}

QString BoxStorageService::description()
{
    return i18n("Box.com is a file hosting that offers cloud storage, file synchronization, and client software.");
}

QUrl BoxStorageService::serviceUrl()
{
    return QUrl(QStringLiteral("https://app.box.com/"));
}

QString BoxStorageService::serviceName()
{
    return QStringLiteral("box");
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
    cap |= DownloadFileCapability;
    cap |= CreateFolderCapability;
    cap |= DeleteFolderCapability;
    cap |= ListFolderCapability;
    cap |= DeleteFileCapability;
    cap |= ShareLinkCapability;
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

QIcon BoxStorageService::icon() const
{
    return QIcon();
}

StorageServiceAbstract::Capabilities BoxStorageService::capabilities() const
{
    return serviceCapabilities();
}

void BoxStorageService::storageServicecreateServiceFolder()
{
    const bool needRefresh = needToRefreshToken();
    if (mToken.isEmpty() || needRefresh) {
        mNextAction->setNextActionType(CreateServiceFolderAction);
        if (mToken.isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, &BoxJob::createFolderDone, this, &BoxStorageService::slotCreateFolderDone);
        connect(job, &BoxJob::actionFailed, this, &BoxStorageService::slotActionFailed);
        job->createServiceFolder();
    }
}

QString BoxStorageService::fillListWidget(StorageServiceTreeWidget *listWidget, const QVariant &data, const QString &currentFolder)
{
    Q_UNUSED(currentFolder);
    listWidget->clear();
    QJsonParseError error;
    QString parentId;
    const QJsonDocument json = QJsonDocument::fromJson(data.toString().toUtf8(), &error);
    if (error.error != QJsonParseError::NoError || json.isNull()) {
        return parentId;
    }

    listWidget->createMoveUpItem();
    const QMap<QString, QVariant> info = json.toVariant().toMap();

    if (info.contains(QStringLiteral("entries"))) {
        const QVariantList entries = info.value(QStringLiteral("entries")).toList();
        Q_FOREACH (const QVariant &v, entries) {
            QVariantMap mapEntries = v.toMap();
            if (mapEntries.contains(QStringLiteral("type"))) {
                const QString type = mapEntries.value(QStringLiteral("type")).toString();
                const QString name = mapEntries.value(QStringLiteral("name")).toString();
                const QString id = mapEntries.value(QStringLiteral("id")).toString();
                StorageServiceTreeWidgetItem *item = Q_NULLPTR;
                if (type == QLatin1String("folder")) {
                    item = listWidget->addFolder(name, id);
                } else if (type == QLatin1String("file")) {
                    item = listWidget->addFile(name, id);
                }
                if (item) {
                    if (mapEntries.contains(QStringLiteral("size"))) {
                        item->setSize(mapEntries.value(QStringLiteral("size")).toULongLong());
                    }
                    if (mapEntries.contains(QStringLiteral("created_at"))) {
                        item->setDateCreated(PimCommon::BoxUtil::convertToDateTime(mapEntries.value(QStringLiteral("created_at")).toString()));
                    }
                    if (mapEntries.contains(QStringLiteral("modified_at"))) {
                        item->setLastModification(PimCommon::BoxUtil::convertToDateTime(mapEntries.value(QStringLiteral("modified_at")).toString()));
                    }
                    item->setStoreInfo(mapEntries);
                }
            }
        }
    }
    //qCDebug(PIMCOMMON_LOG)<<" parentId"<<parentId;
    return parentId;
}

QMap<QString, QString> BoxStorageService::itemInformation(const QVariantMap &variantMap)
{
    qCDebug(PIMCOMMON_LOG) << " variantMap" << variantMap;
    QMap<QString, QString> information;
    if (variantMap.contains(QStringLiteral("type"))) {
        const QString type = variantMap.value(QStringLiteral("type")).toString();
        information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::Type), type == QLatin1String("folder") ? i18n("Folder") : i18n("File"));
    }
    if (variantMap.contains(QStringLiteral("name"))) {
        information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::Name), variantMap.value(QStringLiteral("name")).toString());
    }
    if (variantMap.contains(QStringLiteral("size"))) {
        information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::Size), KFormat().formatByteSize(variantMap.value(QStringLiteral("size")).toULongLong()));
    }
    if (variantMap.contains(QStringLiteral("created_at"))) {
        const QString tmp = variantMap.value(QStringLiteral("created_at")).toString();
        information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::Created), QLocale().toString((PimCommon::BoxUtil::convertToDateTime(tmp)), QLocale::ShortFormat));
    }
    if (variantMap.contains(QStringLiteral("modified_at"))) {
        const QString tmp = variantMap.value(QStringLiteral("modified_at")).toString();
        information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::LastModified), QLocale().toString((PimCommon::BoxUtil::convertToDateTime(tmp)), QLocale::ShortFormat));
    }
    return information;
}

QString BoxStorageService::fileIdentifier(const QVariantMap &variantMap)
{
    if (variantMap.contains(QStringLiteral("id"))) {
        return variantMap.value(QStringLiteral("id")).toString();
    }
    return QString();
}

QString BoxStorageService::fileShareRoot(const QVariantMap &variantMap)
{
    Q_UNUSED(variantMap);
    return QString();
}

QString BoxStorageService::disallowedSymbols() const
{
    return QLatin1String("[/:?*\\|]");
}

QString BoxStorageService::disallowedSymbolsStr() const
{
    return QStringLiteral("\\ / : ? * < > |");
}

