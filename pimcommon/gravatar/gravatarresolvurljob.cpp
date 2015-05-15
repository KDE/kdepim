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
#include <solid/networking.h>

using namespace PimCommon;

GravatarResolvUrlJob::GravatarResolvUrlJob(QObject *parent)
    : QObject(parent),
      mNetworkAccessManager(0),
      mSize(80),
      mHasGravatar(false),
      mUseDefaultPixmap(false),
      mUseCache(false)
{

}

GravatarResolvUrlJob::~GravatarResolvUrlJob()
{

}

bool GravatarResolvUrlJob::canStart() const
{
    if ( Solid::Networking::status() == Solid::Networking::Connected || Solid::Networking::status() == Solid::Networking::Unknown) {
        return !mEmail.trimmed().isEmpty() && (mEmail.contains(QLatin1Char('@')));
    } else {
        return false;
    }
}

KUrl GravatarResolvUrlJob::generateGravatarUrl()
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
        const KUrl url = createUrl();
        Q_EMIT resolvUrl(url);
        bool haveStoredPixmap = false;
        const QPixmap pix = GravatarCache::self()->loadGravatarPixmap(mCalculatedHash, haveStoredPixmap);
        if (haveStoredPixmap && !pix.isNull()) {
            mPixmap = pix;
            mHasGravatar = true;
            Q_EMIT finished(this);
            deleteLater();
        } else {
            if ( Solid::Networking::status() == Solid::Networking::Connected || Solid::Networking::status() == Solid::Networking::Unknown) {
                if (!mNetworkAccessManager) {
                    mNetworkAccessManager = new QNetworkAccessManager(this);
                    connect(mNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotFinishLoadPixmap(QNetworkReply*)));
                }
                QNetworkReply *reply = mNetworkAccessManager->get(QNetworkRequest(url));
                connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
            } else {
                qDebug() <<" network is not connected";
                deleteLater();
                return;
            }
        }
    } else {
        qDebug() << "Gravatar can not start";
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

QString GravatarResolvUrlJob::calculateHash()
{
    //KF5 add support for libravatar
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(mEmail.toLower().toUtf8());
    return QString::fromUtf8(hash.result().toHex());
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

KUrl GravatarResolvUrlJob::createUrl()
{
    mCalculatedHash.clear();
    if (!canStart()) {
        return KUrl();
    }
    KUrl url;
    url.setScheme(QStringLiteral("http"));
    url.setHost(QStringLiteral("www.gravatar.com"));
    url.setPort(80);
    mCalculatedHash = calculateHash();
    url.setPath(QStringLiteral("/avatar/") + mCalculatedHash);
    if (!mUseDefaultPixmap) {
        //Add ?d=404
        url.addQueryItem(QStringLiteral("d"), QStringLiteral("404"));
    }
    if (mSize != 80) {
        url.addQueryItem(QStringLiteral("s"), QString::number(mSize));
    }
    return url;
}
