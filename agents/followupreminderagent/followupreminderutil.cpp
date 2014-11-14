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
#include "followupreminderinfo.h"

#include <QDBusInterface>
#include "followupreminderagentsettings.h"

bool FollowUpReminder::FollowUpReminderUtil::followupReminderAgentWasRegistered()
{
    QDBusInterface interface(QStringLiteral("org.freedesktop.Akonadi.Agent.akonadi_followupreminder_agent"), QStringLiteral("/FollowUpReminder"));
    return interface.isValid();
}

bool FollowUpReminder::FollowUpReminderUtil::followupReminderAgentEnabled()
{
    return FollowUpReminderAgentSettings::self()->enabled();
}

void FollowUpReminder::FollowUpReminderUtil::reload()
{
    QDBusInterface interface(QStringLiteral("org.freedesktop.Akonadi.Agent.akonadi_followupreminder_agent"), QStringLiteral("/FollowUpReminder"));
    if (interface.isValid()) {
        interface.call(QStringLiteral("reload"));
    }
}

void FollowUpReminder::FollowUpReminderUtil::forceReparseConfiguration()
{
    FollowUpReminderAgentSettings::self()->save();
    FollowUpReminderAgentSettings::self()->config()->reparseConfiguration();
}

KSharedConfig::Ptr FollowUpReminder::FollowUpReminderUtil::defaultConfig()
{
    return KSharedConfig::openConfig(QStringLiteral("akonadi_followupreminder_agentrc"));
}

void FollowUpReminder::FollowUpReminderUtil::writeFollowupReminderInfo(KSharedConfig::Ptr config, FollowUpReminder::FollowUpReminderInfo *info, bool forceReload)
{
    if (!info || !info->isValid()) {
        return;
    }

    KConfigGroup general = config->group(QStringLiteral("General"));
    int value = general.readEntry("Number", 0);
    int identifier = info->uniqueIdentifier();
    if (identifier == -1) {
        identifier = value;
    }
    ++value;

    const QString groupName = FollowUpReminder::FollowUpReminderUtil::followUpReminderPattern.arg(identifier);
    // first, delete all filter groups:
    const QStringList filterGroups = config->groupList();
    foreach (const QString &group, filterGroups) {
        if (group == groupName)  {
            config->deleteGroup(group);
        }
    }
    KConfigGroup group = config->group(groupName);
    info->writeConfig(group, identifier);

    general.writeEntry("Number", value);

    config->sync();
    config->reparseConfiguration();
    if (forceReload) {
        reload();
    }
}

bool FollowUpReminder::FollowUpReminderUtil::removeFollowupReminderInfo(KSharedConfig::Ptr config, const QList<qint32> &listRemove, bool forceReload)
{
    if (listRemove.isEmpty()) {
        return false;
    }

    bool needSaveConfig = false;
    KConfigGroup general = config->group(QStringLiteral("General"));
    int value = general.readEntry("Number", 0);

    Q_FOREACH (qint32 identifier, listRemove) {
        const QString groupName = FollowUpReminder::FollowUpReminderUtil::followUpReminderPattern.arg(identifier);
        const QStringList filterGroups = config->groupList();
        foreach (const QString &group, filterGroups) {

            if (group == groupName)  {
                config->deleteGroup(group);
                --value;
                needSaveConfig = true;
            }
        }
    }
    if (needSaveConfig) {
        general.writeEntry("Number", value);

        config->sync();
        config->reparseConfiguration();
        if (forceReload) {
            reload();
        }
    }
    return needSaveConfig;
}
