/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include <QNetworkRequest>
#include <QUrl>
#include <QJsonDocument>

using namespace PimCommon;
GoogleShortUrl::GoogleShortUrl(QObject *parent)
    : PimCommon::AbstractShortUrl(parent)
{
    connect(mNetworkAccessManager, &QNetworkAccessManager::sslErrors, this, &GoogleShortUrl::slotSslErrors);
}

GoogleShortUrl::~GoogleShortUrl()
{

}

void GoogleShortUrl::start()
{
    QNetworkRequest request(QUrl(QStringLiteral("https://www.googleapis.com/urlshortener/v1/url")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    const QString data = QStringLiteral("{\"shortUrl\": \"%1/\"}").arg(mOriginalUrl);

    QNetworkReply *reply = mNetworkAccessManager->post(request, data.toUtf8());
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &GoogleShortUrl::slotErrorFound);
}

QString GoogleShortUrl::shortUrlName() const
{
    return QStringLiteral("Google");
}

void GoogleShortUrl::slotSslErrors(QNetworkReply *reply, const QList<QSslError> &error)
{
    reply->ignoreSslErrors(error);
}

void GoogleShortUrl::slotShortUrlFinished(QNetworkReply *reply)
{
    if (!mErrorFound) {
        const QByteArray data = reply->readAll();
        //qCDebug(PIMCOMMON_LOG) << "void GoogleShortUrl::slotShortUrlFinished(QNetworkReply *reply) " <<  data;
        QJsonParseError error;
        reply->deleteLater();
        const QJsonDocument json = QJsonDocument::fromJson(data, &error);
        if (error.error != QJsonParseError::NoError || json.isNull()) {
            //qCDebug(PIMCOMMON_LOG) << " Error during parsing" << error.errorString();
            Q_EMIT shortUrlFailed(error.errorString());
            return;
        }
        const QMap<QString, QVariant> map = json.toVariant().toMap();

        if (map.contains(QStringLiteral("id")) && map.contains(QStringLiteral("kind"))) {
            Q_EMIT shortUrlDone(map.value(QStringLiteral("id")).toString());
        }
    }
}

