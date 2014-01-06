/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "transportcomboboxactivity.h"
#include "activitymanager.h"

namespace PimActivity {

class TransportComboboxActivityPrivate
{
public:
    TransportComboboxActivityPrivate(ActivityManager *manager, TransportComboboxActivity *qq)
        : q(qq),
          activityManager(manager)
    {
    }

    void connectSignals()
    {
        if (activityManager) {
            q->connect(activityManager, SIGNAL(currentActivityChanged(QString)), q, SLOT(slotCurrentActivityChanged(QString)));
            //Initialize it.
            slotCurrentActivityChanged(activityManager->currentActivity());
        }
    }

    void slotCurrentActivityChanged(const QString &id)
    {
        q->updateComboboxList();
    }

    TransportComboboxActivity *q;
    ActivityManager *activityManager;
};

TransportComboboxActivity::TransportComboboxActivity(QWidget *parent)
    : MailTransport::TransportComboBox(parent), d(new TransportComboboxActivityPrivate(0, this))
{
}

TransportComboboxActivity::TransportComboboxActivity(ActivityManager *manager, QWidget *parent)
    : MailTransport::TransportComboBox(parent), d(new TransportComboboxActivityPrivate(manager, this))
{
    d->connectSignals();
}

TransportComboboxActivity::~TransportComboboxActivity()
{
    delete d;
}

void TransportComboboxActivity::setActivityManager(ActivityManager *manager)
{
    d->activityManager = manager;
    d->connectSignals();
}

void TransportComboboxActivity::updateComboboxList()
{
    //TODO
}

}

#include "moc_transportcomboboxactivity.cpp"
