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

#ifndef TRANSPORTCOMBOBOXACTIVITY_H
#define TRANSPORTCOMBOBOXACTIVITY_H

#include "pimactivity_export.h"

#include <Mailtransport/TransportComboBox>

namespace PimActivity {
class ActivityManager;
class TransportComboboxActivityPrivate;
class PIMACTIVITY_EXPORT TransportComboboxActivity : public MailTransport::TransportComboBox
{
    Q_OBJECT
public:
    explicit TransportComboboxActivity(QWidget *parent = 0);
    explicit TransportComboboxActivity(ActivityManager *manager, QWidget *parent = 0);
    ~TransportComboboxActivity();

    void setActivityManager(ActivityManager *manager);

protected Q_SLOTS:
    void updateComboboxList();

private:
    friend class TransportComboboxActivityPrivate;
    TransportComboboxActivityPrivate * const d;
    Q_PRIVATE_SLOT( d, void slotCurrentActivityChanged(const QString&))
};
}

#endif // TRANSPORTCOMBOBOXACTIVITY_H
