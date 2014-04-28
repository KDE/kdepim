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

#include "calprintbase.h"

#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/utils.h>

#include <KCalendarSystem>
#include <KGlobal>
#include <KSystemTimeZones>

#include <QAbstractTextDocumentLayout>
#include <QPrinter>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <boost/concept_check.hpp>

using namespace PimPrint::Calendar;

//TODO: compute default page height and width based QPainter window
static int sPortraitPageHeight = 792; // page height, for portrait orientation
static int sLandscapePageHeight = 612; // page height, for landscape orientation

static int sPortraitHeaderHeight = 72; // header height, for portrait orientation
static int sLandscapeHeaderHeight = 54; // header height, for landscape orientation
static int sSubHeaderHeight = 20;     // subheader height, for all orientations

static int sPortraitFooterHeight = 16; // footer height, for portrait orientation
static int sLandscapeFooterHeight = 14; // footer height, for landscape orientation

static int sMarginSize = 36;          // margins, for all orientations (.5 inch)
static int sPaddingSize = 7;          // padding between the various top-level boxes

static int sBoxBorderWidth = 2;       // width of the border of all top-level boxes
static int sItemBoxBorderWidth = 0;   // width of the border of all item boxes
static int sTimeLineWidth = 50;       // width of timeline (eg. day and timetable)

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class PimPrint::Calendar::CalPrintBase::Private
{
public:
    Private()
        : mPrinter(0),
          mPageHeight(-1),
          mPageWidth(-1),
          mCalendar(0),
          mCalSystem(0),
          mPrintStyle(CalPrintBase::None),
          mHeaderHeight(-1),
          mSubHeaderHeight(sSubHeaderHeight),
          mFooterHeight(-1),
          mMargins(sMarginSize),
          mPadding(sPaddingSize),
          mBoxBorderWidth(sBoxBorderWidth),
          mItemBoxBorderWidth(sItemBoxBorderWidth),
          mTimeLineWidth(sTimeLineWidth),
          mInfoOptions(0),
          mTypeOptions(0),
          mRangeOptions(0),
          mExtraOptions(0),
          mUseColor(true),
          mUseLandscape(false),
          mPrintFooter(false)
    {
    }

    QPrinter *mPrinter;          // the QPrinter
    QPainter mPainter;           // the QPainter
    int mPageHeight;             // page height, depends on page orientation
    int mPageWidth;              // page height, depends on page orientation
    KCalCore::Calendar::Ptr mCalendar; // the Calendar
    KCalendarSystem *mCalSystem; // the Calendar System
    CalPrintBase::Style mPrintStyle; // the style to print
    int mHeaderHeight;           // height of header, depends on page orientation
    int mSubHeaderHeight;        // subheader height, same for all orientations
    int mFooterHeight;           // height of footer, depends on page orientation
    int mMargins;                // margins, for all orientations
    int mPadding;                // padding between the various top-level boxes
    int mBoxBorderWidth;         // width of the border of all top-level boxes
    int mItemBoxBorderWidth;     // width of the border of all item boxes
    int mTimeLineWidth;          // width of timelines

    InfoOptions mInfoOptions;    // flags for information to print.
    TypeOptions mTypeOptions;    // flags for types to print.
    RangeOptions mRangeOptions;  // flags for time ranges to print.
    ExtraOptions mExtraOptions;  // flags for extra stuff to print.
    bool mUseColor;              // flag, print in color
    bool mUseLandscape;          // flag, print in landscape orientation
    bool mPrintFooter;           // flag, print footer
};
//@endcond

CalPrintBase::CalPrintBase(QPrinter *printer)
    : d(new PimPrint::Calendar::CalPrintBase::Private)
{
    init(printer);
}

CalPrintBase::~CalPrintBase()
{
    finish();
    delete d;
}

void CalPrintBase::init(QPrinter *printer) const
{
    if (!printer) {
        return;
    }

    // Init printer
    d->mPrinter = printer;
    d->mPrinter->setColorMode(QPrinter::GrayScale);

    // Init painter
    d->mPainter.begin(d->mPrinter);

    // FIXME: allow each margin to be set individually.
    // the painter initially begins at 72 dpi per the Qt docs.
    // Currently hard-coding half-inch margins (36 dpi)
    const int margin = margins();
    d->mPainter.setViewport(margin, margin,
                            d->mPainter.viewport().width() - 2 * margin,
                            d->mPainter.viewport().height() - 2 * margin);

    // Init page
    setPageHeight(d->mPainter.window().height());
    setPageWidth(d->mPainter.window().width());
}

void CalPrintBase::finish() const
{
    d->mPainter.end();
    d->mPrinter = 0;
}

void CalPrintBase::setThePrinter(QPrinter *printer)
{
    d->mPrinter = printer;
}

QPrinter *CalPrintBase::thePrinter() const
{
    return d->mPrinter;
}

void CalPrintBase::setPrintCalendar(const KCalCore::Calendar::Ptr &calendar)
{
    d->mCalendar = calendar;
}

KCalCore::Calendar::Ptr CalPrintBase::printCalendar() const
{
    return d->mCalendar;
}

void CalPrintBase::setCalendarSystem(KCalendarSystem *calSystem)
{
    d->mCalSystem = calSystem;
}

KCalendarSystem *CalPrintBase::calendarSystem() const
{
    return d->mCalSystem;
}

void CalPrintBase::setPrintStyle(const Style style)
{
    d->mPrintStyle = style;
}

CalPrintBase::Style CalPrintBase::printStyle() const
{
    return d->mPrintStyle;
}

void CalPrintBase::setPageHeight(const int height) const
{
    d->mPageHeight = height;
}

int CalPrintBase::pageHeight() const
{
    if (d->mPageHeight >= 0) {
        return d->mPageHeight;
    } else if (!d->mUseLandscape) {
        return sPortraitPageHeight;  //TODO: replace with p.window().height()
    } else {
        return sLandscapePageHeight; //TODO: replace with p.window().width()
    }
}

void CalPrintBase::setPageWidth(const int width) const
{
    d->mPageWidth = width;
}

