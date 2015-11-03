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

#ifndef GRAVATARRESOLVURLJOB_H
#define GRAVATARRESOLVURLJOB_H

#include "gravatar_export.h"
#include <QObject>
#include <QUrl>
#include <QPixmap>
#include <QNetworkReply>
namespace Gravatar
{
class GravatarResolvUrlJobPrivate;
class GRAVATAR_EXPORT GravatarResolvUrlJob : public QObject
{
    Q_OBJECT
public:
    explicit GravatarResolvUrlJob(QObject *parent = Q_NULLPTR);
    ~GravatarResolvUrlJob();

    bool canStart() const;
    void start();

    QString email() const;
    void setEmail(const QString &email);

    QUrl generateGravatarUrl(bool useLibravatar);

    bool hasGravatar() const;

    QString calculatedHash() const;

    void setSize(int size);

    int size() const;

    QPixmap pixmap() const;

    bool useDefaultPixmap() const;
    void setUseDefaultPixmap(bool useDefaultPixmap);

    bool useCache() const;
    void setUseCache(bool useCache);

    bool useLibravatar() const;
    void setUseLibravatar(bool useLibravatar);

    bool fallbackGravatar() const;
    void setFallbackGravatar(bool fallbackGravatar);

    bool useHttps() const;
    void setUseHttps(bool useHttps);

Q_SIGNALS:
    void finished(Gravatar::GravatarResolvUrlJob *);
    void resolvUrl(const QUrl &url);

private Q_SLOTS:
    void slotFinishLoadPixmap(QNetworkReply *reply);
    void slotError(QNetworkReply::NetworkError error);

private:
    void startNetworkManager(const QUrl &url);
    QUrl createUrl(bool useLibravatar);
    QString calculateHash(bool useLibravator);
    GravatarResolvUrlJobPrivate *const d;
};
}

#endif // GRAVATARRESOLVURLJOB_H
