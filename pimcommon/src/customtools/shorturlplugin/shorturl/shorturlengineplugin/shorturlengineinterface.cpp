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

#include <KLocalizedString>

#include <QNetworkAccessManager>

using namespace PimCommon;

ShortUrlEngineInterface::ShortUrlEngineInterface(QObject *parent)
    : QObject(parent),
      mErrorFound(false),
      mNetworkAccessManager(new QNetworkAccessManager(this))
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