int CalPrintBase::pageWidth() const
{
    if (d->mPageWidth >= 0) {
        return d->mPageWidth;
    } else if (d->mUseLandscape) {
        return sPortraitPageHeight;  //TODO: replace with p.window().height()
    } else {
        return sLandscapePageHeight; //TODO: replace with p.windows().width()
    }
}

void CalPrintBase::setUseLandscape(const bool useLandscape)
{
    d->mUseLandscape = useLandscape;
}

bool CalPrintBase::useLandscape() const
{
    return d->mUseLandscape;
}

void CalPrintBase::setUseColor(const bool useColor)
{
    d->mUseColor = useColor;
}

bool CalPrintBase::useColor() const
{
    return d->mUseColor;
}

void CalPrintBase::setHeaderHeight(const int height)
{
    d->mHeaderHeight = height;
}

int CalPrintBase::headerHeight() const
{
    if (d->mHeaderHeight >= 0) {
        return d->mHeaderHeight;
    } else if (!d->mUseLandscape) {
        return sPortraitHeaderHeight;
    } else {
        return sLandscapeHeaderHeight;
    }
}

void CalPrintBase::setSubHeaderHeight(const int height)
{
    d->mSubHeaderHeight = height;
}

int CalPrintBase::subHeaderHeight() const
{
    return d->mSubHeaderHeight;
}

void CalPrintBase::setFooterHeight(const int height)
{
    d->mFooterHeight = height;
}

int CalPrintBase::footerHeight() const
{
    if (!d->mPrintFooter) {
        return 0;
    }

    if (d->mFooterHeight >= 0) {
        return d->mFooterHeight;
    } else if (!d->mUseLandscape) {
        return sPortraitFooterHeight;
    } else {
        return sLandscapeFooterHeight;
    }
}

void CalPrintBase::setPadding(const int padding)
{
    d->mPadding = padding;
}

int CalPrintBase::padding() const
{
    return d->mPadding;
}

int CalPrintBase::margins() const
{
    return d->mMargins;
}

int CalPrintBase::boxBorderWidth() const
{
    return d->mBoxBorderWidth;
}

int CalPrintBase::itemBoxBorderWidth() const
{
    return d->mItemBoxBorderWidth;
}

int CalPrintBase::timeLineWidth() const
{
    return d->mTimeLineWidth;
}

void CalPrintBase::setInfoOptions(CalPrintBase::InfoOptions flags)
{
    d->mInfoOptions = flags;
}

CalPrintBase::InfoOptions CalPrintBase::infoOptions() const
{
    return d->mInfoOptions;
}

void CalPrintBase::setTypeOptions(CalPrintBase::TypeOptions flags)
{
    d->mTypeOptions = flags;
}

CalPrintBase::TypeOptions CalPrintBase::typeOptions() const
{
    return d->mTypeOptions;
}

void CalPrintBase::setRangeOptions(CalPrintBase::RangeOptions flags)
{
    d->mRangeOptions = flags;
}

CalPrintBase::RangeOptions CalPrintBase::rangeOptions() const
{
    return d->mRangeOptions;
}

void CalPrintBase::setExtraOptions(CalPrintBase::ExtraOptions flags)
{
    d->mExtraOptions = flags;
}

CalPrintBase::ExtraOptions CalPrintBase::extraOptions() const
{
    return d->mExtraOptions;
}

void CalPrintBase::drawBox(QPainter &p, const int linewidth, const QRect &rect) const
{
    QPen pen(p.pen());
    QPen oldpen(pen);
    // no border
    if (linewidth >= 0) {
        pen.setWidth(linewidth);
        p.setPen(pen);
    } else {
        p.setPen(Qt::NoPen);
    }
    p.drawRect(rect);
    p.setPen(oldpen);
}

void CalPrintBase::drawShadedBox(QPainter &p, const int linewidth,
                                 const QBrush &brush, const QRect &rect) const
{
    QBrush oldbrush(p.brush());
    p.setBrush(brush);
    drawBox(p, linewidth, rect);
    p.setBrush(oldbrush);
}

void CalPrintBase::drawItemString(QPainter &p, const QRect &box,
                                  const QString &str, int flags) const
{
    QRect newbox(box);
    newbox.adjust(3, 1, -1, -1);
    p.drawText(newbox, (flags == -1) ?
               (Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap) : flags, str);
}

void CalPrintBase::drawItemBox(QPainter &p, const int linewidth, const QRect &box,
                               const KCalCore::Incidence::Ptr &incidence,
                               const QString &str, int flags) const
{
    QPen oldpen(p.pen());
    QBrush oldbrush(p.brush());
    QColor bgColor(categoryBgColor(incidence));
    if (d->mUseColor & bgColor.isValid()) {
        p.setBrush(bgColor);
    } else {
        p.setBrush(QColor(232, 232, 232));
    }
    drawBox(p, (linewidth > 0) ? linewidth : itemBoxBorderWidth(), box);
    if (d->mUseColor && bgColor.isValid()) {
        p.setPen(getTextColor(bgColor));
    }
    drawItemString(p, box, str, flags);
    p.setPen(oldpen);
    p.setBrush(oldbrush);
}

void CalPrintBase::drawVerticalBox(QPainter &p, const int linewidth,
                                   const QRect &box,
                                   const QString &str, int flags) const
{
    p.save();
    p.rotate(-90);
    QRect rotatedBox(-box.top() - box.height(), box.left(), box.height(), box.width());
    drawItemBox(p, linewidth, rotatedBox, KCalCore::Incidence::Ptr(), str,
                (flags == -1) ? Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine : flags);

    p.restore();
}

