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

#include "hubicstorageservice.h"
#include "storageservice/widgets/storageservicetreewidget.h"
#include "storageservice/storageservicemanager.h"
#include "pimcommon/storageservice/storageservicejobconfig.h"
#include "hubicjob.h"
#include "pimcommon/storageservice/utils/storageserviceutils.h"

#include "storageservice/settings/storageservicesettings.h"

#include <kwallet.h>

#include <KLocalizedString>

using namespace PimCommon;

HubicStorageService::HubicStorageService(QObject *parent)
    : PimCommon::StorageServiceAbstract(parent)
{
    readConfig();
}

HubicStorageService::~HubicStorageService()
{
}

bool HubicStorageService::needToRefreshToken()
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

void HubicStorageService::shutdownService()
{
    mRefreshToken.clear();
    mToken.clear();
    mExpireDateTime = QDateTime();
}

bool HubicStorageService::hasValidSettings() const
{
    return (!PimCommon::StorageServiceJobConfig::self()->hubicScope().isEmpty() &&
            !PimCommon::StorageServiceJobConfig::self()->hubicClientId().isEmpty() &&
            !PimCommon::StorageServiceJobConfig::self()->hubicClientSecret().isEmpty() &&
            !PimCommon::StorageServiceJobConfig::self()->oauth2RedirectUrl().isEmpty());

}

void HubicStorageService::readConfig()
{
    mExpireDateTime = QDateTime();
    if (StorageServiceSettings::self()->createDefaultFolder()) {
        KWallet::Wallet *wallet = StorageServiceSettings::self()->wallet();
        if (wallet) {
            QStringList lst = wallet->entryList();
            if (lst.contains(storageServiceName())) {
                QMap<QString, QString> map;
                wallet->readMap(storageServiceName(), map);
                if (map.contains(QLatin1String("Refresh Token"))) {
                    mRefreshToken = map.value(QLatin1String("Refresh Token"));
                }
                if (map.contains(QLatin1String("Token"))) {
                    mToken = map.value(QLatin1String("Token"));
                }
                if (map.contains(QLatin1String("Expire Time"))) {
                    mExpireDateTime = QDateTime::fromString(map.value(QLatin1String("Expire Time")));
                }
            }
            mNeedToReadConfigFirst = false;
        }
    }
}

void HubicStorageService::removeConfig()
{
    if (StorageServiceSettings::self()->createDefaultFolder()) {
        const QString walletEntry = storageServiceName();
        KWallet::Wallet *wallet = StorageServiceSettings::self()->wallet();
        if (wallet) {
            wallet->removeEntry(walletEntry);
        }
    }
}

void HubicStorageService::storageServiceauthentication()
{
    HubicJob *job = new HubicJob(this);
    connect(job, &HubicJob::authorizationDone, this, &HubicStorageService::slotAuthorizationDone);
    connect(job, &HubicJob::authorizationFailed, this, &HubicStorageService::slotAuthorizationFailed);
    connect(job, &HubicJob::actionFailed, this, &HubicStorageService::slotActionFailed);
    job->requestTokenAccess();
}

void HubicStorageService::slotAuthorizationFailed(const QString &errorMessage)
{
    mRefreshToken.clear();
    emitAuthentificationFailder(errorMessage);
}

void HubicStorageService::slotAuthorizationDone(const QString &refreshToken, const QString &token, qint64 expireTime)
{
    mRefreshToken = refreshToken;
    mToken = token;
    mExpireDateTime = QDateTime::currentDateTime().addSecs(expireTime);
    if (StorageServiceSettings::self()->createDefaultFolder()) {
        const QString walletEntry = storageServiceName();
        KWallet::Wallet *wallet = StorageServiceSettings::self()->wallet();
        if (wallet) {
            QMap<QString, QString> map;
            map[QLatin1String("Refresh Token")] = mRefreshToken;
            map[QLatin1String("Token")] = mToken;
            map[QLatin1String("Expire Time")] = mExpireDateTime.toString();
            wallet->writeMap(walletEntry, map);
        }
    }
    emitAuthentificationDone();
}

void HubicStorageService::refreshToken()
{
    HubicJob *job = new HubicJob(this);
    job->initializeToken(mRefreshToken, mToken);
    connect(job, &HubicJob::authorizationDone, this, &HubicStorageService::slotAuthorizationDone);
    connect(job, &HubicJob::authorizationFailed, this, &HubicStorageService::slotAuthorizationFailed);
    connect(job, &HubicJob::actionFailed, this, &HubicStorageService::slotActionFailed);
    job->refreshToken();
}

