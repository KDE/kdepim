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
#if 0
        q->connect(manager,SIGNAL(serviceStatusChanged(KActivities::Consumer::ServiceStatus)));
        void activityAdded(const QString & id);
        void activityRemoved(const QString & id);
#endif



        q->addItems(activityManager->listActivities());
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