void CalPrintBase::drawSmallMonth(QPainter &p, const QDate &date, const QRect &box) const
{
    int weekdayCol = weekdayColumn(date.dayOfWeek());
    int month = date.month();
    QDate monthDate(QDate(date.year(), date.month(), 1));
    // correct begin of week
    QDate monthDate2(monthDate.addDays(-weekdayCol));

    double cellWidth = double(box.width()) / double(7);
    int rownr = 3 + (date.daysInMonth() + weekdayCol - 1) / 7;
    // 3 Pixel after month name, 2 after day names, 1 after the calendar
    double cellHeight = (box.height() - 5) / rownr;
    QFont oldFont(p.font());
    //TODO: option for the small month font?
    p.setFont(QFont("sans-serif", int(cellHeight - 2), QFont::Normal));

    // draw the title
    KCalendarSystem *calSys = calendarSystem();
    if (calSys) {
        QRect titleBox(box);
        titleBox.setHeight(int(cellHeight + 1));
        p.drawText(titleBox, Qt::AlignTop | Qt::AlignHCenter, calSys->monthName(date));
    }

    // draw days of week
    QRect wdayBox(box);
    wdayBox.setTop(int(box.top() + 3 + cellHeight));
    wdayBox.setHeight(int(2 * cellHeight) - int(cellHeight));

    if (calSys) {
        for (int col = 0; col < 7; ++col) {
            QString tmpStr = calSys->weekDayName(monthDate2)[0].toUpper();
            wdayBox.setLeft(int(box.left() + col * cellWidth));
            wdayBox.setRight(int(box.left() + (col + 1) * cellWidth));
            p.drawText(wdayBox, Qt::AlignCenter, tmpStr);
            monthDate2 = monthDate2.addDays(1);
        }
    }

    // draw separator line
    int calStartY = wdayBox.bottom() + 2;
    p.drawLine(box.left(), calStartY, box.right(), calStartY);
    monthDate = monthDate.addDays(-weekdayCol);

    for (int row = 0; row < (rownr - 2); row++) {
        for (int col = 0; col < 7; col++) {
            if (monthDate.month() == month) {
                QRect dayRect(int(box.left() + col * cellWidth),
                              int(calStartY + row * cellHeight), 0, 0);
                dayRect.setRight(int(box.left() + (col + 1) * cellWidth));
                dayRect.setBottom(int(calStartY + (row + 1) * cellHeight));
                p.drawText(dayRect, Qt::AlignCenter, QString::number(monthDate.day()));
            }
            monthDate = monthDate.addDays(1);
        }
    }
    p.setFont(oldFont);
}

int CalPrintBase::drawHeader(QPainter &p, const QRect &allbox,
                             const QString &title,
                             const QDate &leftMonth, const QDate &rightMonth,
                             const bool expand,
                             const QColor &backColor) const
{
    // print previous month for month view, print current for to-do, day and week
    int smallMonthWidth = (allbox.width() / 4) - 10;
    if (smallMonthWidth > 100) {
        smallMonthWidth = 100;
    }

    QRect box(allbox);
    QRect textRect(allbox);

    QFont oldFont(p.font());
    //TODO: option for the header font?
    QFont newFont("sans-serif", (textRect.height() < 60) ? 16 : 18, QFont::Bold);
    if (expand) {
        p.setFont(newFont);
        QRect boundingR =
            p.boundingRect(textRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap, title);
        p.setFont(oldFont);
        int h = boundingR.height();
        if (h > allbox.height()) {
            box.setHeight(h);
            textRect.setHeight(h);
        }
    }

    QBrush backBrush;
    if (!backColor.isValid()) {
        backBrush.setColor(QColor(232, 232, 232));
    } else {
        backBrush.setColor(backColor);
    }

    drawShadedBox(p, boxBorderWidth(), backBrush, box);

    // prev month left, current month centered, next month right
    if (rightMonth.isValid()) {
        const QRect rightMonthBox(box.right() - 10 - smallMonthWidth, box.top(),
                                  smallMonthWidth, box.height());
        drawSmallMonth(p, QDate(rightMonth.year(), rightMonth.month(), 1), rightMonthBox);
        textRect.setRight(rightMonthBox.left());
    }
    if (leftMonth.isValid()) {
        const QRect leftMonthBox(box.left() + 10, box.top(),
                                 smallMonthWidth, box.height());
        drawSmallMonth(p, QDate(leftMonth.year(), leftMonth.month(), 1), leftMonthBox);
        textRect.setLeft(leftMonthBox.right());
    }

    // Set the margins
    p.setFont(newFont);
    p.drawText(textRect, Qt::AlignCenter | Qt::AlignVCenter | Qt::TextWordWrap, title);
    p.setFont(oldFont);

    return textRect.bottom();
}

void CalPrintBase::drawSubHeader(QPainter &p, const QRect &box, const QString &str) const
{
    drawShadedBox(p, boxBorderWidth(), QColor(232, 232, 232), box);
    QFont oldfont(p.font());
    //TODO: option for the subheader font?
    p.setFont(QFont("sans-serif", 10, QFont::Bold));
    p.drawText(box, Qt::AlignCenter | Qt::AlignVCenter, str);
    p.setFont(oldfont);
}

int CalPrintBase::drawFooter(QPainter &p, const QRect &footbox) const
{
    QFont oldfont(p.font());
    //TODO: option for the footer font?
    p.setFont(QFont("sans-serif", 6));
    QFontMetrics fm(p.font());
    QString dateStr =
        KGlobal::locale()->formatDateTime(QDateTime::currentDateTime(), KLocale::LongDate);
    p.drawText(footbox, Qt::AlignCenter | Qt::AlignVCenter | Qt::TextSingleLine,
               i18nc("print date: formatted-datetime", "printed: %1", dateStr));
    p.setFont(oldfont);

    return footbox.bottom();
}

