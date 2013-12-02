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

#include "ubuntuonejob.h"
#include "logindialog.h"

#include <qjson/parser.h>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointer>
#include <QDebug>

using namespace PimCommon;

UbuntuOneJob::UbuntuOneJob(QObject *parent)
    : PimCommon::StorageServiceAbstractJob(parent)
{
    connect(mNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotSendDataFinished(QNetworkReply*)));
}

UbuntuOneJob::~UbuntuOneJob()
{

}

void UbuntuOneJob::requestTokenAccess()
{
    QPointer<LoginDialog> dlg = new LoginDialog;
    QString password;
    QString username;
    if (dlg->exec()) {
        password = dlg->password();
        username = dlg->username();
    }
    delete dlg;
    if (!username.isEmpty()) {
        mActionType = RequestToken;
        QNetworkRequest request(QUrl(QLatin1String("https://login.ubuntu.com/api/1.0/authentications")));
        request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

        QUrl postData;
        QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
    }
}

void UbuntuOneJob::uploadFile(const QString &filename)
{

}

void UbuntuOneJob::listFolder()
{

}

void UbuntuOneJob::accountInfo()
{

}

void UbuntuOneJob::initializeToken(const QString &accessToken, const QString &accessTokenSecret, const QString &accessOauthSignature)
{

}

void UbuntuOneJob::createFolder(const QString &filename)
{

}

void UbuntuOneJob::slotSendDataFinished(QNetworkReply *reply)
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
