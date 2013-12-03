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

#include "webdavjob.h"

#include <qjson/parser.h>

#include <QNetworkAccessManager>
#include <QDebug>
#include <QNetworkReply>

using namespace PimCommon;

WebDavJob::WebDavJob(QObject *parent)
    : PimCommon::StorageServiceAbstractJob(parent)
{
    connect(mNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotSendDataFinished(QNetworkReply*)));
}

WebDavJob::~WebDavJob()
{

}

void WebDavJob::requestTokenAccess()
{

}

void WebDavJob::uploadFile(const QString &filename)
{

}

void WebDavJob::listFolder()
{

}

void WebDavJob::accountInfo()
{

}

void WebDavJob::initializeToken(const QString &accessToken, const QString &accessTokenSecret, const QString &accessOauthSignature)
{

}

void WebDavJob::createFolder(const QString &filename)
{

}


void WebDavJob::slotSendDataFinished(QNetworkReply *reply)
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
                break;
            case RequestToken:
                break;
            case AccessToken:
                break;
            case UploadFiles:
                break;
            case CreateFolder:
                break;
            case AccountInfo:
                break;
            case ListFolder:
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
        break;
    case RequestToken:
        break;
    case AccessToken:
        break;
    case UploadFiles:
        break;
    case CreateFolder:
        break;
    case AccountInfo:
        break;
    case ListFolder:
        break;
    default:
        qDebug()<<" Action Type unknown:"<<mActionType;
    }
}
