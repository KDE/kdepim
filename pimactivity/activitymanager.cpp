/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "activitymanager.h"

#include <QDebug>

namespace PimActivity {

class ActivityManagerPrivate
{
public:
    ActivityManagerPrivate(ActivityManager *qq)
        : q(qq)
    {
        consumer = new KActivities::Consumer;
        const QStringList activities = consumer->listActivities();
        Q_FOREACH (const QString &id, activities) {
            insertActivity(id);
        }

        q->connect(consumer,SIGNAL(serviceStatusChanged(KActivities::Consumer::ServiceStatus)),q,SIGNAL(serviceStatusChanged(KActivities::Consumer::ServiceStatus)));
        q->connect(consumer,SIGNAL(activityAdded(QString)),q,SLOT(slotActivityAdded(QString)));
        q->connect(consumer,SIGNAL(activityRemoved(QString)),q,SLOT(slotActivityRemoved(QString)));
    }
    ~ActivityManagerPrivate()
    {
        delete consumer;
    }

    void insertActivity(const QString &id)
    {
        KActivities::Info *activity = new KActivities::Info(id, q);
        activities[id] = activity;
    }

    void slotActivityRemoved(const QString &id)
    {
        KActivities::Info *activity = activities.take(id);
        delete activity;
        emit q->activityRemoved(id);
    }

    void slotActivityAdded(const QString &id)
    {
        insertActivity(id);
        emit q->activityAdded(id);
    }

    QHash<QString, QString> listActivitiesWithRealName() const
    {
        QHash<QString, QString> result;
        QHashIterator<QString, KActivities::Info*> i(activities);
        while (i.hasNext()) {
            i.next();
            result.insert(i.key(), i.value()->name());
        }
        return result;
    }

    QHash<QString, KActivities::Info*> activities;
    ActivityManager *q;
    KActivities::Consumer *consumer;

};

ActivityManager::ActivityManager(QObject *parent)
    : QObject(parent), d(new ActivityManagerPrivate(this))
{
    if (KActivities::Consumer::serviceStatus() == KActivities::Consumer::NotRunning)  {
        qDebug()<<" kactivities is not running";
    }

}

ActivityManager::~ActivityManager()
{
    delete d;
}


bool ActivityManager::isActive() const
{
    return (KActivities::Consumer::serviceStatus() == KActivities::Consumer::Running);
}

QHash<QString, QString> ActivityManager::listActivitiesWithRealName() const
{
    return d->listActivitiesWithRealName();
}

QStringList ActivityManager::listActivities() const
{
    if (isActive()) {
        return d->consumer->listActivities();
    }
    return QStringList();
}

}

#include "activitymanager.moc"
