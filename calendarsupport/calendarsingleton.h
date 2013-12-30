/*
  Copyright (c) 2013 Sérgio Martins <iamsergio@gmail.com>

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

#ifndef CALENDARSUPPORT_CALENDARSINGLETON_H
#define CALENDARSUPPORT_CALENDARSINGLETON_H

#include "calendarsupport_export.h"

namespace Akonadi {
class ETMCalendar;
}

namespace CalendarSupport {

    /**
     * ETMCalendar to be used by kontact plugins to avoid having
     * 3 loaded calendars which occupy lots of memory.
     *
     * KOrganizer itself can't use this one since it disables calendar selection filtering.
     */
    CALENDARSUPPORT_EXPORT Akonadi::ETMCalendar* calendarSingleton();
}

#endif

