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

#include "ur1cashorturl.h"
#include <QNetworkRequest>
#include <QUrl>
#include <qregexp.h>

using namespace PimCommon;
Ur1CaShortUrl::Ur1CaShortUrl(QObject *parent)
    : PimCommon::AbstractShortUrl(parent)
{
    connect(mNetworkAccessManager, &QNetworkAccessManager::sslErrors, this, &Ur1CaShortUrl::slotSslErrors);
}

Ur1CaShortUrl::~Ur1CaShortUrl()
{

}

void Ur1CaShortUrl::start()
{
    QNetworkRequest request(QUrl(QStringLiteral("http://ur1.ca/")));
    const QString data = QStringLiteral("longurl=\"%1\"").arg(mOriginalUrl);

    QNetworkReply *reply = mNetworkAccessManager->post(request, data.toUtf8());
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &Ur1CaShortUrl::slotErrorFound);
}

QString Ur1CaShortUrl::shortUrlName() const
{
    return QStringLiteral("Ur1Ca");
}

void Ur1CaShortUrl::slotSslErrors(QNetworkReply *reply, const QList<QSslError> &error)
{
    reply->ignoreSslErrors(error);
}

void Ur1CaShortUrl::slotShortUrlFinished(QNetworkReply *reply)
{
    if (mErrorFound) {
        reply->deleteLater();
        return;
    }

    QString output = QLatin1String(reply->readAll());
    //qCDebug(PIMCOMMON_LOG) << "void Ur1CaShortUrl::slotShortUrlFinished(QNetworkReply *reply) " << output;
    QRegExp rx(QStringLiteral("<p class=[\'\"]success[\'\"]>(.*)</p>"));
    rx.setMinimal(true);
    output = rx.cap(1);
    rx.setPattern(QStringLiteral("href=[\'\"](.*)[\'\"]"));
    rx.indexIn(output);
    output = rx.cap(1);
    //qCDebug(PIMCOMMON_LOG) << "Short url is: " << output;
    if (!output.isEmpty()) {
        Q_EMIT shortUrlDone(output);
    } else {
        //TODO
        Q_EMIT shortUrlFailed(QString());
    }
    reply->deleteLater();
}
