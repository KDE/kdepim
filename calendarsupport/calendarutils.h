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
#ifndef CALENDARUTILS_H
#define CALENDARUTILS_H

#include "calendarsupport_export.h"

#include <QtCore/QObject>

namespace Akonadi {
class Item;
}

namespace CalendarSupport {

class Calendar;
class CalendarUtilsPrivate;

class CALENDARSUPPORT_EXPORT CalendarUtils : public QObject
{
  Q_OBJECT
public:
  explicit CalendarUtils( Calendar *calendar, QObject *parent = 0 );
  ~CalendarUtils();

  /** Makes the incidence from @param item independent from its parent. Returns
      true when the incidence was made independent, false otherwise.
   */
  bool makeIndependent( const Akonadi::Item &item );

  /** Make all children of the incindence from @param item independent
      Works with any incidence type, although currently we only pass to-dos.
      Returns true when one or more incidence(s) where made independent, false
      otherwise.
   */
  bool makeChildrenIndependent( const Akonadi::Item &item );

private:
  CalendarUtilsPrivate * const d_ptr;
  Q_DECLARE_PRIVATE( CalendarUtils )
};

}

#endif // CALENDARUTILS_H
