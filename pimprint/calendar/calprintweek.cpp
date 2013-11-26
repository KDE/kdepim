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

#include "calprintweek.h"

#include <KLocale>
#include <KGlobal>

using namespace PimPrint::Calendar;

//@cond PRIVATE
class PimPrint::Calendar::CalPrintWeek::Private
{
public:
    Private()
    {
    }

    QDate mStartDate;  // starting date of print
    QDate mEndDate;    // ending date of print
    QTime mStartTime;  // starting time of each day's print
    QTime mEndTime;    // ending time of each day's print
};
//@endcond

CalPrintWeek::CalPrintWeek(QPrinter *printer)
    : CalPrintBase(printer),
      d(new PimPrint::Calendar::CalPrintWeek::Private)
{
    //TODO:
    // set the calendar and calendar system

    // Set default print style
    setPrintStyle(CalPrintBase::WeekTimeTable);

    // TODO: Set default Info options
    //setInfoOptions(CalPrintBase::InfoTimeRange);

    // TODO: Set default Type options
    //setTypeOptions(0);

    // TODO: Set default Range options
    //setRangeOptions(0);

    // TODO:  Set default Extra options
    //setExtraOptions(0);
}

CalPrintWeek::~CalPrintWeek()
{
    delete d;
}

void CalPrintWeek::print(QPainter &p)
{
    switch (printStyle()) {
    case CalPrintBase::WeekFiloFax:
        drawFiloFaxWeek(p);
        break;
    case CalPrintBase::WeekTimeTable:
        drawTimeTableWeek(p);
        break;
    case CalPrintBase::WeekSplitWeek:
    default:
        drawSplitWeek(p);
        break;
    }
}

void CalPrintWeek::setStartDate(const QDate &date)
{
    d->mStartDate = date;
}

QDate CalPrintWeek::startDate() const
{
    return d->mStartDate;
}

void CalPrintWeek::setEndDate(const QDate &date)
{
    d->mEndDate = date;
}

QDate CalPrintWeek::endDate() const
{
    return d->mEndDate;
}

void CalPrintWeek::setStartTime(const QTime &time)
{
    d->mStartTime = time;
}

QTime CalPrintWeek::startTime() const
{
    return d->mStartTime;
}

void CalPrintWeek::setEndTime(const QTime &time)
{
    d->mEndTime = time;
}

QTime CalPrintWeek::endTime() const
{
    return d->mEndTime;
}

QRect CalPrintWeek::drawHeader(QPainter &p, const QDate &date, bool printWeekNumber) const
{
    if (!date.isValid()) {
        return QRect();
    }

    KLocale *local = KGlobal::locale();

    const QDate startDate =  date.addDays(-6);
    const QString line1 = local->formatDate(startDate);
    const QString line2 = local->formatDate(date);

    QString title;
    if (useLandscape()) {
        if (printWeekNumber) {
            title = i18nc("date from - to (week number)", "%1 - %2 (Week %3)",
                          line1, line2, date.weekNumber());
        } else {
            title = i18nc("date from - to", "%1 - %2", line1, line2);
        }
    } else {
        if (printWeekNumber) {
            title = i18nc("date from -\nto (week number)", "%1 -\n%2 (Week %3)",
                          line1, line2, date.weekNumber());
        } else {
            title = i18nc("date from -\nto", "%1 -\n%2", line1, line2);
        }
    }

    const QRect headerBox(0, 0, pageWidth(), headerHeight());
    CalPrintBase::drawHeader(p, headerBox, title, startDate);
    return headerBox;
}

QRect CalPrintWeek::drawFooter(QPainter &p) const
{
    QRect footerBox(0, pageHeight() - footerHeight(), pageWidth(), footerHeight());

    CalPrintBase::drawFooter(p, footerBox);

    return footerBox;
}

