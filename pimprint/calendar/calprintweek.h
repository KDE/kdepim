/*
  This file is part of the PimPrint library.

  Copyright (C) 2012  Allen Winter <winter@kde.org>

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

#ifndef PIMPRINT_CALPRINTWEEK_H
#define PIMPRINT_CALPRINTWEEK_H

#include "calprintbase.h"

namespace PimPrint {

namespace Calendar {

class PIMPRINT_CALENDAR_EXPORT CalPrintWeek : public CalPrintBase
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
    explicit CalPrintWeek(QPrinter *printer);

    virtual ~CalPrintWeek();

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

    void drawFiloFaxWeek(QPainter &p) const;
    void drawTimeTableWeek(QPainter &p) const;
    void drawSplitWeek(QPainter &p) const;

private:
    //TODO: move to dpointer
    QRect drawHeader(QPainter &p, const QDate &date, bool printWeekNumber = false) const;

    QRect drawFooter(QPainter &p) const;

    void drawWeek(QPainter &p,
                  const QDate &qd,
                  const QTime &fromTime, const QTime &toTime,
                  const QRect &box) const;

    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond

};

}

}

#endif
