/*
  Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
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

#include "freeperiodmodel.h"

#include <KCalCore/Period>

#include <KLocale>
#include <KFormat>
#include <KSystemTimeZones>

#include <QLocale>
#include <QSet>

using namespace KPIM;

FreePeriodModel::FreePeriodModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

FreePeriodModel::~FreePeriodModel()
{
}

QVariant FreePeriodModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !hasIndex(index.row(), index.column())) {
        return QVariant();
    }

    if (index.column() == 0) {  //day
        switch (role) {
        case Qt::DisplayRole:
            return day(index.row());
        case Qt::ToolTipRole:
            return tooltipify(index.row());
        case FreePeriodModel::PeriodRole:
            return QVariant::fromValue(mPeriodList.at(index.row()));
        case Qt::TextAlignmentRole:
            return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
        default:
            return QVariant();
        }
    } else { // everything else
        switch (role) {
        case Qt::DisplayRole:
            return date(index.row());
        case Qt::ToolTipRole:
            return tooltipify(index.row());
        case FreePeriodModel::PeriodRole:
            return QVariant::fromValue(mPeriodList.at(index.row()));
        case Qt::TextAlignmentRole:
            return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
        default:
            return QVariant();
        }
    }
}

int FreePeriodModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return mPeriodList.size();
    }
    return 0;
}

int FreePeriodModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;
}

QVariant FreePeriodModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QAbstractItemModel::headerData(section, orientation, role);
}

void FreePeriodModel::slotNewFreePeriods(const KCalCore::Period::List &freePeriods)
{
    beginResetModel();
    mPeriodList.clear();
    mPeriodList = splitPeriodsByDay(freePeriods);
    qSort(mPeriodList);
    endResetModel();
}

KCalCore::Period::List FreePeriodModel::splitPeriodsByDay(
    const KCalCore::Period::List &freePeriods)
{
    KCalCore::Period::List splitList;
    foreach (const KCalCore::Period &period, freePeriods) {
        if (period.start().date() == period.end().date())  {
            splitList << period; // period occurs on the same day
            continue;
        }

        const int validPeriodSecs = 5 * 60; // 5 minutes
        KCalCore::Period tmpPeriod = period;
        while (tmpPeriod.start().date() != tmpPeriod.end().date()) {
            const KDateTime midnight(tmpPeriod.start().date(), QTime(23, 59, 59, 999),
                                     tmpPeriod.start().timeSpec());
            KCalCore::Period firstPeriod(tmpPeriod.start(), midnight);
            KCalCore::Period secondPeriod(midnight.addMSecs(1), tmpPeriod.end());
            if (firstPeriod.duration().asSeconds() >= validPeriodSecs) {
                splitList << firstPeriod;
            }
            tmpPeriod = secondPeriod;
        }
        if (tmpPeriod.duration().asSeconds() >= validPeriodSecs) {
            splitList << tmpPeriod;
        }
    }

    // Perform some jiggery pokery to remove duplicates

#pragma message("port to QT5")
#if 0 //QT5 FIX ME
    QList<KCalCore::Period> tmpList = splitList.toList();
    QSet<KCalCore::Period>set = tmpList.toSet();
    tmpList = QList<KCalCore::Period>::fromSet(set);
    return KCalCore::Period::List::fromList(tmpList);
#else
    return splitList;
#endif
}

QString FreePeriodModel::day(int index) const
{
    KCalCore::Period period = mPeriodList.at(index);
    const QDate startDate = period.start().date();
    return ki18nc("@label Day of the week name, example: Monday,", "%1,").
           subs(QLocale::system().dayName(startDate.dayOfWeek(), QLocale::LongFormat)).toString();
}

QString FreePeriodModel::date(int index) const
{
    KCalCore::Period period = mPeriodList.at(index);

    const QDate startDate = period.start().date();
    const QString startTime = QLocale::system().toString(period.start().time(), QLocale::ShortFormat);
    const QString endTime = QLocale::system().toString(period.end().time(), QLocale::ShortFormat);
    const QString longMonthName = QLocale::system().monthName(startDate.month());
    return ki18nc("@label A time period duration. It is preceded/followed (based on the "
                  "orientation) by the name of the week, see the message above. "
                  "example: 12 June, 8:00am to 9:30am",
                  "%1 %2, %3 to %4").
           subs(startDate.day()).
           subs(longMonthName).
           subs(startTime).
           subs(endTime).toString();
}

QString FreePeriodModel::stringify(int index) const
{
    KCalCore::Period period = mPeriodList.at(index);

    const QDate startDate = period.start().date();
    const QString startTime = KLocale::global()->formatTime(period.start().time(), false, true);
    const QString endTime = KLocale::global()->formatTime(period.end().time(), false, true);
    const QString longMonthName = QLocale::system().monthName(startDate.month(), QLocale::LongFormat);
    const QString dayofWeek = QLocale::system().dayName(startDate.dayOfWeek(), QLocale::LongFormat);

    // TODO i18n, ping chusslove
    return ki18nc("@label A time period duration. KLocale is used to format the components. "
                  "example: Monday, 12 June, 8:00am to 9:30am",
                  "%1, %2 %3, %4 to %5").
           subs(dayofWeek).
           subs(startDate.day()).
           subs(longMonthName).
           subs(startTime).
           subs(endTime).toString();
}

QString FreePeriodModel::tooltipify(int index) const
{
    KDateTime::Spec timeSpec = KSystemTimeZones::local();
    KCalCore::Period period = mPeriodList.at(index);
    unsigned long duration = period.duration().asSeconds() * 1000; // we want milliseconds
    QString toolTip = QStringLiteral("<qt>");
    toolTip += QLatin1String("<b>") + i18nc("@info:tooltip", "Free Period") + QLatin1String("</b>");
    toolTip += QLatin1String("<hr>");
    toolTip += QLatin1String("<i>") + i18nc("@info:tooltip period start time", "Start:") + QLatin1String("</i>&nbsp;");
    toolTip += QLocale::system().toString(period.start().toTimeSpec(timeSpec).dateTime(), QLocale::ShortFormat);
    toolTip += QLatin1String("<br>");
    toolTip += QLatin1String("<i>") + i18nc("@info:tooltip period end time", "End:") + QLatin1String("</i>&nbsp;");
    toolTip += QLocale::system().toString(period.end().toTimeSpec(timeSpec).dateTime(), QLocale::ShortFormat);
    toolTip += QLatin1String("<br>");
    toolTip += QLatin1String("<i>") + i18nc("@info:tooltip period duration", "Duration:") + QLatin1String("</i>&nbsp;");
    toolTip += KFormat().formatSpelloutDuration(duration);
    toolTip += QLatin1String("</qt>");
    return toolTip;
}