void CalPrintWeek::drawWeek(QPainter &p, const QDate &qd,
                            const QTime &fromTime, const QTime &toTime,
                            const QRect &box) const
{
    QDate weekDate = qd;
    const bool portrait = (box.height() > box.width());
    int cellWidth;
    int vcells;
    if (portrait) {
        cellWidth = box.width() / 2;
        vcells = 3;
    } else {
        cellWidth = box.width() / 6;
        vcells = 1;
    }
    const int cellHeight = box.height() / vcells;

    // correct begin of week
    int weekdayCol = weekdayColumn(qd.dayOfWeek());
    weekDate = qd.addDays(-weekdayCol);

    for (int i = 0; i < 7; i++, weekDate = weekDate.addDays(1)) {
        // Saturday and sunday share a cell, so we have to special-case sunday
        int hpos = ((i < 6) ? i : (i - 1)) / vcells;
        int vpos = ((i < 6) ? i : (i - 1)) % vcells;
        QRect dayBox(
            box.left() + cellWidth * hpos,
            box.top() + cellHeight * vpos + ((i == 6) ? (cellHeight / 2) : 0),
            cellWidth, (i < 5) ? (cellHeight) : (cellHeight / 2));
        drawDayBox(p, weekDate,
                   fromTime, toTime,
                   dayBox,
                   true,/*fulldate*/
                   true,/*printRecurDaily*/
                   true /*printRecurWeekly*/);
    } // for i through all weekdays
}

void CalPrintWeek::drawFiloFaxWeek(QPainter &p) const
{
    QDate dateCurWeek, dateStartWeek, dateEndWeek;

    // correct begin and end to first and last day of week
    int weekdayCol = weekdayColumn(d->mStartDate.dayOfWeek());
    dateStartWeek = d->mStartDate.addDays(-weekdayCol);
    weekdayCol = weekdayColumn(d->mEndDate.dayOfWeek());
    dateEndWeek = d->mEndDate.addDays(6 - weekdayCol);

    dateCurWeek = dateStartWeek.addDays(6);

    bool firstTime = true;
    QRect weekBox;

    do {
        const QRect headerBox = drawHeader(p, dateCurWeek, false);

        if (firstTime) {
            weekBox = headerBox;
            weekBox.setTop(headerBox.bottom() + padding());
            weekBox.setBottom(pageHeight() - footerHeight());
            firstTime = false;
        }

        drawWeek(p, dateCurWeek, d->mStartTime, d->mEndTime, weekBox);

        if (extraOptions().testFlag(CalPrintBase::ExtraFooter)) {
            drawFooter(p);
        }

        dateCurWeek = dateCurWeek.addDays(7);
        if (dateCurWeek <= dateEndWeek) {
            thePrinter()->newPage();
        }
    } while (dateCurWeek <= dateEndWeek);
}

void CalPrintWeek::drawTimeTableWeek(QPainter &p) const
{
    QDate dateCurWeek, dateStartWeek, dateEndWeek;

    // correct begin and end to first and last day of week
    int weekdayCol = weekdayColumn(d->mStartDate.dayOfWeek());
    dateStartWeek = d->mStartDate.addDays(-weekdayCol);
    weekdayCol = weekdayColumn(d->mEndDate.dayOfWeek());
    dateEndWeek = d->mEndDate.addDays(6 - weekdayCol);

    dateCurWeek = dateStartWeek.addDays(6);

    bool firstTime = true;
    QRect weekBox;

    do {
        const QRect headerBox = drawHeader(p, dateCurWeek, true);

        if (firstTime) {
            weekBox = headerBox;
            weekBox.setTop(headerBox.bottom() + padding());
            weekBox.setBottom(pageHeight() - footerHeight());
            firstTime = false;
        }

        drawTimeTable(p, weekBox,
                      dateStartWeek, dateCurWeek,
                      d->mStartTime, d->mEndTime);

        if (extraOptions().testFlag(CalPrintBase::ExtraFooter)) {
            drawFooter(p);
        }

        dateStartWeek = dateStartWeek.addDays(7);
        dateCurWeek = dateStartWeek.addDays(6);
        if (dateCurWeek <= dateEndWeek) {
            thePrinter()->newPage();
        }
    } while (dateCurWeek <= dateEndWeek);
}

void CalPrintWeek::drawSplitWeek(QPainter &p) const
{
    // TODO:
}
