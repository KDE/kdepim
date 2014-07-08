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

#include "webdavstorageservice.h"
#include "pimcommon/storageservice/utils/storageserviceutils.h"

#include "storageservice/widgets/storageservicetreewidget.h"
#include "storageservice/widgets/storageservicetreewidgetitem.h"
#include "storageservice/storageservicemanager.h"
#include "storageservice/settings/storageservicesettings.h"
#include "storageservice/webdav/protocol/webdav_url_info.h"
#include "webdavsettingsdialog.h"
#include "webdavjob.h"

#include <kwallet.h>

#include <KLocalizedString>
#include <KLocale>
#include <KDateTime>
#include <KGlobal>

#include <QPointer>
#include <QDebug>
#include <QFileInfo>
#include <QDir>

using namespace PimCommon;

WebDavStorageService::WebDavStorageService(QObject *parent)
    : PimCommon::StorageServiceAbstract(parent)
{
    readConfig();
}

WebDavStorageService::~WebDavStorageService()
{
}

void WebDavStorageService::readConfig()
{
    if (StorageServiceSettings::self()->createDefaultFolder()) {
        KWallet::Wallet *wallet = StorageServiceSettings::self()->wallet();
        if (wallet) {
            QStringList lst = wallet->entryList();
            if (lst.contains(storageServiceName())) {
                QMap<QString, QString> map;
                wallet->readMap( storageServiceName(), map );
                if (map.contains(QLatin1String("public location"))) {
                    mPublicLocation = map.value(QLatin1String("public location"));
                }
                if (map.contains(QLatin1String("service location"))) {
                    mServiceLocation = map.value(QLatin1String("service location"));
                }
                if (map.contains(QLatin1String("username"))) {
                    mUsername = map.value(QLatin1String("username"));
                }
                if (map.contains(QLatin1String("password"))) {
                    mPassword = map.value(QLatin1String("password"));
                }
            }
            mNeedToReadConfigFirst = false;
        }
    }
}

bool WebDavStorageService::needInitialized()
{
    if (mNeedToReadConfigFirst)
        readConfig();
    return (mServiceLocation.isEmpty() || mUsername.isEmpty() || mPassword.isEmpty());
}

void WebDavStorageService::removeConfig()
{
    if (StorageServiceSettings::self()->createDefaultFolder()) {
        const QString walletEntry = storageServiceName();
        KWallet::Wallet *wallet = StorageServiceSettings::self()->wallet();
        if (wallet)
            wallet->removeEntry(walletEntry);
    }
}

void WebDavStorageService::storageServiceauthentication()
{
    WebDavJob *job = new WebDavJob(this);
    connect(job, SIGNAL(authorizationDone(QString,QString,QString,QString)), this, SLOT(slotAuthorizationDone(QString,QString,QString,QString)));
    connect(job, SIGNAL(authorizationFailed(QString)), this, SLOT(slotAuthorizationFailed(QString)));
    connect(job, SIGNAL(actionFailed(QString)), this, SLOT(slotActionFailed(QString)));
    job->requestTokenAccess();
}

void WebDavStorageService::slotAuthorizationDone(const QString &publicLocation, const QString &serviceLocation, const QString &username, const QString &password)
{
    mPublicLocation = publicLocation;
    mServiceLocation = serviceLocation;
    mUsername = username;
    mPassword = password;

    if (StorageServiceSettings::self()->createDefaultFolder()) {
        const QString walletEntry = storageServiceName();
        KWallet::Wallet *wallet = StorageServiceSettings::self()->wallet();
        if (wallet) {
            QMap<QString, QString> map;
            map[QLatin1String( "public location" )] = mPublicLocation;
            map[QLatin1String( "service location" )] = mServiceLocation;
            map[QLatin1String( "username" )] = mUsername;
            map[QLatin1String( "password" )] = mPassword;
            wallet->writeMap( walletEntry, map);
        }
    }
    emitAuthentificationDone();
}

void WebDavStorageService::slotAuthorizationFailed(const QString &errorMessage)
{
    mPublicLocation.clear();
    mServiceLocation.clear();
    mUsername.clear();
    mPassword.clear();
    emitAuthentificationFailder(errorMessage);
}

void WebDavStorageService::connectDefaultSlot(WebDavJob *job)
{
    connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
    connect(job, SIGNAL(authorizationFailed(QString)), this, SLOT(slotAuthorizationFailed(QString)));
    connect(job, SIGNAL(authorizationDone(QString,QString,QString,QString)), this, SLOT(slotAuthorizationDone(QString,QString,QString,QString)));
}

