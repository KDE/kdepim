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

using namespace PimCommon;

DropBoxToken::DropBoxToken(QObject *parent)
    : QObject(parent),
      mNetworkAccessManager(new QNetworkAccessManager(this))
{
    mAppKey = QLatin1String("enpbp89z8hryano");
    mAppSecret = QLatin1String("axwbl0d9sbajt0i");
}

DropBoxToken::~DropBoxToken()
{

}

void DropBoxToken::getTokenAccess()
{
    QNetworkRequest request(QUrl(QLatin1String("https://api.dropbox.com/1/oauth/access_token")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

    QUrl postData;
    postData.addQueryItem(QLatin1String("oauth_consumer_key"), mAppKey);
    postData.addQueryItem(QLatin1String(""), mAppKey);

    QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
/*
    KUrl url("https://api.dropbox.com/1/oauth/access_token");
    url.addQueryItem("oauth_consumer_key",m_oauth_consumer_key);
    url.addQueryItem("oauth_nonce", nonce);
    url.addQueryItem("oauth_signature",m_access_oauth_signature);
    url.addQueryItem("oauth_signature_method",m_oauth_signature_method);
    url.addQueryItem("oauth_timestamp", QString::number(timestamp));
    url.addQueryItem("oauth_version",m_oauth_version);
    url.addQueryItem("oauth_token",m_oauthToken);
*/
}

void DropBoxToken::slotError(QNetworkReply::NetworkError /*error*/)
{
    //TODO
}
