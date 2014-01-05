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

#ifndef COMBOBOXACTIVITY_H
#define COMBOBOXACTIVITY_H

#include <KComboBox>

namespace PimActivity {
class ActivityManager;
class ComboBoxActivityPrivate;
class ComboBoxActivity : public KComboBox
{
    Q_OBJECT
public:
    explicit ComboBoxActivity(ActivityManager *activityManager, QWidget *parent = 0);
    ~ComboBoxActivity();

Q_SIGNALS:
    //Emit activity identity
    void activityChanged(const QString &id);
private:
    friend class ComboBoxActivityPrivate;
    ComboBoxActivityPrivate * const d;
    Q_PRIVATE_SLOT( d, void slotActivityAdded(const QString&) )
    Q_PRIVATE_SLOT( d, void slotActivityRemoved(const QString&) )
    Q_PRIVATE_SLOT( d, void slotActivityStatusChanged(KActivities::Consumer::ServiceStatus) )
    Q_PRIVATE_SLOT( d, void slotActivityChanged(int) )
};
}

#endif // COMBOBOXACTIVITY_H
