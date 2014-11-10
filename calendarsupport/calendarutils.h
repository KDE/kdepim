/*
  Copyright (C) 2010 Bertjan Broeksema <broeksema@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#ifndef CALENDARSUPPORT_CALENDARUTILS_H
#define CALENDARSUPPORT_CALENDARUTILS_H

#include "calendarsupport_export.h"

#include <Collection>
#include <Akonadi/Calendar/ETMCalendar>

#include <QtCore/QObject>

namespace Akonadi
{
class Item;
}

namespace CalendarSupport
{

class CalendarUtilsPrivate;

/** Some calendar/Incidence related utilitly methods.

    NOTE: this class will only start an modify job for an Item when no other job
          started by this class for the same Item is still running.
 */
class CALENDARSUPPORT_EXPORT CalendarUtils : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new CalendarUtils instance. The instance does not take owner ship
     * over the Calendar.
     */
    explicit CalendarUtils(const Akonadi::ETMCalendar::Ptr &calendar, QObject *parent = 0);

    ~CalendarUtils();

    /**
     * Returns the Caledar on which this utils class is operating.
     */
    Akonadi::ETMCalendar::Ptr calendar() const;

    /**
     * Makes the incidence from @param item independent from its parent. Returns
     * true when the ModifyJob to make the incidence independent was actually
     * started, false otherwise. This method is async, either actionFailed or
     * actionFinished will be emitted when the operation finished or failed.
     */
    bool makeIndependent(const Akonadi::Item &item);

    /**
     * Makes all children of the incindence from @param item independent
     * Returns true when one or more incidence(s) where made independent,
     * false otherwise.
     */
    bool makeChildrenIndependent(const Akonadi::Item &item);

    /** Todo specific methods ***************************************************/

    /**
     * Deletes the completed todos from all active collections in the Calendar.
     */
    void purgeCompletedTodos();

Q_SIGNALS:
    void actionFailed(const Akonadi::Item &item, const QString &msg);
    void actionFinished(const Akonadi::Item &item);

private:
    CalendarUtilsPrivate *const d_ptr;
    Q_DECLARE_PRIVATE(CalendarUtils)

    Q_PRIVATE_SLOT(d_ptr,
                   void handleChangeFinish(int changeId,
                                           const Akonadi::Item &item,
                                           Akonadi::IncidenceChanger::ResultCode resultCode,
                                           const QString &errorString))
};

}

#endif // CALENDARUTILS_H
