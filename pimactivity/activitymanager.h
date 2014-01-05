/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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

#ifndef ACTIVITYMANAGER_H
#define ACTIVITYMANAGER_H

#include "pimactivity_export.h"
#include <kactivities/consumer.h>

#include <KSharedConfig>

#include <QObject>

namespace PimActivity {
class ActivityManagerPrivate;
class PIMACTIVITY_EXPORT ActivityManager : public QObject
{
    Q_OBJECT
public:
    explicit ActivityManager(QObject *parent = 0);
    ~ActivityManager();

    enum SelectComponent {
        None = 0,
        Identity = 1,
        MailTransport = 2,
        Collection = 4
    };
    Q_DECLARE_FLAGS( SelectComponents, SelectComponent )

    void setSelectComponents(ActivityManager::SelectComponents selection);
    ActivityManager::SelectComponents selectComponents() const;

    bool isActive() const;

    QStringList listActivities() const;
    QHash<QString, QString> listActivitiesWithRealName() const;
    QString currentActivity() const;

    void setEnabledActivity(bool enabled);
    bool isEnabledActivity() const;

    static KSharedConfigPtr configFromActivity(const QString &id);

Q_SIGNALS:
    void serviceStatusChanged(KActivities::Consumer::ServiceStatus);
    void activityAdded(const QString &);
    void activityRemoved(const QString &);
    void currentActivityChanged(const QString &);
    void enabledActivityChanged(bool enabled);

private:
    friend class ActivityManagerPrivate;
    ActivityManagerPrivate * const d;
    Q_PRIVATE_SLOT( d, void slotActivityAdded(const QString &))
    Q_PRIVATE_SLOT( d, void slotActivityRemoved(const QString &))
};
}

#endif // ACTIVITYMANAGER_H
