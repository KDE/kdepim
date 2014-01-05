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

#include "testsettingsjob.h"

using namespace PimCommon;

TestSettingsJob::TestSettingsJob(QObject *parent)
    : QObject(parent)
{
}

TestSettingsJob::~TestSettingsJob()
{

}

QString TestSettingsJob::youSendItApiKey() const
{
    //TODO customize it
    return QLatin1String("fnab8fkgwrka7v6zs2ycd34a");
}

QString TestSettingsJob::dropboxOauthConsumerKey() const
{
    return QLatin1String("e40dvomckrm48ci");
}

QString TestSettingsJob::dropboxOauthSignature() const
{
    return QLatin1String("0icikya464lny9g&");
}
