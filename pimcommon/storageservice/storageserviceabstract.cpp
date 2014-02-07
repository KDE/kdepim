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

#include "storageserviceabstract.h"
#include <QDebug>

using namespace PimCommon;

StorageServiceAbstract::StorageServiceAbstract(QObject *parent)
    : QObject(parent),
      mNextAction(new NextAction),
      mInProgress(false)
{
}

StorageServiceAbstract::~StorageServiceAbstract()
{
    delete mNextAction;
}

bool StorageServiceAbstract::hasUploadOrDownloadInProgress() const
{
    return (mUploadReply || mDownloadReply);
}

void StorageServiceAbstract::cancelUploadDownloadFile()
{
    cancelUploadFile();
    cancelDownloadFile();
}

void StorageServiceAbstract::cancelUploadFile()
{
    if (mUploadReply) {
        mUploadReply->abort();
    }
}

void StorageServiceAbstract::cancelDownloadFile()
{
    if (mDownloadReply) {
        mDownloadReply->abort();
    }
}

bool StorageServiceAbstract::isInProgress() const
{
    return mInProgress;
}

void StorageServiceAbstract::changeProgressState(bool state)
{
    mInProgress = state;
    Q_EMIT inProgress(state);
}

void StorageServiceAbstract::downloadFile(const QString &name, const QString &fileId, const QString &destination)
{
    if (mInProgress) {
        qDebug()<<"downloadFile: still in progress";
        return;
    }
    changeProgressState(true);
    storageServicedownloadFile(name, fileId, destination);
}

void StorageServiceAbstract::uploadFile(const QString &filename, const QString &uploadAsName, const QString &destination)
{
    if (mInProgress) {
        qDebug()<<"uploadFile: still in progress";
        return;
    }

    changeProgressState(true);
    storageServiceuploadFile(filename, uploadAsName, destination);
}

void StorageServiceAbstract::accountInfo()
{
    if (mInProgress) {
        qDebug()<<"accountInfo: still in progress";
        return;
    }
    changeProgressState(true);
    storageServiceaccountInfo();
}

void StorageServiceAbstract::createFolder(const QString &foldername, const QString &destination)
{
    if (mInProgress) {
        qDebug()<<"createFolder: still in progress";
        return;
    }
    changeProgressState(true);
    storageServicecreateFolder(foldername, destination);
}

void StorageServiceAbstract::listFolder(const QString &folder)
{
    if (mInProgress) {
        qDebug()<<"listFolder; still in progress";
        return;
    }
    changeProgressState(true);
    storageServicelistFolder(folder);
}

void StorageServiceAbstract::authentication()
{
    if (mInProgress) {
        qDebug()<<"authentication: still in progress";
        return;
    }
    changeProgressState(true);
    storageServiceauthentication();
}

void StorageServiceAbstract::shareLink(const QString &root, const QString &path)
{
    if (mInProgress) {
        qDebug()<<"shareLink: still in progress";
        return;
    }
    changeProgressState(true);
    storageServiceShareLink(root, path);
}

void StorageServiceAbstract::createServiceFolder()
{
    if (mInProgress) {
        qDebug()<<"createServiceFolder: still in progress";
        return;
    }
    changeProgressState(true);
    storageServicecreateServiceFolder();
}

void StorageServiceAbstract::deleteFile(const QString &filename)
{
    if (mInProgress) {
        qDebug()<<"deleteFile: still in progress";
        return;
    }
    changeProgressState(true);
    storageServicedeleteFile(filename);
}

void StorageServiceAbstract::deleteFolder(const QString &foldername)
{
    if (mInProgress) {
        qDebug()<<"deleteFolder: still in progress";
        return;
    }
    changeProgressState(true);
    storageServicedeleteFolder(foldername);
}

