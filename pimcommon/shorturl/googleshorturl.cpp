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

#include <qjson/parser.h>


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
    mErrorFound = false;
    QNetworkRequest request(QUrl(QLatin1String("https://www.googleapis.com/urlshortener/v1/url")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));

    const QString data = QString::fromLatin1("{\"longUrl\": \"%1/\"}").arg(mOriginalUrl);

    QNetworkReply *reply = mNetworkAccessManager->post(request, data.toUtf8());
    qDebug()<<" reply "<<reply->url();
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void GoogleShortUrl::slotShortUrlFinished(QNetworkReply *reply)
{
    reply->deleteLater();
    if (mErrorFound)
        return;

    const QString jsonData = QString::fromUtf8(reply->readAll());

    QJson::Parser parser;
    bool ok;
    const QMap<QString, QVariant> map = parser.parse(jsonData.toUtf8(), &ok).toMap();
    if (!ok) {
        qDebug()<<" Error during parsing";
        return;
    }
    if (map.contains(QLatin1String("id")) && map.contains(QLatin1String("kind"))) {
        Q_EMIT shortUrlDone(map.value(QLatin1String("id")).toString());
    }
}

void GoogleShortUrl::slotError(QNetworkReply::NetworkError error)
{
    mErrorFound = true;
    Q_EMIT shortUrlFailed(i18n("Error reported by server: \'%1\'", error));
}


#include "googleshorturl.moc"
