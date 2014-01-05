/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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

#include "boxjob.h"
#include "pimcommon/storageservice/storageserviceabstract.h"

#include <qjson/parser.h>

#include <QDebug>

using namespace PimCommon;

BoxJob::BoxJob(QObject *parent)
    : PimCommon::OAuth2Job(parent)
{
    mClientId = QLatin1String("o4sn4e0dvz50pd3ps6ao3qxehvqv8dyo");
    mClientSecret = QLatin1String("wLdaOgrblYzi1Y6WN437wStvqighmSJt");
    mRedirectUri = QLatin1String("https://bugs.kde.org/");
    mServiceUrl = QLatin1String("https://app.box.com");
    mApiUrl = QLatin1String("https://api.box.com");
    mAuthorizePath = QLatin1String("/api/oauth2/authorize/");
    mPathToken = QLatin1String("/api/oauth2/token/");
    mFolderInfoPath = QLatin1String("/2.0/folders/");
    mFileInfoPath = QLatin1String("/2.0/files/");
    mCurrentAccountInfoPath = QLatin1String("/2.0/users/me");
}

BoxJob::~BoxJob()
{

}

void BoxJob::parseAccountInfo(const QString &data)
{
    QJson::Parser parser;
    bool ok;

    const QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    //qDebug()<<" info"<<info;
    PimCommon::AccountInfo accountInfo;
    if (info.contains(QLatin1String("space_used"))) {
        accountInfo.shared = info.value(QLatin1String("space_used")).toLongLong();
    }
    if (info.contains(QLatin1String("space_amount"))) {
        accountInfo.quota = info.value(QLatin1String("space_amount")).toLongLong();
    }
    Q_EMIT accountInfoDone(accountInfo);


    deleteLater();
}