void HubicStorageService::storageServicelistFolder(const QString &folder)
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
        HubicJob *job = new HubicJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, &HubicJob::listFolderDone, this, &HubicStorageService::slotListFolderDone);
        connect(job, &HubicJob::actionFailed, this, &HubicStorageService::slotActionFailed);
        job->listFolder(folder);
    }
}

void HubicStorageService::storageServicecreateFolder(const QString &name, const QString &destination)
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
        HubicJob *job = new HubicJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, &HubicJob::createFolderDone, this, &HubicStorageService::slotCreateFolderDone);
        connect(job, &HubicJob::actionFailed, this, &HubicStorageService::slotActionFailed);
        job->createFolder(name, destination);
    }
}

void HubicStorageService::storageServiceaccountInfo()
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
        HubicJob *job = new HubicJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, &HubicJob::accountInfoDone, this, &HubicStorageService::slotAccountInfoDone);
        connect(job, &HubicJob::actionFailed, this, &HubicStorageService::slotActionFailed);
        job->accountInfo();
    }
}

QString HubicStorageService::name()
{
    return i18n("Hubic");
}

void HubicStorageService::storageServiceuploadFile(const QString &filename, const QString &uploadAsName, const QString &destination)
{
    const bool needRefresh = needToRefreshToken();
    if (mToken.isEmpty() || needRefresh) {
        mNextAction->setNextActionType(UploadFileAction);
        mNextAction->setNextActionName(filename);
        mNextAction->setUploadAsName(uploadAsName);
        mNextAction->setNextActionFolder(destination);
        if (mToken.isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        HubicJob *job = new HubicJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, &HubicJob::uploadFileDone, this, &HubicStorageService::slotUploadFileDone);
        connect(job, &HubicJob::actionFailed, this, &HubicStorageService::slotActionFailed);
        connect(job, &HubicJob::shareLinkDone, this, &HubicStorageService::slotShareLinkDone);
        connect(job, &HubicJob::uploadDownloadFileProgress, this, &HubicStorageService::slotuploadDownloadFileProgress);
        connect(job, &HubicJob::uploadFileFailed, this, &HubicStorageService::slotUploadFileFailed);
        mUploadReply = job->uploadFile(filename, uploadAsName, destination);
    }
}

QString HubicStorageService::description()
{
    return i18n("Hubic is a file hosting service operated by Ovh, Inc. that offers cloud storage, file synchronization, and client software.");
}

QUrl HubicStorageService::serviceUrl()
{
    return QUrl(QLatin1String("https://hubic.com"));
}

QString HubicStorageService::serviceName()
{
    return QLatin1String("hubic");
}

QString HubicStorageService::iconName()
{
    return QString();
}

StorageServiceAbstract::Capabilities HubicStorageService::serviceCapabilities()
{
    StorageServiceAbstract::Capabilities cap;
#if 0 //Disable for the moment
    cap |= AccountInfoCapability;
    //cap |= UploadFileCapability;
    //cap |= DownloadFileCapability;
    cap |= CreateFolderCapability;
    cap |= DeleteFolderCapability;
    cap |= ListFolderCapability;
    //cap |= ShareLinkCapability;
    cap |= DeleteFileCapability;
    //cap |= RenameFolderCapability;
    //cap |= RenameFileCapabilitity;
    //cap |= MoveFileCapability;
    //cap |= MoveFolderCapability;
    //cap |= CopyFileCapability;
    //cap |= CopyFolderCapability;
#endif

    return cap;
}

void HubicStorageService::storageServiceShareLink(const QString &root, const QString &path)
{
    const bool needRefresh = needToRefreshToken();
    if (mToken.isEmpty() || needRefresh) {
        mNextAction->setNextActionType(ShareLinkAction);
        mNextAction->setRootPath(root);
        mNextAction->setPath(path);
        if (mToken.isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        HubicJob *job = new HubicJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, &HubicJob::shareLinkDone, this, &HubicStorageService::slotShareLinkDone);
        connect(job, &HubicJob::actionFailed, this, &HubicStorageService::slotActionFailed);
        job->shareLink(root, path);
    }
}

QString HubicStorageService::storageServiceName() const
{
    return serviceName();
}

