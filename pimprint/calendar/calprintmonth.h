/*
 *  This file is part of the PimPrint library.
 *
 *  Copyright (C) 2013 Allen Winter <winter@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to the
 *  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef PIMPRINT_CALPRINTMONTH_H
#define PIMPRINT_CALPRINTMONTH_H

#include "calprintbase.h"

namespace PimPrint
{

namespace Calendar
{

class PIMPRINT_CALENDAR_EXPORT CalPrintMonth : public CalPrintBase
{
    Q_PROPERTY(QDate startDate
               READ startDate WRITE setStartDate)

    Q_PROPERTY(QDate endDate
               READ endDate WRITE setEndDate)

public:
    explicit CalPrintMonth(QPrinter *printer);

    virtual ~CalPrintMonth();

    void print(QPainter &p);

    /**
     * Sets the printout starting date. Only the month and year are considered.
     * @param dt is the starting date to print.
     * @see startDate(), setEndDate()
     */
    void setStartDate(const QDate &date);

    /**
     * Returns the current print starting date.
     * @see setStartDate(), endDate()
     */
    QDate startDate() const;

    /**
     * Sets the printout ending date. Only the month and year are considered.
     * @param dt is the ending date to print.
     * @see endDate(), setStartDate()
     */
    void setEndDate(const QDate &date);

    /**
     * Returns the current print ending date.
     * @see setEndDate(), startDate()
     */
    QDate endDate() const;

private:
    //TODO: move to dpointer

    /**
     * Draw the month table of the month containing the date @p date.
     * Each day gets one box (using drawDayBox) that contains a list of incidences
     * on that day. The incidences are arranged in a matrix, with the first column
     * being the first day of the week (so it might display some days of the
     * previous and the next month).  Above the matrix is a bar showing the
     * weekdays (drawn using drawDaysOfWeek).
     *
     * @param p QPainter of the printout
     * @param box coordinates of the month box.
     * @param date Arbitrary date within the month to be printed.
     * @param fromTime Start time of the displayed time range
     * @param toTime End time of the displayed time range
     */
    void drawMonthTable(QPainter &p, const QRect &box,
                        const QDate &date,
                        const QTime &fromTime, const QTime &toTime) const;

    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond

};

}

}

#endif
