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
      mNextAction(NoneAction)
{
}

StorageServiceAbstract::~StorageServiceAbstract()
{

}

void StorageServiceAbstract::executeNextAction()
{
    switch(mNextAction) {
    case NoneAction:
        break;
    case RequestToken:
        authentication();
        break;
    case AccessToken:
        break;
    case UploadFile:
        //uploadFile();
        break;
    case CreateFolder:
        //createFolder();
        break;
    case ListFolder:
        listFolder();
        break;
    case AccountInfo:
        accountInfo();
        break;
    case ShareLink:
        //shareLink();
        break;
    case CreateServiceFolder:
        createServiceFolder();
        break;
    case DownLoadFile:
        //downloadFile();
        break;
    case DeleteFile:
        break;
    case DeleteFolder:
        break;
    }
}

void StorageServiceAbstract::slotAccountInfoDone(const PimCommon::AccountInfo &info)
{
    Q_EMIT accountInfoDone(storageServiceName(), info);
}

void StorageServiceAbstract::slotActionFailed(const QString &error)
{
    qDebug()<<" error found "<<error;
    Q_EMIT actionFailed(storageServiceName(), error);
}

void StorageServiceAbstract::slotShareLinkDone(const QString &url)
{
    Q_EMIT shareLinkDone(storageServiceName(), url);
}

void StorageServiceAbstract::slotUploadFileProgress(qint64 done, qint64 total)
{
    Q_EMIT uploadFileProgress(storageServiceName(), done, total);
}

void StorageServiceAbstract::slotCreateFolderDone(const QString &folderName)
{
    Q_EMIT createFolderDone(storageServiceName(), folderName);
}

void StorageServiceAbstract::slotUploadFileDone(const QString &filename)
{
    Q_EMIT uploadFileDone(storageServiceName(), filename);
}

void StorageServiceAbstract::slotListFolderDone(const QStringList &listFolder)
{
    Q_EMIT listFolderDone(storageServiceName(), listFolder);
}

void StorageServiceAbstract::slotDownLoadFileDone(const QString &fileName)
{
    Q_EMIT downLoadFileDone(storageServiceName(), fileName);
}

void StorageServiceAbstract::emitAuthentificationDone()
{
    Q_EMIT authenticationDone(storageServiceName());
    executeNextAction();
}

#include "moc_storageserviceabstract.cpp"
