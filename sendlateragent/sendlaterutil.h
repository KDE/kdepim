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

#ifndef SENDLATERUTIL_H
#define SENDLATERUTIL_H

#include "sendlater_export.h"
#include <KSharedConfig>

namespace SendLater {
class SendLaterInfo;
namespace SendLaterUtil
{
    SENDLATER_EXPORT bool compareSendLaterInfo(SendLater::SendLaterInfo *left, SendLater::SendLaterInfo *right);

    SENDLATER_EXPORT KSharedConfig::Ptr defaultConfig();

    SENDLATER_EXPORT void writeSendLaterInfo(SendLater::SendLaterInfo *info, bool forceReload=true);

    SENDLATER_EXPORT bool sentLaterAgentWasRegistered();

    SENDLATER_EXPORT bool sentLaterAgentEnabled();

    SENDLATER_EXPORT void reload();

    SENDLATER_EXPORT void changeRecurrentDate(SendLater::SendLaterInfo *info);

    static QString sendLaterPattern = QLatin1String("SendLaterItem %1");
}
}
#endif // SENDLATERUTIL_H
