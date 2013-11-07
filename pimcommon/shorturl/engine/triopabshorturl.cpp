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


#include "triopabshorturl.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>


using namespace PimCommon;

TriopabShortUrl::TriopabShortUrl(QObject *parent)
    : PimCommon::AbstractShortUrl(parent)
{
}

TriopabShortUrl::~TriopabShortUrl()
{
}

void TriopabShortUrl::start()
{
    const QString requestUrl = QString::fromLatin1("http://to.ly/api.php?longurl=%1").arg(mOriginalUrl);
    QNetworkReply *reply = mNetworkAccessManager->get(QNetworkRequest(requestUrl));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotErrorFound(QNetworkReply::NetworkError)));
}

void TriopabShortUrl::slotShortUrlFinished(QNetworkReply *reply)
{
    reply->deleteLater();
    if (mErrorFound)
        return;

    const QString data = QString::fromUtf8(reply->readAll());
    if (!data.isEmpty()) {
        Q_EMIT shortUrlDone(data);
    }
}

