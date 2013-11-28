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


using namespace PimCommon;

DropBoxStorageService::DropBoxStorageService(QObject *parent)
    : PimCommon::StorageServiceAbstract(parent),
      mDropBoxToken(new PimCommon::DropBoxToken)
{
    readConfig();
    if (mAccessToken.isEmpty()) {
        mDropBoxToken->getTokenAccess();
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
    mAccessTokenSecret = grp.readEntry("Access Secret");
    mAccessOauthToken = grp.readEntry("Access Oauth Token");
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
