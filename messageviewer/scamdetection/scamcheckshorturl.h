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

#ifndef SCAMCHECKSHORTURL_H
#define SCAMCHECKSHORTURL_H

#include "messageviewer_export.h"

#include <QObject>

#include <QUrl>

#include <QStringList>
#include <QNetworkReply>
#include <Solid/Networking>

class QNetworkAccessManager;
class QNetworkReply;
namespace MessageViewer {
class MESSAGEVIEWER_EXPORT ScamCheckShortUrl : public QObject
{
    Q_OBJECT
public:
    explicit ScamCheckShortUrl(QObject *parent=0);
    ~ScamCheckShortUrl();

    static bool isShortUrl(const QUrl &url);

    void expandedUrl(const QUrl &url);

    static void loadLongUrlServices();

private Q_SLOTS:
    void slotExpandFinished(QNetworkReply*);
    void slotError(QNetworkReply::NetworkError error);
    void slotSystemNetworkStatusChanged(Solid::Networking::Status status);

Q_SIGNALS:
    void urlExpanded(const QString &shortUrl, const QString &expandedUrl);
    void expandUrlError(QNetworkReply::NetworkError error);

private:
    static QStringList sSupportedServices;
    QNetworkAccessManager *mNetworkAccessManager;
    bool mNetworkUp;
};
}

#endif // SCAMCHECKSHORTURL_H