void WebDavStorageService::storageServiceShareLink(const QString &root, const QString &path)
{
    if (needInitialized()) {
        mNextAction->setRootPath(root);
        mNextAction->setPath(path);
        mNextAction->setNextActionType(ShareLinkAction);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        job->initializeToken(mPublicLocation, mServiceLocation, mUsername, mPassword);
        connectDefaultSlot(job);
        connect(job, SIGNAL(shareLinkDone(QString)), this, SLOT(slotShareLinkDone(QString)));
        job->shareLink(root, path);
    }
}

void WebDavStorageService::storageServicedownloadFile(const QString &name, const QString &fileId, const QString &destination)
{
    if (needInitialized()) {
        mNextAction->setNextActionType(DownLoadFileAction);
        mNextAction->setNextActionName(name);
        mNextAction->setDownloadDestination(destination);
        mNextAction->setFileId(fileId);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        job->initializeToken(mPublicLocation, mServiceLocation, mUsername, mPassword);
        connect(job, SIGNAL(downLoadFileDone(QString)), this, SLOT(slotDownLoadFileDone(QString)));
        connect(job, SIGNAL(downLoadFileFailed(QString)), this, SLOT(slotDownLoadFileFailed(QString)));
        connect(job, SIGNAL(uploadDownloadFileProgress(qint64,qint64)), SLOT(slotuploadDownloadFileProgress(qint64,qint64)));
        connectDefaultSlot(job);
        mDownloadReply = job->downloadFile(name, fileId, destination);
    }
}

void WebDavStorageService::storageServicecreateServiceFolder()
{
    if (needInitialized()) {
        mNextAction->setNextActionType(CreateServiceFolderAction);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        job->initializeToken(mPublicLocation, mServiceLocation, mUsername, mPassword);
        connect(job, SIGNAL(createFolderDone(QString)), this, SLOT(slotCreateFolderDone(QString)));
        connectDefaultSlot(job);
        job->createServiceFolder();
    }
}

void WebDavStorageService::storageServicedeleteFile(const QString &filename)
{
    if (needInitialized()) {
        mNextAction->setNextActionType(DeleteFileAction);
        mNextAction->setNextActionName(filename);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        job->initializeToken(mPublicLocation, mServiceLocation, mUsername, mPassword);
        connect(job, SIGNAL(deleteFileDone(QString)), SLOT(slotDeleteFileDone(QString)));
        connectDefaultSlot(job);
        job->deleteFile(filename);
    }
}

void WebDavStorageService::storageServicedeleteFolder(const QString &foldername)
{
    if (needInitialized()) {
        mNextAction->setNextActionType(DeleteFolderAction);
        mNextAction->setNextActionName(foldername);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        job->initializeToken(mPublicLocation, mServiceLocation, mUsername, mPassword);
        connect(job, SIGNAL(deleteFolderDone(QString)), SLOT(slotDeleteFolderDone(QString)));
        connectDefaultSlot(job);
        job->deleteFolder(foldername);
    }
}

void WebDavStorageService::storageServiceRenameFolder(const QString &source, const QString &destination)
{
    if (needInitialized()) {
        mNextAction->setNextActionType(RenameFolderAction);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        job->initializeToken(mPublicLocation, mServiceLocation, mUsername, mPassword);
        connect(job, SIGNAL(renameFolderDone(QString)), SLOT(slotRenameFolderDone(QString)));
        connectDefaultSlot(job);
        job->renameFolder(source, destination);
    }
}

void WebDavStorageService::storageServiceRenameFile(const QString &source, const QString &destination)
{
    if (needInitialized()) {
        mNextAction->setNextActionType(RenameFileAction);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        job->initializeToken(mPublicLocation, mServiceLocation, mUsername, mPassword);
        connect(job, SIGNAL(renameFileDone(QString)), SLOT(slotRenameFileDone(QString)));
        connectDefaultSlot(job);
        job->renameFile(source, destination);
    }
}

void WebDavStorageService::storageServiceMoveFolder(const QString &source, const QString &destination)
{
    if (needInitialized()) {
        mNextAction->setNextActionType(MoveFolderAction);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        job->initializeToken(mPublicLocation, mServiceLocation, mUsername, mPassword);
        connect(job, SIGNAL(moveFolderDone(QString)), SLOT(slotMoveFolderDone(QString)));
        connectDefaultSlot(job);
        job->moveFolder(source, destination);
    }
}

void WebDavStorageService::storageServiceMoveFile(const QString &source, const QString &destination)
{
    if (needInitialized()) {
        mNextAction->setNextActionType(MoveFileAction);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        job->initializeToken(mPublicLocation, mServiceLocation, mUsername, mPassword);
        connect(job, SIGNAL(moveFileDone(QString)), SLOT(slotMoveFileDone(QString)));
        connectDefaultSlot(job);
        job->moveFile(source, destination);
    }
}

