/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include "sendlaterutil.h"
#include "sendlaterinfo.h"
#include "sendlateragentsettings.h"
#include "libsendlater_debug.h"
#include <KConfigGroup>

#include <QDBusInterface>
#include <QStringList>

bool SendLater::SendLaterUtil::compareSendLaterInfo(SendLater::SendLaterInfo *left, SendLater::SendLaterInfo *right)
{
    if (left->dateTime() == right->dateTime()) {
        //Set no recursive first.
        if (left->isRecurrence())  {
            return false;
        }
    }
    return left->dateTime() < right->dateTime();
}

void SendLater::SendLaterUtil::changeRecurrentDate(SendLater::SendLaterInfo *info)
{
    if (info && info->isRecurrence()) {
        qCDebug(LIBSENDLATER_LOG) << "BEFORE SendLater::SendLaterUtil::changeRecurrentDate " << info->dateTime().toString();
        QDateTime newInfoDateTime = info->dateTime();
        newInfoDateTime = updateRecurence(info, newInfoDateTime);
        qCDebug(LIBSENDLATER_LOG) << " QDateTime::currentDateTime()" << QDateTime::currentDateTime().toString();
        while (newInfoDateTime <= QDateTime::currentDateTime()) {
            newInfoDateTime = updateRecurence(info, newInfoDateTime);
        }
        info->setDateTime(newInfoDateTime);
        qCDebug(LIBSENDLATER_LOG) << "AFTER SendLater::SendLaterUtil::changeRecurrentDate " << info->dateTime().toString() << " info" << info << "New date" << newInfoDateTime;
        writeSendLaterInfo(defaultConfig(), info, true);
    }
}

QDateTime SendLater::SendLaterUtil::updateRecurence(SendLater::SendLaterInfo *info, QDateTime dateTime)
{
    switch (info->recurrenceUnit()) {
    case SendLater::SendLaterInfo::Days:
        dateTime = dateTime.addDays(info->recurrenceEachValue());
        break;
    case SendLater::SendLaterInfo::Weeks:
        dateTime = dateTime.addDays(info->recurrenceEachValue() * 7);
        break;
    case SendLater::SendLaterInfo::Months:
        dateTime = dateTime.addMonths(info->recurrenceEachValue());
        break;
    case SendLater::SendLaterInfo::Years:
        dateTime = dateTime.addYears(info->recurrenceEachValue());
        break;
    }
    return dateTime;
}

KSharedConfig::Ptr SendLater::SendLaterUtil::defaultConfig()
{
    return KSharedConfig::openConfig(QStringLiteral("akonadi_sendlater_agentrc"), KConfig::SimpleConfig);
}

void SendLater::SendLaterUtil::writeSendLaterInfo(KSharedConfig::Ptr config, SendLater::SendLaterInfo *info, bool forceReload)
{
    if (!info || !info->isValid()) {
        return;
    }

    const QString groupName = SendLater::SendLaterUtil::sendLaterPattern.arg(info->itemId());
    // first, delete all filter groups:
    const QStringList filterGroups = config->groupList();
    foreach (const QString &group, filterGroups) {
        if (group == groupName) {
            config->deleteGroup(group);
        }
    }
    KConfigGroup group = config->group(groupName);
    info->writeConfig(group);
    config->sync();
    config->reparseConfiguration();
    qCDebug(LIBSENDLATER_LOG) << " reparse config";
    if (forceReload) {
        reload();
    }
}

bool SendLater::SendLaterUtil::sentLaterAgentWasRegistered()
{
    QDBusInterface interface(QStringLiteral("org.freedesktop.Akonadi.Agent.akonadi_sendlater_agent"), QStringLiteral("/SendLaterAgent"));
    return interface.isValid();
}

void SendLater::SendLaterUtil::forceReparseConfiguration()
{
    SendLaterAgentSettings::self()->save();
    SendLaterAgentSettings::self()->config()->reparseConfiguration();
}

bool SendLater::SendLaterUtil::sentLaterAgentEnabled()
{
    return SendLaterAgentSettings::self()->enabled();
}

void SendLater::SendLaterUtil::reload()
{
    qCDebug(LIBSENDLATER_LOG) << " void SendLater::SendLaterUtil::reload()";
    QDBusInterface interface(QStringLiteral("org.freedesktop.Akonadi.Agent.akonadi_sendlater_agent"), QStringLiteral("/SendLaterAgent"));
    if (interface.isValid()) {
        interface.call(QStringLiteral("reload"));
    } else {
        qCDebug(LIBSENDLATER_LOG) << " Can not reload list";
    }
}

