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
#include <QCryptographicHash>
#include <QStringList>
#include <QDebug>

using namespace PimCommon;

GravatarResolvUrlJob::GravatarResolvUrlJob(QObject *parent)
    : QObject(parent)
{

}

GravatarResolvUrlJob::~GravatarResolvUrlJob()
{

}

bool GravatarResolvUrlJob::canStart() const
{
    return !mEmail.trimmed().isEmpty() && (mEmail.contains(QLatin1Char('@')));
}

KUrl GravatarResolvUrlJob::generateGravatarUrl()
{
    return createUrl();
}

bool GravatarResolvUrlJob::hasGravatar() const
{
    //TODO
    return true;
}

void GravatarResolvUrlJob::start()
{
    if (canStart()) {
        mCalculatedHash.clear();
        const KUrl url = createUrl();
    } else {
        //TODO return message Error.
        deleteLater();
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
    url.setScheme(QLatin1String("http"));
    url.setHost(QLatin1String("www.gravatar.com"));
    url.setPort(80);
    mCalculatedHash = calculateHash();
    url.setPath(QLatin1String("/avatar/") + mCalculatedHash);
    //Add ?d=404
    url.addQueryItem(QLatin1String("d"), QLatin1String("404"));
    return url;
}
