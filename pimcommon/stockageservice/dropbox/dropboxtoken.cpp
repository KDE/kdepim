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


#include "dropboxtoken.h"
#include "stockageservice/stockageauthviewdialog.h"

#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QDateTime>
#include <QStringList>
#include <QDebug>
#include <QPointer>

using namespace PimCommon;

QString generateNonce(qint32 length)
{
    QString clng;

    for(int i=0; i<length; ++i) {
        clng += QString::number(int( qrand() / (RAND_MAX + 1.0) * (16 + 1 - 0) + 0 ), 16).toUpper();
    }

    return clng;
}


DropBoxToken::DropBoxToken(QObject *parent)
    : QObject(parent),
      mNetworkAccessManager(new QNetworkAccessManager(this)),
      mError(false)
{
    mAppKey = QLatin1String("lrk7teqmtlxqf9p");
    mAppSecret = QLatin1String("662xpu5is6ip8wi&");
    mOauthVersion = QLatin1String("1.0");
    mOauthSignatureMethod = QLatin1String("PLAINTEXT");
    mTimestamp = QString::number(QDateTime::currentMSecsSinceEpoch()/1000);
    mNonce = generateNonce(8);
    connect(mNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotSendDataFinished(QNetworkReply*)));
}

DropBoxToken::~DropBoxToken()
{

}

void DropBoxToken::requestTokenAccess()
{
    mActionType = RequestToken;
    QNetworkRequest request(QUrl(QLatin1String("https://api.dropbox.com/1/oauth/request_token")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

    QUrl postData;

    postData.addQueryItem(QLatin1String("oauth_consumer_key"), mAppKey);
    postData.addQueryItem(QLatin1String("oauth_nonce"), mNonce);
    postData.addQueryItem(QLatin1String("oauth_signature"), mAppSecret);
    postData.addQueryItem(QLatin1String("oauth_signature_method"),mOauthSignatureMethod);
    postData.addQueryItem(QLatin1String("oauth_timestamp"), mTimestamp);
    postData.addQueryItem(QLatin1String("oauth_version"), mOauthVersion);
    qDebug()<<" https://api.dropbox.com/1/oauth/request_token"<<postData;

    QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void DropBoxToken::getTokenAccess()
{
    mActionType = AccessToken;
    mError = false;
    QNetworkRequest request(QUrl(QLatin1String("https://api.dropbox.com/1/oauth/access_token")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

    QUrl postData;

    postData.addQueryItem(QLatin1String("oauth_consumer_key"), mAppKey);
    postData.addQueryItem(QLatin1String("oauth_signature"), mAppSecret);
    postData.addQueryItem(QLatin1String("oauth_nonce"), mNonce);
    postData.addQueryItem(QLatin1String("oauth_signature_method"),mOauthSignatureMethod);
    postData.addQueryItem(QLatin1String("oauth_timestamp"), mTimestamp);
    postData.addQueryItem(QLatin1String("oauth_version"), mOauthVersion);
    postData.addQueryItem(QLatin1String("oauth_token"), mOauthToken);
    qDebug()<<" postdata"<<postData;

    QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void DropBoxToken::slotError(QNetworkReply::NetworkError /*error*/)
{
    qDebug()<<" Error ";
    mError = true;
}

void DropBoxToken::slotSendDataFinished(QNetworkReply *reply)
{
    const QString data = QString::fromUtf8(reply->readAll());
    reply->deleteLater();
    if (mError) {
        qDebug()<<" error type "<<data;
        return;
    }

    switch(mActionType) {
    case NoneAction:
        break;
    case RequestToken:
        parseRequestToken(data);
        break;
    case AccessToken:
    case UploadFiles:
    case CreateFolder:
    case AccountInfo:
        break;
    default:
        qDebug()<<" Action Type unknown:"<<mActionType;
    }

    mActionType = NoneAction;
    qDebug()<<"slotSendDataFinished**********************";
}

void DropBoxToken::parseRequestToken(const QString &result)
{
    const QStringList split = result.split(QLatin1Char('&'));
    qDebug()<<" DATA "<<result;
    if (split.count() == 2) {
        const QStringList tokenSecretList = split.at(0).split(QLatin1Char('='));
        mAppSecret = tokenSecretList.at(1);
        const QStringList tokenList = split.at(1).split(QLatin1Char('='));
        mOauthToken = tokenList.at(1);
        mAccessOauthSignature = mAppSecret + mAppSecret;
    } else {
        qDebug()<<" data is not good: "<<result;
    }
    doAuthentification();
}

void DropBoxToken::doAuthentification()
{
    QUrl url(QLatin1String("https://api.dropbox.com/1/oauth/authorize"));
    url.addQueryItem(QLatin1String("oauth_token"), mOauthToken);
    qDebug()<<" void DropBoxToken::doAuthentification()"<<url;
    QPointer<StockageAuthViewDialog> dlg = new StockageAuthViewDialog;
    dlg->setUrl(url);
    if (dlg->exec()) {
        //TODO
    }
    delete dlg;
}

void DropBoxToken::uploadFile(const QString &filename)
{
    mActionType = UploadFiles;
    mError = false;
    QNetworkRequest request(QUrl(QLatin1String("https://api-content.dropbox.com/1/files_put/dropbox/"))); //Add path (need to store default path
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
}

void DropBoxToken::accountInfo()
{
    mActionType = AccountInfo;
    mError = false;
    QNetworkRequest request(QUrl(QLatin1String("https://api.dropbox.com/1/account/info"))); //Add path (need to store default path
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    //QNetworkReply *reply = mNetworkAccessManager->get(request, postData.encodedQuery());
    //connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}
