/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
    Copyright (C) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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

#ifndef RECURRENCEPRESETS_H
#define RECURRENCEPRESETS_H

#include <QtCore/QBitArray>

class QString;
class QStringList;

namespace KCal {
class Recurrence;
}

class KDateTime;

namespace IncidenceEditorsNG {

namespace RecurrencePresets {

  /**
   * Returns the availble presets.
   */
  QStringList availablePresets();

  /**
   * Returns a recurrence preset for given name. The name <em>must</em> be one
   * of availablePresets().
   *
   * Note: The caller takes ownership over the pointer.
   */
  KCal::Recurrence* preset( const QString &name, const KDateTime &start);

  /**
   * Returns the index of the preset in availablePresets for the given recurrence,
   * or -1 if no preset is equal to the given recurrence.
   */
  int presetIndex( const KCal::Recurrence &recurrence, const KDateTime &start );

} // RecurrencePresets

} // IncidenceEditorsNG

#endif // RECURRENCEPRESETS_H
