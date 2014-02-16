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

#include "storageservice/settings/storageservicesettings.h"

#include <kwallet.h>

#include <qjson/parser.h>

#include <KDateTime>
#include <KLocalizedString>
#include <KGlobal>
#include <KLocale>

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

void UbuntuoneStorageService::shutdownService()
{
    mCustomerSecret.clear();
    mToken.clear();
    mCustomerKey.clear();
    mTokenSecret.clear();
}

void UbuntuoneStorageService::readConfig()
{
    if (StorageServiceSettings::self()->createDefaultFolder()) {
        KWallet::Wallet *wallet = StorageServiceSettings::self()->wallet();
        if (wallet) {
            QStringList lst = wallet->entryList();
            if (lst.contains(storageServiceName())) {
                QMap<QString, QString> map;
                wallet->readMap( storageServiceName(), map );
                if (map.contains(QLatin1String("Customer Secret"))) {
                    mCustomerSecret = map.value(QLatin1String("Customer Secret"));
                }
                if (map.contains(QLatin1String("Token"))) {
                    mToken = map.value(QLatin1String("Token"));
                }
                if (map.contains(QLatin1String("Customer Key"))) {
                    mCustomerKey = map.value(QLatin1String("Customer Key"));
                }
                if (map.contains(QLatin1String("Token Secret"))) {
                    mTokenSecret = map.value(QLatin1String("Token Secret"));
                }
            }
            mNeedToReadConfigFirst = false;
        }
    }
}

void UbuntuoneStorageService::slotAuthorizationDone(const QString &customerSecret, const QString &token, const QString &customerKey, const QString &tokenSecret)
{
    mCustomerSecret = customerSecret;
    mToken = token;
    mCustomerKey = customerKey;
    mTokenSecret = tokenSecret;

    if (StorageServiceSettings::self()->createDefaultFolder()) {
        const QString walletEntry = storageServiceName();
        KWallet::Wallet *wallet = StorageServiceSettings::self()->wallet();
        if (wallet) {
            QMap<QString, QString> map;
            map[QLatin1String( "Customer Secret" )] = mCustomerSecret;
            map[QLatin1String( "Token" )] = mToken;
            map[QLatin1String( "Customer Key" )] = mCustomerKey;
            map[QLatin1String( "Token Secret" )] = mTokenSecret;
            wallet->writeMap( walletEntry, map);
        }
    }
    emitAuthentificationDone();
}

void UbuntuoneStorageService::removeConfig()
{
    if (StorageServiceSettings::self()->createDefaultFolder()) {
        const QString walletEntry = storageServiceName();
        KWallet::Wallet *wallet = StorageServiceSettings::self()->wallet();
        if (wallet)
            wallet->removeEntry(walletEntry);
    }
}

void UbuntuoneStorageService::storageServiceauthentication()
{
    UbuntuOneJob *job = new UbuntuOneJob(this);
    connect(job, SIGNAL(authorizationDone(QString,QString,QString,QString)), this, SLOT(slotAuthorizationDone(QString,QString,QString,QString)));
    connect(job, SIGNAL(authorizationFailed(QString)), this, SLOT(slotAuthorizationFailed(QString)));
    connect(job, SIGNAL(actionFailed(QString)), this, SLOT(slotActionFailed(QString)));
    job->requestTokenAccess();
}

bool UbuntuoneStorageService::checkNeedAuthenticate()
{
    if (mNeedToReadConfigFirst)
        readConfig();
    return mToken.isEmpty();
}

