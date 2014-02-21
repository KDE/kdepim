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
#include "boxutil.h"
#include "boxjob.h"
#include "storageservice/settings/storageservicesettings.h"
#include "pimcommon/storageservice/storageservicejobconfig.h"
#include "pimcommon/storageservice/utils/storageserviceutils.h"

#include <qjson/parser.h>

#include <kwallet.h>

#include <KLocalizedString>
#include <KLocale>
#include <KGlobal>

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
        }
        mNeedToReadConfigFirst = false;
    }
}

void BoxStorageService::removeConfig()
{
    if (StorageServiceSettings::self()->createDefaultFolder()) {
        const QString walletEntry = storageServiceName();
        KWallet::Wallet *wallet = StorageServiceSettings::self()->wallet();
        if (wallet)
            wallet->removeEntry(walletEntry);
    }
}

void BoxStorageService::storageServiceauthentication()
{
    BoxJob *job = new BoxJob(this);
    connect(job, SIGNAL(authorizationDone(QString,QString,qint64)), this, SLOT(slotAuthorizationDone(QString,QString,qint64)));
    connect(job, SIGNAL(authorizationFailed(QString)), this, SLOT(slotAuthorizationFailed(QString)));
    connect(job, SIGNAL(actionFailed(QString)), this, SLOT(slotActionFailed(QString)));
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

bool BoxStorageService::needToRefreshToken()
{
    if (mNeedToReadConfigFirst)
        readConfig();
    if (mExpireDateTime < QDateTime::currentDateTime())
        return true;
    else
        return false;
}

void BoxStorageService::refreshToken()
{
    BoxJob *job = new BoxJob(this);
    job->initializeToken(mRefreshToken, mToken);
    connect(job, SIGNAL(authorizationDone(QString,QString,qint64)), this, SLOT(slotAuthorizationDone(QString,QString,qint64)));
    connect(job, SIGNAL(authorizationFailed(QString)), this, SLOT(slotAuthorizationFailed(QString)));
    job->refreshToken();
}

void BoxStorageService::storageServiceShareLink(const QString &root, const QString &path)
{
    const bool needRefresh = needToRefreshToken();
    if (mToken.isEmpty() || needRefresh) {
        mNextAction->setNextActionType(ShareLink);
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
        connect(job, SIGNAL(shareLinkDone(QString)), this, SLOT(slotShareLinkDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->shareLink(root, path);
    }
}

void BoxStorageService::storageServicedownloadFile(const QString &name, const QString &fileId, const QString &destination)
{
    const bool needRefresh = needToRefreshToken();
    if (mToken.isEmpty() || needRefresh) {
        mNextAction->setNextActionType(DownLoadFile);
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
        connect(job, SIGNAL(downLoadFileDone(QString)), this, SLOT(slotDownLoadFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        connect(job, SIGNAL(downLoadFileFailed(QString)), this, SLOT(slotDownLoadFileFailed(QString)));
        connect(job, SIGNAL(uploadDownloadFileProgress(qint64,qint64)), SLOT(slotuploadDownloadFileProgress(qint64,qint64)));
        mDownloadReply = job->downloadFile(name, fileId, destination);
    }
}

void BoxStorageService::storageServicedeleteFile(const QString &filename)
{
    const bool needRefresh = needToRefreshToken();
    if (mToken.isEmpty() || needRefresh) {
        mNextAction->setNextActionType(DeleteFile);
        mNextAction->setNextActionName(filename);
        if (mToken.isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, SIGNAL(deleteFileDone(QString)), SLOT(slotDeleteFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->deleteFile(filename);
    }
}

void BoxStorageService::storageServicedeleteFolder(const QString &foldername)
{
    const bool needRefresh = needToRefreshToken();
    if (mToken.isEmpty() || needRefresh) {
        mNextAction->setNextActionType(DeleteFolder);
        mNextAction->setNextActionFolder(foldername);
        if (mToken.isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, SIGNAL(deleteFolderDone(QString)), SLOT(slotDeleteFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->deleteFolder(foldername);
    }
}

void BoxStorageService::storageServiceRenameFolder(const QString &source, const QString &destination)
{
    const bool needRefresh = needToRefreshToken();
    if (mToken.isEmpty() || needRefresh) {
        mNextAction->setNextActionType(RenameFolder);
        mNextAction->setRenameFolder(source, destination);
        if (mToken.isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, SIGNAL(renameFolderDone(QString)), SLOT(slotRenameFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->renameFolder(source, destination);
    }
}

void BoxStorageService::storageServiceRenameFile(const QString &source, const QString &destination)
{
    const bool needRefresh = needToRefreshToken();
    if (mToken.isEmpty() || needRefresh) {
        mNextAction->setNextActionType(RenameFile);
        mNextAction->setRenameFolder(source, destination);
        if (mToken.isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, SIGNAL(renameFileDone(QString)), SLOT(slotRenameFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->renameFile(source, destination);
    }
}

void BoxStorageService::storageServiceMoveFolder(const QString &source, const QString &destination)
{
    const bool needRefresh = needToRefreshToken();
    if (mToken.isEmpty() || needRefresh) {
        mNextAction->setNextActionType(MoveFolder);
        mNextAction->setRenameFolder(source, destination);
        if (mToken.isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, SIGNAL(moveFolderDone(QString)), SLOT(slotMoveFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->moveFolder(source, destination);
    }
}

void BoxStorageService::storageServiceMoveFile(const QString &source, const QString &destination)
{
    const bool needRefresh = needToRefreshToken();
    if (mToken.isEmpty() || needRefresh) {
        mNextAction->setNextActionType(MoveFile);
        mNextAction->setRenameFolder(source, destination);
        if (mToken.isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, SIGNAL(moveFileDone(QString)), SLOT(slotMoveFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->moveFile(source, destination);
    }
}

void BoxStorageService::storageServiceCopyFile(const QString &source, const QString &destination)
{
    const bool needRefresh = needToRefreshToken();
    if (mToken.isEmpty() || needRefresh) {
        mNextAction->setNextActionType(CopyFile);
        mNextAction->setRenameFolder(source, destination);
        if (mToken.isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, SIGNAL(copyFileDone(QString)), SLOT(slotCopyFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->copyFile(source, destination);
    }
}

void BoxStorageService::storageServiceCopyFolder(const QString &source, const QString &destination)
{
    const bool needRefresh = needToRefreshToken();
    if (mToken.isEmpty() || needRefresh) {
        mNextAction->setNextActionType(CopyFolder);
        mNextAction->setRenameFolder(source, destination);
        if (mToken.isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, SIGNAL(copyFolderDone(QString)), SLOT(slotCopyFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->copyFolder(source, destination);
    }
}

void BoxStorageService::storageServicelistFolder(const QString &folder)
{
    const bool needRefresh = needToRefreshToken();
    if (mToken.isEmpty() || needRefresh) {
        mNextAction->setNextActionType(ListFolder);
        mNextAction->setNextActionFolder(folder);
        if (mToken.isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, SIGNAL(listFolderDone(QVariant)), this, SLOT(slotListFolderDone(QVariant)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->listFolder(folder);
    }
}

void BoxStorageService::storageServicecreateFolder(const QString &name, const QString &destination)
{
    const bool needRefresh = needToRefreshToken();
    if (mToken.isEmpty() || needRefresh) {
        mNextAction->setNextActionType(CreateFolder);
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
        connect(job, SIGNAL(createFolderDone(QString)), this, SLOT(slotCreateFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->createFolder(name, destination);
    }
}

void BoxStorageService::storageServiceaccountInfo()
{
    const bool needRefresh = needToRefreshToken();
    if (mToken.isEmpty() || needRefresh) {
        mNextAction->setNextActionType(AccountInfo);
        if (mToken.isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job,SIGNAL(accountInfoDone(PimCommon::AccountInfo)), this, SLOT(slotAccountInfoDone(PimCommon::AccountInfo)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
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
        mNextAction->setNextActionType(UploadFile);
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
        connect(job, SIGNAL(uploadFileDone(QString)), this, SLOT(slotUploadFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        connect(job, SIGNAL(shareLinkDone(QString)), this, SLOT(slotShareLinkDone(QString)));
        connect(job, SIGNAL(uploadDownloadFileProgress(qint64,qint64)), SLOT(slotuploadDownloadFileProgress(qint64,qint64)));
        connect(job, SIGNAL(uploadFileFailed(QString)), this, SLOT(slotUploadFileFailed(QString)));
        mUploadReply = job->uploadFile(filename, uploadAsName, destination);
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
    const bool needRefresh = needToRefreshToken();
    if (mToken.isEmpty() || needRefresh) {
        mNextAction->setNextActionType(CreateServiceFolder);
        if (mToken.isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        BoxJob *job = new BoxJob(this);
        job->initializeToken(mRefreshToken, mToken);
        connect(job, SIGNAL(createFolderDone(QString)), this, SLOT(slotCreateFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->createServiceFolder();
    }
}

QString BoxStorageService::fillListWidget(StorageServiceTreeWidget *listWidget, const QVariant &data, const QString &currentFolder)
{
    Q_UNUSED(currentFolder);
    listWidget->clear();
    QJson::Parser parser;
    bool ok;

    const QMap<QString, QVariant> info = parser.parse(data.toString().toUtf8(), &ok).toMap();
    listWidget->createMoveUpItem();
    QString parentId;

    if (info.contains(QLatin1String("entries"))) {
        const QVariantList entries = info.value(QLatin1String("entries")).toList();
        Q_FOREACH (const QVariant &v, entries) {
            QVariantMap mapEntries = v.toMap();
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
                    if (mapEntries.contains(QLatin1String("size"))) {
                        item->setSize(mapEntries.value(QLatin1String("size")).toULongLong());
                    }
                    if (mapEntries.contains(QLatin1String("created_at"))) {
                        item->setDateCreated(PimCommon::BoxUtil::convertToDateTime(mapEntries.value(QLatin1String("created_at")).toString()));
                    }
                    if (mapEntries.contains(QLatin1String("modified_at"))) {
                        item->setLastModification(PimCommon::BoxUtil::convertToDateTime(mapEntries.value(QLatin1String("modified_at")).toString()));
                    }
                    item->setStoreInfo(mapEntries);
                }
            }
        }
    }
    //qDebug()<<" parentId"<<parentId;
    return parentId;
}

QMap<QString, QString> BoxStorageService::itemInformation(const QVariantMap &variantMap)
{
    qDebug()<<" variantMap" <<variantMap;
    QMap<QString, QString> information;
    if (variantMap.contains(QLatin1String("type"))) {
        const QString type = variantMap.value(QLatin1String("type")).toString();
        information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::Type), type == QLatin1String("folder") ? i18n("Folder") : i18n("File"));
    }
    if (variantMap.contains(QLatin1String("name"))) {
        information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::Name), variantMap.value(QLatin1String("name")).toString());
    }
    if (variantMap.contains(QLatin1String("size"))) {
        information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::Size), KGlobal::locale()->formatByteSize(variantMap.value(QLatin1String("size")).toULongLong()));
    }
    if (variantMap.contains(QLatin1String("created_at"))) {
        const QString tmp = variantMap.value(QLatin1String("created_at")).toString();
        information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::Created), KGlobal::locale()->formatDateTime(PimCommon::BoxUtil::convertToDateTime( tmp )));
    }
    if (variantMap.contains(QLatin1String("modified_at"))) {
        const QString tmp = variantMap.value(QLatin1String("modified_at")).toString();
        information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::LastModified), KGlobal::locale()->formatDateTime(PimCommon::BoxUtil::convertToDateTime( tmp )));
    }
    return information;
}

QString BoxStorageService::fileIdentifier(const QVariantMap &variantMap)
{
    if (variantMap.contains(QLatin1String("id"))) {
        return variantMap.value(QLatin1String("id")).toString();
    }
    return QString();
}

QString BoxStorageService::fileShareRoot(const QVariantMap &variantMap)
{
    Q_UNUSED(variantMap);
    return QString();
}

QRegExp BoxStorageService::disallowedSymbols() const
{
    return QRegExp(QLatin1String("[/:?*\\|]"));
}

QString BoxStorageService::disallowedSymbolsStr() const
{
    return QLatin1String("\\ / : ? * < > |");
}

#include "moc_boxstorageservice.cpp"
