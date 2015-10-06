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

using namespace PimCommon;

IsgdShortUrlEngineInterface::IsgdShortUrlEngineInterface(QObject *parent)
    : PimCommon::ShortUrlEngineInterface(parent)
{

}


IsgdShortUrlEngineInterface::~IsgdShortUrlEngineInterface()
{

}

void IsgdShortUrlEngineInterface::setShortUrl(const QString &url)
{

}

void IsgdShortUrlEngineInterface::generateShortUrl()
{
#if 0
    const QString requestUrl = QStringLiteral("http://is.gd/create.php?%1&url=%2").arg(QStringLiteral("format=json")).arg(mOriginalUrl);
    QNetworkRequest request = QNetworkRequest(QUrl(requestUrl));

    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &IsGdShortUrl::slotErrorFound);
#endif
}

