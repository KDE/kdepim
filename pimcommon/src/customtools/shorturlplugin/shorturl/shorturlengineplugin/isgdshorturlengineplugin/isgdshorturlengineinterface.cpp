/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "isgdshorturlengineinterface.h"
#include "pimcommon/shorturlengineplugin.h"

#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>

using namespace PimCommon;

IsgdShortUrlEngineInterface::IsgdShortUrlEngineInterface(PimCommon::ShortUrlEnginePlugin *plugin, QObject *parent)
    : PimCommon::ShortUrlEngineInterface(plugin, parent)
{

}

IsgdShortUrlEngineInterface::~IsgdShortUrlEngineInterface()
{

}

QString IsgdShortUrlEngineInterface::engineName() const
{
    return mEnginePlugin->engineName();
}

void IsgdShortUrlEngineInterface::generateShortUrl()
{
    const QString requestUrl = QStringLiteral("http://is.gd/create.php?%1&url=%2").arg(QStringLiteral("format=json")).arg(mOriginalUrl);
    QNetworkRequest request = QNetworkRequest(QUrl(requestUrl));

    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &IsgdShortUrlEngineInterface::slotErrorFound);
}

void IsgdShortUrlEngineInterface::slotSslErrors(QNetworkReply *reply, const QList<QSslError> &error)
{
    reply->ignoreSslErrors(error);
}

void IsgdShortUrlEngineInterface::slotShortUrlFinished(QNetworkReply *reply)
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
        //qCDebug(PIMCOMMON_LOG) << " Error during parsing" << error.errorString();
        Q_EMIT shortUrlFailed(error.errorString());
        return;
    }
    const QMap<QString, QVariant> map = json.toVariant().toMap();

    if (map.contains(QStringLiteral("shorturl"))) {
        Q_EMIT shortUrlGenerated(map.value(QStringLiteral("shorturl")).toString());
    }
}
