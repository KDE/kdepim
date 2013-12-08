/*
  This file is part of the PimPrint library.

  Copyright (C) 2012-2013  Allen Winter <winter@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#ifndef PIMPRINT_CALPRINTDAY_H
#define PIMPRINT_CALPRINTDAY_H

#include "calprintbase.h"

#include <KLocale>

#include <QDate>
#include <QTime>

class KCalendarSystem;

namespace PimPrint
{

namespace Calendar
{

class PrintCellItem;

class PIMPRINT_CALENDAR_EXPORT CalPrintDay : public CalPrintBase
{
    Q_PROPERTY(QDate startDate
               READ startDate WRITE setStartDate)

    Q_PROPERTY(QDate endDate
               READ endDate WRITE setEndDate)

    Q_PROPERTY(QTime startTime
               READ startTime WRITE setStartTime)

    Q_PROPERTY(QTime endTime
               READ endTime WRITE setEndTime)

public:
    explicit CalPrintDay(QPrinter *printer);

    virtual ~CalPrintDay();

    void print(QPainter &p);

    /**
     * Sets the printout starting date.
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
     * Sets the printout ending date.
     * @param dt is the ending date to print.
     * @see endDate(), setStartDate()
     */
    void setEndDate(const QDate &date);

    /**
     * Returns the current print ending date.
     * @see setEndDate(), startDate()
     */
    QDate endDate() const;

    /**
     * Sets the printout starting time.
     * Each day printed will start at this time.
     *
     * @param dt is the starting time to print.
     * @see startTime(), setEndTime()
     */
    void setStartTime(const QTime &time);

    /**
     * Returns the current print starting time.
     * @see setStartTime(), endTime()
     */
    QTime startTime() const;

    /**
     * Sets the printout ending time.
     * Each day printed will end at this time.
     *
     * @param dt is the ending time to print.
     * @see endTime(), setStartTime()
     */
    void setEndTime(const QTime &time);

    /**
     * Returns the current print ending time.
     * @see setEndTime(), startTime()
     */
    QTime endTime() const;

    void drawDayFiloFax(QPainter &p) const;
    void drawDaySingleTimeTable(QPainter &p) const;
    void drawDayTimeTable(QPainter &p) const;

private:
    //TODO: move to dpointer
    QRect drawHeader(QPainter &p,
                     const QDate &startDate, const QDate &endDate) const;

    QRect drawFooter(QPainter &p) const;

    /**
     * Draws the (filofax) table for a bunch of days, using drawDayBox.
     *
     * @param p QPainter of the printout.
     * @param start Start date.
     * @param end End date.
     * @param startTime Start time of the displayed time range.
     * @param endTime End time of the displayed time range.
     * @param box coordinates of the week box.
    */
    void drawDays(QPainter &p, const QDate &start, const QDate &end,
                  const QTime &startTime, const QTime &endTime,
                  const QRect &box) const;

    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond

};

}

}

#endif