void StorageServiceAbstract::renameFolder(const QString &source, const QString &destination)
{
    if (mInProgress) {
        qDebug()<<"renameFolder: still in progress";
        return;
    }
    changeProgressState(true);
    storageServiceRenameFolder(source, destination);
}

void StorageServiceAbstract::renameFile(const QString &source, const QString &destination)
{
    if (mInProgress) {
        qDebug()<<"renameFile: still in progress";
        return;
    }
    changeProgressState(true);
    storageServiceRenameFile(source, destination);
}

void StorageServiceAbstract::moveFile(const QString &source, const QString &destination)
{
    if (mInProgress) {
        qDebug()<<"moveFile: still in progress";
        return;
    }
    changeProgressState(true);
    storageServiceMoveFile(source, destination);
}

void StorageServiceAbstract::moveFolder(const QString &source, const QString &destination)
{
    if (mInProgress) {
        qDebug()<<"moveFolder: still in progress";
        return;
    }
    changeProgressState(true);
    storageServiceMoveFolder(source, destination);
}

void StorageServiceAbstract::copyFile(const QString &source, const QString &destination)
{
    if (mInProgress) {
        qDebug()<<"copyFile: still in progress";
        return;
    }
    changeProgressState(true);
    storageServiceCopyFile(source, destination);
}

void StorageServiceAbstract::copyFolder(const QString &source, const QString &destination)
{
    if (mInProgress) {
        qDebug()<<"copyFolder: still in progress";
        return;
    }
    changeProgressState(true);
    storageServiceCopyFolder(source, destination);
}

QRegExp StorageServiceAbstract::disallowedSymbols() const
{
    return QRegExp();
}

QString StorageServiceAbstract::disallowedSymbolsStr() const
{
    return QString();
}

qlonglong StorageServiceAbstract::maximumUploadFileSize() const
{
    return -1;
}

void StorageServiceAbstract::executeNextAction()
{
    switch(mNextAction->nextActionType()) {
    case NoneAction:
        break;
    case RequestToken:
        storageServiceauthentication();
        break;
    case AccessToken:
        break;
    case UploadFile:
        storageServiceuploadFile(mNextAction->nextActionName(), mNextAction->uploadAsName(), mNextAction->nextActionFolder());
        break;
    case CreateFolder:
        storageServicecreateFolder(mNextAction->nextActionName(), mNextAction->nextActionFolder());
        break;
    case ListFolder:
        storageServicelistFolder(mNextAction->nextActionFolder());
        break;
    case AccountInfo:
        storageServiceaccountInfo();
        break;
    case ShareLink:
        storageServiceShareLink(mNextAction->rootPath(), mNextAction->path());
        break;
    case CreateServiceFolder:
        storageServicecreateServiceFolder();
        break;
    case DownLoadFile:
        storageServicedownloadFile(mNextAction->nextActionName(), mNextAction->fileId(), mNextAction->downloadDestination());
        break;
    case DeleteFile:
        storageServicedeleteFile(mNextAction->nextActionName());
        break;
    case DeleteFolder:
        storageServicedeleteFolder(mNextAction->nextActionFolder());
        break;
    case RenameFolder:
        storageServiceRenameFolder(mNextAction->renameSource(), mNextAction->renameDestination());
        break;
    case RenameFile:
        storageServiceRenameFile(mNextAction->renameSource(), mNextAction->renameDestination());
        break;
    case MoveFile:
        storageServiceMoveFile(mNextAction->renameSource(), mNextAction->renameDestination());
        break;
    case MoveFolder:
        storageServiceMoveFolder(mNextAction->renameSource(), mNextAction->renameDestination());
        break;
    case CopyFile:
        storageServiceCopyFile(mNextAction->renameSource(), mNextAction->renameDestination());
        break;
    case CopyFolder:
        storageServiceCopyFolder(mNextAction->renameSource(), mNextAction->renameDestination());
        break;
    }
}

