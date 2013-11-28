/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "dropboxstorageservice.h"
#include "dropboxtoken.h"

#include <KLocale>
#include <KConfig>
#include <KGlobal>
#include <KConfigGroup>

#include <QDebug>


using namespace PimCommon;

DropBoxStorageService::DropBoxStorageService(QObject *parent)
    : PimCommon::StorageServiceAbstract(parent),
      mDropBoxToken(new PimCommon::DropBoxToken),
      mInitialized(false)
{
    connect(mDropBoxToken, SIGNAL(authorizationDone(QString,QString,QString)), this, SLOT(slotAuthorizationDone(QString,QString,QString)));
    connect(mDropBoxToken, SIGNAL(authorizationFailed()), this, SLOT(slotAuthorizationFailed()));
    readConfig();
    if (mAccessToken.isEmpty()) {
        qDebug()<<" try to got accesstoken";
        mDropBoxToken->requestTokenAccess();
    } else {
        mDropBoxToken->initializeToken(mAccessToken, mAccessTokenSecret, mAccessOauthSignature);
        qDebug()<<" access ok";
        mInitialized = true;
    }
}

DropBoxStorageService::~DropBoxStorageService()
{
    delete mDropBoxToken;
}

void DropBoxStorageService::readConfig()
{
    KConfigGroup grp(KGlobal::config(), "Dropbox Settings");

    mAccessToken = grp.readEntry("Access Token");
    mAccessTokenSecret = grp.readEntry("Access Token Secret");
    mAccessOauthSignature = grp.readEntry("Access Oauth Signature");
}

void DropBoxStorageService::slotAuthorizationDone(const QString &accessToken, const QString &accessTokenSecret, const QString &accessOauthSignature)
{
    qDebug()<<"*****************************accessToken "<<accessToken<<" accessTokenSecret"<<accessTokenSecret<<"accessOauthSignature "<<accessOauthSignature;
    mAccessToken = accessToken;
    mAccessTokenSecret = accessTokenSecret;
    mAccessOauthSignature = accessOauthSignature;
    KConfigGroup grp(KGlobal::config(), "Dropbox Settings");
    grp.writeEntry("Access Token", mAccessToken);
    grp.writeEntry("Access Token Secret", mAccessTokenSecret);
    grp.writeEntry("Access Oauth Signature", mAccessOauthSignature);
    grp.sync();
    KGlobal::config()->sync();
    mInitialized = true;
}

void DropBoxStorageService::listFolder()
{
    mDropBoxToken->listFolders();
}

void DropBoxStorageService::slotAuthorizationFailed()
{
    mAccessToken.clear();
    mAccessTokenSecret.clear();
    mAccessOauthSignature.clear();
    mInitialized = true;
}

QString DropBoxStorageService::name() const
{
    return i18n("DropBox");
}

qint64 DropBoxStorageService::maximumSize() const
{
    return -1;
}

qint64 DropBoxStorageService::currentSize() const
{
    return -1;
}

QUrl DropBoxStorageService::sharedUrl() const
{
    return QUrl();
}

void DropBoxStorageService::uploadFile(const QString &filename)
{
    if (mInitialized) {

    } else {
        mDropBoxToken->getTokenAccess();
    }
    //TODO
}

QString DropBoxStorageService::description() const
{
    return QString();
}

QUrl DropBoxStorageService::serviceUrl() const
{
    return QUrl();
}
