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

#include "triopabshorturlengineinterface.h"
#include "pimcommon/shorturlengineplugin.h"

#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>

using namespace PimCommon;

TripAbShortUrlEngineInterface::TripAbShortUrlEngineInterface(ShortUrlEnginePlugin *plugin, QObject *parent)
    : PimCommon::ShortUrlEngineInterface(plugin, parent)
{

}

TripAbShortUrlEngineInterface::~TripAbShortUrlEngineInterface()
{

}

QString TripAbShortUrlEngineInterface::engineName() const
{
    return mEnginePlugin->engineName();
}


void TripAbShortUrlEngineInterface::generateShortUrl()
{
    const QString requestUrl = QStringLiteral("http://to.ly/api.php?longurl=%1").arg(mOriginalUrl);
    QNetworkReply *reply = mNetworkAccessManager->get(QNetworkRequest(QUrl(requestUrl)));
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &TripAbShortUrlEngineInterface::slotErrorFound);
}

void TripAbShortUrlEngineInterface::slotShortUrlFinished(QNetworkReply *reply)
{
    if (!mErrorFound) {
        const QString data = QString::fromUtf8(reply->readAll());
        if (!data.isEmpty()) {
            Q_EMIT shortUrlGenerated(data);
        }
    }
    reply->deleteLater();
}