void CalPrintBase::drawTimeLine(QPainter &p,
                                const QTime &startTime, const QTime &endTime,
                                const QRect &box) const
{
    drawBox(p, boxBorderWidth(), box);

    int totalsecs = startTime.secsTo(endTime);
    float minlen = (float)box.height() * 60. / (float)totalsecs;
    float cellHeight = (60. * (float)minlen);
    float currY = box.top();
    // TODO: Don't use half of the width, but less, for the minutes!
    int xcenter = box.left() + box.width() / 2;

    QTime curTime(startTime);
    if (startTime.minute() > 30) {
        curTime = QTime(startTime.hour() + 1, 0, 0);
    } else if (startTime.minute() > 0) {
        curTime = QTime(startTime.hour(), 30, 0);
        float yy = currY + minlen * (float)startTime.secsTo(curTime) / 60.;
        p.drawLine(xcenter, (int)yy, box.right(), (int)yy);
        curTime = QTime(startTime.hour() + 1, 0, 0);
    }
    currY += (float(startTime.secsTo(curTime) * minlen) / 60.);

    while (curTime < endTime) {
        p.drawLine(box.left(), (int)currY, box.right(), (int)currY);
        int newY = (int)(currY + cellHeight / 2.);
        QString numStr;
        if (newY < box.bottom()) {
            QFont oldFont(p.font());
            // draw the time:
            if (!KGlobal::locale()->use12Clock()) {
                p.drawLine(xcenter, (int)newY, box.right(), (int)newY);
                numStr.setNum(curTime.hour());
                //TODO: option for this font?
                if (cellHeight > 30) {
                    p.setFont(QFont("sans-serif", 14, QFont::Bold));
                } else {
                    p.setFont(QFont("sans-serif", 12, QFont::Bold));
                }
                p.drawText(box.left() + 4, (int)currY + 2,
                           box.width() / 2 - 2, (int)cellHeight,
                           Qt::AlignTop | Qt::AlignRight, numStr);
                //TODO: option for this font?
                p.setFont(QFont("helvetica", 10, QFont::Normal));
                p.drawText(xcenter + 4, (int)currY + 2,
                           box.width() / 2 + 2, (int)(cellHeight / 2) - 3,
                           Qt::AlignTop | Qt::AlignLeft, "00");
            } else {
                p.drawLine(box.left(), (int)newY, box.right(), (int)newY);
                QTime time(curTime.hour(), 0);
                numStr = KGlobal::locale()->formatTime(time);
                //TODO: option for this font?
                if (box.width() < 60) {
                    p.setFont(QFont("sans-serif", 7, QFont::Bold));     // for weekprint
                } else {
                    p.setFont(QFont("sans-serif", 12, QFont::Bold));     // for dayprint
                }
                p.drawText(box.left() + 2, (int)currY + 2, box.width() - 4, (int)cellHeight / 2 - 3,
                           Qt::AlignTop | Qt::AlignLeft, numStr);
            }
            currY += cellHeight;
            p.setFont(oldFont);
        } // enough space for half-hour line and time
        if (curTime.secsTo(endTime) > 3600) {
            curTime = curTime.addSecs(3600);
        } else {
            curTime = endTime;
        }
    }
}

void CalPrintBase::drawTimeTable(QPainter &p, const QRect &box,
                                 const QDate &startDate, const QDate &endDate,
                                 const QTime &startTime, const QTime &endTime,
                                 bool expandAll) const
{
    QTime myStartTime = startTime;
    QTime myEndTime = endTime;
    if (expandAll) {
        QDate curDate(startDate);
        KDateTime::Spec timeSpec = KSystemTimeZones::local();
        while (curDate <= endDate) {
            KCalCore::Event::List eventList = printCalendar()->events(curDate, timeSpec);
            Q_FOREACH (const KCalCore::Event::Ptr &event, eventList) {
                Q_ASSERT(event);
                if (event->allDay()) {
                    continue;
                }
                if (event->dtStart().time() < myStartTime) {
                    myStartTime = event->dtStart().time();
                }
                if (event->dtEnd().time() > myEndTime) {
                    myEndTime = event->dtEnd().time();
                }
            }
            curDate = curDate.addDays(1);
        }
    }

    // timeline is 1 hour:
    int alldayHeight = (int)(3600. * box.height() / (myStartTime.secsTo(myEndTime) + 3600.));

    QRect dowBox(box);
    dowBox.setLeft(box.left() + timeLineWidth());
    dowBox.setHeight(d->mSubHeaderHeight);
    drawDaysOfWeek(p, dowBox, startDate, endDate);

    QRect tlBox(box);
    tlBox.setWidth(timeLineWidth());
    tlBox.setTop(dowBox.bottom() + boxBorderWidth() + alldayHeight);
    drawTimeLine(p, myStartTime, myEndTime, tlBox);

    // draw each day
    QDate curDate(startDate);
    KDateTime::Spec timeSpec = KSystemTimeZones::local();
    int i = 0;
    double cellWidth = double(dowBox.width()) / double(startDate.daysTo(endDate) + 1);
    const QList<QDate> workDays = CalendarSupport::workDays(startDate, endDate);
    while (curDate <= endDate) {
        QRect allDayBox(dowBox.left() + int(i * cellWidth),
                        dowBox.bottom() + boxBorderWidth(),
                        int((i + 1) * cellWidth) - int(i * cellWidth),
                        alldayHeight);
        QRect dayBox(allDayBox);
        dayBox.setTop(tlBox.top());
        dayBox.setBottom(box.bottom());
        KCalCore::Event::List eventList =
            printCalendar()->events(curDate, timeSpec,
                                    KCalCore::EventSortStartDate,
                                    KCalCore::SortDirectionAscending);

        alldayHeight = drawAllDayBox(p, allDayBox, curDate, eventList);

        drawAgendaDayBox(p, dayBox, curDate, eventList, myStartTime, myEndTime, workDays);

        i++;
        curDate = curDate.addDays(1);
    }
}

