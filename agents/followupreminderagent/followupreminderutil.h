/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#ifndef FOLLOWUPREMINDERUTIL_H
#define FOLLOWUPREMINDERUTIL_H
#include "followupreminder_export.h"
#include <KSharedConfig>
namespace FollowUpReminder {
class FollowUpReminderInfo;
namespace FollowUpReminderUtil {
FOLLOWUPREMINDER_EXPORT bool followupReminderAgentWasRegistered();

FOLLOWUPREMINDER_EXPORT bool followupReminderAgentEnabled();

FOLLOWUPREMINDER_EXPORT void reload();

FOLLOWUPREMINDER_EXPORT void forceReparseConfiguration();

FOLLOWUPREMINDER_EXPORT KSharedConfig::Ptr defaultConfig();

FOLLOWUPREMINDER_EXPORT void writeFollowupReminderInfo(KSharedConfig::Ptr config, FollowUpReminder::FollowUpReminderInfo *info, bool forceReload);

FOLLOWUPREMINDER_EXPORT bool removeFollowupReminderInfo(KSharedConfig::Ptr config, const QList<qint32> &listRemove);
static QString followUpReminderPattern = QLatin1String("FollowupReminderItem %1");
}
}

#endif // FOLLOWUPREMINDERUTIL_H
