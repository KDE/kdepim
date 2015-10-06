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

#include "tinyurlengineinterface.h"
#include <KLocalizedString>

#include <QNetworkAccessManager>

using namespace PimCommon;

TinyUrlEngineInterface::TinyUrlEngineInterface(QObject *parent)
    : PimCommon::ShortUrlEngineInterface(parent)
{
    connect(mNetworkAccessManager, &QNetworkAccessManager::finished, this, &TinyUrlEngineInterface::slotShortUrlFinished);
}

TinyUrlEngineInterface::~TinyUrlEngineInterface()
{

}

void TinyUrlEngineInterface::setShortUrl(const QString &url)
{
    mErrorFound = false;
    if (!url.trimmed().startsWith(QStringLiteral("http://")) &&
            !url.trimmed().startsWith(QStringLiteral("https://")) &&
            !url.trimmed().startsWith(QStringLiteral("ftp://")) &&
            !url.trimmed().startsWith(QStringLiteral("ftps://"))) {
        mOriginalUrl = QLatin1String("http://") + url;
    } else {
        mOriginalUrl = url;
    }
}

void TinyUrlEngineInterface::generateShortUrl()
{
    const QString requestUrl = QStringLiteral("http://tinyurl.com/api-create.php?url=%1").arg(mOriginalUrl);
    QNetworkReply *reply = mNetworkAccessManager->get(QNetworkRequest(QUrl(requestUrl)));
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &TinyUrlEngineInterface::slotErrorFound);
}

void TinyUrlEngineInterface::slotShortUrlFinished(QNetworkReply *reply)
{
    if (!mErrorFound) {
        const QString data = QString::fromUtf8(reply->readAll());
        if (!data.isEmpty()) {
            Q_EMIT shortUrlGenerated(data);
        }
    }
    reply->deleteLater();
}

void TinyUrlEngineInterface::slotErrorFound(QNetworkReply::NetworkError error)
{
    mErrorFound = true;
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    Q_EMIT shortUrlFailed(i18n("Error reported by server:\n\'%1\'", (reply ? reply->errorString() : QString::number(error))));
}