void WebDavStorageService::storageServiceCopyFile(const QString &source, const QString &destination)
{
    if (needInitialized()) {
        mNextAction->setNextActionType(CopyFileAction);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        job->initializeToken(mPublicLocation, mServiceLocation, mUsername, mPassword);
        connect(job, SIGNAL(copyFileDone(QString)), SLOT(slotCopyFileDone(QString)));
        connectDefaultSlot(job);
        job->copyFile(source, destination);
    }
}

void WebDavStorageService::storageServiceCopyFolder(const QString &source, const QString &destination)
{
    if (needInitialized()) {
        mNextAction->setNextActionType(CopyFolderAction);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        job->initializeToken(mPublicLocation, mServiceLocation, mUsername, mPassword);
        connect(job, SIGNAL(copyFolderDone(QString)), SLOT(slotCopyFolderDone(QString)));
        connectDefaultSlot(job);
        job->copyFolder(source, destination);
    }
}


StorageServiceAbstract::Capabilities WebDavStorageService::capabilities() const
{
    return serviceCapabilities();
}

QString WebDavStorageService::fillListWidget(StorageServiceTreeWidget *listWidget, const QVariant &data, const QString &currentPath)
{
    //qDebug()<<" data"<<data;
    listWidget->clear();
    listWidget->createMoveUpItem();
    QString currentFolder = currentPath;
    if (currentFolder.isEmpty()) {
        QUrl url(mServiceLocation);
        currentFolder = url.path() + QLatin1Char('/');
    }

    const QList<QWebdavUrlInfo> lst = QWebdavUrlInfo::parseListInfo(data.toString());
    Q_FOREACH(const QWebdavUrlInfo &info, lst) {
        if (QUrl(info.name()).path() == currentFolder)
          continue;
        StorageServiceTreeWidgetItem *item = 0;
        if (info.isDir()) {
            QFileInfo folderInfo(info.name());
            item = listWidget->addFolder(folderInfo.dir().dirName(), info.name());
        } else {
            const QString mimetype = info.mimeType();
            QFileInfo fileInfo(info.name());
            item = listWidget->addFile(fileInfo.fileName(), info.name(), mimetype);
        }
        QDateTime t = info.createdAt();
        if (t.isValid())
            item->setDateCreated(KDateTime(t));
        t = info.lastModified();
        if (t.isValid())
            item->setLastModification(KDateTime(t));
        const qint64 size = info.size();
        if (size>=0)
            item->setSize(size);
        item->setStoreInfo(QVariantMap(info.properties()));
    }
    QString parentFolder;
    if (!currentFolder.isEmpty()) {
        QStringList parts = currentFolder.split(QLatin1String("/"), QString::SkipEmptyParts);
        if (!parts.isEmpty()) {
          parts.removeLast();
        }

        parentFolder = parts.join(QLatin1String("/"));
        if (parentFolder.isEmpty()) {
            parentFolder = QLatin1String("/");
        }
        if (!parentFolder.endsWith(QLatin1Char('/'))) {
            parentFolder += QLatin1String("/");
        }
        if (!parentFolder.startsWith(QLatin1Char('/'))) {
            parentFolder.prepend(QLatin1String("/"));
        }
    }
    //qDebug()<<" currentFolder "<<currentFolder<<" parentFolder" <<parentFolder;
    return parentFolder;
}

QMap<QString, QString> WebDavStorageService::itemInformation(const QVariantMap &variantMap)
{
    //qDebug()<<" variantMap"<<variantMap;
    QMap<QString, QString> information;
    if (variantMap.contains(QLatin1String("path"))) {
        information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::Name), variantMap.value(QLatin1String("path")).toString());
    }
    if (variantMap.contains(QLatin1String("isDir"))) {
        information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::Type), variantMap.value(QLatin1String("isDir")).toBool() ? i18n("Directory") : i18n("File"));
    }
    if (variantMap.contains(QLatin1String("getcontentlength"))) {
        information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::Size), KGlobal::locale()->formatByteSize(variantMap.value(QLatin1String("getcontentlength")).toString().toLongLong() ));
    }
    if (variantMap.contains(QLatin1String("lastmodified"))) {
        information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::LastModified), KGlobal::locale()->formatDateTime(QDateTime::fromString(variantMap.value(QLatin1String("lastmodified")).toString())));
    }
    if (variantMap.contains(QLatin1String("creationdate"))) {
        information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::Created), KGlobal::locale()->formatDateTime(QDateTime::fromString(variantMap.value(QLatin1String("creationdate")).toString())));
    }
    //qDebug()<<" information"<<information;
    return information;
}

QString WebDavStorageService::fileIdentifier(const QVariantMap &variantMap)
{
    if (variantMap.contains(QLatin1String("path"))) {
        return variantMap.value(QLatin1String("path")).toString();
    }
    return QString();
}

