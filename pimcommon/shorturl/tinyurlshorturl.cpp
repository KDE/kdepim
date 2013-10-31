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


#include "tinyurlshorturl.h"

#include <KLocale>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>


using namespace PimCommon;

TinyurlShortUrl::TinyurlShortUrl(QObject *parent)
    : PimCommon::AbstractShortUrl(parent),
      mNetworkAccessManager(new QNetworkAccessManager(this))
{
    connect(mNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotShortUrlFinished(QNetworkReply*)));
}

TinyurlShortUrl::~TinyurlShortUrl()
{
}

void TinyurlShortUrl::start()
{
    const QString requestUrl = QString::fromLatin1("http://tinyurl.com/api-create.php?url=%1").arg(mOriginalUrl);
    QNetworkReply *reply = mNetworkAccessManager->get(QNetworkRequest(requestUrl));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void TinyurlShortUrl::slotShortUrlFinished(QNetworkReply *reply)
{
    reply->deleteLater();
    if (mErrorFound)
        return;

    const QString data = QString::fromUtf8(reply->readAll());
    if (!data.isEmpty()) {
        Q_EMIT shortUrlDone(data);
    }
}

void TinyurlShortUrl::slotError(QNetworkReply::NetworkError error)
{
    mErrorFound = true;
    Q_EMIT shortUrlFailed(i18n("Error reported by server: \'%1\'", error));
}



#include "tinyurlshorturl.moc"