void CalPrintBase::drawAgendaDayBox(QPainter &p,
                                    const QRect &box,
                                    const QDate &date,
                                    const KCalCore::Event::List &eventList,
                                    const QTime &startTime,
                                    const QTime &endTime,
                                    const QList<QDate> &workDays) const
{
    QTime myFromTime, myToTime;
    if (startTime.isValid()) {
        myFromTime = startTime;
    } else {
        myFromTime = QTime(0, 0, 0);
    }
    if (endTime.isValid()) {
        myToTime = endTime;
    } else {
        myToTime = QTime(23, 59, 59);
    }

    if (!workDays.contains(date)) {
        drawShadedBox(p, boxBorderWidth(), QColor(232, 232, 232), box);
    } else {
        drawBox(p, boxBorderWidth(), box);
    }

    const bool printConf = typeOptions().testFlag(
                               CalPrintBase::TypeConfidential); //TODO should be false by default
    const bool printPrivate = typeOptions().testFlag(
                                  CalPrintBase::TypePrivate);   //TODO should be false by default

    if (rangeOptions().testFlag(CalPrintBase::RangeTimeExpand)) {//TODO should be false by default
        // Adapt start/end times to include complete events
        Q_FOREACH (const KCalCore::Event::Ptr &event, eventList) {
            Q_ASSERT(event);
            if ((!printConf    && event->secrecy() == KCalCore::Incidence::SecrecyConfidential) ||
                (!printPrivate && event->secrecy() == KCalCore::Incidence::SecrecyPrivate)) {
                continue;
            }
            // skip items without times so that we do not adjust for all day items
            if (event->allDay()) {
                continue;
            }
            if (event->dtStart().time() < myFromTime) {
                myFromTime = event->dtStart().time();
            }
            if (event->dtEnd().time() > myToTime) {
                myToTime = event->dtEnd().time();
            }
        }
    }

    // calculate the height of a cell and of a minute
    int totalsecs = myFromTime.secsTo(myToTime);
    float minlen = box.height() * 60. / totalsecs;
    float cellHeight = 60. * minlen;
    float currY = box.top();

    // print grid:
    QTime curTime(QTime(myFromTime.hour(), 0, 0));
    currY += myFromTime.secsTo(curTime) * minlen / 60;

    while (curTime < myToTime && curTime.isValid()) {
        if (currY > box.top()) {
            p.drawLine(box.left(), int(currY), box.right(), int(currY));
        }
        currY += cellHeight / 2;
        if ((currY > box.top()) && (currY < box.bottom())) {
            // enough space for half-hour line
            QPen oldPen(p.pen());
            p.setPen(QColor(192, 192, 192));     //TODO: define this color
            p.drawLine(box.left(), int(currY), box.right(), int(currY));

            p.setPen(oldPen);
        }
        if (curTime.secsTo(myToTime) > 3600) {
            curTime = curTime.addSecs(3600);
        } else {
            curTime = myToTime;
        }
        currY += cellHeight / 2;
    }

    KDateTime startPrintDate = KDateTime(date, myFromTime);
    KDateTime endPrintDate = KDateTime(date, myToTime);

    // Calculate horizontal positions and widths of events taking into account
    // overlapping events

    QList<CellItem *> cells;

    Q_FOREACH (const KCalCore::Event::Ptr &event, eventList) {
        if (event->allDay()) {
            continue;
        }
        QList<KDateTime>::ConstIterator it;
        QList<KDateTime> times = event->startDateTimesForDate(date);
        for (it = times.constBegin(); it != times.constEnd(); ++it) {
            cells.append(new PrintCellItem(event, (*it), event->endDateForStart(*it)));
        }
    }

    QListIterator<CellItem *> it1(cells);
    while (it1.hasNext()) {
        CellItem *placeItem = it1.next();
        CellItem::placeItem(cells, placeItem);

    }

    QListIterator<CellItem *> it2(cells);
    while (it2.hasNext()) {
        PrintCellItem *placeItem = static_cast<PrintCellItem *>(it2.next());
        drawAgendaItem(placeItem, p, startPrintDate, endPrintDate, minlen, box);
    }
}

void CalPrintBase::drawAgendaItem(PrintCellItem *item, QPainter &p,
                                  const KDateTime &startPrintDate,
                                  const KDateTime &endPrintDate,
                                  float minlen, const QRect &box) const
{
    KCalCore::Event::Ptr event = item->event();

    // start/end of print area for event
    KDateTime startTime = item->start();
    KDateTime endTime = item->end();
    if ((startTime < endPrintDate && endTime > startPrintDate) ||
        (endTime > startPrintDate && startTime < endPrintDate)) {
        if (startTime < startPrintDate) {
            startTime = startPrintDate;
        }
        if (endTime > endPrintDate) {
            endTime = endPrintDate;
        }
        int currentWidth = box.width() / item->subCells();
        int currentX = box.left() + item->subCell() * currentWidth;
        int currentYPos =
            int(box.top() + startPrintDate.secsTo(startTime) * minlen / 60.);
        int currentHeight =
            int(box.top() + startPrintDate.secsTo(endTime) * minlen / 60.) - currentYPos;

        QRect eventBox(currentX, currentYPos, currentWidth, currentHeight);
        QString str;
        if (!infoOptions().testFlag(CalPrintBase::InfoTimeRange)) {
            if (event->location().isEmpty()) {
                str = cleanString(event->summary());
            } else {
                str = i18nc("summary, location", "%1, %2",
                            cleanString(event->summary()),
                            cleanString(event->location()));
            }
        } else {
            if (event->location().isEmpty()) {
                str = i18nc("starttime - endtime summary",
                            "%1-%2 %3",
                            KGlobal::locale()->formatTime(item->start().toLocalZone().time()),
                            KGlobal::locale()->formatTime(item->end().toLocalZone().time()),
                            cleanString(event->summary()));
            } else {
                str = i18nc("starttime - endtime summary, location",
                            "%1-%2 %3, %4",
                            KGlobal::locale()->formatTime(item->start().toLocalZone().time()),
                            KGlobal::locale()->formatTime(item->end().toLocalZone().time()),
                            cleanString(event->summary()),
                            cleanString(event->location()));
            }
        }
        if (infoOptions().testFlag(CalPrintBase::InfoDescription) &&
            !event->description().isEmpty()) {
            str += '\n';
            if (event->descriptionIsRich()) {
                str += toPlainText(event->description());
            } else {
                str += event->description();
            }
        }
        QFont oldFont(p.font());
        //TODO: font option?
        if (eventBox.height() < 24) {
            if (eventBox.height() < 12) {
                if (eventBox.height() < 8) {
                    p.setFont(QFont("sans-serif", 4));
                } else {
                    p.setFont(QFont("sans-serif", 5));
                }
            } else {
                p.setFont(QFont("sans-serif", 6));
            }
        } else {
            p.setFont(QFont("sans-serif", 8));
        }
        drawItemBox(p, itemBoxBorderWidth(), eventBox, event, str);
        p.setFont(oldFont);
    }
}

void CalPrintBase::drawDaysOfWeekBox(QPainter &p, const QRect &box, const QDate &date) const
{
    KCalendarSystem *calSys = calendarSystem();
    drawSubHeader(p, box, (calSys) ? (calSys->weekDayName(date)) : QString());
}

void CalPrintBase::drawDaysOfWeek(QPainter &p, const QRect &box,
                                  const QDate &fromDate, const QDate &toDate) const
{
    double cellWidth = double(box.width()) / double(fromDate.daysTo(toDate) + 1);
    QDate cellDate(fromDate);
    QRect dateBox(box);
    int i = 0;

    while (cellDate <= toDate) {
        dateBox.setLeft(box.left() + int(i * cellWidth));
        dateBox.setRight(box.left() + int((i + 1) * cellWidth));
        drawDaysOfWeekBox(p, dateBox, cellDate);
        cellDate = cellDate.addDays(1);
        i++;
    }
}

