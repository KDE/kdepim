/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>
  based on ktp code

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

#include "scamcheckshorturl.h"

#include <KGlobal>
#include <KStandardDirs>

#include <qjson/parser.h>

#include <QFile>
#include <QVariantMap>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>

using namespace MessageViewer;
QStringList ScamCheckShortUrl::sSupportedServices = QStringList();

ScamCheckShortUrl::ScamCheckShortUrl(QObject *parent)
    : QObject(parent),
      mNetworkAccessManager(new QNetworkAccessManager(this))
{
    loadLongUrlServices();
    connect(mNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotExpandFinished(QNetworkReply*)));
}

ScamCheckShortUrl::~ScamCheckShortUrl()
{
}

void ScamCheckShortUrl::expandedUrl(const KUrl &url)
{

    QUrl newUrl = QString::fromLatin1("http://api.longurl.org/v2/expand?url=%1&format=json").arg(url.url());

    qDebug()<<" newUrl "<<newUrl;
    QNetworkReply *reply = mNetworkAccessManager->get(QNetworkRequest(newUrl));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void ScamCheckShortUrl::slotExpandFinished(QNetworkReply *reply)
{
    reply->deleteLater();
    const QString jsonData = QString::fromUtf8(reply->readAll());

    qDebug() << jsonData;

    QJson::Parser parser;
    bool ok;

    qDebug()<<" parser.parse(jsonData.toUtf8(), &ok)"<<parser.parse(jsonData.toUtf8(), &ok);
    const QVariantList json = parser.parse(jsonData.toUtf8(), &ok).toList();
    if (!ok) {
        //Q_EMIT translateFailed(ok);
        return;
    }
    qDebug()<<" json "<<json;
}

void ScamCheckShortUrl::slotError(QNetworkReply::NetworkError error)
{
    qDebug()<<" void ScamCheckShortUrl::slotError(QNetworkReply::NetworkError error)";
    Q_EMIT expandUrlError(error);
}

bool ScamCheckShortUrl::isShortUrl(const KUrl &url)
{
    if (!url.path().isEmpty() && QString::compare(url.path(),QLatin1String("/")) && sSupportedServices.contains(url.host())) {
        return true;
    } else  {
        return false;
    }
}

void ScamCheckShortUrl::loadLongUrlServices()
{
    QFile servicesFile(KGlobal::dirs()->findResource("data", QLatin1String("messageviewer/longurlServices.json")));
    if (!servicesFile.open(QIODevice::ReadOnly)) {
        qDebug()<<" json file not found";
    } else {
        const QVariantMap response = QJson::Parser().parse(&servicesFile).toMap();
        sSupportedServices = response.uniqueKeys();
    }
}

#include "scamcheckshorturl.moc"
