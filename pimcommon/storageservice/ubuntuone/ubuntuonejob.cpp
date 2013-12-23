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
    mActionType = PimCommon::StorageServiceAbstract::RequestToken;
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
    mActionType = PimCommon::StorageServiceAbstract::UploadFile;
    mError = false;
    qDebug()<<" upload file not implemented";
    deleteLater();
}

void UbuntuOneJob::listFolder(const QString &folder)
{
    mActionType = PimCommon::StorageServiceAbstract::ListFolder;
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
    mActionType = PimCommon::StorageServiceAbstract::AccountInfo;
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
    mActionType = PimCommon::StorageServiceAbstract::CreateFolder;
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
    mActionType = PimCommon::StorageServiceAbstract::ShareLink;
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
        case PimCommon::StorageServiceAbstract::NoneAction:
            deleteLater();
            break;
        case PimCommon::StorageServiceAbstract::RequestToken:
            Q_EMIT authorizationFailed(errorStr);
            deleteLater();
            break;
        case PimCommon::StorageServiceAbstract::AccessToken:
            Q_EMIT authorizationFailed(errorStr);
            deleteLater();
            break;
        case PimCommon::StorageServiceAbstract::UploadFile:
        case PimCommon::StorageServiceAbstract::CreateFolder:
        case PimCommon::StorageServiceAbstract::AccountInfo:
        case PimCommon::StorageServiceAbstract::ListFolder:
        case PimCommon::StorageServiceAbstract::DownLoadFile:
        case PimCommon::StorageServiceAbstract::CreateServiceFolder:
        case PimCommon::StorageServiceAbstract::DeleteFile:
        case PimCommon::StorageServiceAbstract::DeleteFolder:
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
    case PimCommon::StorageServiceAbstract::NoneAction:
        deleteLater();
        break;
    case PimCommon::StorageServiceAbstract::RequestToken:
        parseRequestToken(data);
        break;
    case PimCommon::StorageServiceAbstract::AccessToken:
        parseAccessToken(data);
        break;
    case PimCommon::StorageServiceAbstract::UploadFile:
        parseUploadFile(data);
        break;
    case PimCommon::StorageServiceAbstract::CreateFolder:
        parseCreateFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::AccountInfo:
        parseAccountInfo(data);
        break;
    case PimCommon::StorageServiceAbstract::ListFolder:
        parseListFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::CreateServiceFolder:
        parseCreateServiceFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::DownLoadFile:
    case PimCommon::StorageServiceAbstract::DeleteFile:
    case PimCommon::StorageServiceAbstract::DeleteFolder:
        //TODO
        deleteLater();
        break;
    default:
        qDebug()<<" Action Type unknown:"<<mActionType;
    }
}

void UbuntuOneJob::parseCreateServiceFolder(const QString &data)
{
    qDebug()<<" create service folder not implemented";
    deleteLater();
}

void UbuntuOneJob::parseListFolder(const QString &data)
{
    qDebug()<<" data "<<data;
    QJson::Parser parser;
    bool ok;

    QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    qDebug()<<" info "<<info;
    QStringList listFolder;
    if (info.contains(QLatin1String("user_node_paths"))) {
        qDebug()<<" list folder "<<info.value(QLatin1String("user_node_paths"));
        QList<QVariant> lst = info.value(QLatin1String("user_node_paths")).toList();
        Q_FOREACH (const QVariant &v, lst)
            listFolder.append(v.toString());
    }
    Q_EMIT listFolderDone(listFolder);
    deleteLater();
}

void UbuntuOneJob::createServiceFolder()
{
    mActionType = PimCommon::StorageServiceAbstract::CreateServiceFolder;
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

void UbuntuOneJob::downloadFile(const QString &filename)
{
    mActionType = PimCommon::StorageServiceAbstract::DownLoadFile;
    mError = false;
    deleteLater();
}

void UbuntuOneJob::deleteFile(const QString &filename)
{
    mActionType = PimCommon::StorageServiceAbstract::DeleteFile;
    mError = false;
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    deleteLater();
}

void UbuntuOneJob::deleteFolder(const QString &foldername)
{
    mActionType = PimCommon::StorageServiceAbstract::DeleteFolder;
    mError = false;
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    deleteLater();
}

void UbuntuOneJob::parseCreateFolder(const QString &data)
{
    //TODO
    qDebug()<<" data "<<data;
    Q_EMIT createFolderDone(QString());
    deleteLater();
}

void UbuntuOneJob::parseUploadFile(const QString &data)
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
    mActionType = PimCommon::StorageServiceAbstract::AccessToken;

    QNetworkRequest request(QUrl(QLatin1String("https://one.ubuntu.com/oauth/sso-finished-so-get-tokens/")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    QUrl postData;
    postData.addQueryItem(QLatin1String("token"), mToken);
    postData.addQueryItem(QLatin1String("token_secret"), mTokenSecret);
    postData.addQueryItem(QLatin1String("consumer_secret"), mCustomerSecret);
    postData.addQueryItem(QLatin1String("consumer_key"), mCustomerKey);
    postData.addQueryItem(QLatin1String("oauth_signature_method"), mOauthSignatureMethod);
    qDebug()<<" postData" <<postData;
    QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));    
}

void UbuntuOneJob::parseAccessToken(const QString &data)
{
    qDebug()<<" parseAccessToken :"<<data;
    deleteLater();
}
