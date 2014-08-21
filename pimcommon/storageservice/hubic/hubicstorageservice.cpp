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
    if (mNeedToReadConfigFirst)
        readConfig();
    if (mExpireDateTime < QDateTime::currentDateTime())
        return true;
    else
        return false;
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
                wallet->readMap( storageServiceName(), map );
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
        if (wallet)
            wallet->removeEntry(walletEntry);
    }
}

void HubicStorageService::storageServiceauthentication()
{
    HubicJob *job = new HubicJob(this);
    connect(job, SIGNAL(authorizationDone(QString,QString,qint64)), this, SLOT(slotAuthorizationDone(QString,QString,qint64)));
    connect(job, SIGNAL(authorizationFailed(QString)), this, SLOT(slotAuthorizationFailed(QString)));
    connect(job, SIGNAL(actionFailed(QString)), this, SLOT(slotActionFailed(QString)));
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
            map[QLatin1String( "Refresh Token" )] = mRefreshToken;
            map[QLatin1String( "Token" )] = mToken;
            map[QLatin1String( "Expire Time" )] = mExpireDateTime.toString();
            wallet->writeMap( walletEntry, map);
        }
    }
    emitAuthentificationDone();
}

void HubicStorageService::refreshToken()
{
    HubicJob *job = new HubicJob(this);
    job->initializeToken(mRefreshToken, mToken);
    connect(job, SIGNAL(authorizationDone(QString,QString,qint64)), this, SLOT(slotAuthorizationDone(QString,QString,qint64)));
    connect(job, SIGNAL(authorizationFailed(QString)), this, SLOT(slotAuthorizationFailed(QString)));
    connect(job, SIGNAL(actionFailed(QString)), this, SLOT(slotActionFailed(QString)));
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
        connect(job, SIGNAL(listFolderDone(QVariant)), this, SLOT(slotListFolderDone(QVariant)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
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
        connect(job, SIGNAL(createFolderDone(QString)), this, SLOT(slotCreateFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
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
        connect(job,SIGNAL(accountInfoDone(PimCommon::AccountInfo)), this, SLOT(slotAccountInfoDone(PimCommon::AccountInfo)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
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
        connect(job, SIGNAL(uploadFileDone(QString)), this, SLOT(slotUploadFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        connect(job, SIGNAL(shareLinkDone(QString)), this, SLOT(slotShareLinkDone(QString)));
        connect(job, SIGNAL(uploadDownloadFileProgress(qint64,qint64)), SLOT(slotuploadDownloadFileProgress(qint64,qint64)));
        connect(job, SIGNAL(uploadFileFailed(QString)), this, SLOT(slotUploadFileFailed(QString)));
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
        connect(job, SIGNAL(shareLinkDone(QString)), this, SLOT(slotShareLinkDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
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
        connect(job, SIGNAL(downLoadFileDone(QString)), this, SLOT(slotDownLoadFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        connect(job, SIGNAL(downLoadFileFailed(QString)), this, SLOT(slotDownLoadFileFailed(QString)));
        connect(job, SIGNAL(uploadDownloadFileProgress(qint64,qint64)), SLOT(slotuploadDownloadFileProgress(qint64,qint64)));
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
        connect(job, SIGNAL(createFolderDone(QString)), this, SLOT(slotCreateFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
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
        connect(job, SIGNAL(deleteFileDone(QString)), SLOT(slotDeleteFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
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
        connect(job, SIGNAL(deleteFolderDone(QString)), SLOT(slotDeleteFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
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
        connect(job, SIGNAL(renameFolderDone(QString)), SLOT(slotRenameFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
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
        connect(job, SIGNAL(renameFileDone(QString)), SLOT(slotRenameFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
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
        connect(job, SIGNAL(moveFolderDone(QString)), SLOT(slotMoveFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
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
        connect(job, SIGNAL(moveFileDone(QString)), SLOT(slotMoveFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
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
        connect(job, SIGNAL(copyFileDone(QString)), SLOT(slotCopyFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
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
        connect(job, SIGNAL(copyFolderDone(QString)), SLOT(slotCopyFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
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

#include "moc_hubicstorageservice.cpp"
