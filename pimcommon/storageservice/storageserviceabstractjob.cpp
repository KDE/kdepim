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

#include "storageserviceabstractjob.h"

#include <KLocalizedString>

#include <QNetworkAccessManager>
#include <QDebug>

using namespace PimCommon;

StorageServiceAbstractJob::StorageServiceAbstractJob(QObject *parent)
    : QObject(parent),
      mNetworkAccessManager(new QNetworkAccessManager(this)),
      mActionType(PimCommon::StorageServiceAbstract::NoneAction),
      mError(false)
{
    qDebug()<<"StorageServiceAbstractJob::StorageServiceAbstractJob() "<<this;
}

StorageServiceAbstractJob::~StorageServiceAbstractJob()
{
    qDebug()<<"StorageServiceAbstractJob::~StorageServiceAbstractJob() "<<this;
}

void StorageServiceAbstractJob::slotError(QNetworkReply::NetworkError error)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    qDebug()<<" Error "<<error<<" reply"<<reply->errorString();
    mError = true;
}

void StorageServiceAbstractJob::errorMessage(PimCommon::StorageServiceAbstract::ActionType type, const QString &errorStr)
{
    QString error;
    switch(type) {
    case PimCommon::StorageServiceAbstract::NoneAction:
        break;
    case PimCommon::StorageServiceAbstract::RequestToken:
        error = i18n("Request Token returns an error: %1",errorStr);
        break;
    case PimCommon::StorageServiceAbstract::AccessToken:
        error = i18n("Access Token returns an error: %1",errorStr);
        break;
    case PimCommon::StorageServiceAbstract::UploadFile:
        error = i18n("Upload File returns an error: %1",errorStr);
        break;
    case PimCommon::StorageServiceAbstract::CreateFolder:
        error = i18n("Create Folder returns an error: %1",errorStr);
        break;
    case PimCommon::StorageServiceAbstract::AccountInfo:
        error = i18n("Get account info returns an error: %1",errorStr);
        break;
    case PimCommon::StorageServiceAbstract::ListFolder:
        error = i18n("List folder returns an error: %1",errorStr);
        break;
    case PimCommon::StorageServiceAbstract::ShareLink:
        error = i18n("Share Link returns an error: %1",errorStr);
        break;
    case PimCommon::StorageServiceAbstract::CreateServiceFolder:
        error = i18n("Create Service Folder returns an error: %1",errorStr);
        break;
    case PimCommon::StorageServiceAbstract::DownLoadFile:
        error = i18n("Download file returns an error: %1",errorStr);
        break;
    default:
        break;
    }
    if (!error.isEmpty())
        Q_EMIT actionFailed(error);
}
