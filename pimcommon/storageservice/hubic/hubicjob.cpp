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

#include "hubicjob.h"
#include "pimcommon/storageservice/storageserviceabstract.h"
#include "pimcommon/storageservice/storageservicejobconfig.h"

#include <qjson/parser.h>

#include <QDebug>

using namespace PimCommon;

HubicJob::HubicJob(QObject *parent)
    : PimCommon::OAuth2Job(parent)
{
    mClientId = QLatin1String("api_hubic_zBKQ6UDUj2vDT7ciDsgjmXA78OVDnzJi");
    mClientSecret = QLatin1String("pkChgk2sRrrCEoVHmYYCglEI9E2Y2833Te5Vn8n2J6qPdxLU6K8NPUvzo1mEhyzf");
    mRedirectUri = QLatin1String("https://bugs.kde.org/");
    mServiceUrl = QLatin1String("https://api.hubic.com");
    mApiUrl = QLatin1String("https://api.hubic.com");
    mScope = QLatin1String("usage.r,account.r,credentials.r,links.wd");
    mAuthorizePath = QLatin1String("/oauth/auth/");
    mPathToken = QLatin1String("/oauth/token/");
    mCurrentAccountInfoPath = QLatin1String("/1.0/account/usage");
    //FIXME
    //mFolderInfoPath = QLatin1String("/2.0/folders/");
    //mFileInfoPath = QLatin1String("/2.0/files/");
}

HubicJob::~HubicJob()
{

}

void HubicJob::parseAccountInfo(const QString &data)
{
    QJson::Parser parser;
    bool ok;

    const QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    PimCommon::AccountInfo accountInfo;
    if (info.contains(QLatin1String("used"))) {
        accountInfo.shared = info.value(QLatin1String("used")).toLongLong();
    }
    if (info.contains(QLatin1String("quota"))) {
        accountInfo.quota = info.value(QLatin1String("quota")).toLongLong();
    }
    Q_EMIT accountInfoDone(accountInfo);


    qDebug()<<" info"<<info;
    //TODO
    deleteLater();
}

