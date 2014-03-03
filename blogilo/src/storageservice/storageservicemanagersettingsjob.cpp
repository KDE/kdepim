/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "storageservicemanagersettingsjob.h"

using namespace PimCommon;

StorageServiceManagerSettingsJob::StorageServiceManagerSettingsJob(QObject *parent)
    : QObject(parent)
{
}

StorageServiceManagerSettingsJob::~StorageServiceManagerSettingsJob()
{

}

QString StorageServiceManagerSettingsJob::youSendItApiKey() const
{
    //TODO customize it
    return QLatin1String("fnab8fkgwrka7v6zs2ycd34a");
}

QString StorageServiceManagerSettingsJob::dropboxOauthConsumerKey() const
{
    //TODO customize it
    return QLatin1String("e40dvomckrm48ci");
}

QString StorageServiceManagerSettingsJob::dropboxOauthSignature() const
{
    //TODO customize it
    return QLatin1String("0icikya464lny9g&");
}

QString StorageServiceManagerSettingsJob::boxClientId() const
{
    //Blogilo id
    return QLatin1String("y7out38qqnbdmfsa099h00fmnzuhudgo");
}

QString StorageServiceManagerSettingsJob::boxClientSecret() const
{
    //Blogilo secret
    return QLatin1String("wEN5qWmwkEE6nZHm3jUUKmRcy8blApDH");
}

QString StorageServiceManagerSettingsJob::hubicClientId() const
{
    return QLatin1String("api_hubic_zBKQ6UDUj2vDT7ciDsgjmXA78OVDnzJi");
}

QString StorageServiceManagerSettingsJob::hubicClientSecret() const
{
    return QLatin1String("pkChgk2sRrrCEoVHmYYCglEI9E2Y2833Te5Vn8n2J6qPdxLU6K8NPUvzo1mEhyzf");
}

QString StorageServiceManagerSettingsJob::dropboxRootPath() const
{
    return QLatin1String("dropbox");
}

QString StorageServiceManagerSettingsJob::oauth2RedirectUrl() const
{
    return QLatin1String("https://bugs.kde.org/");
}

QString StorageServiceManagerSettingsJob::ubuntuOneAttachmentVolume() const
{
    return QLatin1String("/~/KMail Attachments");
}

QString StorageServiceManagerSettingsJob::hubicScope() const
{
    return QLatin1String("usage.r,account.r,credentials.r,links.wd");
}

QString StorageServiceManagerSettingsJob::ubuntuOneTokenName() const
{
    return QLatin1String("kde storagemanager");
}

QString StorageServiceManagerSettingsJob::gdriveClientId() const
{
    return QLatin1String("735222197981-mrcgtaqf05914buqjkts7mk79blsquas.apps.googleusercontent.com");
    //return QLatin1String("76182239499-2krm3lvlrqrj446loaqrrep594n3u2o8.apps.googleusercontent.com");
}

QString StorageServiceManagerSettingsJob::gdriveClientSecret() const
{
    return QLatin1String("4MJOS0u1-_AUEKJ0ObA-j22U");
    //return QLatin1String("7SinUSCxfbrJYN7az3VvxTJ9");
}

QString StorageServiceManagerSettingsJob::defaultUploadFolder() const
{
    return QString();
}