QString WebDavStorageService::fileShareRoot(const QVariantMap &variantMap)
{
    return QString();
}

void WebDavStorageService::storageServicelistFolder(const QString &folder)
{
    if (needInitialized()) {
        mNextAction->setNextActionType(ListFolderAction);
        mNextAction->setNextActionFolder(folder);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        job->initializeToken(mPublicLocation, mServiceLocation, mUsername, mPassword);
        connect(job, SIGNAL(listFolderDone(QVariant)), this, SLOT(slotListFolderDone(QVariant)));
        connectDefaultSlot(job);
        job->listFolder(folder);
    }
}

void WebDavStorageService::storageServicecreateFolder(const QString &name, const QString &destination)
{
    if (needInitialized()) {
        mNextAction->setNextActionType(CreateFolderAction);
        mNextAction->setNextActionName(name);
        mNextAction->setNextActionFolder(destination);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        job->initializeToken(mPublicLocation, mServiceLocation, mUsername, mPassword);
        connect(job, SIGNAL(createFolderDone(QString)), this, SLOT(slotCreateFolderDone(QString)));
        connectDefaultSlot(job);
        job->createFolder(name, destination);
    }
}

void WebDavStorageService::storageServiceaccountInfo()
{
    if (needInitialized()) {
        mNextAction->setNextActionType(AccountInfoAction);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        job->initializeToken(mPublicLocation, mServiceLocation, mUsername, mPassword);
        connect(job,SIGNAL(accountInfoDone(PimCommon::AccountInfo)), this, SLOT(slotAccountInfoDone(PimCommon::AccountInfo)));
        connectDefaultSlot(job);
        job->accountInfo();
    }
}

QString WebDavStorageService::name()
{
    return i18n("Webdav");
}

void WebDavStorageService::storageServiceuploadFile(const QString &filename, const QString &uploadAsName, const QString &destination)
{
    if (needInitialized()) {
        mNextAction->setNextActionType(UploadFileAction);
        mNextAction->setNextActionName(filename);
        mNextAction->setNextActionFolder(destination);
        mNextAction->setUploadAsName(uploadAsName);
        storageServiceauthentication();
    } else {
        WebDavJob *job = new WebDavJob(this);
        job->initializeToken(mPublicLocation, mServiceLocation, mUsername, mPassword);
        connectDefaultSlot(job);
        connect(job, SIGNAL(uploadFileDone(QString)), this, SLOT(slotUploadFileDone(QString)));
        connect(job, SIGNAL(shareLinkDone(QString)), this, SLOT(slotShareLinkDone(QString)));
        connect(job, SIGNAL(uploadDownloadFileProgress(qint64,qint64)), SLOT(slotuploadDownloadFileProgress(qint64,qint64)));
        connect(job, SIGNAL(uploadFileFailed(QString)), this, SLOT(slotUploadFileFailed(QString)));
        mUploadReply = job->uploadFile(filename, uploadAsName, destination);
    }
}

QString WebDavStorageService::description()
{
    return i18n("The WebDAV service allows access to any Web application that uses the WebDAV protocol, such as ownCloud, Kolab, and others.");
}

QUrl WebDavStorageService::serviceUrl()
{
    return QUrl(QLatin1String(""));
}

QString WebDavStorageService::serviceName()
{
    return QLatin1String("webdav");
}

QString WebDavStorageService::iconName()
{
    return QLatin1String("folder-remote");
}

StorageServiceAbstract::Capabilities WebDavStorageService::serviceCapabilities()
{
    StorageServiceAbstract::Capabilities cap;
    cap |= AccountInfoCapability;
    cap |= UploadFileCapability;
    cap |= DownloadFileCapability;
    cap |= CreateFolderCapability;
    cap |= DeleteFolderCapability;
    cap |= ListFolderCapability;
    //cap |= ShareLinkCapability;
    cap |= DeleteFileCapability;
    cap |= RenameFolderCapability;
    cap |= RenameFileCapabilitity;
    cap |= MoveFileCapability;
    cap |= MoveFolderCapability;
    cap |= CopyFileCapability;
    cap |= CopyFolderCapability;


    return cap;
}

QString WebDavStorageService::storageServiceName() const
{
    return serviceName();
}

KIcon WebDavStorageService::icon() const
{
    return KIcon(WebDavStorageService::iconName());
}

void WebDavStorageService::shutdownService()
{
    mPublicLocation.clear();
    mServiceLocation.clear();
    mUsername.clear();
    mPassword.clear();
}

bool WebDavStorageService::hasValidSettings() const
{
    return true;
}

#include "moc_webdavstorageservice.cpp"