void StorageServiceAbstract::slotDeleteFolderDone(const QString &folder)
{
    Q_EMIT deleteFolderDone(storageServiceName(), folder);
    changeProgressState(false);
}

void StorageServiceAbstract::slotDeleteFileDone(const QString &filename)
{
    Q_EMIT deleteFileDone(storageServiceName(), filename);
    changeProgressState(false);
}

void StorageServiceAbstract::slotAccountInfoDone(const PimCommon::AccountInfo &info)
{
    Q_EMIT accountInfoDone(storageServiceName(), info);
    changeProgressState(false);
}

void StorageServiceAbstract::slotActionFailed(const QString &error)
{
    //qDebug()<<" error found "<<error;
    Q_EMIT actionFailed(storageServiceName(), error);
    changeProgressState(false);
}

void StorageServiceAbstract::slotShareLinkDone(const QString &url)
{
    Q_EMIT shareLinkDone(storageServiceName(), url);
    changeProgressState(false);
}

void StorageServiceAbstract::slotuploadDownloadFileProgress(qint64 done, qint64 total)
{
    Q_EMIT uploadDownloadFileProgress(storageServiceName(), done, total);
    changeProgressState(false);
}

void StorageServiceAbstract::slotCreateFolderDone(const QString &folderName)
{
    Q_EMIT createFolderDone(storageServiceName(), folderName);
    changeProgressState(false);
}

void StorageServiceAbstract::slotUploadFileDone(const QString &filename)
{
    Q_EMIT uploadFileDone(storageServiceName(), filename);
    changeProgressState(false);
}

void StorageServiceAbstract::slotListFolderDone(const QVariant &listFolder)
{
    Q_EMIT listFolderDone(storageServiceName(), listFolder);
    changeProgressState(false);
}

void StorageServiceAbstract::slotDownLoadFileDone(const QString &fileName)
{
    Q_EMIT downLoadFileDone(storageServiceName(), fileName);
    changeProgressState(false);
}

void StorageServiceAbstract::emitAuthentificationFailder(const QString &errorMessage)
{
    Q_EMIT authenticationFailed(storageServiceName(), errorMessage);
    changeProgressState(false);
}

void StorageServiceAbstract::slotRenameFolderDone(const QString &folderName)
{
    Q_EMIT renameFolderDone(storageServiceName(), folderName);
    changeProgressState(false);
}

void StorageServiceAbstract::slotRenameFileDone(const QString &filename)
{
    Q_EMIT renameFileDone(storageServiceName(), filename);
    changeProgressState(false);
}

void StorageServiceAbstract::slotMoveFolderDone(const QString &folderName)
{
    Q_EMIT moveFolderDone(storageServiceName(), folderName);
    changeProgressState(false);
}

void StorageServiceAbstract::slotMoveFileDone(const QString &filename)
{
    Q_EMIT moveFileDone(storageServiceName(), filename);
    changeProgressState(false);
}

void StorageServiceAbstract::slotCopyFileDone(const QString &filename)
{
    Q_EMIT copyFileDone(storageServiceName(), filename);
    changeProgressState(false);
}

void StorageServiceAbstract::slotCopyFolderDone(const QString &filename)
{
    Q_EMIT copyFolderDone(storageServiceName(), filename);
    changeProgressState(false);
}

void StorageServiceAbstract::emitAuthentificationDone()
{
    Q_EMIT authenticationDone(storageServiceName());
    if (mNextAction->nextActionType() != NoneAction)
        executeNextAction();
    else {
        changeProgressState(false);
    }
}

void StorageServiceAbstract::slotDownLoadFileFailed(const QString &filename)
{
    Q_EMIT downLoadFileFailed(storageServiceName(), filename);
    changeProgressState(false);
}

void StorageServiceAbstract::slotUploadFileFailed(const QString &filename)
{
    Q_EMIT downLoadFileFailed(storageServiceName(), filename);
    changeProgressState(false);
}

#include "moc_storageserviceabstract.cpp"
