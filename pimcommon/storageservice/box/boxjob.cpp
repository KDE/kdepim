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

#include "boxjob.h"

#include <qjson/parser.h>

#include <QNetworkAccessManager>
#include <QDebug>
#include <QNetworkReply>
#include <QPointer>

using namespace PimCommon;

BoxJob::BoxJob(QObject *parent)
    : PimCommon::StorageServiceAbstractJob(parent)
{
    mClientId = QLatin1String("");
    mClientSecret = QLatin1String("");
    mRedirectUri = QLatin1String("https://bugs.kde.org/");
    connect(mNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotSendDataFinished(QNetworkReply*)));
}

BoxJob::~BoxJob()
{

}

void BoxJob::requestTokenAccess()
{
    //TODO
}

void BoxJob::uploadFile(const QString &filename)
{
    mActionType = UploadFiles;
}

void BoxJob::listFolder()
{
    mActionType = ListFolder;
}

void BoxJob::accountInfo()
{
    mActionType = AccountInfo;
}

void BoxJob::createFolder(const QString &filename)
{
    mActionType = CreateFolder;
}

void BoxJob::slotSendDataFinished(QNetworkReply *reply)
{
    const QString data = QString::fromUtf8(reply->readAll());
    reply->deleteLater();
    if (mError) {
        qDebug()<<" error type "<<data;
        QJson::Parser parser;
        bool ok;

        QMap<QString, QVariant> error = parser.parse(data.toUtf8(), &ok).toMap();
        if (error.contains(QLatin1String("error"))) {
            const QString errorStr = error.value(QLatin1String("error")).toString();
            switch(mActionType) {
            case NoneAction:
                deleteLater();
                break;
            case RequestToken:
                deleteLater();
                break;
            case AccessToken:
                errorMessage(mActionType, errorStr);
                deleteLater();
                break;
            case UploadFiles:
                errorMessage(mActionType, errorStr);
                deleteLater();
                break;
            case CreateFolder:
                errorMessage(mActionType, errorStr);
                deleteLater();
                break;
            case AccountInfo:
                errorMessage(mActionType, errorStr);
                deleteLater();
                break;
            case ListFolder:
                errorMessage(mActionType, errorStr);
                deleteLater();
                break;
            default:
                qDebug()<<" Action Type unknown:"<<mActionType;
                deleteLater();
                break;
            }
        }
        return;
    }
    switch(mActionType) {
    case NoneAction:
        deleteLater();
        break;
    case RequestToken:
        deleteLater();
        break;
    case AccessToken:
        deleteLater();
        break;
    case UploadFiles:
        parseUploadFiles(data);
        break;
    case CreateFolder:
        parseCreateFolder(data);
        break;
    case AccountInfo:
        parseAccountInfo(data);
        break;
    case ListFolder:
        parseListFolder(data);
        break;
    default:
        qDebug()<<" Action Type unknown:"<<mActionType;
        deleteLater();
    }
}

void BoxJob::parseUploadFiles(const QString &data)
{
    qDebug()<<" data "<<data;
    deleteLater();
}

void BoxJob::parseCreateFolder(const QString &data)
{
    qDebug()<<" data "<<data;
    deleteLater();
}

void BoxJob::parseAccountInfo(const QString &data)
{
    qDebug()<<" data "<<data;
    deleteLater();
}

void BoxJob::parseListFolder(const QString &data)
{
    qDebug()<<" data "<<data;
    deleteLater();
}


void BoxJob::shareLink(const QString &root, const QString &path)
{

}
