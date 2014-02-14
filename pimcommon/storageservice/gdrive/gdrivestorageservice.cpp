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
#include "storageservice/storageservicemanager.h"
#include "gdrivejob.h"
#include "storageservice/settings/storageservicesettings.h"

#include <kwallet.h>

#include <libkgapi2/drive/file.h>

#include <KLocalizedString>
#include <KGlobal>

#include <QPointer>
#include <QDebug>

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
    connect(job, SIGNAL(authorizationDone(QString,QString,QDateTime,QString)), this, SLOT(slotAuthorizationDone(QString,QString,QDateTime,QString)));
    connect(job, SIGNAL(authorizationFailed(QString)), this, SLOT(slotAuthorizationFailed(QString)));
    connect(job, SIGNAL(actionFailed(QString)), this, SLOT(slotActionFailed(QString)));
    job->refreshToken();
}

void GDriveStorageService::storageServiceauthentication()
{
    GDriveJob *job = new GDriveJob(this);
    job->initializeToken(mAccount);
    connect(job, SIGNAL(authorizationDone(QString,QString,QDateTime,QString)), this, SLOT(slotAuthorizationDone(QString,QString,QDateTime,QString)));
    connect(job, SIGNAL(authorizationFailed(QString)), this, SLOT(slotAuthorizationFailed(QString)));
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
        mNextAction->setNextActionType(ShareLink);
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
        connect(job, SIGNAL(shareLinkDone(QString)), this, SLOT(slotShareLinkDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->shareLink(root, path);
    }
}

void GDriveStorageService::storageServicedownloadFile(const QString &name, const QString &fileId, const QString &destination)
{
    if (mNeedToReadConfigFirst)
        readConfig();

    if (mAccount->accessToken().isEmpty() || needToRefreshToken()) {
        mNextAction->setNextActionType(DownLoadFile);
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
        connect(job, SIGNAL(downLoadFileDone(QString)), this, SLOT(slotDownLoadFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        connect(job, SIGNAL(downLoadFileFailed(QString)), this, SLOT(slotDownLoadFileFailed(QString)));
        connect(job, SIGNAL(uploadDownloadFileProgress(qint64,qint64)), SLOT(slotuploadDownloadFileProgress(qint64,qint64)));
        mDownloadReply = job->downloadFile(name, fileId, destination);
    }
}

void GDriveStorageService::storageServicedeleteFile(const QString &filename)
{
    if (mNeedToReadConfigFirst)
        readConfig();

    if (mAccount->accessToken().isEmpty() || needToRefreshToken()) {
        mNextAction->setNextActionType(DeleteFile);
        mNextAction->setNextActionName(filename);
        if (mAccount->accessToken().isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mAccount);
        connect(job, SIGNAL(deleteFileDone(QString)), SLOT(slotDeleteFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->deleteFile(filename);
    }
}

void GDriveStorageService::storageServicedeleteFolder(const QString &foldername)
{
    if (mNeedToReadConfigFirst)
        readConfig();

    if (mAccount->accessToken().isEmpty() || needToRefreshToken()) {
        mNextAction->setNextActionType(DeleteFolder);
        mNextAction->setNextActionFolder(foldername);
        if (mAccount->accessToken().isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mAccount);
        connect(job, SIGNAL(deleteFolderDone(QString)), SLOT(slotDeleteFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->deleteFolder(foldername);
    }
}

void GDriveStorageService::storageServiceRenameFolder(const QString &source, const QString &destination)
{
    if (mNeedToReadConfigFirst)
        readConfig();

    if (mAccount->accessToken().isEmpty() || needToRefreshToken()) {
        mNextAction->setNextActionType(RenameFolder);
        mNextAction->setRenameFolder(source, destination);
        if (mAccount->accessToken().isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mAccount);
        connect(job, SIGNAL(renameFolderDone(QString)), SLOT(slotRenameFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->renameFolder(source, destination);
    }
}

void GDriveStorageService::storageServiceRenameFile(const QString &source, const QString &destination)
{
    if (mNeedToReadConfigFirst)
        readConfig();

    if (mAccount->accessToken().isEmpty() || needToRefreshToken()) {
        mNextAction->setNextActionType(RenameFile);
        mNextAction->setRenameFolder(source, destination);
        if (mAccount->accessToken().isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mAccount);
        connect(job, SIGNAL(renameFileDone(QString)), SLOT(slotRenameFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->renameFile(source, destination);
    }
}

void GDriveStorageService::storageServiceMoveFolder(const QString &source, const QString &destination)
{
    if (mNeedToReadConfigFirst)
        readConfig();

    if (mAccount->accessToken().isEmpty() || needToRefreshToken()) {
        mNextAction->setNextActionType(MoveFolder);
        mNextAction->setRenameFolder(source, destination);
        if (mAccount->accessToken().isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mAccount);
        connect(job, SIGNAL(moveFolderDone(QString)), SLOT(slotMoveFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->moveFolder(source, destination);
    }
}

void GDriveStorageService::storageServiceMoveFile(const QString &source, const QString &destination)
{
    if (mNeedToReadConfigFirst)
        readConfig();

    if (mAccount->accessToken().isEmpty() || needToRefreshToken()) {
        mNextAction->setNextActionType(MoveFile);
        mNextAction->setRenameFolder(source, destination);
        if (mAccount->accessToken().isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mAccount);
        connect(job, SIGNAL(moveFileDone(QString)), SLOT(slotMoveFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->moveFile(source, destination);
    }
}

void GDriveStorageService::storageServiceCopyFile(const QString &source, const QString &destination)
{
    if (mNeedToReadConfigFirst)
        readConfig();

    if (mAccount->accessToken().isEmpty() || needToRefreshToken()) {
        mNextAction->setNextActionType(CopyFile);
        mNextAction->setRenameFolder(source, destination);
        if (mAccount->accessToken().isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mAccount);
        connect(job, SIGNAL(copyFileDone(QString)), SLOT(slotCopyFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->copyFile(source, destination);
    }
}

void GDriveStorageService::storageServiceCopyFolder(const QString &source, const QString &destination)
{
    if (mNeedToReadConfigFirst)
        readConfig();

    if (mAccount->accessToken().isEmpty() || needToRefreshToken()) {
        mNextAction->setNextActionType(CopyFolder);
        mNextAction->setRenameFolder(source, destination);
        if (mAccount->accessToken().isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mAccount);
        connect(job, SIGNAL(copyFolderDone(QString)), SLOT(slotCopyFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->copyFolder(source, destination);
    }
}

QMap<QString, QString> GDriveStorageService::itemInformation(const QVariantMap &variantMap)
{
    return QMap<QString, QString>();
}

void GDriveStorageService::storageServicelistFolder(const QString &folder)
{
    if (mNeedToReadConfigFirst)
        readConfig();

    if (mAccount->accessToken().isEmpty() || needToRefreshToken()) {
        mNextAction->setNextActionType(ListFolder);
        mNextAction->setNextActionFolder(folder);
        if (mAccount->accessToken().isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mAccount);
        connect(job, SIGNAL(listFolderDone(QVariant)), this, SLOT(slotListFolderDone(QVariant)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->listFolder(folder);
    }
}

void GDriveStorageService::storageServicecreateFolder(const QString &name, const QString &destination)
{
    if (mNeedToReadConfigFirst)
        readConfig();

    if (mAccount->accessToken().isEmpty() || needToRefreshToken()) {
        mNextAction->setNextActionType(CreateFolder);
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
        connect(job, SIGNAL(createFolderDone(QString)), this, SLOT(slotCreateFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->createFolder(name, destination);
    }
}

void GDriveStorageService::storageServiceaccountInfo()
{
    if (mNeedToReadConfigFirst)
        readConfig();

    if (mAccount->accessToken().isEmpty() || needToRefreshToken()) {
        mNextAction->setNextActionType(AccountInfo);
        if (mAccount->accessToken().isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mAccount);
        connect(job,SIGNAL(accountInfoDone(PimCommon::AccountInfo)), this, SLOT(slotAccountInfoDone(PimCommon::AccountInfo)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
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
        mNextAction->setNextActionType(UploadFile);
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
        connect(job, SIGNAL(uploadFileDone(QString)), this, SLOT(slotUploadFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        connect(job, SIGNAL(shareLinkDone(QString)), this, SLOT(slotShareLinkDone(QString)));
        connect(job, SIGNAL(uploadDownloadFileProgress(qint64,qint64)), SLOT(slotuploadDownloadFileProgress(qint64,qint64)));
        connect(job, SIGNAL(uploadFileFailed(QString)), this, SLOT(slotUploadFileFailed(QString)));
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
    //cap |= CopyFileCapability;
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

KIcon GDriveStorageService::icon() const
{
    return KIcon(iconName());
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
        mNextAction->setNextActionType(CreateServiceFolder);
        if (mAccount->accessToken().isEmpty()) {
            storageServiceauthentication();
        } else {
            refreshToken();
        }
    } else {
        GDriveJob *job = new GDriveJob(this);
        job->initializeToken(mAccount);
        connect(job, SIGNAL(createFolderDone(QString)), this, SLOT(slotCreateFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->createServiceFolder();
    }
}

QString GDriveStorageService::fillListWidget(StorageServiceTreeWidget *listWidget, const QVariant &data, const QString &currentFolder)
{
    listWidget->clear();
    listWidget->createMoveUpItem();
    const QStringList lst = data.toStringList();
    Q_FOREACH(const QString &item, lst) {
        qDebug()<<" item "<<item;
        KGAPI2::Drive::FilePtr file = KGAPI2::Drive::File::fromJSON(item.toLatin1());
        if (file) {
            if (file->isFolder()) {
                StorageServiceTreeWidgetItem *item = listWidget->addFolder(file->title(), file->id());
                item->setDateCreated(file->createdDate());
                item->setLastModification(file->modifiedDate());
            } else {
                StorageServiceTreeWidgetItem *item = listWidget->addFile(file->title(), file->id(), file->mimeType());
                item->setSize(file->fileSize());
                item->setDateCreated(file->createdDate());
                item->setLastModification(file->modifiedDate());
            }
            //TODO store value
            //TODO other :)
        }

    }
    return QString(); //TODO
}

#include "moc_gdrivestorageservice.cpp"
