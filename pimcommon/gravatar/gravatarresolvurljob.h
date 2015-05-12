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

#include "pimcommon_export.h"
#include <QObject>
#include <KUrl>
#include <QPixmap>
#include <QNetworkReply>
class QNetworkAccessManager;
namespace PimCommon {
class PIMCOMMON_EXPORT GravatarResolvUrlJob : public QObject
{
    Q_OBJECT
public:
    explicit GravatarResolvUrlJob(QObject *parent = 0);
    ~GravatarResolvUrlJob();

    bool canStart() const;
    void start();

    QString email() const;
    void setEmail(const QString &email);

    KUrl generateGravatarUrl();

    bool hasGravatar() const;

    QString calculatedHash() const;

    void setSize(int size);

    int size() const;

    QPixmap pixmap() const;


    bool useDefaultPixmap() const;
    void setUseDefaultPixmap(bool useDefaultPixmap);

Q_SIGNALS:
    void urlResolved(const KUrl &url);

private Q_SLOTS:
    void slotFinishLoadPixmap(QNetworkReply *reply);
    void slotError(QNetworkReply::NetworkError error);

private:
    QPixmap mPixmap;
    KUrl createUrl();
    QString calculateHash();
    QString mEmail;
    QString mCalculatedHash;
    QNetworkAccessManager *mNetworkAccessManager;
    int mSize;
    bool mHasGravatar;
    bool mUseDefaultPixmap;
};
}

#endif // GRAVATARRESOLVURLJOB_H
