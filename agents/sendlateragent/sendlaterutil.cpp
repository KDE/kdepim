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


#include "sendlaterutil.h"
#include "sendlaterinfo.h"
#include "sendlateragentsettings.h"

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
        //qDebug()<<" SendLater::SendLaterUtil::changeRecurrentDate "<<info->dateTime().toString();
        QDateTime newInfoDateTime = info->dateTime();
        while (newInfoDateTime < QDateTime::currentDateTime()) {
            switch(info->recurrenceUnit()) {
            case SendLater::SendLaterInfo::Days:
                newInfoDateTime = newInfoDateTime.addDays(info->recurrenceEachValue());
                break;
            case SendLater::SendLaterInfo::Weeks:
                newInfoDateTime = newInfoDateTime.addDays(info->recurrenceEachValue()*7);
                break;
            case SendLater::SendLaterInfo::Months:
                newInfoDateTime = newInfoDateTime.addMonths(info->recurrenceEachValue());
                break;
            case SendLater::SendLaterInfo::Years:
                newInfoDateTime = newInfoDateTime.addYears(info->recurrenceEachValue());
                break;
            }
        }
        info->setDateTime(newInfoDateTime);
        //qDebug()<<"AFTER SendLater::SendLaterUtil::changeRecurrentDate "<<info->dateTime().toString();
        writeSendLaterInfo(defaultConfig(), info, false);
    }
}

KSharedConfig::Ptr SendLater::SendLaterUtil::defaultConfig()
{
    return KSharedConfig::openConfig( QLatin1String("akonadi_sendlater_agentrc") );
}

void SendLater::SendLaterUtil::writeSendLaterInfo(KSharedConfig::Ptr config, SendLater::SendLaterInfo *info, bool forceReload)
{
    if (!info || !info->isValid())
        return;

    const QString groupName = SendLater::SendLaterUtil::sendLaterPattern.arg(info->itemId());
    // first, delete all filter groups:
    const QStringList filterGroups =config->groupList();
    foreach ( const QString &group, filterGroups ) {
        if (group == groupName) {
            config->deleteGroup( group );
        }
    }
    KConfigGroup group = config->group(groupName);
    info->writeConfig(group);
    config->sync();
    config->reparseConfiguration();
    if (forceReload)
        reload();
}

bool SendLater::SendLaterUtil::sentLaterAgentWasRegistered()
{
    QDBusInterface interface( QLatin1String("org.freedesktop.Akonadi.Agent.akonadi_sendlater_agent"), QLatin1String("/SendLaterAgent") );
    return interface.isValid();
}

void SendLater::SendLaterUtil::forceReparseConfiguration()
{
    SendLaterAgentSettings::self()->writeConfig();
    SendLaterAgentSettings::self()->config()->reparseConfiguration();
}

bool SendLater::SendLaterUtil::sentLaterAgentEnabled()
{
    return SendLaterAgentSettings::self()->enabled();
}

void SendLater::SendLaterUtil::reload()
{
    QDBusInterface interface( QLatin1String("org.freedesktop.Akonadi.Agent.akonadi_sendlater_agent"), QLatin1String("/SendLaterAgent") );
    if (interface.isValid()) {
        interface.call(QLatin1String("reload"));
    } else {
        qDebug()<<" Can not reload list";
    }
}

