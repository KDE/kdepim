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
#include <kactivities/consumer.h>

#include <QDebug>

namespace PimActivity {

class ActivityManagerPrivate
{
public:
    ActivityManagerPrivate()
    {
        consumer = new KActivities::Consumer;
    }
    ~ActivityManagerPrivate()
    {
        delete consumer;
    }

    KActivities::Consumer *consumer;

};

ActivityManager::ActivityManager(QObject *parent)
    : QObject(parent), d(new ActivityManagerPrivate)
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

QStringList ActivityManager::listActivities() const
{
    if (isActive()) {
        return d->consumer->listActivities();
    }
    return QStringList();
}

}

#include "activitymanager.moc"
