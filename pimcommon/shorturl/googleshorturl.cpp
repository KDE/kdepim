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

#include "googleshorturl.h"

#include <KLocale>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>

using namespace PimCommon;
GoogleShortUrl::GoogleShortUrl(QObject *parent)
    : PimCommon::AbstractShortUrl(parent),
      mNetworkAccessManager(new QNetworkAccessManager(this))
{
    connect(mNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotShortUrlFinished(QNetworkReply*)));
}

GoogleShortUrl::~GoogleShortUrl()
{

}

void GoogleShortUrl::start()
{
    QNetworkRequest request(QUrl(QLatin1String("https://www.googleapis.com/urlshortener/v1/url")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));

    QUrl postData;
    postData.addQueryItem(QLatin1String("longUrl"), mOriginalUrl);
    QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void GoogleShortUrl::slotShortUrlFinished(QNetworkReply *reply)
{
    reply->deleteLater();
    const QString jsonData = QString::fromUtf8(reply->readAll());

    qDebug()<<"void GoogleShortUrl::slotShortUrlFinished(QNetworkReply*)" << jsonData;
    //Q_EMIT void shortUrlDone(const QString &url);
}

void GoogleShortUrl::slotError(QNetworkReply::NetworkError error)
{
    qDebug()<<" error !!!!!!!!!"<<error;
    Q_EMIT shortUrlFailed(i18n("Error reported by server"));
}


#include "googleshorturl.moc"
