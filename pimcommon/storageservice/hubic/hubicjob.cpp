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

#include "hubicjob.h"
#include "storageservice/storageauthviewdialog.h"

#include <qjson/parser.h>

#include <QNetworkAccessManager>
#include <QDebug>
#include <QNetworkReply>
#include <QPointer>

using namespace PimCommon;

HubicJob::HubicJob(QObject *parent)
    : PimCommon::StorageServiceAbstractJob(parent),
      mExpireInTime(0)
{
    mClientId = QLatin1String("api_hubic_zBKQ6UDUj2vDT7ciDsgjmXA78OVDnzJi");
    mClientSecret = QLatin1String("pkChgk2sRrrCEoVHmYYCglEI9E2Y2833Te5Vn8n2J6qPdxLU6K8NPUvzo1mEhyzf");
    mRedirectUri = QLatin1String("https://bugs.kde.org/");
    connect(mNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotSendDataFinished(QNetworkReply*)));
}

HubicJob::~HubicJob()
{

}

void HubicJob::initializeToken(const QString &refreshToken)
{
    mRefreshToken = refreshToken;
}

void HubicJob::requestTokenAccess()
{
    mActionType = RequestToken;
    QUrl url(QLatin1String("https://api.hubic.com/oauth/auth/"));
    url.addQueryItem(QLatin1String("response_type"), QLatin1String("code"));
    url.addQueryItem(QLatin1String("client_id"), mClientId);
    url.addQueryItem(QLatin1String("redirect_uri"), mRedirectUri);
    url.addQueryItem(QLatin1String("scope"),QLatin1String("account.r,links.rw"));
    mAuthUrl = url;
    qDebug()<<" url"<<url;
    delete mAuthDialog;
    mAuthDialog = new PimCommon::StorageAuthViewDialog;
    connect(mAuthDialog, SIGNAL(urlChanged(QUrl)), this, SLOT(slotRedirect(QUrl)));
    mAuthDialog->setUrl(url);
    if (mAuthDialog->exec()) {
        //TODO
    } else {
        //TODO
    }
    delete mAuthDialog;
}

void HubicJob::slotRedirect(const QUrl &url)
{
    if (url != mAuthUrl) {
        qDebug()<<" Redirect !"<<url;
        mAuthDialog->accept();
        parseRedirectUrl(url);
    }
}

void HubicJob::parseRedirectUrl(const QUrl &url)
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
        //TODO emit error signal
        deleteLater();
    }
}

void HubicJob::getTokenAccess(const QString &authorizeCode)
{
    mActionType = AccessToken;
    QNetworkRequest request(QUrl(QLatin1String("https://api.hubic.com/oauth/token/")));
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

void HubicJob::uploadFile(const QString &filename)
{

}

void HubicJob::listFolder()
{

}

void HubicJob::accountInfo()
{
    mActionType = AccountInfo;
    QNetworkRequest request(QUrl(QLatin1String("https://api.hubic.com/oauth/account/")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    //FIXME
    /*
    QUrl postData;
    postData.addQueryItem(QLatin1String("redirect_uri"), mRedirectUri);
    postData.addQueryItem(QLatin1String("grant_type"), QLatin1String("authorization_code"));
    postData.addQueryItem(QLatin1String("client_id"), mClientId);
    postData.addQueryItem(QLatin1String("client_secret"), mClientSecret);
    QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
*/
}

void HubicJob::createFolder(const QString &filename)
{

}

void HubicJob::shareLink(const QString &root, const QString &path)
{

}


void HubicJob::slotSendDataFinished(QNetworkReply *reply)
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
                Q_EMIT authorizationFailed();
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
    qDebug()<<" data: "<<data;
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
        parseUploadFile(data);
        break;
    case CreateFolder:
        parseCreateFolder(data);
        break;
    case AccountInfo:
        parseAccountInfo(data);
        break;
    case ListFolder:
        deleteLater();
        break;
    default:
        qDebug()<<" Action Type unknown:"<<mActionType;
        deleteLater();
        break;
    }
}

void HubicJob::parseAccountInfo(const QString &data)
{
    //TODO
    deleteLater();
}

void HubicJob::parseCreateFolder(const QString &data)
{
    //TODO
    Q_EMIT createFolderDone();
    deleteLater();
}

void HubicJob::parseUploadFile(const QString &data)
{
    //TODO
    Q_EMIT uploadFileDone();
    deleteLater();
}


void HubicJob::parseAccessToken(const QString &data)
{
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
    //TODO save it.
    deleteLater();
}


void HubicJob::refreshToken()
{
    mActionType = AccessToken;
    QNetworkRequest request(QUrl(QLatin1String("https://api.hubic.com/oauth/token")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

    QUrl postData;

    postData.addQueryItem(QLatin1String("refresh_token"), mRefreshToken);
    postData.addQueryItem(QLatin1String("grant_type"), QLatin1String("refresh_token"));
    postData.addQueryItem(QLatin1String("client_id"), mClientId);
    postData.addQueryItem(QLatin1String("client_secret"), mClientSecret);
    qDebug()<<"https://api.hubic.com/oauth/token "<<postData;

    QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));

}
