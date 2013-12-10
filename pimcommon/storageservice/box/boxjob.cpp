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
#include "storageservice/storageauthviewdialog.h"
#include <qjson/parser.h>

#include <KLocalizedString>

#include <QNetworkAccessManager>
#include <QDebug>
#include <QNetworkReply>
#include <QPointer>

using namespace PimCommon;

BoxJob::BoxJob(QObject *parent)
    : PimCommon::StorageServiceAbstractJob(parent),
      mExpireInTime(0)
{
    mClientId = QLatin1String("o4sn4e0dvz50pd3ps6ao3qxehvqv8dyo");
    mClientSecret = QLatin1String("wLdaOgrblYzi1Y6WN437wStvqighmSJt");
    mRedirectUri = QLatin1String("https://bugs.kde.org/");
    connect(mNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotSendDataFinished(QNetworkReply*)));
}

BoxJob::~BoxJob()
{

}

void BoxJob::initializeToken(const QString &refreshToken)
{
    mRefreshToken = refreshToken;
}

void BoxJob::requestTokenAccess()
{
    mActionType = RequestToken;
    QUrl url(QLatin1String("https://app.box.com/api/oauth2/authorize/"));
    url.addQueryItem(QLatin1String("response_type"), QLatin1String("code"));
    url.addQueryItem(QLatin1String("client_id"), mClientId);
    url.addQueryItem(QLatin1String("redirect_uri"), mRedirectUri);
    mAuthUrl = url;
    qDebug()<<" url"<<url;
    delete mAuthDialog;
    mAuthDialog = new PimCommon::StorageAuthViewDialog;
    connect(mAuthDialog, SIGNAL(urlChanged(QUrl)), this, SLOT(slotRedirect(QUrl)));
    mAuthDialog->setUrl(url);
    if (mAuthDialog->exec()) {
        delete mAuthDialog;
    } else {
        Q_EMIT authorizationFailed(i18n("Authorization Canceled."));
        deleteLater();
        delete mAuthDialog;
    }
}

void BoxJob::slotRedirect(const QUrl &url)
{
    if (url != mAuthUrl) {
        qDebug()<<" Redirect !"<<url;
        mAuthDialog->accept();
        parseRedirectUrl(url);
    }
}


void BoxJob::parseRedirectUrl(const QUrl &url)
{
    const QList<QPair<QString, QString> > listQuery = url.queryItems();
    qDebug()<< "listQuery "<<listQuery;

    QString authorizeCode;
    for (int i = 0; i < listQuery.size(); ++i) {
        const QPair<QString, QString> item = listQuery.at(i);
        if (item.first == QLatin1String("code")) {
            authorizeCode = item.second;
            break;
        } else if (item.first == QLatin1String("error")) {
            //TODO
            //parse error.

        }
    }
    if (!authorizeCode.isEmpty()) {
        getTokenAccess(authorizeCode);
    } else {
        //TODO?
        Q_EMIT authorizationFailed(QString());
        deleteLater();
    }

}

void BoxJob::getTokenAccess(const QString &authorizeCode)
{
    mActionType = AccessToken;
    QNetworkRequest request(QUrl(QLatin1String("https://app.box.com/api/oauth2/token/")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    QUrl postData;
    postData.addQueryItem(QLatin1String("code"), authorizeCode);
    postData.addQueryItem(QLatin1String("redirect_uri"), mRedirectUri);
    postData.addQueryItem(QLatin1String("grant_type"), QLatin1String("authorization_code"));
    postData.addQueryItem(QLatin1String("client_id"), mClientId);
    postData.addQueryItem(QLatin1String("client_secret"), mClientSecret);
    QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
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
        parseAccessToken(data);
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

void BoxJob::parseAccessToken(const QString &data)
{
    qDebug()<<" data "<<data;
    QJson::Parser parser;
    bool ok;

    const QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    qDebug()<<" info"<<info;
    if (info.contains(QLatin1String("refresh_token"))) {
        mRefreshToken = info.value(QLatin1String("refresh_token")).toString();
    }
    if (info.contains(QLatin1String("access_token"))) {
        mToken = info.value(QLatin1String("access_token")).toString();
    }
    if (info.contains(QLatin1String("expires_in"))) {
        mExpireInTime = info.value(QLatin1String("expires_in")).toLongLong();
    }
    Q_EMIT authorizationDone(mRefreshToken);

    deleteLater();
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
