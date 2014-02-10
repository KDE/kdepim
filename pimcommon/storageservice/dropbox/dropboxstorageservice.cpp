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

#include "dropboxstorageservice.h"
#include "storageservice/widgets/storageservicetreewidget.h"
#include "storageservice/storageservicemanager.h"
#include "dropboxutil.h"
#include "dropboxjob.h"

#include <qjson/parser.h>

#include <KLocalizedString>
#include <KConfig>
#include <KGlobal>
#include <KLocale>
#include <KConfigGroup>
#include <KMessageBox>

#include <QDebug>


using namespace PimCommon;

DropBoxStorageService::DropBoxStorageService(QObject *parent)
    : PimCommon::StorageServiceAbstract(parent)
{
    readConfig();
}

DropBoxStorageService::~DropBoxStorageService()
{
}

void DropBoxStorageService::removeConfig()
{
    KConfig config(StorageServiceManager::kconfigName());
    KConfigGroup grp(&config, "Dropbox Settings");
    grp.deleteGroup();
    config.sync();
}

void DropBoxStorageService::readConfig()
{
    KConfig config(StorageServiceManager::kconfigName());
    KConfigGroup grp(&config, "Dropbox Settings");
    mAccessToken = grp.readEntry("Access Token");
    mAccessTokenSecret = grp.readEntry("Access Token Secret");
    mAccessOauthSignature = grp.readEntry("Access Oauth Signature");
}

void DropBoxStorageService::storageServiceauthentication()
{
    DropBoxJob *job = new DropBoxJob(this);
    connect(job, SIGNAL(authorizationDone(QString,QString,QString)), this, SLOT(slotAuthorizationDone(QString,QString,QString)));
    connect(job, SIGNAL(authorizationFailed(QString)), this, SLOT(slotAuthorizationFailed(QString)));
    connect(job, SIGNAL(actionFailed(QString)), this, SLOT(slotActionFailed(QString)));
    job->requestTokenAccess();
}

