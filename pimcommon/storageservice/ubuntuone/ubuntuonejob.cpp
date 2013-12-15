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
#include "pimcommon/storageservice/logindialog.h"
#include "pimcommon/storageservice/storageserviceutils.h"

#include <KLocalizedString>

#include <qjson/parser.h>

#include <QNetworkAccessManager>
#include <QAuthenticator>
#include <QPointer>
#include <QDebug>

using namespace PimCommon;

UbuntuOneJob::UbuntuOneJob(QObject *parent)
    : PimCommon::StorageServiceAbstractJob(parent)
{
    mAttachmentVolume = QLatin1String("/~/KMail Attachments");
    mOauthVersion = QLatin1String("1.0");
    mOauthSignatureMethod = QLatin1String("PLAINTEXT");
    mNonce = PimCommon::StorageServiceUtils::generateNonce(8);
    mTimestamp = QString::number(QDateTime::currentMSecsSinceEpoch()/1000);
    connect(mNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotSendDataFinished(QNetworkReply*)));
    connect(mNetworkAccessManager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), SLOT(slotAuthenticationRequired(QNetworkReply*,QAuthenticator*)));
}

UbuntuOneJob::~UbuntuOneJob()
{

}

void UbuntuOneJob::requestTokenAccess()
{
    mActionType = RequestToken;
    mError = false;
    QUrl url(QLatin1String("https://login.ubuntu.com/api/1.0/authentications"));
    url.addQueryItem(QLatin1String("ws.op"), QLatin1String("authenticate"));
    url.addQueryItem(QLatin1String("token_name"), QLatin1String("Ubuntu One @ foo") );
    qDebug()<<" postData "<<url;
    QNetworkRequest request(url);
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void UbuntuOneJob::uploadFile(const QString &filename)
{
    mActionType = UploadFiles;
    mError = false;
    qDebug()<<" upload file not implemented";
    deleteLater();
}

void UbuntuOneJob::listFolder(const QString &folder)
{
    mActionType = ListFolder;
    mError = false;
    QUrl url(QLatin1String("https://one.ubuntu.com/api/file_storage/v1"));
    url.addQueryItem(QLatin1String("oauth_consumer_key"), mCustomerKey);
    url.addQueryItem(QLatin1String("oauth_nonce"), mNonce);

    const QString mAccessOauth = mCustomerSecret + QLatin1String("%26") + mTokenSecret;

    url.addQueryItem(QLatin1String("oauth_signature"), mAccessOauth);
    url.addQueryItem(QLatin1String("oauth_signature_method"), mOauthSignatureMethod);
    url.addQueryItem(QLatin1String("oauth_timestamp"), mTimestamp);
    url.addQueryItem(QLatin1String("oauth_version"), mOauthVersion);
    url.addQueryItem(QLatin1String("oauth_token"), mToken);
    QNetworkRequest request(url);
    qDebug()<<" url "<<url;
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void UbuntuOneJob::accountInfo()
{
    mActionType = AccountInfo;
    mError = false;
    QUrl url(QLatin1String("https://one.ubuntu.com/api/quota/"));
    url.addQueryItem(QLatin1String("oauth_consumer_key"), mCustomerKey);
    url.addQueryItem(QLatin1String("oauth_nonce"), mNonce);

    const QString mAccessOauth = mCustomerSecret + QLatin1String("%26") + mTokenSecret;

    url.addQueryItem(QLatin1String("oauth_signature"), mAccessOauth);
    url.addQueryItem(QLatin1String("oauth_signature_method"), mOauthSignatureMethod);
    url.addQueryItem(QLatin1String("oauth_timestamp"), mTimestamp);
    url.addQueryItem(QLatin1String("oauth_version"), mOauthVersion);
    url.addQueryItem(QLatin1String("oauth_token"), mToken);
    QNetworkRequest request(url);
    qDebug()<<" url "<<url;
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void UbuntuOneJob::createFolder(const QString &foldername)
{
    mActionType = CreateFolder;
    mError = false;
    if (foldername.isEmpty()) {
        qDebug()<<" foldername is empty";
    }
    QNetworkRequest request(QUrl(QLatin1String("https://one.ubuntu.com/api/file_storage/v1/volumes/~/") + foldername));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    QUrl postData;
    const QString mAccessOauth = mCustomerSecret + QLatin1String("%26") + mTokenSecret;

    postData.addQueryItem(QLatin1String("oauth_signature"), mAccessOauth);
    postData.addQueryItem(QLatin1String("token"), mToken);
    postData.addQueryItem(QLatin1String("token_secret"), mTokenSecret);
    postData.addQueryItem(QLatin1String("consumer_secret"), mCustomerSecret);
    postData.addQueryItem(QLatin1String("consumer_key"), mCustomerKey);
    QNetworkReply *reply = mNetworkAccessManager->put(request, postData.encodedQuery());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void UbuntuOneJob::shareLink(const QString &root, const QString &path)
{
    mActionType = ShareLink;
    mError = false;
    qDebug()<<" not implemented";
    deleteLater();
}

void UbuntuOneJob::initializeToken(const QString &customerSecret, const QString &token, const QString &customerKey, const QString &tokenSecret)
{
    mCustomerSecret = customerSecret;
    mToken = token;
    mCustomerKey = customerKey;
    mTokenSecret = tokenSecret;
}

void UbuntuOneJob::slotSendDataFinished(QNetworkReply *reply)
{
    const QString data = QString::fromUtf8(reply->readAll());
    reply->deleteLater();
    if (mError) {
        qDebug()<<" error type "<<data;

        const QString errorStr = data;
        switch(mActionType) {
        case NoneAction:
            deleteLater();
            break;
        case RequestToken:
            Q_EMIT authorizationFailed(errorStr);
            deleteLater();
            break;
        case AccessToken:
            Q_EMIT authorizationFailed(errorStr);
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
        case CreateServiceFolder:
            errorMessage(mActionType, errorStr);
            deleteLater();
            break;
        default:
            qDebug()<<" Action Type unknown:"<<mActionType;
            deleteLater();
            break;
        }
        return;
    }
    qDebug()<<" Data ? "<<data;
    switch(mActionType) {
    case NoneAction:
        deleteLater();
        break;
    case RequestToken:
        parseRequestToken(data);
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
    case CreateServiceFolder:
        //TODO
        deleteLater();
        break;
    default:
        qDebug()<<" Action Type unknown:"<<mActionType;
    }
}

void UbuntuOneJob::parseListFolder(const QString &data)
{
    qDebug()<<" data "<<data;
    QJson::Parser parser;
    bool ok;

    QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    qDebug()<<" info "<<info;
    if (info.contains(QLatin1String("user_node_paths"))) {
        qDebug()<<" list folder "<<info.value(QLatin1String("user_node_paths"));
        QVariantList lst = info.value(QLatin1String("user_node_paths")).toList();
        if (lst.contains(mAttachmentVolume)) {
            qDebug()<<" Has kmail folder";
        } else {
            qDebug()<<"doesn't have kmail folder";
            //FIXME create folder !
        }
    }
    //TODO
    Q_EMIT listFolderDone(QStringList());
    deleteLater();
}

void UbuntuOneJob::createServiceFolder()
{
    mActionType = CreateServiceFolder;
    QNetworkRequest request(QUrl(QLatin1String("https://one.ubuntu.com/api/file_storage/v1/volumes/") + mAttachmentVolume));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    QUrl postData;
    const QString mAccessOauth = mCustomerSecret + QLatin1String("%26") + mTokenSecret;

    postData.addQueryItem(QLatin1String("oauth_signature"), mAccessOauth);
    postData.addQueryItem(QLatin1String("token"), mToken);
    postData.addQueryItem(QLatin1String("token_secret"), mTokenSecret);
    postData.addQueryItem(QLatin1String("consumer_secret"), mCustomerSecret);
    postData.addQueryItem(QLatin1String("consumer_key"), mCustomerKey);
    QNetworkReply *reply = mNetworkAccessManager->put(request, postData.encodedQuery());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void UbuntuOneJob::parseCreateFolder(const QString &data)
{
    //TODO
    qDebug()<<" data "<<data;
    Q_EMIT createFolderDone(QString());
    deleteLater();
}

void UbuntuOneJob::parseUploadFiles(const QString &data)
{
    qDebug()<<" data "<<data;
    //TODO
    Q_EMIT uploadFileDone(QString());
    deleteLater();
}

void UbuntuOneJob::parseAccountInfo(const QString &data)
{
    QJson::Parser parser;
    bool ok;

    QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    PimCommon::AccountInfo accountInfo;
    if (info.contains(QLatin1String("used"))) {
       accountInfo.shared = info.value(QLatin1String("used")).toLongLong();
    }
    if (info.contains(QLatin1String("total"))) {
        accountInfo.accountSize = info.value(QLatin1String("total")).toLongLong();
    }
    Q_EMIT accountInfoDone(accountInfo);
    deleteLater();
}

void UbuntuOneJob::slotAuthenticationRequired(QNetworkReply *, QAuthenticator *auth)
{
    QPointer<LoginDialog> dlg = new LoginDialog;
    if (dlg->exec()) {
        auth->setUser(dlg->username());
        auth->setPassword(dlg->password());
    } else {
        Q_EMIT authorizationFailed(i18n("Authentication Canceled."));
        deleteLater();
    }
    delete dlg;
}

void UbuntuOneJob::parseRequestToken(const QString &data)
{
    QJson::Parser parser;
    bool ok;

    QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    qDebug()<<" info"<<info;
    if (info.contains(QLatin1String("consumer_secret"))) {
        mCustomerSecret = info.value(QLatin1String("consumer_secret")).toString();
    }
    if (info.contains(QLatin1String("token"))) {
        mToken = info.value(QLatin1String("token")).toString();
    }
    if (info.contains(QLatin1String("consumer_key"))) {
        mCustomerKey = info.value(QLatin1String("consumer_key")).toString();

    }
    if (info.contains(QLatin1String("token_secret"))) {
        mTokenSecret = info.value(QLatin1String("token_secret")).toString();
    }
    Q_EMIT authorizationDone(mCustomerSecret, mToken, mCustomerKey, mTokenSecret);
    finishGetToken();
}


void UbuntuOneJob::finishGetToken()
{
    //FIXME
    mError = false;
    mActionType = AccessToken;

    QNetworkRequest request(QUrl(QLatin1String("https://one.ubuntu.com/oauth/sso-finished-so-get-tokens/")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    QUrl postData;
    postData.addQueryItem(QLatin1String("token"), mToken);
    postData.addQueryItem(QLatin1String("token_secret"), mTokenSecret);
    postData.addQueryItem(QLatin1String("consumer_secret"), mCustomerSecret);
    postData.addQueryItem(QLatin1String("consumer_key"), mCustomerKey);

    QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));    
}

void UbuntuOneJob::parseAccessToken(const QString &data)
{
    qDebug()<<" parseAccessToken :"<<data;
    deleteLater();
}
