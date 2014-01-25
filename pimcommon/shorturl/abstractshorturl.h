/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#ifndef ABSTRACTSHORTURL_H
#define ABSTRACTSHORTURL_H

#include "pimcommon_export.h"

#include <QObject>
#include <QNetworkReply>


namespace PimCommon {
class PIMCOMMON_EXPORT AbstractShortUrl : public QObject
{
    Q_OBJECT
public:
    explicit AbstractShortUrl(QObject *parent=0);
    ~AbstractShortUrl();

    virtual QString shortUrlName() const = 0;
    virtual void shortUrl(const QString &url);
    virtual void start() = 0;

Q_SIGNALS:
    void shortUrlDone(const QString &url);
    void shortUrlFailed(const QString &error);

protected Q_SLOTS:
    void slotErrorFound(QNetworkReply::NetworkError error);
    virtual void slotShortUrlFinished(QNetworkReply *reply);

protected:
    QString mOriginalUrl;
    bool mErrorFound;
    QNetworkAccessManager *mNetworkAccessManager;
};
}

#endif // ABSTRACTSHORTURL_H