void UbuntuoneStorageService::storageServicelistFolder(const QString &folder)
{
    if (checkNeedAuthenticate()) {
        mNextAction->setNextActionType(ListFolder);
        mNextAction->setNextActionFolder(folder);
        storageServiceauthentication();
    } else {
        UbuntuOneJob *job = new UbuntuOneJob(this);
        connect(job, SIGNAL(listFolderDone(QVariant)), this, SLOT(slotListFolderDone(QVariant)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->initializeToken(mCustomerSecret, mToken, mCustomerKey, mTokenSecret);
        job->listFolder(folder);
    }
}

void UbuntuoneStorageService::storageServicecreateFolder(const QString &name, const QString &destination)
{
    if (checkNeedAuthenticate()) {
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
    if (checkNeedAuthenticate()) {
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

void UbuntuoneStorageService::storageServiceuploadFile(const QString &filename, const QString &uploadAsName, const QString &destination)
{
    if (checkNeedAuthenticate()) {
        mNextAction->setNextActionType(UploadFile);
        mNextAction->setNextActionName(filename);
        mNextAction->setNextActionFolder(destination);
        mNextAction->setUploadAsName(uploadAsName);
        storageServiceauthentication();
    } else {
        UbuntuOneJob *job = new UbuntuOneJob(this);
        job->initializeToken(mCustomerSecret, mToken, mCustomerKey, mTokenSecret);
        connect(job, SIGNAL(uploadFileDone(QString)), this, SLOT(slotUploadFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        connect(job, SIGNAL(shareLinkDone(QString)), this, SLOT(slotShareLinkDone(QString)));
        connect(job, SIGNAL(uploadDownloadFileProgress(qint64,qint64)), SLOT(slotuploadDownloadFileProgress(qint64,qint64)));
        connect(job, SIGNAL(uploadFileFailed(QString)), this, SLOT(slotUploadFileFailed(QString)));
        mUploadReply = job->uploadFile(filename, uploadAsName, destination);
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
    if (checkNeedAuthenticate()) {
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

void UbuntuoneStorageService::storageServicedownloadFile(const QString &name, const QString &fileId, const QString &destination)
{
    if (checkNeedAuthenticate()) {
        mNextAction->setNextActionType(DownLoadFile);
        mNextAction->setNextActionName(name);
        mNextAction->setDownloadDestination(destination);
        mNextAction->setFileId(fileId);
        storageServiceauthentication();
    } else {
        UbuntuOneJob *job = new UbuntuOneJob(this);
        job->initializeToken(mCustomerSecret, mToken, mCustomerKey, mTokenSecret);
        connect(job, SIGNAL(downLoadFileDone(QString)), this, SLOT(slotDownLoadFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        connect(job, SIGNAL(downLoadFileFailed(QString)), this, SLOT(slotDownLoadFileFailed(QString)));
        connect(job, SIGNAL(uploadDownloadFileProgress(qint64,qint64)), SLOT(slotuploadDownloadFileProgress(qint64,qint64)));
        mDownloadReply = job->downloadFile(name, fileId, destination);
    }
}

void UbuntuoneStorageService::storageServicecreateServiceFolder()
{
    if (checkNeedAuthenticate()) {
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
    if (checkNeedAuthenticate()) {
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
    if (checkNeedAuthenticate()) {
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
    if (checkNeedAuthenticate()) {
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
    if (checkNeedAuthenticate()) {
        mNextAction->setNextActionType(RenameFile);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        UbuntuOneJob *job = new UbuntuOneJob(this);
        job->initializeToken(mCustomerSecret, mToken, mCustomerKey, mTokenSecret);
        connect(job, SIGNAL(renameFileDone(QString)), SLOT(slotRenameFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->renameFile(source, destination);
    }
}

void UbuntuoneStorageService::storageServiceMoveFolder(const QString &source, const QString &destination)
{
    if (checkNeedAuthenticate()) {
        mNextAction->setNextActionType(MoveFolder);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        UbuntuOneJob *job = new UbuntuOneJob(this);
        job->initializeToken(mCustomerSecret, mToken, mCustomerKey, mTokenSecret);
        connect(job, SIGNAL(moveFolderDone(QString)), SLOT(slotMoveFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->moveFolder(source, destination);
    }
}

void UbuntuoneStorageService::storageServiceMoveFile(const QString &source, const QString &destination)
{
    if (checkNeedAuthenticate()) {
        mNextAction->setNextActionType(MoveFolder);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        UbuntuOneJob *job = new UbuntuOneJob(this);
        job->initializeToken(mCustomerSecret, mToken, mCustomerKey, mTokenSecret);
        connect(job, SIGNAL(moveFileDone(QString)), SLOT(slotMoveFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->moveFile(source, destination);
    }
}

void UbuntuoneStorageService::storageServiceCopyFile(const QString &source, const QString &destination)
{
    if (checkNeedAuthenticate()) {
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
    if (checkNeedAuthenticate()) {
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

QString UbuntuoneStorageService::fillListWidget(StorageServiceTreeWidget *listWidget, const QVariant &data, const QString &currentFolder)
{
    Q_UNUSED(currentFolder);
    listWidget->clear();
    listWidget->createMoveUpItem();
    QJson::Parser parser;
    bool ok;
    QString parentFolder;
    QMap<QString, QVariant> info = parser.parse(data.toString().toUtf8(), &ok).toMap();
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
                    if (map.contains(QLatin1String("when_created"))) {
                        const KDateTime t = KDateTime(QDateTime::fromString(map.value(QLatin1String("when_created")).toString(), QLatin1String("yyyy-MM-ddThh:mm:ssZ")));
                        item->setDateCreated(t);
                    }
                    if (map.contains(QLatin1String("when_changed"))) {
                        const KDateTime t = KDateTime(QDateTime::fromString(map.value(QLatin1String("when_changed")).toString(), QLatin1String("yyyy-MM-ddThh:mm:ssZ")));
                        item->setLastModification(t);
                    }
                } else if (kind == QLatin1String("file")) {
                    const QString path = map.value(QLatin1String("path")).toString();
                    item = listWidget->addFile(path, path);
                    if (map.contains(QLatin1String("size"))) {
                        item->setSize(map.value(QLatin1String("size")).toULongLong());
                    }
                    if (map.contains(QLatin1String("when_created"))) {
                        const KDateTime t = KDateTime(QDateTime::fromString(map.value(QLatin1String("when_created")).toString(), QLatin1String("yyyy-MM-ddThh:mm:ssZ")));
                        item->setDateCreated(t);
                    }
                    if (map.contains(QLatin1String("when_changed"))) {
                        const KDateTime t = KDateTime(QDateTime::fromString(map.value(QLatin1String("when_changed")).toString(), QLatin1String("yyyy-MM-ddThh:mm:ssZ")));
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

QMap<QString, QString> UbuntuoneStorageService::itemInformation(const QVariantMap &variantMap)
{
    qDebug()<<" variantMap "<<variantMap;
    QMap<QString, QString> information;
    if (variantMap.contains(QLatin1String("kind"))) {
        information.insert(i18n("Type:"), variantMap.value(QLatin1String("kind")).toString() == QLatin1String("file") ? i18n("File") : i18n("Folder"));
    }
    if (variantMap.contains(QLatin1String("volume_path"))) {
        information.insert(i18n("Volume path:"), variantMap.value(QLatin1String("volume_path")).toString());
    }
    if (variantMap.contains(QLatin1String("size"))) {
        information.insert(i18n("Size:"), KGlobal::locale()->formatByteSize(variantMap.value(QLatin1String("size")).toULongLong()));
    }
    if (variantMap.contains(QLatin1String("when_created"))) {
        const KDateTime t = KDateTime(QDateTime::fromString(variantMap.value(QLatin1String("when_created")).toString(), QLatin1String("yyyy-MM-ddThh:mm:ssZ")));
        information.insert(i18n("Created:"), KGlobal::locale()->formatDateTime(t));
    }
    if (variantMap.contains(QLatin1String("when_changed"))) {
        const KDateTime t = KDateTime(QDateTime::fromString(variantMap.value(QLatin1String("when_changed")).toString(), QLatin1String("yyyy-MM-ddThh:mm:ssZ")));
        information.insert(i18n("Last Changed:"), KGlobal::locale()->formatDateTime(t));
    }
    if (variantMap.contains(QLatin1String("is_public"))) {
        const bool value = variantMap.value(QLatin1String("is_public")).toString() == QLatin1String("true");
        information.insert(i18n("File is public:"), (value ? i18n("Yes") : i18n("No")));
        if (variantMap.contains(QLatin1String("public_url"))) {
            const QString publicurl = variantMap.value(QLatin1String("public_url")).toString();
            if (!publicurl.isEmpty())
                information.insert(i18n("Public link:"), publicurl);
        }
    }
    return information;
}

QString UbuntuoneStorageService::fileIdentifier(const QVariantMap &variantMap)
{
    return QString();
}

QString UbuntuoneStorageService::fileShareRoot(const QVariantMap &variantMap)
{
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

#include "moc_ubuntuonestorageservice.cpp"
