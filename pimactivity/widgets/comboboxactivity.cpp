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

#include "comboboxactivity.h"
#include "activitymanager.h"
#include <KActivities/Consumer>

namespace PimActivity {

class ComboBoxActivityPrivate
{
public:
    ComboBoxActivityPrivate(ComboBoxActivity *qq, ActivityManager *manager)
        : q(qq),
          activityManager(manager)
    {
        const QHash<QString, QString> list = activityManager->listActivitiesWithRealName();
        QHashIterator<QString, QString> i(list);
        while (i.hasNext()) {
            i.next();
            q->addItem(i.value(), i.key());
        }
        q->connect(manager, SIGNAL(activityAdded(QString)), q, SLOT(slotActivityAdded(QString)));
        q->connect(manager, SIGNAL(activityRemoved(QString)), q, SLOT(slotActivityRemoved(QString)));
        q->connect(manager, SIGNAL(serviceStatusChanged(KActivities::Consumer::ServiceStatus)), q, SLOT(slotActivityStatusChanged(KActivities::Consumer::ServiceStatus)));
        q->connect(q, SIGNAL(activated(int)), q, SLOT(slotActivityChanged(int)));
        q->setEnabled(activityManager->isActive());
    }

    void slotActivityAdded(const QString &name)
    {
        q->addItem(name);
    }

    void slotActivityRemoved(const QString &name)
    {
        q->removeItem(q->findText(name));
    }

    void slotActivityStatusChanged(KActivities::Consumer::ServiceStatus status)
    {
        q->setEnabled(status == KActivities::Consumer::Running);
    }

    void slotActivityChanged(int index)
    {
        Q_EMIT q->activityChanged(q->itemData(index).toString());
    }

    ComboBoxActivity *q;
    ActivityManager *activityManager;
};

ComboBoxActivity::ComboBoxActivity(ActivityManager *activityManager, QWidget *parent)
    : KComboBox(parent), d(new ComboBoxActivityPrivate(this, activityManager))
{
}

ComboBoxActivity::~ComboBoxActivity()
{
    delete d;
}

}

#include "comboboxactivity.moc"
