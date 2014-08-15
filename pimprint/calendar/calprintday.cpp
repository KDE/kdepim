/*
  This file is part of the PimPrint library.

  Copyright (C) 2012-2013 Allen Winter <winter@kde.org>

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

#include "calprintday.h"

#include <calendarsupport/utils.h>

#include <KCalCore/Event>
#include <KCalCore/Todo>

#include <KGlobal>
#include <KSystemTimeZones>

#include <qmath.h>
#include <KLocalizedString>

using namespace PimPrint::Calendar;

//@cond PRIVATE
class PimPrint::Calendar::CalPrintDay::Private
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

CalPrintDay::CalPrintDay(QPrinter *printer)
    : CalPrintBase(printer),
      d(new PimPrint::Calendar::CalPrintDay::Private)
{
    //TODO:
    // set the calendar and calendar system

    // Set default print style
    setPrintStyle(CalPrintBase::DayTimeTable);

    // Set default Info options
    setInfoOptions(CalPrintBase::InfoTimeRange);

    // Set default Type options
    setTypeOptions(0);

    // Set default Range options
    setRangeOptions(0);

    // Set default Extra options
    setExtraOptions(0);
}

CalPrintDay::~CalPrintDay()
{
    delete d;
}

void CalPrintDay::print(QPainter &p)
{
    switch (printStyle()) {
    case CalPrintBase::DayFiloFax:
        drawDayFiloFax(p);
        break;
    case CalPrintBase::DaySingleTimeTable:
        drawDaySingleTimeTable(p);
        break;
    case CalPrintBase::DayTimeTable:
    default:
        drawDayTimeTable(p);
        break;
    }
}

void CalPrintDay::setStartDate(const QDate &date)
{
    d->mStartDate = date;
}

QDate CalPrintDay::startDate() const
{
    return d->mStartDate;
}

void CalPrintDay::setEndDate(const QDate &date)
{
    d->mEndDate = date;
}

QDate CalPrintDay::endDate() const
{
    return d->mEndDate;
}

void CalPrintDay::setStartTime(const QTime &time)
{
    d->mStartTime = time;
}

QTime CalPrintDay::startTime() const
{
    return d->mStartTime;
}

void CalPrintDay::setEndTime(const QTime &time)
{
    d->mEndTime = time;
}

QTime CalPrintDay::endTime() const
{
    return d->mEndTime;
}

QRect CalPrintDay::drawHeader(QPainter &p,
                              const QDate &startDate, const QDate &endDate) const
{
    if (!startDate.isValid()) {
        return QRect();
    }

    KLocale *local = KLocale::global();

    QString title;
    if (endDate.isValid()) {
        const QString line1 = local->formatDate(startDate);
        const QString line2 = local->formatDate(endDate);
        if (useLandscape()) {
            title = i18nc("date from-to", "%1 - %2", line1, line2);
        } else {
            title = i18nc("date from-\nto", "%1 -\n%2", line1, line2);
        }
    } else {
        title = local->formatDate(startDate);
    }

    const QRect headerBox(0, 0, pageWidth(), headerHeight());
    CalPrintBase::drawHeader(p, headerBox, title, startDate);
    return headerBox;
}

QRect CalPrintDay::drawFooter(QPainter &p) const
{
    QRect footerBox(0, pageHeight() - footerHeight(), pageWidth(), footerHeight());

    CalPrintBase::drawFooter(p, footerBox);

    return footerBox;
}

void CalPrintDay::drawDays(QPainter &p,
                           const QDate &start, const QDate &end,
                           const QTime &startTime, const QTime &endTime,
                           const QRect &box) const
{
    const int numberOfDays = start.daysTo(end) + 1;

    int cellWidth, vcells;
    if (!useLandscape()) {
        // 2 columns
        vcells = qCeil(static_cast<double>(numberOfDays) / 2.0);
        if (numberOfDays > 1) {
            cellWidth = box.width() / 2;
        } else {
            cellWidth = box.width();
        }
    } else {
        // landscape: N columns
        vcells = 1;
        cellWidth = box.width() / numberOfDays;
    }

    const int cellHeight = box.height() / vcells;
    QDate weekDate = start;
    for (int i = 0; i < numberOfDays; ++i, weekDate = weekDate.addDays(1)) {
        const int hpos = i / vcells;
        const int vpos = i % vcells;
        const QRect dayBox(box.left() + cellWidth * hpos,
                           box.top() + cellHeight * vpos,
                           cellWidth,
                           cellHeight);

        drawDayBox(p, dayBox, weekDate, startTime, endTime, true);

    } // for i through all selected days
}

void CalPrintDay::drawDayFiloFax(QPainter &p) const
{
    QDate curDay(d->mStartDate);

    QRect headerBox = drawHeader(p, d->mStartDate, d->mEndDate);

    QRect daysBox(headerBox);
    daysBox.setTop(headerBox.bottom() + padding());
    daysBox.setBottom(pageHeight() - footerHeight());

    drawDays(p, d->mStartDate, d->mEndDate, d->mStartTime, d->mEndTime, daysBox);

    if (extraOptions().testFlag(CalPrintBase::ExtraFooter)) {
        drawFooter(p);
    }
}

void CalPrintDay::drawDaySingleTimeTable(QPainter &p) const
{
    QDate curDay(d->mStartDate);

    QRect headerBox = drawHeader(p, d->mStartDate, d->mEndDate);

    QRect daysBox(headerBox);
    daysBox.setTop(headerBox.bottom() + padding());
    daysBox.setBottom(pageHeight() - footerHeight());

    drawTimeTable(p, daysBox,
                  d->mStartDate, d->mEndDate,
                  d->mStartTime, d->mEndTime,
                  rangeOptions().testFlag(CalPrintBase::RangeTimeExpand));

    if (extraOptions().testFlag(CalPrintBase::ExtraFooter)) {
        drawFooter(p);
    }
}

void CalPrintDay::drawDayTimeTable(QPainter &p) const
{
    QDate curDay(d->mStartDate);

    KDateTime::Spec timeSpec = KSystemTimeZones::local();

    QTime curStartTime(d->mStartTime);
    QTime curEndTime(d->mEndTime);

    // For an invalid time range, simply show one hour, starting at the hour
    // before the given start time
    if (curEndTime <= curStartTime) {
        curStartTime = QTime(curStartTime.hour(), 0, 0);
        curEndTime = curStartTime.addSecs(3600);
    }

    do {
        QRect headerBox = drawHeader(p, curDay, QDate());

        KCalCore::Event::List eventList =
            printCalendar()->events(curDay, timeSpec,
                                    KCalCore::EventSortStartDate,
                                    KCalCore::SortDirectionAscending);

        // split out the all day events as they will be printed in a separate box
        KCalCore::Event::List alldayEvents, timedEvents;
        Q_FOREACH (const KCalCore::Event::Ptr &event, eventList) {
            if (event->allDay()) {
                alldayEvents.append(event);
            } else {
                timedEvents.append(event);
            }
        }

        const int fontSize = 11;
        QFont textFont("sans-serif", fontSize, QFont::Normal);
        p.setFont(textFont);
        const int lineSpacing = p.fontMetrics().lineSpacing();

        const int tlWidth = timeLineWidth();
        const int padMargin = padding();
        const int maxAllDayEvents = 8; // the max we allow to be printed, sorry.
        int allDayHeight = qMin(alldayEvents.count(), maxAllDayEvents) * lineSpacing;
        allDayHeight = qMax(allDayHeight, (5 * lineSpacing)) + (2 * padMargin);
        QRect allDayBox(tlWidth + padMargin, headerBox.bottom() + padMargin,
                        pageWidth() - tlWidth - padMargin, allDayHeight);
        if (alldayEvents.count() > 0) {
            // draw the side bar for all-day events
            QFont oldFont(p.font());
            p.setFont(QFont("sans-serif", 9, QFont::Normal));
            drawVerticalBox(p,
                            boxBorderWidth(),
                            QRect(0, headerBox.bottom() + padMargin,
                                  tlWidth, allDayHeight),
                            i18n("Today's Events"),
                            Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextWordWrap);
            p.setFont(oldFont);

            // now draw at most maxAllDayEvents in the all-day box
            drawBox(p, boxBorderWidth(), allDayBox);

            QRect itemBox(allDayBox);
            itemBox.setLeft(tlWidth + (2 * padMargin));
            itemBox.setTop(itemBox.top() + padMargin);
            itemBox.setBottom(itemBox.top() + lineSpacing);
            int count = 0;
            Q_FOREACH (const KCalCore::Event::Ptr &event, alldayEvents) {
                if (count == maxAllDayEvents) {
                    break;
                }
                count++;
                QString str;
                if (event->location().isEmpty()) {
                    str = cleanString(event->summary());
                } else {
                    str = i18nc("summary, location", "%1, %2",
                                cleanString(event->summary()),
                                cleanString(event->location()));
                }
                drawItemString(p, itemBox, str);
                itemBox.setTop(itemBox.bottom());
                itemBox.setBottom(itemBox.top() + lineSpacing);
            }
        } else {
            allDayBox.setBottom(headerBox.bottom());
        }

        QRect dayBox(allDayBox);
        dayBox.setTop(allDayBox.bottom() + padMargin);
        dayBox.setBottom(pageHeight() - footerHeight());
        QList<QDate> workDays = CalendarSupport::workDays(curDay, curDay);
        drawAgendaDayBox(p, dayBox, curDay, timedEvents,
                         curStartTime, curEndTime,
                         workDays);

        QRect tlBox(dayBox);
        tlBox.setLeft(0);
        tlBox.setWidth(tlWidth);
        drawTimeLine(p, curStartTime, curEndTime, tlBox);

        if (extraOptions().testFlag(CalPrintBase::ExtraFooter)) {
            drawFooter(p);
        }

        curDay = curDay.addDays(1);
        if (curDay <= d->mEndDate) {
            thePrinter()->newPage();
        }
    } while (curDay <= d->mEndDate);
}