int CalPrintBase::drawAllDayBox(QPainter &p, const QRect &box,
                                const QDate &date, const KCalCore::Event::List &eventList,
                                bool expandAll) const
{
    KCalCore::Event::List::Iterator it;
    int offset = box.top();
    QString multiDayStr;

    KCalCore::Event::List evList = eventList;
    KCalCore::Event::Ptr hd = holidayEvent(date);
    if (hd) {
        evList.prepend(hd);
    }

    const bool printConf = typeOptions().testFlag(
                               CalPrintBase::TypeConfidential); //TODO should be false by default
    const bool printPrivate = typeOptions().testFlag(
                                  CalPrintBase::TypePrivate);   //TODO should be false by default

    it = evList.begin();
    while (it != evList.end()) {
        KCalCore::Event::Ptr currEvent = *it;
        if ((!printConf    && currEvent->secrecy() == KCalCore::Incidence::SecrecyConfidential) ||
            (!printPrivate && currEvent->secrecy() == KCalCore::Incidence::SecrecyPrivate)) {
            continue;
        }
        if (currEvent && currEvent->allDay()) {
            // set the colors according to the categories
            if (expandAll) {
                QRect eventBox(box);
                eventBox.setTop(offset);
                drawItemBox(p, itemBoxBorderWidth(), eventBox, currEvent, currEvent->summary());
                offset += box.height();
            } else {
                if (!multiDayStr.isEmpty()) {
                    multiDayStr += ", ";
                }
                multiDayStr += currEvent->summary();
            }
            it = evList.erase(it);
        } else {
            ++it;
        }
    }

    int ret = box.height();
    QRect eventBox(box);
    if (!expandAll) {
        if (!multiDayStr.isEmpty()) {
            drawShadedBox(p, boxBorderWidth(), QColor(180, 180, 180), eventBox);
            drawItemString(p, eventBox, multiDayStr);
        } else {
            drawBox(p, boxBorderWidth(), eventBox);
        }
    } else {
        ret = offset - box.top();
        eventBox.setBottom(ret);
        drawBox(p, boxBorderWidth(), eventBox);
    }
    return ret;
}

void CalPrintBase::drawDayIncidence(QPainter &p, const QRect &dayBox,
                                    const QString &time,
                                    const QString &summary,
                                    const QString &description,
                                    int &textY,
                                    bool richDescription) const
{
    qDebug() << "summary =" << summary;

    int flags = Qt::AlignLeft | Qt::OpaqueMode;
    QFontMetrics fm = p.fontMetrics();
    const int borderWidth = p.pen().width() + 1;
    QRect timeBound = p.boundingRect(dayBox.x() + borderWidth,
                                     dayBox.y() + textY,
                                     dayBox.width(), fm.lineSpacing(),
                                     flags, time);

    int summaryWidth = time.isEmpty() ? 0 : timeBound.width() + 3;
    QRect summaryBound = QRect(dayBox.x() + borderWidth + summaryWidth,
                               dayBox.y() + textY + 1,
                               dayBox.width() - summaryWidth - (borderWidth * 2),
                               dayBox.height() - textY);

    QString summaryText = summary;
    QString descText = toPlainText(description);
    bool boxOverflow = false;

    const bool includeDescription = infoOptions().testFlag(
                                        CalPrintBase::InfoDescription); // TODO false by default

    if (extraOptions().testFlag(CalPrintBase::ExtraSingleLine)) {//TODO should be true by default
        if (includeDescription && !descText.isEmpty()) {
            summaryText += ", " + descText;
        }
        int totalHeight = fm.lineSpacing() + borderWidth;
        int textBoxHeight = (totalHeight > (dayBox.height() - textY)) ?
                            dayBox.height() - textY :
                            totalHeight;
        summaryBound.setHeight(textBoxHeight);
        QRect lineRect(dayBox.x() + borderWidth, dayBox.y() + textY,
                       dayBox.width() - (borderWidth * 2), textBoxHeight);
        drawBox(p, 1, lineRect);
        if (!time.isEmpty()) {
            p.drawText(timeBound, flags, time);
        }
        p.drawText(summaryBound, flags, summaryText);
    } else {
        QTextDocument textDoc;
        QTextCursor textCursor(&textDoc);
        if (richDescription) {
            QTextCursor textCursor(&textDoc);
            textCursor.insertText(summaryText);
            if (includeDescription && !description.isEmpty()) {
                textCursor.insertText("\n");
                textCursor.insertHtml(description);
            }
        } else {
            textCursor.insertText(summaryText);
            if (includeDescription && !descText.isEmpty()) {
                textCursor.insertText("\n");
                textCursor.insertText(descText);
            }
        }
        textDoc.setPageSize(QSize(summaryBound.width(), summaryBound.height()));
        p.save();
        QRect clipBox(0, 0, summaryBound.width(), summaryBound.height());
        p.setFont(p.font());
        p.translate(summaryBound.x(), summaryBound.y());
        summaryBound.setHeight(textDoc.documentLayout()->documentSize().height());
        if (summaryBound.bottom() > dayBox.bottom()) {
            summaryBound.setBottom(dayBox.bottom());
        }
        clipBox.setHeight(summaryBound.height());
        p.restore();

        p.save();
        QRect backBox(timeBound.x(), timeBound.y(),
                      dayBox.width() - (borderWidth * 2), clipBox.height());
        drawBox(p, 1, backBox);

        if (!time.isEmpty()) {
            if (timeBound.bottom() > dayBox.bottom()) {
                timeBound.setBottom(dayBox.bottom());
            }
            timeBound.moveTop(timeBound.y() + (summaryBound.height() - timeBound.height()) / 2);
            p.drawText(timeBound, flags, time);
        }
        p.translate(summaryBound.x(), summaryBound.y());
        textDoc.drawContents(&p, clipBox);
        p.restore();
        boxOverflow = textDoc.pageCount() > 1;
    }
    if (summaryBound.bottom() < dayBox.bottom()) {
        QPen oldPen(p.pen());
        p.setPen(QPen());
        p.drawLine(dayBox.x(), summaryBound.bottom(),
                   dayBox.x() + dayBox.width(), summaryBound.bottom());
        p.setPen(oldPen);
    }
    textY += summaryBound.height();

    // show that we have overflowed the box
    if (boxOverflow) {
        QPolygon poly(3);
        int x = dayBox.x() + dayBox.width();
        int y = dayBox.y() + dayBox.height();
        poly.setPoint(0, x - 10, y);
        poly.setPoint(1, x, y - 10);
        poly.setPoint(2, x, y);
        QBrush oldBrush(p.brush());
        p.setBrush(QBrush(Qt::black));
        p.drawPolygon(poly);
        p.setBrush(oldBrush);
        textY = dayBox.height();
    }
}

