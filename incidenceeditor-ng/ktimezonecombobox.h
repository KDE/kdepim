/*
  Copyright (C) 2007 Bruno Virlet <bruno.virlet@gmail.com>
  Copyright 2008-2009 Allen Winter <winter@kde.org>

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

#ifndef INCIDENCEEDITOR_KTIMEZONECOMBOBOX_H
#define INCIDENCEEDITOR_KTIMEZONECOMBOBOX_H

#include "incidenceeditors_ng_export.h"

#include <KComboBox>
#include <KDateTime>

namespace KCalCore
{
class ICalTimeZones;
}

namespace IncidenceEditorNG
{

/**
 * A combobox that shows the system timezones available in KSystemTimeZones::zones()
 * and provides methods to easily select the item corresponding to a given
 * KDateTime::Spec or to retrieve the KDateTime::Spec associated with the
 * selected item.
 */
class INCIDENCEEDITORS_NG_EXPORT KTimeZoneComboBox : public KComboBox
{
    Q_OBJECT
public:
    /**
     * Creates a new time zone combobox.
     *
     * @param parent The parent widget.
     */
    explicit KTimeZoneComboBox(QWidget *parent = 0);

    /**
     * Creates a new time zone combobox.
     *
     * @param additionalZones Additional time zones that shall be included in the combobox.
     * @param parent The parent widget.
     */
    explicit KTimeZoneComboBox(const KCalCore::ICalTimeZones *additionalZones, QWidget *parent = 0);

    /**
     * Destroys the time zone combobox.
     */
    ~KTimeZoneComboBox();

    /**
      Sets additional time @p zones (usually from a calendar) which should be displayed
      additionally to the system time zones.
    */
    void setAdditionalTimeZones(const KCalCore::ICalTimeZones *zones);

    /**
     * Selects the item in the combobox corresponding to the given @p spec.
     */
    void selectTimeSpec(const KDateTime::Spec &spec);

    /**
     * Convenience version of selectTimeSpec(const KDateTime::Spec &).
     * Selects the local time zone specified in the user settings.
     */
    void selectLocalTimeSpec();

    /**
     * If @p floating is true, selects floating time zone, otherwise
     * if @spec is valid, selects @p spec time zone, if not selects
     * local time zone.
     */
    void setFloating(bool floating, const KDateTime::Spec &spec = KDateTime::Spec());

    /**
     * Return the timespec associated with the currently selected item.
     */
    KDateTime::Spec selectedTimeSpec() const;

private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
