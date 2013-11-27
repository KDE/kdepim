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

#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QDateTime>
#include <QStringList>
#include <QDebug>
using namespace PimCommon;

QString generateNonce(qint32 length)
{
    QString clng;

    for(int i=0; i<length; ++i)
    {
        clng += QString::number(int( qrand() / (RAND_MAX + 1.0) * (16 + 1 - 0) + 0 ), 16).toUpper();
    }

    return clng;
}


DropBoxToken::DropBoxToken(QObject *parent)
    : QObject(parent),
      mNetworkAccessManager(new QNetworkAccessManager(this))
{
    mAppKey = QLatin1String("enpbp89z8hryano");
    mAppSecret = QLatin1String("axwbl0d9sbajt0i");
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
}

void DropBoxToken::slotSendDataFinished(QNetworkReply *reply)
{
    qDebug()<<"slotSendDataFinished**********************";
    const QString data = QString::fromUtf8(reply->readAll());

    reply->deleteLater();
    const QStringList split = data.split(QLatin1Char('&'));
    qDebug()<<" DATA "<<data;
    if (split.count() == 2) {
        const QStringList tokenSecretList = split.at(0).split(QLatin1Char('='));
        mAppSecret = tokenSecretList.at(1);
        const QStringList tokenList = split.at(1).split(QLatin1Char('='));
        mAppKey = tokenList.at(1);
        mAccessOauthSignature = mAppSecret + mAppSecret;
    }
}


