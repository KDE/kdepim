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

#include "ur1cashorturlengineinterface.h"
#include "pimcommon/shorturlengineplugin.h"
#include "ur1cashorturlengineplugin_debug.h"

#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>

using namespace PimCommon;

Ur1CaShortUrlEngineInterface::Ur1CaShortUrlEngineInterface(ShortUrlEnginePlugin *plugin, QObject *parent)
    : PimCommon::ShortUrlEngineInterface(plugin, parent)
{
    connect(mNetworkAccessManager, &QNetworkAccessManager::sslErrors, this, &Ur1CaShortUrlEngineInterface::slotSslErrors);
    connect(mNetworkAccessManager, &QNetworkAccessManager::finished, this, &Ur1CaShortUrlEngineInterface::slotShortUrlFinished);
}

Ur1CaShortUrlEngineInterface::~Ur1CaShortUrlEngineInterface()
{

}

QString Ur1CaShortUrlEngineInterface::engineName() const
{
    return mEnginePlugin->engineName();
}

void Ur1CaShortUrlEngineInterface::generateShortUrl()
{
    QUrl url(QStringLiteral("http://ur1.ca/"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("longurl"), mOriginalUrl);

    url.setQuery(query);
    QByteArray postData;
    postData = url.encodedQuery();
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("text/plain"));

    QNetworkReply *reply = mNetworkAccessManager->post(request, postData);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &Ur1CaShortUrlEngineInterface::slotErrorFound);
}

void Ur1CaShortUrlEngineInterface::slotSslErrors(QNetworkReply *reply, const QList<QSslError> &error)
{
    reply->ignoreSslErrors(error);
}

void Ur1CaShortUrlEngineInterface::slotShortUrlFinished(QNetworkReply *reply)
{
    if (mErrorFound) {
        reply->deleteLater();
        return;
    }
    QString output = QLatin1String(reply->readAll());
    qCDebug(UR1CASHORTURLENGINEPLUGIN_LOG) << "void Ur1CaShortUrl::slotShortUrlFinished(QNetworkReply *reply) " << output;
    QRegExp rx(QStringLiteral("<p class=[\'\"]success[\'\"]>(.*)</p>"));
    rx.setMinimal(true);
    output = rx.cap(1);
    rx.setPattern(QStringLiteral("href=[\'\"](.*)[\'\"]"));
    rx.indexIn(output);
    output = rx.cap(1);
    qCDebug(UR1CASHORTURLENGINEPLUGIN_LOG) << "Short url is: " << output;
    if (!output.isEmpty()) {
        Q_EMIT shortUrlGenerated(output);
    } else {
        //TODO
        Q_EMIT shortUrlFailed(QString());
    }
    reply->deleteLater();
}