void DropBoxStorageService::storageServiceShareLink(const QString &root, const QString &path)
{
    if (mAccessToken.isEmpty()) {
        mNextAction->setNextActionType(ShareLink);
        mNextAction->setRootPath(root);
        mNextAction->setPath(path);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken,mAccessTokenSecret,mAccessOauthSignature);
        connect(job, SIGNAL(shareLinkDone(QString)), this, SLOT(slotShareLinkDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->shareLink(root, path);
    }
}

void DropBoxStorageService::storageServicecreateServiceFolder()
{
    if (mAccessToken.isEmpty()) {
        mNextAction->setNextActionType(CreateServiceFolder);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken,mAccessTokenSecret,mAccessOauthSignature);
        connect(job, SIGNAL(createFolderDone(QString)), this, SLOT(slotCreateFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->createServiceFolder();
    }
}

void DropBoxStorageService::slotAuthorizationDone(const QString &accessToken, const QString &accessTokenSecret, const QString &accessOauthSignature)
{
    mAccessToken = accessToken;
    mAccessTokenSecret = accessTokenSecret;
    mAccessOauthSignature = accessOauthSignature;
    KConfig config(StorageServiceManager::kconfigName());
    KConfigGroup grp(&config, "Dropbox Settings");
    grp.writeEntry("Access Token", mAccessToken);
    grp.writeEntry("Access Token Secret", mAccessTokenSecret);
    grp.writeEntry("Access Oauth Signature", mAccessOauthSignature);
    grp.sync();
    config.sync();
    emitAuthentificationDone();
}

void DropBoxStorageService::storageServicelistFolder(const QString &folder)
{
    if (mAccessToken.isEmpty()) {
        mNextAction->setNextActionType(ListFolder);
        mNextAction->setNextActionFolder(folder);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken,mAccessTokenSecret,mAccessOauthSignature);
        connect(job, SIGNAL(listFolderDone(QVariant)), this, SLOT(slotListFolderDone(QVariant)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->listFolder(folder);
    }
}

void DropBoxStorageService::storageServiceaccountInfo()
{
    if (mAccessToken.isEmpty()) {
        mNextAction->setNextActionType(AccountInfo);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken,mAccessTokenSecret,mAccessOauthSignature);
        connect(job, SIGNAL(accountInfoDone(PimCommon::AccountInfo)), this, SLOT(slotAccountInfoDone(PimCommon::AccountInfo)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->accountInfo();
    }
}

void DropBoxStorageService::storageServicecreateFolder(const QString &name, const QString &destination)
{
    if (mAccessToken.isEmpty()) {
        mNextAction->setNextActionType(CreateFolder);
        mNextAction->setNextActionName(name);
        mNextAction->setNextActionFolder(destination);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken,mAccessTokenSecret,mAccessOauthSignature);
        connect(job, SIGNAL(createFolderDone(QString)), this, SLOT(slotCreateFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->createFolder(name, destination);
    }
}

void DropBoxStorageService::storageServiceuploadFile(const QString &filename, const QString &uploadAsName, const QString &destination)
{
    if (mAccessToken.isEmpty()) {
        mNextAction->setNextActionType(UploadFile);
        mNextAction->setNextActionName(filename);
        mNextAction->setNextActionFolder(destination);
        mNextAction->setUploadAsName(uploadAsName);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken,mAccessTokenSecret,mAccessOauthSignature);
        connect(job, SIGNAL(uploadFileDone(QString)), this, SLOT(slotUploadFileDone(QString)));
        connect(job, SIGNAL(shareLinkDone(QString)), this, SLOT(slotShareLinkDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        connect(job, SIGNAL(uploadDownloadFileProgress(qint64,qint64)), SLOT(slotuploadDownloadFileProgress(qint64,qint64)));
        connect(job, SIGNAL(uploadFileFailed(QString)), this, SLOT(slotUploadFileFailed(QString)));
        mUploadReply = job->uploadFile(filename, uploadAsName, destination);
    }
}

void DropBoxStorageService::slotAuthorizationFailed(const QString &errorMessage)
{
    mAccessToken.clear();
    mAccessTokenSecret.clear();
    mAccessOauthSignature.clear();
    emitAuthentificationFailder(errorMessage);
}

QString DropBoxStorageService::name()
{
    return i18n("Dropbox");
}

QString DropBoxStorageService::description()
{
    return i18n("Dropbox is a file hosting service operated by Dropbox, Inc. that offers cloud storage, file synchronization, and client software.");
}

QUrl DropBoxStorageService::serviceUrl()
{
    return QUrl(QLatin1String("https://www.dropbox.com/"));
}

QString DropBoxStorageService::serviceName()
{
    return QLatin1String("dropbox");
}

QString DropBoxStorageService::iconName()
{
    return QLatin1String("kdepim-dropbox");
}

StorageServiceAbstract::Capabilities DropBoxStorageService::serviceCapabilities()
{
    StorageServiceAbstract::Capabilities cap;
    cap |= AccountInfoCapability;
    cap |= UploadFileCapability;
    cap |= DownloadFileCapability;
    cap |= CreateFolderCapability;
    cap |= DeleteFolderCapability;
    cap |= DeleteFileCapability;
    cap |= ListFolderCapability;
    cap |= ShareLinkCapability;
    cap |= RenameFolderCapability;
    cap |= RenameFileCapabilitity;
    cap |= MoveFileCapability;
    cap |= MoveFolderCapability;
    cap |= CopyFileCapability;
    cap |= CopyFolderCapability;


    return cap;
}

QString DropBoxStorageService::storageServiceName() const
{
    return serviceName();
}

void DropBoxStorageService::storageServicedownloadFile(const QString &name, const QString &fileId, const QString &destination)
{
    if (mAccessToken.isEmpty()) {
        mNextAction->setNextActionType(DownLoadFile);
        mNextAction->setNextActionName(name);
        mNextAction->setDownloadDestination(destination);
        mNextAction->setFileId(fileId);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken,mAccessTokenSecret,mAccessOauthSignature);
        connect(job, SIGNAL(downLoadFileDone(QString)), this, SLOT(slotDownLoadFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        connect(job, SIGNAL(downLoadFileFailed(QString)), this, SLOT(slotDownLoadFileFailed(QString)));
        connect(job, SIGNAL(uploadDownloadFileProgress(qint64,qint64)), SLOT(slotuploadDownloadFileProgress(qint64,qint64)));
        mDownloadReply = job->downloadFile(name, fileId, destination);
    }
}

void DropBoxStorageService::storageServicedeleteFile(const QString &filename)
{
    if (mAccessToken.isEmpty()) {
        mNextAction->setNextActionType(DeleteFile);
        mNextAction->setNextActionName(filename);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken,mAccessTokenSecret,mAccessOauthSignature);
        connect(job, SIGNAL(deleteFileDone(QString)), SLOT(slotDeleteFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->deleteFile(filename);
    }
}

void DropBoxStorageService::storageServicedeleteFolder(const QString &foldername)
{
    if (mAccessToken.isEmpty()) {
        mNextAction->setNextActionType(DeleteFolder);
        mNextAction->setNextActionFolder(foldername);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken,mAccessTokenSecret,mAccessOauthSignature);
        connect(job, SIGNAL(deleteFolderDone(QString)), SLOT(slotDeleteFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->deleteFolder(foldername);
    }
}

void DropBoxStorageService::storageServiceRenameFolder(const QString &source, const QString &destination)
{
    if (mAccessToken.isEmpty()) {
        mNextAction->setNextActionType(RenameFolder);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken,mAccessTokenSecret,mAccessOauthSignature);
        connect(job, SIGNAL(renameFolderDone(QString)), SLOT(slotRenameFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->renameFolder(source, destination);
    }
}

void DropBoxStorageService::storageServiceRenameFile(const QString &source, const QString &destination)
{
    if (mAccessToken.isEmpty()) {
        mNextAction->setNextActionType(RenameFile);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken,mAccessTokenSecret,mAccessOauthSignature);
        connect(job, SIGNAL(renameFileDone(QString)), SLOT(slotRenameFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->renameFile(source, destination);
    }
}

void DropBoxStorageService::storageServiceMoveFolder(const QString &source, const QString &destination)
{
    if (mAccessToken.isEmpty()) {
        mNextAction->setNextActionType(MoveFolder);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken,mAccessTokenSecret,mAccessOauthSignature);
        connect(job, SIGNAL(moveFolderDone(QString)), SLOT(slotMoveFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->moveFolder(source, destination);
    }
}

void DropBoxStorageService::storageServiceMoveFile(const QString &source, const QString &destination)
{
    if (mAccessToken.isEmpty()) {
        mNextAction->setNextActionType(RenameFolder);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken,mAccessTokenSecret,mAccessOauthSignature);
        connect(job, SIGNAL(moveFileDone(QString)), SLOT(slotMoveFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->moveFile(source, destination);
    }
}

void DropBoxStorageService::storageServiceCopyFile(const QString &source, const QString &destination)
{
    if (mAccessToken.isEmpty()) {
        mNextAction->setNextActionType(CopyFile);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken,mAccessTokenSecret,mAccessOauthSignature);
        connect(job, SIGNAL(copyFileDone(QString)), SLOT(slotCopyFileDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->copyFile(source, destination);
    }
}

void DropBoxStorageService::storageServiceCopyFolder(const QString &source, const QString &destination)
{
    if (mAccessToken.isEmpty()) {
        mNextAction->setNextActionType(CopyFolder);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken,mAccessTokenSecret,mAccessOauthSignature);
        connect(job, SIGNAL(copyFolderDone(QString)), SLOT(slotCopyFolderDone(QString)));
        connect(job, SIGNAL(actionFailed(QString)), SLOT(slotActionFailed(QString)));
        job->copyFolder(source, destination);
    }
}

QString DropBoxStorageService::fileIdentifier(const QVariantMap &variantMap)
{
    if (variantMap.contains(QLatin1String("path"))) {
        return variantMap.value(QLatin1String("path")).toString();
    }
    return QString();
}

QString DropBoxStorageService::fileShareRoot(const QVariantMap &variantMap)
{
    if (variantMap.contains(QLatin1String("root"))) {
        return variantMap.value(QLatin1String("root")).toString();
    }
    return QString();
}

StorageServiceAbstract::Capabilities DropBoxStorageService::capabilities() const
{
    return serviceCapabilities();
}

QString DropBoxStorageService::fillListWidget(StorageServiceTreeWidget *listWidget, const QVariant &data, const QString &currentFolder)
{
    Q_UNUSED(currentFolder);
    listWidget->clear();
    QJson::Parser parser;
    bool ok;
    QString parentFolder;
    listWidget->createMoveUpItem();
    QMap<QString, QVariant> info = parser.parse(data.toString().toUtf8(), &ok).toMap();
    if (info.contains(QLatin1String("path"))) {
        const QString path = info.value(QLatin1String("path")).toString();
        if (parentFolder.isEmpty()) {
            if (!path.isEmpty()) {
                if (path == QLatin1String("/")) {
                    parentFolder = path;
                } else {
                    QStringList parts = path.split(QLatin1String("/"));
                    parts.removeLast();
                    parentFolder = parts.join(QLatin1String("/"));
                    if (parentFolder.isEmpty()) {
                        parentFolder = QLatin1String("/");
                    }
                }
            }
        }
    }
    if (info.contains(QLatin1String("contents"))) {
        const QVariantList lst = info.value(QLatin1String("contents")).toList();
        Q_FOREACH (const QVariant &variant, lst) {
            const QVariantMap qwer = variant.toMap();
            //qDebug()<<" qwer "<<qwer;
            if (qwer.contains(QLatin1String("is_dir"))) {
                const bool isDir = qwer.value(QLatin1String("is_dir")).toBool();
                const QString name = qwer.value(QLatin1String("path")).toString();

                const QString itemName = name.right((name.length() - name.lastIndexOf(QLatin1Char('/'))) - 1);


                StorageServiceTreeWidgetItem *item;
                if (isDir) {
                    item = listWidget->addFolder(itemName, name);
                } else {
                    QString mimetype;
                    if (qwer.contains(QLatin1String("mime_type"))) {
                        mimetype = qwer.value(QLatin1String("mime_type")).toString();
                    }
                    item = listWidget->addFile(itemName, name, mimetype);
                    if (qwer.contains(QLatin1String("bytes"))) {
                        item->setSize(qwer.value(QLatin1String("bytes")).toULongLong());
                    }
                }
                if (qwer.contains(QLatin1String("client_mtime"))) {
                    QString tmp = qwer.value(QLatin1String("client_mtime")).toString();
                    item->setDateCreated(PimCommon::DropBoxUtil::convertToDateTime( tmp ));
                }
                if (qwer.contains(QLatin1String("modified"))) {
                    QString tmp = qwer.value(QLatin1String("modified")).toString();
                    item->setLastModification(PimCommon::DropBoxUtil::convertToDateTime( tmp ));
                }
                item->setStoreInfo(qwer);
            }
        }
    }
    return parentFolder;
}

QMap<QString, QString> DropBoxStorageService::itemInformation(const QVariantMap &variantMap)
{
    QMap<QString, QString> information;

    const bool isDir = variantMap.value(QLatin1String("is_dir")).toBool();
    const QString name = variantMap.value(QLatin1String("path")).toString();

    const QString itemName = name.right((name.length() - name.lastIndexOf(QLatin1Char('/'))) - 1);
    information.insert(i18n("Type:"), isDir ? i18n("Folder") : i18n("File"));
    information.insert(i18n("Name:"), itemName);

    if (variantMap.contains(QLatin1String("client_mtime"))) {
        const QString tmp = variantMap.value(QLatin1String("client_mtime")).toString();
        information.insert(i18n("Created"), KGlobal::locale()->formatDateTime(PimCommon::DropBoxUtil::convertToDateTime( tmp )));
    }
    if (variantMap.contains(QLatin1String("modified"))) {
        const QString tmp = variantMap.value(QLatin1String("modified")).toString();
        information.insert(i18n("Last Modified:"), KGlobal::locale()->formatDateTime(PimCommon::DropBoxUtil::convertToDateTime( tmp )));
    }
    if (variantMap.contains(QLatin1String("root"))) {
        information.insert(i18n("Storage path:"), variantMap.value(QLatin1String("root")).toString());
    }
    return information;
}

KIcon DropBoxStorageService::icon() const
{
    return KIcon(iconName());
}

QRegExp DropBoxStorageService::disallowedSymbols() const
{
    return QRegExp(QLatin1String("[/:?*<>\"|]"));
}

QString DropBoxStorageService::disallowedSymbolsStr() const
{
    return QLatin1String("\\ / : ? * < > \" |");
}

qlonglong DropBoxStorageService::maximumUploadFileSize() const
{
    return 150000000;
}

#include "moc_dropboxstorageservice.cpp"
