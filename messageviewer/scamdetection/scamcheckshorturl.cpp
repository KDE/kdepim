/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>
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

#include <libkdepim/misc/broadcaststatus.h>



#include <KLocalizedString>


#include <QFile>
#include <QVariantMap>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <QNetworkConfigurationManager>
#include <QJsonDocument>

using namespace MessageViewer;
QStringList ScamCheckShortUrl::sSupportedServices = QStringList();

ScamCheckShortUrl::ScamCheckShortUrl(QObject *parent)
    : QObject(parent),
      mNetworkAccessManager(new QNetworkAccessManager(this))
{
    loadLongUrlServices();
    connect(mNetworkAccessManager, &QNetworkAccessManager::finished, this, &ScamCheckShortUrl::slotExpandFinished);
    mNetworkConfigurationManager = new QNetworkConfigurationManager();
}

ScamCheckShortUrl::~ScamCheckShortUrl()
{
    delete mNetworkConfigurationManager;
}

void ScamCheckShortUrl::expandedUrl(const QUrl &url)
{
    if (!mNetworkConfigurationManager->isOnline()) {
        KPIM::BroadcastStatus::instance()->setStatusMsg( i18n( "No network connection detected, we cannot expand url.") );
        return;
    }
    const QUrl newUrl = QString::fromLatin1("http://api.longurl.org/v2/expand?url=%1&format=json").arg(url.url());

    //qDebug()<<" newUrl "<<newUrl;
    QNetworkReply *reply = mNetworkAccessManager->get(QNetworkRequest(newUrl));
    reply->setProperty("shortUrl", url.url());
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &ScamCheckShortUrl::slotError);
}

void ScamCheckShortUrl::slotExpandFinished(QNetworkReply *reply)
{
    QUrl shortUrl;
    if (!reply->property("shortUrl").isNull()) {
        shortUrl.setUrl(reply->property("shortUrl").toString());
    }
    reply->deleteLater();
    QJsonDocument jsonDoc = QJsonDocument::fromBinaryData(reply->readAll());
    if (!jsonDoc.isNull()) {
        const QMap<QString, QVariant> map = jsonDoc.toVariant().toMap();
        QUrl longUrl;
        if (map.contains(QLatin1String("long-url"))) {
            longUrl.setUrl(map.value(QLatin1String("long-url")).toString());
        } else {
            return;
        }
        KPIM::BroadcastStatus::instance()->setStatusMsg( i18n( "Short url \'%1\' redirects to \'%2\'.", shortUrl.url(), longUrl.toDisplayString() ) );

    }
}

void ScamCheckShortUrl::slotError(QNetworkReply::NetworkError error)
{
    Q_EMIT expandUrlError(error);
}

bool ScamCheckShortUrl::isShortUrl(const QUrl &url)
{
    if (!url.path().isEmpty() && QString::compare(url.path(),QLatin1String("/")) && sSupportedServices.contains(url.host())) {
        return true;
    } else  {
        return false;
    }
}

void ScamCheckShortUrl::loadLongUrlServices()
{
    QFile servicesFile(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("messageviewer/longurlServices.json")));
    if (servicesFile.open(QIODevice::ReadOnly)) {
        QJsonParseError error;
        const QJsonDocument json = QJsonDocument::fromJson(servicesFile.readAll(), &error);
        if (error.error != QJsonParseError::NoError || json.isNull()) {
            qDebug()<<" Error during read longurlServices.json";
            return;
        }
        const QMap<QString, QVariant> response = json.toVariant().toMap();
        sSupportedServices = response.uniqueKeys();
    } else {
        qDebug()<<" json file \'longurlServices.json\' not found";
    }
}
