/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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
      mNextAction(NoneAction),
      mInProgress(false)
{
}

StorageServiceAbstract::~StorageServiceAbstract()
{

}

bool StorageServiceAbstract::isInProgress() const
{
    return mInProgress;
}

void StorageServiceAbstract::downloadFile(const QString &filename)
{
    if (mInProgress) {
        qDebug()<<" still in progress";
        return;
    }
    mInProgress = true;
    storageServicedownloadFile(filename);
}

void StorageServiceAbstract::uploadFile(const QString &filename)
{
    if (mInProgress) {
        qDebug()<<" still in progress";
        return;
    }

    mInProgress = true;
    storageServiceuploadFile(filename);
}

void StorageServiceAbstract::accountInfo()
{
    if (mInProgress) {
        qDebug()<<" still in progress";
        return;
    }
    mInProgress = true;
    storageServiceaccountInfo();
}

void StorageServiceAbstract::createFolder(const QString &folder)
{
    if (mInProgress) {
        qDebug()<<" still in progress";
        return;
    }
    mInProgress = true;
    storageServicecreateFolder(folder);
}

void StorageServiceAbstract::listFolder()
{
    if (mInProgress) {
        qDebug()<<" still in progress";
        return;
    }
    mInProgress = true;
    storageServicelistFolder();
}

void StorageServiceAbstract::authentication()
{
    if (mInProgress) {
        qDebug()<<" still in progress";
        return;
    }
    mInProgress = true;
    storageServiceauthentication();
}

void StorageServiceAbstract::shareLink(const QString &root, const QString &path)
{
    if (mInProgress) {
        qDebug()<<" still in progress";
        return;
    }
    mInProgress = true;
    storageServiceShareLink(root, path);
}

void StorageServiceAbstract::createServiceFolder()
{
    if (mInProgress) {
        qDebug()<<" still in progress";
        return;
    }
    mInProgress = true;
    storageServicecreateServiceFolder();
}

void StorageServiceAbstract::deleteFile(const QString &filename)
{
    if (mInProgress) {
        qDebug()<<" still in progress";
        return;
    }
    mInProgress = true;
    storageServicedeleteFile(filename);
}

void StorageServiceAbstract::deleteFolder(const QString &foldername)
{
    if (mInProgress) {
        qDebug()<<" still in progress";
        return;
    }
    mInProgress = true;
    storageServicedeleteFolder(foldername);
}

void StorageServiceAbstract::executeNextAction()
{
    switch(mNextAction) {
    case NoneAction:
        break;
    case RequestToken:
        storageServiceauthentication();
        break;
    case AccessToken:
        break;
    case UploadFile:
        //storageServiceuploadFile();
        break;
    case CreateFolder:
        //storageServicecreateFolder();
        break;
    case ListFolder:
        storageServicelistFolder();
        break;
    case AccountInfo:
        storageServiceaccountInfo();
        break;
    case ShareLink:
        //storageServiceshareLink();
        break;
    case CreateServiceFolder:
        storageServicecreateServiceFolder();
        break;
    case DownLoadFile:
        //storageServicedownloadFile();
        break;
    case DeleteFile:
        //storageServicedeleteFile();
        break;
    case DeleteFolder:
        //storageServicedeleteFolder();
        break;
    }
}

void StorageServiceAbstract::slotDeleteFolderDone(const QString &folder)
{
    Q_EMIT deleteFolderDone(storageServiceName(), folder);
    mInProgress = false;
}

void StorageServiceAbstract::slotDeleteFileDone(const QString &filename)
{
    Q_EMIT deleteFileDone(storageServiceName(), filename);
    mInProgress = false;
}

void StorageServiceAbstract::slotAccountInfoDone(const PimCommon::AccountInfo &info)
{
    Q_EMIT accountInfoDone(storageServiceName(), info);
    mInProgress = false;
}

void StorageServiceAbstract::slotActionFailed(const QString &error)
{
    //qDebug()<<" error found "<<error;
    Q_EMIT actionFailed(storageServiceName(), error);
    mInProgress = false;
}

void StorageServiceAbstract::slotShareLinkDone(const QString &url)
{
    Q_EMIT shareLinkDone(storageServiceName(), url);
    mInProgress = false;
}

void StorageServiceAbstract::slotUploadFileProgress(qint64 done, qint64 total)
{
    Q_EMIT uploadFileProgress(storageServiceName(), done, total);
    mInProgress = false;
}

void StorageServiceAbstract::slotCreateFolderDone(const QString &folderName)
{
    Q_EMIT createFolderDone(storageServiceName(), folderName);
    mInProgress = false;
}

void StorageServiceAbstract::slotUploadFileDone(const QString &filename)
{
    Q_EMIT uploadFileDone(storageServiceName(), filename);
    mInProgress = false;
}

void StorageServiceAbstract::slotListFolderDone(const QStringList &listFolder)
{
    Q_EMIT listFolderDone(storageServiceName(), listFolder);
    mInProgress = false;
}

void StorageServiceAbstract::slotDownLoadFileDone(const QString &fileName)
{
    Q_EMIT downLoadFileDone(storageServiceName(), fileName);
    mInProgress = false;
}

void StorageServiceAbstract::emitAuthentificationFailder(const QString &errorMessage)
{
    Q_EMIT authenticationFailed(storageServiceName(), errorMessage);
    mInProgress = false;
}

void StorageServiceAbstract::emitAuthentificationDone()
{
    Q_EMIT authenticationDone(storageServiceName());
    if (mNextAction != NoneAction)
        executeNextAction();
    else
        mInProgress = false;
}

#include "moc_storageserviceabstract.cpp"
