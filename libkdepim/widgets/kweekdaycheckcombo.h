/*
    Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
    Copyright (C) 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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

#ifndef KWEEKDAYCHECKCOMBO_H
#define KWEEKDAYCHECKCOMBO_H

#include "widgets/kcheckcombobox.h"

#include "kdepim_export.h"

#include <QBitArray>
#include <QDate>

namespace KPIM
{

//FIXME: This class assumes all weeks have 7 days. We should use KCalenderSystem instead.
/**
 * A combobox that is populated with the days of the week from the current
 * KCalenderSystem. The days are checkable.
 * @note: KCalenderSystem numbers weekdays starting with 1, however this widget is 0 indexed and handles the conversion to the 1 based system internally. Use this widget as a normal 0 indexed container.
 * @see KCalenderSystem
 */
class KDEPIM_EXPORT KWeekdayCheckCombo : public KCheckComboBox
{

    Q_OBJECT
public:
    /**
    * @param first5Checked if true the first 5 weekdays will be checked by default
    */
    explicit KWeekdayCheckCombo(QWidget *parent = 0, bool first5Checked = false);
    virtual ~KWeekdayCheckCombo();

    /**
     * Retrieve the checked days
     * @param days a 7 bit array indicating the checked days (bit 0 = Monday, value 1 = checked).
     */
    QBitArray days() const;

    /**
     * Set the checked days on this combobox
     * @param days a 7 bit array indicating the days to check/uncheck (bit 0 = Monday, value 1 = check).
     * @param disableDays if not empty, the corresponding days will be disabled, all others enabled (bit 0 = Monday, value 1 = disable).
     * @see days()
     */
    void setDays(const QBitArray &days, const QBitArray &disableDays = QBitArray());

    /**
     * Returns the index of the weekday represented by the
     * QDate object.
     */
    int weekdayIndex(const QDate &date) const;
};

}
#endif // KWEEKDAYCHECKCOMBO_H