void CalPrintBase::drawNoteLines(QPainter &p, const QRect &box, int startY) const
{
    int lineHeight = int(p.fontMetrics().lineSpacing() * 1.5);
    int linePos = box.y();
    int startPos = startY;
    // adjust line to start at multiple from top of box for alignment
    while (linePos < startPos) {
        linePos += lineHeight;
    }
    QPen oldPen(p.pen());
    p.setPen(Qt::DotLine);
    while (linePos < box.bottom()) {
        p.drawLine(box.left() + padding(), linePos,
                   box.right() - padding(), linePos);
        linePos += lineHeight;
    }
    p.setPen(oldPen);
}

void CalPrintBase::drawDayBox(QPainter &p,
                              const QRect &box,
                              const QDate &date,
                              const QTime &startTime, const QTime &endTime,
                              bool fullDate) const
{
    QString dayNumStr;
    const KLocale *local = KGlobal::locale();

    QTime myStartTime, myEndTime;
    if (startTime.isValid()) {
        myStartTime = startTime;
    } else {
        myStartTime = QTime(0, 0, 0);
    }
    if (endTime.isValid()) {
        myEndTime = endTime;
    } else {
        myEndTime = QTime(23, 59, 59);
    }

    if (fullDate && calendarSystem()) {
        dayNumStr = i18nc("weekday, shortmonthname daynumber",
                          "%1, %2 <numid>%3</numid>",
                          calendarSystem()->weekDayName(date),
                          calendarSystem()->monthName(date, KCalendarSystem::ShortName),
                          date.day());
    } else {
        dayNumStr = QString::number(date.day());
    }

    QRect subHeaderBox(box);
    subHeaderBox.setHeight(subHeaderHeight());
    drawShadedBox(p, boxBorderWidth(), p.background(), box);
    drawShadedBox(p, 0, QColor(232, 232, 232), subHeaderBox);
    drawBox(p, boxBorderWidth(), box);
    QString hstring(holidayString(date));
    const QFont oldFont(p.font());

    QRect headerTextBox(subHeaderBox);
    headerTextBox.setLeft(subHeaderBox.left() + 5);
    headerTextBox.setRight(subHeaderBox.right() - 5);
    if (!hstring.isEmpty()) {
        //TODO: option for this font
        p.setFont(QFont("sans-serif", 8, QFont::Bold, true));
        p.drawText(headerTextBox, Qt::AlignLeft | Qt::AlignVCenter, hstring);
    }
    //TODO: option for this font
    p.setFont(QFont("sans-serif", 10, QFont::Bold));
    p.drawText(headerTextBox, Qt::AlignRight | Qt::AlignVCenter, dayNumStr);

    const KCalCore::Event::List eventList =
        printCalendar()->events(date, KSystemTimeZones::local(),
                                KCalCore::EventSortStartDate,
                                KCalCore::SortDirectionAscending);

    QString timeText;
    //TODO: option for this font
    p.setFont(QFont("sans-serif", 7));

    const bool printConf = typeOptions().testFlag(
                               CalPrintBase::TypeConfidential); //TODO should be false by default
    const bool printPrivate = typeOptions().testFlag(
                                  CalPrintBase::TypePrivate);   //TODO should be false by default
    const bool printRecurDaily = rangeOptions().testFlag(
                                     CalPrintBase::RangeRecurDaily); //TODO should be false by default?
    const bool printRecurWeekly = rangeOptions().testFlag(
                                      CalPrintBase::RangeRecurWeekly); //TODO should be false by defualt?

    int textY = subHeaderHeight(); // gives the relative y-coord of the next printed entry
    unsigned int visibleEventsCounter = 0;
    Q_FOREACH (const KCalCore::Event::Ptr &currEvent, eventList) {
        Q_ASSERT(currEvent);
        if (!currEvent->allDay()) {
            if (currEvent->dtEnd().toLocalZone().time() <= myStartTime ||
                currEvent->dtStart().toLocalZone().time() > myEndTime) {
                continue;
            }
        }
        if ((!printRecurDaily  && currEvent->recurrenceType() == KCalCore::Recurrence::rDaily) ||
            (!printRecurWeekly && currEvent->recurrenceType() == KCalCore::Recurrence::rWeekly)) {
            continue;
        }
        if ((!printConf    && currEvent->secrecy() == KCalCore::Incidence::SecrecyConfidential) ||
            (!printPrivate && currEvent->secrecy() == KCalCore::Incidence::SecrecyPrivate)) {
            continue;
        }
        if (currEvent->allDay() || currEvent->isMultiDay()) {
            timeText.clear();
        } else {
            timeText = local->formatTime(currEvent->dtStart().toLocalZone().time()) + ' ';
        }
        p.save();
        setColorsByIncidenceCategory(p, currEvent);
        QString summaryStr = currEvent->summary();
        if (!currEvent->location().isEmpty()) {
            summaryStr = i18nc("summary, location",
                               "%1, %2", summaryStr, currEvent->location());
        }
        drawDayIncidence(p, box, timeText,
                         summaryStr, currEvent->description(),
                         textY, currEvent->descriptionIsRich());
        p.restore();
        visibleEventsCounter++;

        if (textY >= box.height()) {
            const QChar downArrow(0x21e3);

            const unsigned int invisibleIncidences =
                (eventList.count() - visibleEventsCounter) + printCalendar()->todos(date).count();
            if (invisibleIncidences > 0) {
                const QString warningMsg =
                    QString("%1 (%2)").arg(downArrow).arg(invisibleIncidences);

                QFontMetrics fm(p.font());
                QRect msgRect = fm.boundingRect(warningMsg);
                msgRect.setRect(box.right() - msgRect.width() - 2,
                                box.bottom() - msgRect.height() - 2,
                                msgRect.width(), msgRect.height());

                p.save();
                p.setPen(Qt::red);   //krazy:exclude=qenums we don't allow custom print colors
                p.drawText(msgRect, Qt::AlignLeft, warningMsg);
                p.restore();
            }
            break;
        }
    }

    if (textY < box.height()) {
        KCalCore::Todo::List todoList = printCalendar()->todos(date);
        Q_FOREACH (const KCalCore::Todo::Ptr &todo, todoList) {
            if (!todo->allDay()) {
                if ((todo->hasDueDate() && todo->dtDue().toLocalZone().time() <= myStartTime) ||
                    (todo->hasStartDate() && todo->dtStart().toLocalZone().time() > myEndTime)) {
                    continue;
                }
            }
            if ((!printRecurDaily  && todo->recurrenceType() == KCalCore::Recurrence::rDaily) ||
                (!printRecurWeekly && todo->recurrenceType() == KCalCore::Recurrence::rWeekly)) {
                continue;
            }
            if ((!printConf    && todo->secrecy() == KCalCore::Incidence::SecrecyConfidential) ||
                (!printPrivate && todo->secrecy() == KCalCore::Incidence::SecrecyPrivate)) {
                continue;
            }
            if (todo->hasStartDate() && !todo->allDay()) {
                timeText =
                    KGlobal::locale()->formatTime(todo->dtStart().toLocalZone().time()) + ' ';
            } else {
                timeText.clear();
            }
            p.save();
            setColorsByIncidenceCategory(p, todo);
            QString summaryStr = todo->summary();
            if (!todo->location().isEmpty()) {
                summaryStr = i18nc("summary, location",
                                   "%1, %2", summaryStr, todo->location());
            }

            QString str;
            if (todo->hasDueDate()) {
                if (!todo->allDay()) {
                    str = i18nc("to-do summary (Due: datetime)", "%1 (Due: %2)",
                                summaryStr,
                                KGlobal::locale()->formatDateTime(todo->dtDue().toLocalZone()));
                } else {
                    str = i18nc("to-do summary (Due: date)", "%1 (Due: %2)",
                                summaryStr,
                                KGlobal::locale()->formatDate(
                                    todo->dtDue().toLocalZone().date(), KLocale::ShortDate));
                }
            } else {
                str = summaryStr;
            }
            drawDayIncidence(p, box, timeText,
                             i18n("To-do: %1", str), todo->description(),
                             textY, todo->descriptionIsRich());
            p.restore();
        }
    }
    if (extraOptions().testFlag(CalPrintBase::ExtraNoteLines)) {//TODO should be false by default
        drawNoteLines(p, box, box.y() + textY);
    }

    p.setFont(oldFont);
}

