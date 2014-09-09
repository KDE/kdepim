    /*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#include "gdrivestorageservice.h"
#include "storageservice/widgets/storageservicetreewidget.h"
#include "storageservice/widgets/storageservicetreewidgetitem.h"
#include "storageservice/storageservicemanager.h"
#include "gdrivejob.h"
#include "storageservice/settings/storageservicesettings.h"
#include "pimcommon/storageservice/storageservicejobconfig.h"
#include "pimcommon/storageservice/utils/storageserviceutils.h"

#include <kwallet.h>
#include <KLocale>

#include <kgapi/drive/file.h>

#include <KLocalizedString>

#include <QDebug>
#include <KFormat>
#include <QLocale>

using namespace PimCommon;

GDriveStorageService::GDriveStorageService(QObject *parent)
    : PimCommon::StorageServiceAbstract(parent)
{
    readConfig();
}

GDriveStorageService::~GDriveStorageService()
{
}

void GDriveStorageService::shutdownService()
{
    mAccount = KGAPI2::AccountPtr();
    mExpireDateTime = QDateTime();
}

bool GDriveStorageService::hasValidSettings() const
{
    return (!PimCommon::StorageServiceJobConfig::self()->gdriveClientId().isEmpty() &&
            !PimCommon::StorageServiceJobConfig::self()->gdriveClientSecret().isEmpty());

}

bool GDriveStorageService::needToRefreshToken() const
{
    if (mExpireDateTime < QDateTime::currentDateTime())
        return true;
    else
        return false;
}

void GDriveStorageService::readConfig()
{
    mExpireDateTime = QDateTime();
    QString accountName;
    QString refreshTokenStr;
    QString accessTokenStr;
    if (StorageServiceSettings::self()->createDefaultFolder()) {
        KWallet::Wallet *wallet = StorageServiceSettings::self()->wallet();
        if (wallet) {
            QStringList lst = wallet->entryList();
            if (lst.contains(storageServiceName())) {
                QMap<QString, QString> map;
                wallet->readMap( storageServiceName(), map );
                if (map.contains(QLatin1String("Account Name"))) {
                    accountName = map.value(QLatin1String("Account Name"));
                }
                if (map.contains(QLatin1String("Refresh Token"))) {
                    refreshTokenStr = map.value(QLatin1String("Refresh Token"));
                }
                if (map.contains(QLatin1String("Token"))) {
                    accessTokenStr = map.value(QLatin1String("Token"));
                }
                if (map.contains(QLatin1String("Expire Time"))) {
                    mExpireDateTime = QDateTime::fromString(map.value(QLatin1String("Expire Time")));
                }
            }
            mNeedToReadConfigFirst = false;
        }
    }
    QList<QUrl> scopeUrls;

    scopeUrls<<QUrl( QLatin1String("https://www.googleapis.com/auth/drive") );
    scopeUrls<<QUrl( QLatin1String("https://www.googleapis.com/auth/drive.file"));
    scopeUrls<<QUrl( QLatin1String("https://www.googleapis.com/auth/drive.metadata.readonly" ));
    scopeUrls<<QUrl( QLatin1String("https://www.googleapis.com/auth/drive.readonly") );

    if (accountName.isEmpty() || refreshTokenStr.isEmpty() || accessTokenStr.isEmpty()) {
        mAccount = KGAPI2::AccountPtr(new KGAPI2::Account());
        mAccount->setScopes(scopeUrls);
    } else {
        mAccount = KGAPI2::AccountPtr(new KGAPI2::Account(accountName, accessTokenStr, refreshTokenStr, scopeUrls));
    }
}

void GDriveStorageService::removeConfig()
{
    if (StorageServiceSettings::self()->createDefaultFolder()) {
        const QString walletEntry = storageServiceName();
        KWallet::Wallet *wallet = StorageServiceSettings::self()->wallet();
        if (wallet)
            wallet->removeEntry(walletEntry);
    }
}

void GDriveStorageService::refreshToken()
{
    GDriveJob *job = new GDriveJob(this);
    job->initializeToken(mAccount);
    connect(job, &GDriveJob::authorizationDone, this, &GDriveStorageService::slotAuthorizationDone);
    connect(job, &GDriveJob::authorizationFailed, this, &GDriveStorageService::slotAuthorizationFailed);
    connect(job, &GDriveJob::actionFailed, this, &GDriveStorageService::slotActionFailed);
    job->refreshToken();
}

void GDriveStorageService::storageServiceauthentication()
{
    GDriveJob *job = new GDriveJob(this);
    job->initializeToken(mAccount);
    connect(job, &GDriveJob::authorizationDone, this, &GDriveStorageService::slotAuthorizationDone);
    connect(job, &GDriveJob::authorizationFailed, this, &GDriveStorageService::slotAuthorizationFailed);
    job->requestTokenAccess();
}

void GDriveStorageService::slotAuthorizationFailed(const QString &errorMessage)
{
    mAccount->setRefreshToken(QString());
    mAccount->setAccessToken(QString());
    emitAuthentificationFailder(errorMessage);
}

void GDriveStorageService::slotAuthorizationDone(const QString &refreshToken, const QString &token, const QDateTime &expireTime, const QString &accountName)
{
    mAccount->setRefreshToken(refreshToken);
    mAccount->setAccessToken(token);
    mExpireDateTime = expireTime;

    if (StorageServiceSettings::self()->createDefaultFolder()) {
        const QString walletEntry = storageServiceName();
        KWallet::Wallet *wallet = StorageServiceSettings::self()->wallet();
        if (wallet) {
            QMap<QString, QString> map;
            map[QLatin1String( "Account Name" )] = accountName;
            map[QLatin1String( "Token" )] = token;
            map[QLatin1String( "Refresh Token" )] = refreshToken;
            map[QLatin1String( "Expire Time" )] = mExpireDateTime.toString();
            wallet->writeMap( walletEntry, map);
        }
    }
    emitAuthentificationDone();
}

void GDriveStorageService::storageServiceShareLink(const QString &root, const QString &path)
{
    if (mNeedToReadConfigFirst)
        readConfig();
    if (mAccount->accessToken().isEmpty() || needToRefreshToken()) {
        mNextAction->setNextActionType(ShareLinkAction);
        mNextAction->setPath(path);
        mNextAction->setRootPath(root);
        if (mAccount->accessToken().isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mAccount);
        connect(job, &GDriveJob::shareLinkDone, this, &GDriveStorageService::slotShareLinkDone);
        connect(job, &GDriveJob::actionFailed, this, &GDriveStorageService::slotActionFailed);
        job->shareLink(root, path);
    }
}

void GDriveStorageService::storageServicedownloadFile(const QString &name, const QString &fileId, const QString &destination)
{
    if (mNeedToReadConfigFirst)
        readConfig();

    if (mAccount->accessToken().isEmpty() || needToRefreshToken()) {
        mNextAction->setNextActionType(DownLoadFileAction);
        mNextAction->setNextActionName(name);
        mNextAction->setDownloadDestination(destination);
        mNextAction->setFileId(fileId);
        if (mAccount->accessToken().isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mAccount);
        connect(job, &GDriveJob::downLoadFileDone, this, &GDriveStorageService::slotDownLoadFileDone);
        connect(job, &GDriveJob::actionFailed, this, &GDriveStorageService::slotActionFailed);
        connect(job, &GDriveJob::downLoadFileFailed, this, &GDriveStorageService::slotDownLoadFileFailed);
        connect(job, &GDriveJob::uploadDownloadFileProgress, this, &GDriveStorageService::slotuploadDownloadFileProgress);
        mDownloadReply = job->downloadFile(name, fileId, destination);
    }
}

void GDriveStorageService::storageServicedeleteFile(const QString &filename)
{
    if (mNeedToReadConfigFirst)
        readConfig();

    if (mAccount->accessToken().isEmpty() || needToRefreshToken()) {
        mNextAction->setNextActionType(DeleteFileAction);
        mNextAction->setNextActionName(filename);
        if (mAccount->accessToken().isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mAccount);
        connect(job, &GDriveJob::deleteFileDone, this, &GDriveStorageService::slotDeleteFileDone);
        connect(job, &GDriveJob::actionFailed, this, &GDriveStorageService::slotActionFailed);
        job->deleteFile(filename);
    }
}

void GDriveStorageService::storageServicedeleteFolder(const QString &foldername)
{
    if (mNeedToReadConfigFirst)
        readConfig();

    if (mAccount->accessToken().isEmpty() || needToRefreshToken()) {
        mNextAction->setNextActionType(DeleteFolderAction);
        mNextAction->setNextActionFolder(foldername);
        if (mAccount->accessToken().isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mAccount);
        connect(job, &GDriveJob::deleteFolderDone, this, &GDriveStorageService::slotDeleteFolderDone);
        connect(job, &GDriveJob::actionFailed, this, &GDriveStorageService::slotActionFailed);
        job->deleteFolder(foldername);
    }
}

void GDriveStorageService::storageServiceRenameFolder(const QString &source, const QString &destination)
{
    if (mNeedToReadConfigFirst)
        readConfig();

    if (mAccount->accessToken().isEmpty() || needToRefreshToken()) {
        mNextAction->setNextActionType(RenameFolderAction);
        mNextAction->setRenameFolder(source, destination);
        if (mAccount->accessToken().isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mAccount);
        connect(job, &GDriveJob::renameFolderDone, this, &GDriveStorageService::slotRenameFolderDone);
        connect(job, &GDriveJob::actionFailed, this, &GDriveStorageService::slotActionFailed);
        job->renameFolder(source, destination);
    }
}

void GDriveStorageService::storageServiceRenameFile(const QString &source, const QString &destination)
{
    if (mNeedToReadConfigFirst)
        readConfig();

    if (mAccount->accessToken().isEmpty() || needToRefreshToken()) {
        mNextAction->setNextActionType(RenameFileAction);
        mNextAction->setRenameFolder(source, destination);
        if (mAccount->accessToken().isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mAccount);
        connect(job, &GDriveJob::renameFileDone, this, &GDriveStorageService::slotRenameFileDone);
        connect(job, &GDriveJob::actionFailed, this, &GDriveStorageService::slotActionFailed);
        job->renameFile(source, destination);
    }
}

void GDriveStorageService::storageServiceMoveFolder(const QString &source, const QString &destination)
{
    if (mNeedToReadConfigFirst)
        readConfig();

    if (mAccount->accessToken().isEmpty() || needToRefreshToken()) {
        mNextAction->setNextActionType(MoveFolderAction);
        mNextAction->setRenameFolder(source, destination);
        if (mAccount->accessToken().isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mAccount);
        connect(job, &GDriveJob::moveFolderDone, this, &GDriveStorageService::slotMoveFolderDone);
        connect(job, &GDriveJob::actionFailed, this, &GDriveStorageService::slotActionFailed);
        job->moveFolder(source, destination);
    }
}

void GDriveStorageService::storageServiceMoveFile(const QString &source, const QString &destination)
{
    if (mNeedToReadConfigFirst)
        readConfig();

    if (mAccount->accessToken().isEmpty() || needToRefreshToken()) {
        mNextAction->setNextActionType(MoveFileAction);
        mNextAction->setRenameFolder(source, destination);
        if (mAccount->accessToken().isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mAccount);
        connect(job, &GDriveJob::moveFileDone, this, &GDriveStorageService::slotMoveFileDone);
        connect(job, &GDriveJob::actionFailed, this, &GDriveStorageService::slotActionFailed);
        job->moveFile(source, destination);
    }
}

void GDriveStorageService::storageServiceCopyFile(const QString &source, const QString &destination)
{
    if (mNeedToReadConfigFirst)
        readConfig();

    if (mAccount->accessToken().isEmpty() || needToRefreshToken()) {
        mNextAction->setNextActionType(CopyFileAction);
        mNextAction->setRenameFolder(source, destination);
        if (mAccount->accessToken().isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mAccount);
        connect(job, &GDriveJob::copyFileDone, this, &GDriveStorageService::slotCopyFileDone);
        connect(job, &GDriveJob::actionFailed, this, &GDriveStorageService::slotActionFailed);
        job->copyFile(source, destination);
    }
}

void GDriveStorageService::storageServiceCopyFolder(const QString &source, const QString &destination)
{
    if (mNeedToReadConfigFirst)
        readConfig();

    if (mAccount->accessToken().isEmpty() || needToRefreshToken()) {
        mNextAction->setNextActionType(CopyFolderAction);
        mNextAction->setRenameFolder(source, destination);
        if (mAccount->accessToken().isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mAccount);
        connect(job, &GDriveJob::copyFolderDone, this, &GDriveStorageService::slotCopyFolderDone);
        connect(job, &GDriveJob::actionFailed, this, &GDriveStorageService::slotActionFailed);
        job->copyFolder(source, destination);
    }
}

QMap<QString, QString> GDriveStorageService::itemInformation(const QVariantMap &variantMap)
{
    QMap<QString, QString> information;
    //qDebug()<<" variantMap"<<variantMap;
    KGAPI2::Drive::FilePtr file = KGAPI2::Drive::File::fromJSON(variantMap);
    if (file) {
        information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::Type), file->isFolder() ? i18n("Folder") : i18n("File"));
        information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::Name), file->title());
        if (!file->isFolder()) {
            information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::Size), KFormat().formatByteSize(file->fileSize()));
        }

        information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::Created), QLocale().toString((file->createdDate()), QLocale::ShortFormat));
        information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::LastModified), QLocale().toString((file->modifiedDate()), QLocale::ShortFormat));
        //TODO more infos

    }
    return information;
}

void GDriveStorageService::storageServicelistFolder(const QString &folder)
{
    if (mNeedToReadConfigFirst)
        readConfig();

    if (mAccount->accessToken().isEmpty() || needToRefreshToken()) {
        mNextAction->setNextActionType(ListFolderAction);
        mNextAction->setNextActionFolder(folder);
        if (mAccount->accessToken().isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mAccount);
        connect(job, &GDriveJob::listFolderDone, this, &GDriveStorageService::slotListFolderDone);
        connect(job, &GDriveJob::actionFailed, this, &GDriveStorageService::slotActionFailed);
        job->listFolder(folder);
    }
}

void GDriveStorageService::storageServicecreateFolder(const QString &name, const QString &destination)
{
    if (mNeedToReadConfigFirst)
        readConfig();

    if (mAccount->accessToken().isEmpty() || needToRefreshToken()) {
        mNextAction->setNextActionType(CreateFolderAction);
        mNextAction->setNextActionName(name);
        mNextAction->setNextActionFolder(destination);
        if (mAccount->accessToken().isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mAccount);
        connect(job, &GDriveJob::createFolderDone, this, &GDriveStorageService::slotCreateFolderDone);
        connect(job, &GDriveJob::actionFailed, this, &GDriveStorageService::slotActionFailed);
        job->createFolder(name, destination);
    }
}

void GDriveStorageService::storageServiceaccountInfo()
{
    if (mNeedToReadConfigFirst)
        readConfig();

    if (mAccount->accessToken().isEmpty() || needToRefreshToken()) {
        mNextAction->setNextActionType(AccountInfoAction);
        if (mAccount->accessToken().isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mAccount);
        connect(job, &GDriveJob::accountInfoDone, this, &GDriveStorageService::slotAccountInfoDone);
        connect(job, &GDriveJob::actionFailed, this, &GDriveStorageService::slotActionFailed);
        job->accountInfo();
    }
}

QString GDriveStorageService::name()
{
    return i18n("GoogleDrive");
}

void GDriveStorageService::storageServiceuploadFile(const QString &filename, const QString &uploadAsName, const QString &destination)
{
    if (mNeedToReadConfigFirst)
        readConfig();

    if (mAccount->accessToken().isEmpty() || needToRefreshToken()) {
        mNextAction->setNextActionType(UploadFileAction);
        mNextAction->setNextActionName(filename);
        mNextAction->setNextActionFolder(destination);
        mNextAction->setUploadAsName(uploadAsName);
        if (mAccount->accessToken().isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mAccount);
        connect(job, &GDriveJob::uploadFileDone, this, &GDriveStorageService::slotUploadFileDone);
        connect(job, &GDriveJob::actionFailed, this, &GDriveStorageService::slotActionFailed);
        connect(job, &GDriveJob::shareLinkDone, this, &GDriveStorageService::slotShareLinkDone);
        connect(job, &GDriveJob::uploadDownloadFileProgress, this, &GDriveStorageService::slotuploadDownloadFileProgress);
        connect(job, &GDriveJob::uploadFileFailed, this, &GDriveStorageService::slotUploadFileFailed);
        mUploadReply = job->uploadFile(filename, uploadAsName, destination);
    }
}

QString GDriveStorageService::description()
{
    return i18n("Googledrive is a file hosting that offers cloud storage, file synchronization, and client software.");
}

QUrl GDriveStorageService::serviceUrl()
{
    return QUrl(QLatin1String("http://www.google.com/drive"));
}

QString GDriveStorageService::serviceName()
{
    return QLatin1String("googledrive");
}

QString GDriveStorageService::iconName()
{
    return QLatin1String("kdepim-googledrive");
}

StorageServiceAbstract::Capabilities GDriveStorageService::serviceCapabilities()
{
    StorageServiceAbstract::Capabilities cap;
    cap |= AccountInfoCapability;
    cap |= UploadFileCapability;
    //cap |= DownloadFileCapability;
    cap |= CreateFolderCapability;
    cap |= DeleteFolderCapability;
    cap |= ListFolderCapability;
    cap |= DeleteFileCapability;
    //cap |= ShareLinkCapability;
    cap |= RenameFolderCapability;
    cap |= RenameFileCapabilitity;
    //cap |= MoveFileCapability;
    //cap |= MoveFolderCapability;
    cap |= CopyFileCapability;
    //cap |= CopyFolderCapability;
    return cap;
}


QString GDriveStorageService::storageServiceName() const
{
    return serviceName();
}

QString GDriveStorageService::fileIdentifier(const QVariantMap &variantMap)
{
    //TODO
    return QString();
}

QString GDriveStorageService::fileShareRoot(const QVariantMap &variantMap)
{
    return QString();
}

QIcon GDriveStorageService::icon() const
{
    return QIcon::fromTheme(iconName());
}

StorageServiceAbstract::Capabilities GDriveStorageService::capabilities() const
{
    return serviceCapabilities();
}

void GDriveStorageService::storageServicecreateServiceFolder()
{
    if (mNeedToReadConfigFirst)
        readConfig();

    if (mAccount->accessToken().isEmpty() || needToRefreshToken()) {
        mNextAction->setNextActionType(CreateServiceFolderAction);
        if (mAccount->accessToken().isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mAccount);
        connect(job, &GDriveJob::createFolderDone, this, &GDriveStorageService::slotCreateFolderDone);
        connect(job, &GDriveJob::actionFailed, this, &GDriveStorageService::slotActionFailed);
        job->createServiceFolder();
    }
}

QString GDriveStorageService::fillListWidget(StorageServiceTreeWidget *listWidget, const QVariant &data, const QString &currentFolder)
{
#if 0 //QT5 port to QJSonDocument
    listWidget->clear();
    listWidget->createMoveUpItem();
    const QStringList lst = data.toStringList();
    Q_FOREACH(const QString &item, lst) {
        const QByteArray dataItem = item.toLatin1();
        QJson::Parser parser;
        bool ok;
        const QVariant data = parser.parse(dataItem, &ok);
        if (ok) {
            const QVariantMap varData = data.toMap();
            KGAPI2::Drive::FilePtr file = KGAPI2::Drive::File::fromJSON(varData);
            if (file) {
                StorageServiceTreeWidgetItem *treeWidgetItem = 0;
                if (file->isFolder()) {
                    treeWidgetItem = listWidget->addFolder(file->title(), file->id());
                    treeWidgetItem->setDateCreated(file->createdDate());
                    treeWidgetItem->setLastModification(file->modifiedDate());
                } else {
                    treeWidgetItem = listWidget->addFile(file->title(), file->id(), file->mimeType());
                    treeWidgetItem->setSize(file->fileSize());
                    treeWidgetItem->setDateCreated(file->createdDate());
                    treeWidgetItem->setLastModification(file->modifiedDate());
                }
                treeWidgetItem->setStoreInfo(varData);
            }
        }
    }
#endif
    return QString(); //TODO
}

bool GDriveStorageService::hasCancelSupport() const
{
    return false;
}


#include "moc_gdrivestorageservice.cpp"
