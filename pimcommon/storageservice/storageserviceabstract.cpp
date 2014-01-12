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

bool StorageServiceAbstract::isInProgress() const
{
    return mInProgress;
}

void StorageServiceAbstract::changeProgressState(bool state)
{
    mInProgress = state;
    Q_EMIT inProgress(state);
}

void StorageServiceAbstract::downloadFile(const QString &filename)
{
    if (mInProgress) {
        qDebug()<<" still in progress";
        return;
    }
    changeProgressState(true);
    storageServicedownloadFile(filename);
}

void StorageServiceAbstract::uploadFile(const QString &filename)
{
    if (mInProgress) {
        qDebug()<<" still in progress";
        return;
    }

    changeProgressState(true);
    storageServiceuploadFile(filename);
}

void StorageServiceAbstract::accountInfo()
{
    if (mInProgress) {
        qDebug()<<" still in progress";
        return;
    }
    changeProgressState(true);
    storageServiceaccountInfo();
}

void StorageServiceAbstract::createFolder(const QString &folder)
{
    if (mInProgress) {
        qDebug()<<" still in progress";
        return;
    }
    changeProgressState(true);
    storageServicecreateFolder(folder);
}

void StorageServiceAbstract::listFolder()
{
    if (mInProgress) {
        qDebug()<<" still in progress";
        return;
    }
    changeProgressState(true);
    storageServicelistFolder();
}

void StorageServiceAbstract::authentication()
{
    if (mInProgress) {
        qDebug()<<" still in progress";
        return;
    }
    changeProgressState(true);
    storageServiceauthentication();
}

void StorageServiceAbstract::shareLink(const QString &root, const QString &path)
{
    if (mInProgress) {
        qDebug()<<" still in progress";
        return;
    }
    changeProgressState(true);
    storageServiceShareLink(root, path);
}

void StorageServiceAbstract::createServiceFolder()
{
    if (mInProgress) {
        qDebug()<<" still in progress";
        return;
    }
    changeProgressState(true);
    storageServicecreateServiceFolder();
}

void StorageServiceAbstract::deleteFile(const QString &filename)
{
    if (mInProgress) {
        qDebug()<<" still in progress";
        return;
    }
    changeProgressState(true);
    storageServicedeleteFile(filename);
}

void StorageServiceAbstract::deleteFolder(const QString &foldername)
{
    if (mInProgress) {
        qDebug()<<" still in progress";
        return;
    }
    changeProgressState(true);
    storageServicedeleteFolder(foldername);
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
        storageServiceuploadFile(mNextAction->nextActionFileName());
        break;
    case CreateFolder:
        storageServicecreateFolder(mNextAction->nextActionFolder());
        break;
    case ListFolder:
        storageServicelistFolder();
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
        storageServicedownloadFile(mNextAction->nextActionFileName());
        break;
    case DeleteFile:
        storageServicedeleteFile(mNextAction->nextActionFileName());
        break;
    case DeleteFolder:
        storageServicedeleteFolder(mNextAction->nextActionFolder());
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

void StorageServiceAbstract::slotUploadFileProgress(qint64 done, qint64 total)
{
    Q_EMIT uploadFileProgress(storageServiceName(), done, total);
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

void StorageServiceAbstract::slotListFolderDone(const QStringList &listFolder)
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

void StorageServiceAbstract::emitAuthentificationDone()
{
    Q_EMIT authenticationDone(storageServiceName());
    if (mNextAction->nextActionType() != NoneAction)
        executeNextAction();
    else {
        changeProgressState(false);
    }
}

#include "moc_storageserviceabstract.cpp"
