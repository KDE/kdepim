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

#include "followupreminderutil.h"

#include <QDBusInterface>
#include "followupreminderagentsettings.h"

bool FollowUpReminder::FollowUpReminderUtil::sentLaterAgentWasRegistered()
{
    QDBusInterface interface( QLatin1String("org.freedesktop.Akonadi.Agent.akonadi_followupreminder_agent"), QLatin1String("/FollowUpReminder") );
    return interface.isValid();
}

bool FollowUpReminder::FollowUpReminderUtil::sentLaterAgentEnabled()
{
    return FollowUpReminderAgentSettings::self()->enabled();
}

void FollowUpReminder::FollowUpReminderUtil::reload()
{
    QDBusInterface interface( QLatin1String("org.freedesktop.Akonadi.Agent.akonadi_followupreminder_agent"), QLatin1String("/FollowUpReminder") );
    if (interface.isValid()) {
        interface.call(QLatin1String("reload"));
    }
}

void FollowUpReminder::FollowUpReminderUtil::forceReparseConfiguration()
{
    FollowUpReminderAgentSettings::self()->writeConfig();
    FollowUpReminderAgentSettings::self()->config()->reparseConfiguration();
}

