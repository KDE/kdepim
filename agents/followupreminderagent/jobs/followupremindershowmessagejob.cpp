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

#include "followupremindershowmessagejob.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusConnectionInterface>
#include <ktoolinvocation.h>

FollowUpReminderShowMessageJob::FollowUpReminderShowMessageJob(Akonadi::Item::Id id, QObject *parent)
    : QObject(parent),
      mId(id)
{
}

FollowUpReminderShowMessageJob::~FollowUpReminderShowMessageJob()
{
}

void FollowUpReminderShowMessageJob::start()
{
    if (mId < 0) {
        qDebug()<<" value < 0";
        deleteLater();
        return;
    }
    const QString kmailInterface = QLatin1String("org.kde.kmail");
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(kmailInterface)) {
        // Program is not already running, so start it
        QString errmsg;
        if (KToolInvocation::startServiceByDesktopName(QLatin1String("kmail2"), QString(), &errmsg)) {
            qDebug()<<" Can not start kmail"<<errmsg;
            deleteLater();
            return;
        }
    }
    QDBusInterface kmail(kmailInterface, QLatin1String("/KMail"), QLatin1String("org.kde.kmail.kmail"));
    kmail.call(QLatin1String("showMail"), mId);
    deleteLater();
}
