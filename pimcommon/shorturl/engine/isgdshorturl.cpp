/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#include "isgdshorturl.h"

#include <QJsonDocument>
#include <QNetworkRequest>
#include <QUrl>
#include <qregexp.h>
#include "pimcommon_debug.h"

using namespace PimCommon;
IsGdShortUrl::IsGdShortUrl(QObject *parent)
    : PimCommon::AbstractShortUrl(parent)
{
    connect(mNetworkAccessManager, &QNetworkAccessManager::sslErrors, this, &IsGdShortUrl::slotSslErrors);
}

IsGdShortUrl::~IsGdShortUrl()
{

}

void IsGdShortUrl::start()
{
    const QString requestUrl = QStringLiteral("http://is.gd/create.php?%1&url=%2").arg(QStringLiteral("format=json")).arg(mOriginalUrl);
    QNetworkRequest request(requestUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &IsGdShortUrl::slotErrorFound);
}

QString IsGdShortUrl::shortUrlName() const
{
    return QStringLiteral("Is.gd");
}

void IsGdShortUrl::slotSslErrors(QNetworkReply *reply, const QList<QSslError> &error)
{
    reply->ignoreSslErrors(error);
}

void IsGdShortUrl::slotShortUrlFinished(QNetworkReply *reply)
{
    if (mErrorFound) {
        return;
    }

    const QByteArray data = reply->readAll();
    QJsonParseError error;
    const QJsonDocument json = QJsonDocument::fromJson(data, &error);
    //qCDebug(PIMCOMMON_LOG) << "void IsGdShortUrl::slotShortUrlFinished(QNetworkReply *reply) " << data;

    reply->deleteLater();

    if (error.error != QJsonParseError::NoError || json.isNull()) {
        qCDebug(PIMCOMMON_LOG) << " Error during parsing" << error.errorString();
        Q_EMIT shortUrlFailed(error.errorString());
        return;
    }
    const QMap<QString, QVariant> map = json.toVariant().toMap();

    if (map.contains(QStringLiteral("shorturl"))) {
        Q_EMIT shortUrlDone(map.value(QStringLiteral("shorturl")).toString());
    }
}
