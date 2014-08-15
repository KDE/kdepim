/*
 T his file is part of *the PimPrint library.

 Copyright (C) 2013 Allen Winter <winter@kde.org>

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

#include "calprintmonth.h"

#include <KCalendarSystem>
#include <KLocalizedString>

using namespace PimPrint::Calendar;

//@cond PRIVATE
class PimPrint::Calendar::CalPrintMonth::Private
{
public:
    Private()
    {
    }

    QDate mStartDate;  // starting date of print (for month and year)
    QDate mEndDate;    // ending date of print (for month and year)
};
//@endcond

CalPrintMonth::CalPrintMonth(QPrinter *printer)
    : CalPrintBase(printer),
      d(new PimPrint::Calendar::CalPrintMonth::Private)
{
    //TODO:
    // set the calendar and calendar system

    // Set default print style
    setPrintStyle(CalPrintBase::MonthClassic);

    // Set default Info options
    setInfoOptions(0);

    // Set default Type options
    setTypeOptions(0);

    // Set default Range options
    setRangeOptions(0);

    // Set default Extra options
    setExtraOptions(0);
}

CalPrintMonth::~CalPrintMonth()
{
    delete d;
}

void CalPrintMonth::print(QPainter &p)
{
    QDate fromMonth = d->mStartDate.addDays(-(d->mStartDate.day() - 1));
    QDate toMonth = d->mEndDate.addDays(d->mEndDate.daysInMonth() - d->mEndDate.day());

    QRect headerBox(0, 0, pageWidth(), headerHeight());
    QRect footerBox(0, pageHeight() - footerHeight(), pageWidth(), footerHeight());

    QRect monthBox(0, 0, pageWidth(), pageHeight() - footerHeight());
    monthBox.setTop(headerBox.bottom() + padding());

    QDate curMonth = fromMonth;
    do {
        QString title(i18nc("monthname year", "%1 %2",
                            calendarSystem()->monthName(curMonth),
                            QString::number(curMonth.year())));
        QDate tmp(fromMonth);
        int weekdayCol = weekdayColumn(tmp.dayOfWeek());
        tmp = tmp.addDays(-weekdayCol);

        drawHeader(p, headerBox, title, curMonth.addMonths(-1), curMonth.addMonths(1));

        drawMonthTable(p, monthBox, curMonth, QTime(), QTime());

        if (extraOptions().testFlag(CalPrintBase::ExtraFooter)) {
            drawFooter(p, footerBox);
        }

        curMonth = curMonth.addDays(curMonth.daysInMonth());
        if (curMonth <= toMonth) {
            thePrinter()->newPage();
        }
    } while (curMonth <= toMonth);

}

void CalPrintMonth::setStartDate(const QDate &date)
{
    d->mStartDate = date;
}

QDate CalPrintMonth::startDate() const
{
    return d->mStartDate;
}

void CalPrintMonth::setEndDate(const QDate &date)
{
    d->mEndDate = date;
}

QDate CalPrintMonth::endDate() const
{
    return d->mEndDate;
}

void CalPrintMonth::drawMonthTable(QPainter &p, const QRect &box,
                                   const QDate &date,
                                   const QTime &fromTime, const QTime &toTime) const
{
    int yoffset = subHeaderHeight();
    int xoffset = 0;
    QDate monthDate(QDate(date.year(), date.month(), 1));
    QDate monthFirst(monthDate);
    QDate monthLast(monthDate.addMonths(1).addDays(-1));

    int weekdayCol = weekdayColumn(monthDate.dayOfWeek());
    monthDate = monthDate.addDays(-weekdayCol);

    if (extraOptions().testFlag(CalPrintBase::ExtraWeekNumbers)) {
        xoffset += 14;
    }

    int rows = (weekdayCol + date.daysInMonth() - 1) / 7 + 1;
    double cellHeight = (box.height() - yoffset) / (1. * rows);
    double cellWidth = (box.width() - xoffset) / 7.;

    // Precalculate the grid...
    // rows is at most 6, so using 8 entries in the array is fine, too!
    int coledges[8], rowedges[8];
    for (int i = 0; i <= 7; ++i) {
        rowedges[i] = int(box.top() + yoffset + i * cellHeight);
        coledges[i] = int(box.left() + xoffset + i * cellWidth);
    }

    if (extraOptions().testFlag(CalPrintBase::ExtraWeekNumbers)) {
        //TODO: make this into a protected method for CalPrintBase
        QFont oldFont(p.font());
        QFont newFont(p.font());
        newFont.setPointSize(6);
        p.setFont(newFont);
        QDate weekDate(monthDate);
        for (int row = 0; row < rows; ++row) {
            int calWeek = weekDate.weekNumber();
            QRect rc(box.left(), rowedges[row],
                     coledges[0] - 3 - box.left(), rowedges[row + 1] - rowedges[row]);
            p.drawText(rc, Qt::AlignRight | Qt::AlignVCenter, QString::number(calWeek));
            weekDate = weekDate.addDays(7);
        }
        p.setFont(oldFont);
    }

    QRect daysOfWeekBox(box);
    daysOfWeekBox.setHeight(subHeaderHeight());
    daysOfWeekBox.setLeft(box.left() + xoffset);
    drawDaysOfWeek(p, daysOfWeekBox, monthDate, monthDate.addDays(6));

    QColor back = p.background().color();
    bool darkbg = false;
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < 7; ++col) {
            // show days from previous/next month with a grayed background
            if ((monthDate < monthFirst) || (monthDate > monthLast)) {
                p.setBackground(back.darker(120));
                darkbg = true;
            }
            QRect dayBox(coledges[col], rowedges[row],
                         coledges[col + 1] - coledges[col], rowedges[row + 1] - rowedges[row]);
            drawDayBox(p, dayBox, monthDate, fromTime, toTime);
            if (darkbg) {
                p.setBackground(back);
                darkbg = false;
            }
            monthDate = monthDate.addDays(1);
        }
    }
}
