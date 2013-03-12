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
#ifndef ACTIVITYWARNING_H
#define ACTIVITYWARNING_H

#include <KDE/KMessageWidget>
#include <KActivities/Consumer>

namespace PimActivity {
class ActivityManager;
class ActivityWarning : public KMessageWidget
{
    Q_OBJECT
public:
    explicit ActivityWarning(PimActivity::ActivityManager *activityManager, QWidget *parent = 0);
    ~ActivityWarning();

private Q_SLOTS:
    void setServiceStatusChanged(KActivities::Consumer::ServiceStatus status);
};
}

#endif // ACTIVITYWARNING_H