void HubicStorageService::storageServicedownloadFile(const QString &name, const QString &fileId, const QString &destination)
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
        HubicJob *job = new HubicJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, &HubicJob::downLoadFileDone, this, &HubicStorageService::slotDownLoadFileDone);
        connect(job, &HubicJob::actionFailed, this, &HubicStorageService::slotActionFailed);
        connect(job, &HubicJob::downLoadFileFailed, this, &HubicStorageService::slotDownLoadFileFailed);
        connect(job, &HubicJob::uploadDownloadFileProgress, this, &HubicStorageService::slotuploadDownloadFileProgress);
        mDownloadReply = job->downloadFile(name, fileId, destination);
    }
}

void HubicStorageService::storageServicecreateServiceFolder()
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
        HubicJob *job = new HubicJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, &HubicJob::createFolderDone, this, &HubicStorageService::slotCreateFolderDone);
        connect(job, &HubicJob::actionFailed, this, &HubicStorageService::slotActionFailed);
        job->createServiceFolder();
    }
}

void HubicStorageService::storageServicedeleteFile(const QString &filename)
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
        HubicJob *job = new HubicJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, &HubicJob::deleteFileDone, this, &HubicStorageService::slotDeleteFileDone);
        connect(job, &HubicJob::actionFailed, this, &HubicStorageService::slotActionFailed);
        job->deleteFile(filename);
    }
}

void HubicStorageService::storageServicedeleteFolder(const QString &foldername)
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
        HubicJob *job = new HubicJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, &HubicJob::deleteFolderDone, this, &HubicStorageService::slotDeleteFolderDone);
        connect(job, &HubicJob::actionFailed, this, &HubicStorageService::slotActionFailed);
        job->deleteFolder(foldername);
    }
}

void HubicStorageService::storageServiceRenameFolder(const QString &source, const QString &destination)
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
        HubicJob *job = new HubicJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, &HubicJob::renameFolderDone, this, &HubicStorageService::slotRenameFolderDone);
        connect(job, &HubicJob::actionFailed, this, &HubicStorageService::slotActionFailed);
        job->renameFolder(source, destination);
    }
}

void HubicStorageService::storageServiceRenameFile(const QString &source, const QString &destination)
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
        HubicJob *job = new HubicJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, &HubicJob::renameFileDone, this, &HubicStorageService::slotRenameFileDone);
        connect(job, &HubicJob::actionFailed, this, &HubicStorageService::slotActionFailed);
        job->renameFile(source, destination);
    }
}

void HubicStorageService::storageServiceMoveFolder(const QString &source, const QString &destination)
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
        HubicJob *job = new HubicJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, &HubicJob::moveFolderDone, this, &HubicStorageService::slotMoveFolderDone);
        connect(job, &HubicJob::actionFailed, this, &HubicStorageService::slotActionFailed);
        job->moveFolder(source, destination);
    }
}

void HubicStorageService::storageServiceMoveFile(const QString &source, const QString &destination)
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
        HubicJob *job = new HubicJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, &HubicJob::moveFileDone, this, &HubicStorageService::slotMoveFileDone);
        connect(job, &HubicJob::actionFailed, this, &HubicStorageService::slotActionFailed);
        job->moveFile(source, destination);
    }
}

void HubicStorageService::storageServiceCopyFile(const QString &source, const QString &destination)
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
        HubicJob *job = new HubicJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, &HubicJob::copyFileDone, this, &HubicStorageService::slotCopyFileDone);
        connect(job, &HubicJob::actionFailed, this, &HubicStorageService::slotActionFailed);
        job->copyFile(source, destination);
    }
}

void HubicStorageService::storageServiceCopyFolder(const QString &source, const QString &destination)
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
        HubicJob *job = new HubicJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, &HubicJob::copyFolderDone, this, &HubicStorageService::slotCopyFolderDone);
        connect(job, &HubicJob::actionFailed, this, &HubicStorageService::slotActionFailed);
        job->copyFolder(source, destination);
    }
}

QIcon HubicStorageService::icon() const
{
    return QIcon();
}

StorageServiceAbstract::Capabilities HubicStorageService::capabilities() const
{
    return serviceCapabilities();
}

QString HubicStorageService::fillListWidget(StorageServiceTreeWidget *listWidget, const QVariant &data, const QString &currentFolder)
{
    Q_UNUSED(currentFolder);
    listWidget->clear();
    listWidget->createMoveUpItem();
    return QString();
}

QMap<QString, QString> HubicStorageService::itemInformation(const QVariantMap &variantMap)
{
    return QMap<QString, QString>();
}

QString HubicStorageService::fileIdentifier(const QVariantMap &variantMap)
{
    return QString();
}

QString HubicStorageService::fileShareRoot(const QVariantMap &variantMap)
{
    return QString();
}