// Utility Functions that could be moved into CalendarSupport or somesuch
int CalPrintBase::weekdayColumn(const int weekday) const
{
    int w = weekday + 7 - KGlobal::locale()->weekStartDay();
    return w % 7;
}

QString CalPrintBase::cleanString(const QString &str) const
{
    QString ret = str;
    return ret.replace('\n', ' ');
}

QString CalPrintBase::toPlainText(const QString &htmlText) const
{
    return QTextDocumentFragment::fromHtml(htmlText).toPlainText();
}

QColor CalPrintBase::getTextColor(const QColor &c) const
{
    double luminance = (c.red() * 0.299) + (c.green() * 0.587) + (c.blue() * 0.114);
    return (luminance > 128.0) ? QColor(0, 0, 0) : QColor(255, 255, 255);
}

QColor CalPrintBase::categoryColor(const QStringList &categories) const
{
    if (categories.isEmpty()) {
        return CalendarSupport::KCalPrefs::instance()->unsetCategoryColor();
    }
    // FIXME: Correctly treat events with multiple categories
    const QString cat = categories.first();
    QColor bgColor;
    if (cat.isEmpty()) {
        bgColor = CalendarSupport::KCalPrefs::instance()->unsetCategoryColor();
    } else {
        bgColor = CalendarSupport::KCalPrefs::instance()->categoryColor(cat);
    }
    return bgColor;
}

QColor CalPrintBase::categoryBgColor(const KCalCore::Incidence::Ptr &incidence) const
{
    if (incidence) {
        QColor backColor = categoryColor(incidence->categories());
        if (incidence->type() == KCalCore::Incidence::TypeTodo) {
            if ((incidence.staticCast<KCalCore::Todo>())->isOverdue()) {
                //TODO make this available from CalendarSupport::KCalPrefs
                backColor = QColor(255, 100, 100);   //was KOPrefs::instance()->todoOverdueColor();
            }
        }
        return backColor;
    } else {
        return QColor();
    }
}

void CalPrintBase::setColorsByIncidenceCategory(QPainter &p,
        const KCalCore::Incidence::Ptr &incidence) const
{
    QColor bgColor = categoryBgColor(incidence);
    if (bgColor.isValid()) {
        p.setBrush(bgColor);
    }
    QColor tColor(getTextColor(bgColor));
    if (tColor.isValid()) {
        p.setPen(tColor);
    }
}

QString CalPrintBase::holidayString(const QDate &date) const
{
    QStringList lst = CalendarSupport::holiday(date);
    return lst.join(i18nc("@item:intext delimiter for joining holiday names", ","));
}

KCalCore::Event::Ptr CalPrintBase::holidayEvent(const QDate &date) const
{
    QString hstring(holidayString(date));
    if (hstring.isEmpty()) {
        return KCalCore::Event::Ptr();
    }

    KCalCore::Event::Ptr holiday(new KCalCore::Event);
    holiday->setSummary(hstring);
    holiday->setCategories(i18n("Holiday"));

    KDateTime kdt(date, QTime(), KSystemTimeZones::local());
    holiday->setDtStart(kdt);
    holiday->setDtEnd(kdt);
    holiday->setAllDay(true);

    return holiday;
}
