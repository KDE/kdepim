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

#include "boxjob.h"

using namespace PimCommon;

BoxJob::BoxJob(QObject *parent)
    : PimCommon::OAuth2Job(parent)
{
    mClientId = QLatin1String("o4sn4e0dvz50pd3ps6ao3qxehvqv8dyo");
    mClientSecret = QLatin1String("wLdaOgrblYzi1Y6WN437wStvqighmSJt");
    mRedirectUri = QLatin1String("https://bugs.kde.org/");
    mServiceUrl = QLatin1String("https://app.box.com");
    mAuthorizePath = QLatin1String("/api/oauth2/authorize/");
}

BoxJob::~BoxJob()
{

}
