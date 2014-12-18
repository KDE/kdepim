/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#include <QNetworkRequest>
#include <QUrl>
#include <qregexp.h>
#include "pimcommon_debug.h"

using namespace PimCommon;
IsGdShortUrl::IsGdShortUrl(QObject *parent)
    : PimCommon::AbstractShortUrl(parent)
{
    connect(mNetworkAccessManager, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(slotSslErrors(QNetworkReply*,QList<QSslError>)));
}

IsGdShortUrl::~IsGdShortUrl()
{

}

void IsGdShortUrl::start()
{
    const QString requestUrl = QStringLiteral("http://is.gd/create.php?url=%1").arg(mOriginalUrl);
    QNetworkRequest request(requestUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &IsGdShortUrl::slotErrorFound);
}

QString IsGdShortUrl::shortUrlName() const
{
    return QLatin1String("Is.gd");
}

void IsGdShortUrl::slotSslErrors(QNetworkReply *reply, const QList<QSslError> &error)
{
    reply->ignoreSslErrors(error);
}

void IsGdShortUrl::slotShortUrlFinished(QNetworkReply *reply)
{
    reply->deleteLater();
    if (mErrorFound) {
        return;
    }

    QString output = QString::fromLatin1(reply->readAll());
    qCDebug(PIMCOMMON_LOG) << "void IsGdShortUrl::slotShortUrlFinished(QNetworkReply *reply) " << output;
    //TODO
    if (!output.isEmpty()) {
        Q_EMIT shortUrlDone(output);
    }
}
