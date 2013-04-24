/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "identitycomboboxactivity.h"
#include "activitymanager.h"

namespace PimActivity {
class IdentityComboboxActivityPrivate
{
public:
    IdentityComboboxActivityPrivate(ActivityManager *manager, IdentityComboboxActivity *qq)
        : activityManager(manager),
          q(qq)
    {
    }

    void connectSignals()
    {
        if (activityManager) {
            q->connect(activityManager, SIGNAL(currentActivityChanged(QString)), q, SLOT(slotCurrentActivityChanged(QString)));
        }
    }

    void slotCurrentActivityChanged(const QString &id)
    {
        //TODO
        q->updateComboboxList(id);
    }

    ActivityManager *activityManager;
    IdentityComboboxActivity *q;
};


IdentityComboboxActivity::IdentityComboboxActivity(QWidget *parent)
    : KComboBox(parent), d(new IdentityComboboxActivityPrivate(0, this))
{
}


IdentityComboboxActivity::IdentityComboboxActivity(ActivityManager *manager, QWidget *parent)
    : KComboBox(parent), d(new IdentityComboboxActivityPrivate(manager, this))
{
    d->connectSignals();
}

IdentityComboboxActivity::~IdentityComboboxActivity()
{
    delete d;
}

void IdentityComboboxActivity::setActivityManager(ActivityManager *manager)
{
    d->activityManager = manager;
    d->connectSignals();
}

void IdentityComboboxActivity::updateComboboxList(const QString &id)
{
    //TODO
}


}

#include "identitycomboboxactivity.moc"
