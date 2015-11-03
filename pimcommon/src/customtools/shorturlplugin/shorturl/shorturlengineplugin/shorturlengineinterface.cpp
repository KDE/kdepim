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

#include "shorturlengineinterface.h"
#include "shorturlengineplugin.h"

#include <KLocalizedString>

#include <QNetworkAccessManager>

using namespace PimCommon;

ShortUrlEngineInterface::ShortUrlEngineInterface(PimCommon::ShortUrlEnginePlugin *plugin, QObject *parent)
    : QObject(parent),
      mErrorFound(false),
      mNetworkAccessManager(new QNetworkAccessManager(this)),
      mEnginePlugin(plugin)
{

}

ShortUrlEngineInterface::~ShortUrlEngineInterface()
{

}

void ShortUrlEngineInterface::slotErrorFound(QNetworkReply::NetworkError error)
{
    mErrorFound = true;
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    Q_EMIT shortUrlFailed(i18n("Error reported by server:\n\'%1\'", (reply ? reply->errorString() : QString::number(error))));
}

void ShortUrlEngineInterface::setShortUrl(const QString &url)
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

QString ShortUrlEngineInterface::pluginName() const
{
    return mEnginePlugin->pluginName();
}
