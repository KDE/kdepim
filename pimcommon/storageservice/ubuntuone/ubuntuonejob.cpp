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
    QPointer<LoginDialog> dlg = new LoginDialog;
    if (dlg->exec()) {
        mPassword = dlg->password();
        mUsername = dlg->username();
    }
    delete dlg;
    if (!mUsername.isEmpty()) {
        mActionType = RequestToken;
        QUrl url(QLatin1String("https://login.ubuntu.com/api/1.0/authentications"));
        url.addQueryItem(QLatin1String("ws.op"), QLatin1String("authenticate"));
        url.addQueryItem(QLatin1String("token_name"), QLatin1String("Ubuntu One @ foo") );
        qDebug()<<" postData "<<url;
        QNetworkRequest request(url);
        QNetworkReply *reply = mNetworkAccessManager->get(request);
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
    qDebug()<<" accountInfo "<<url;
    QNetworkRequest request(url);
    qDebug()<<" url "<<url;
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));

}

void UbuntuOneJob::createFolder(const QString &filename)
{

}

void UbuntuOneJob::shareLink(const QString &root, const QString &path)
{

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
    qDebug()<<" Data ? "<<data;
    switch(mActionType) {
    case NoneAction:
        deleteLater();
        break;
    case RequestToken:
        parseRequestToken(data);
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
    }
}

void UbuntuOneJob::parseListFolder(const QString &data)
{
    //TODO
    Q_EMIT listFolderDone();
    deleteLater();
}

void UbuntuOneJob::parseCreateFolder(const QString &data)
{
    //TODO
    Q_EMIT createFolderDone();
    deleteLater();
}

void UbuntuOneJob::parseUploadFiles(const QString &data)
{
    Q_EMIT uploadFileDone();
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
    qDebug()<<"slotAuthenticationRequired ";
    auth->setUser(mUsername);
    auth->setPassword(mPassword);
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
    mActionType = AccessToken;
    QUrl url(QLatin1String("https://one.ubuntu.com/oauth/sso-finished-so-get-tokens/"));
    QNetworkRequest request(url);
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}
