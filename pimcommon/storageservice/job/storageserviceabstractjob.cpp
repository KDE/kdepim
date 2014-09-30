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

#include "storageserviceabstractjob.h"

#include <KLocalizedString>

#include <QNetworkAccessManager>
#include <QDebug>
#include <QFile>

using namespace PimCommon;

StorageServiceAbstractJob::StorageServiceAbstractJob(QObject *parent)
    : QObject(parent),
      mNetworkAccessManager(new QNetworkAccessManager(this)),
      mActionType(PimCommon::StorageServiceAbstract::NoneAction),
      mError(false)
{
    connect(mNetworkAccessManager, &QNetworkAccessManager::sslErrors, this, &StorageServiceAbstractJob::slotSslErrors);
    qDebug() << "StorageServiceAbstractJob::StorageServiceAbstractJob() " << this;
}

StorageServiceAbstractJob::~StorageServiceAbstractJob()
{
    qDebug() << "StorageServiceAbstractJob::~StorageServiceAbstractJob() " << this;
}

void StorageServiceAbstractJob::slotSslErrors(QNetworkReply *reply, const QList<QSslError> &error)
{
    qDebug() << " void StorageServiceAbstractJob::slotSslErrors(QNetworkReply *reply, const QList<QSslError> &error)" << error.count();
    reply->ignoreSslErrors(error);
}

void StorageServiceAbstractJob::slotError(QNetworkReply::NetworkError error)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    qDebug() << " Error " << error << " reply" << reply->errorString();
    mError = true;
    mErrorMsg = reply->errorString();
}

void StorageServiceAbstractJob::errorMessage(PimCommon::StorageServiceAbstract::ActionType type, const QString &errorStr)
{
    QString error;
    switch (type) {
    case PimCommon::StorageServiceAbstract::NoneAction:
        break;
    case PimCommon::StorageServiceAbstract::RequestTokenAction:
        error = i18n("Request Token returns an error: %1", errorStr);
        break;
    case PimCommon::StorageServiceAbstract::AccessTokenAction:
        error = i18n("Access Token returns an error: %1", errorStr);
        break;
    case PimCommon::StorageServiceAbstract::UploadFileAction:
        error = i18n("Upload File returns an error: %1", errorStr);
        break;
    case PimCommon::StorageServiceAbstract::CreateFolderAction:
        error = i18n("Create Folder returns an error: %1", errorStr);
        break;
    case PimCommon::StorageServiceAbstract::AccountInfoAction:
        error = i18n("Get account info returns an error: %1", errorStr);
        break;
    case PimCommon::StorageServiceAbstract::ListFolderAction:
        error = i18n("List folder returns an error: %1", errorStr);
        break;
    case PimCommon::StorageServiceAbstract::ShareLinkAction:
        error = i18n("Share Link returns an error: %1", errorStr);
        break;
    case PimCommon::StorageServiceAbstract::CreateServiceFolderAction:
        error = i18n("Create Service Folder returns an error: %1", errorStr);
        break;
    case PimCommon::StorageServiceAbstract::DownLoadFileAction:
        error = i18n("Download file returns an error: %1", errorStr);
        break;
    case PimCommon::StorageServiceAbstract::DeleteFileAction:
        error = i18n("Delete File returns an error: %1", errorStr);
        break;
    case PimCommon::StorageServiceAbstract::DeleteFolderAction:
        error = i18n("Delete Folder returns an error: %1", errorStr);
        break;
    case PimCommon::StorageServiceAbstract::RenameFolderAction:
        error = i18n("Rename Folder returns an error: %1", errorStr);
        break;
    case PimCommon::StorageServiceAbstract::RenameFileAction:
        error = i18n("Rename File returns an error: %1", errorStr);
        break;
    case PimCommon::StorageServiceAbstract::MoveFileAction:
        error = i18n("Move File returns an error: %1", errorStr);
        break;
    case PimCommon::StorageServiceAbstract::MoveFolderAction:
        error = i18n("Move Folder returns an error: %1", errorStr);
        break;
    case PimCommon::StorageServiceAbstract::CopyFileAction:
        error = i18n("Copy File returns an error: %1", errorStr);
        break;
    case PimCommon::StorageServiceAbstract::CopyFolderAction:
        error = i18n("Copy Folder returns an error: %1", errorStr);
        break;
    }
    if (!error.isEmpty()) {
        Q_EMIT actionFailed(error);
    }
}

void StorageServiceAbstractJob::slotDownloadReadyRead()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    mDownloadFile->write(reply->readAll());
}

void StorageServiceAbstractJob::slotuploadDownloadFileProgress(qint64 done, qint64 total)
{
    Q_EMIT uploadDownloadFileProgress(done, total);
}
