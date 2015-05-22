/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  based on code from Sune Vuorela <sune@vuorela.dk> (Rawatar source code)

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

#include "gravatarresolvurljob.h"
#include "gravatarcache.h"
#include <QCryptographicHash>
#include <QStringList>
#include <QPixmap>
#include <QUrlQuery>
#include "pimcommon_debug.h"
#include <solid/networking.h>

using namespace PimCommon;

GravatarResolvUrlJob::GravatarResolvUrlJob(QObject *parent)
    : QObject(parent),
      mNetworkAccessManager(0),
      mSize(80),
      mHasGravatar(false),
      mUseDefaultPixmap(false),
      mUseCache(false),
      mUseLibravatar(false),
      mFallbackGravatar(true)
{

}

GravatarResolvUrlJob::~GravatarResolvUrlJob()
{

}

bool GravatarResolvUrlJob::canStart() const
{
    if (Solid::Networking::status() == Solid::Networking::Connected || Solid::Networking::status() == Solid::Networking::Unknown) {
        return !mEmail.trimmed().isEmpty() && (mEmail.contains(QLatin1Char('@')));
    } else {
        return false;
    }
}

QUrl GravatarResolvUrlJob::generateGravatarUrl()
{
    return createUrl();
}

bool GravatarResolvUrlJob::hasGravatar() const
{
    return mHasGravatar;
}

void GravatarResolvUrlJob::start()
{
    mHasGravatar = false;
    if (canStart()) {
        mCalculatedHash.clear();
        const QUrl url = createUrl();
        Q_EMIT resolvUrl(url);
        bool haveStoredPixmap = false;
        const QPixmap pix = GravatarCache::self()->loadGravatarPixmap(mCalculatedHash, haveStoredPixmap);
        if (haveStoredPixmap && !pix.isNull()) {
            mPixmap = pix;
            mHasGravatar = true;
            Q_EMIT finished(this);
            deleteLater();
        } else {
            if (Solid::Networking::status() == Solid::Networking::Connected || Solid::Networking::status() == Solid::Networking::Unknown) {
                if (!mNetworkAccessManager) {
                    mNetworkAccessManager = new QNetworkAccessManager(this);
                    connect(mNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotFinishLoadPixmap(QNetworkReply*)));
                }
                QNetworkReply *reply = mNetworkAccessManager->get(QNetworkRequest(url));
                connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
            } else {
                qCDebug(PIMCOMMON_LOG) << " network is not connected";
                deleteLater();
                return;
            }
        }
    } else {
        qCDebug(PIMCOMMON_LOG) << "Gravatar can not start";
        deleteLater();
    }
}

void GravatarResolvUrlJob::slotFinishLoadPixmap(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        mPixmap.loadFromData(reply->readAll());
        mHasGravatar = true;
        //For the moment don't use cache other we will store a lot of pixmap
        if (!mUseDefaultPixmap) {
            GravatarCache::self()->saveGravatarPixmap(mCalculatedHash, mPixmap);
        }
    } else if (mUseLibravatar && mFallbackGravatar) {
        //TODO
    }
    reply->deleteLater();
    Q_EMIT finished(this);
    deleteLater();
}

void GravatarResolvUrlJob::slotError(QNetworkReply::NetworkError error)
{
    if (error == QNetworkReply::ContentNotFoundError) {
        mHasGravatar = false;
    }
}

QString GravatarResolvUrlJob::email() const
{
    return mEmail;
}

void GravatarResolvUrlJob::setEmail(const QString &email)
{
    mEmail = email;
}

QString GravatarResolvUrlJob::calculateHash(bool useLibravator)
{
    QCryptographicHash hash(useLibravator ? QCryptographicHash::Sha256 : QCryptographicHash::Md5);
    hash.addData(mEmail.toLower().toUtf8());
    return QString::fromUtf8(hash.result().toHex());
}

bool GravatarResolvUrlJob::fallbackGravatar() const
{
    return mFallbackGravatar;
}

void GravatarResolvUrlJob::setFallbackGravatar(bool fallbackGravatar)
{
    mFallbackGravatar = fallbackGravatar;
}

bool GravatarResolvUrlJob::useLibravatar() const
{
    return mUseLibravatar;
}

void GravatarResolvUrlJob::setUseLibravatar(bool useLibravatar)
{
    mUseLibravatar = useLibravatar;
}

bool GravatarResolvUrlJob::useCache() const
{
    return mUseCache;
}

void GravatarResolvUrlJob::setUseCache(bool useCache)
{
    mUseCache = useCache;
}

bool GravatarResolvUrlJob::useDefaultPixmap() const
{
    return mUseDefaultPixmap;
}

void GravatarResolvUrlJob::setUseDefaultPixmap(bool useDefaultPixmap)
{
    mUseDefaultPixmap = useDefaultPixmap;
}

int GravatarResolvUrlJob::size() const
{
    return mSize;
}

QPixmap GravatarResolvUrlJob::pixmap() const
{
    return mPixmap;
}

void GravatarResolvUrlJob::setSize(int size)
{
    if (size <= 0) {
        size = 80;
    } else if (size > 2048) {
        size = 2048;
    }
    mSize = size;
}

QString GravatarResolvUrlJob::calculatedHash() const
{
    return mCalculatedHash;
}

QUrl GravatarResolvUrlJob::createUrl()
{
    QUrl url;
    mCalculatedHash.clear();
    if (!canStart()) {
        return url;
    }
    QUrlQuery query;
    if (!mUseDefaultPixmap) {
        //Add ?d=404
        query.addQueryItem(QStringLiteral("d"), QStringLiteral("404"));
    }
    if (mSize != 80) {
        query.addQueryItem(QStringLiteral("s"), QString::number(mSize));
    }
    url.setScheme(QStringLiteral("http"));
    if (mUseLibravatar) {
        url.setHost(QStringLiteral("cdn.libravatar.org"));
    } else {
        url.setHost(QStringLiteral("www.gravatar.com"));
    }
    url.setPort(80);
    mCalculatedHash = calculateHash(mUseLibravatar);
    url.setPath(QLatin1String("/avatar/") + mCalculatedHash);
    url.setQuery(query);
    return url;
}
