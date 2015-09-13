/*
 * Copyright (c) 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * As a special exception, permission is given to link this program
 * with any edition of Qt, and distribute the resulting executable,
 * without including the source code for Qt in the source distribution.
 */

#include "viewcalendar.h"
#include "agendaview.h"
#include "helper.h"
#include "calendarview_debug.h"

#include <CalendarSupport/Utils>

using namespace EventViews;

ViewCalendar::~ViewCalendar()
{
}

MultiViewCalendar::~MultiViewCalendar()
{
}

KCalCore::Calendar::Ptr MultiViewCalendar::getCalendar() const
{
    return KCalCore::Calendar::Ptr();
}

KCalCore::Incidence::List MultiViewCalendar::incidences() const
{
    KCalCore::Incidence::List list;
    foreach (const ViewCalendar::Ptr &cal, mSubCalendars) {
        if (cal->getCalendar()) {
            list += cal->getCalendar()->incidences();
        }
    }
    return list;
}

int MultiViewCalendar::calendars() const
{
    return mSubCalendars.size();
}

ViewCalendar::Ptr MultiViewCalendar::findCalendar(const KCalCore::Incidence::Ptr &incidence) const
{
    foreach (const ViewCalendar::Ptr &cal, mSubCalendars) {
        if (cal->isValid(incidence)) {
            return cal;
        }
    }
    return ViewCalendar::Ptr();
}

ViewCalendar::Ptr MultiViewCalendar::findCalendar(const QString &incidenceIdentifier) const
{
    foreach (const ViewCalendar::Ptr &cal, mSubCalendars) {
        if (cal->isValid(incidenceIdentifier)) {
            return cal;
        }
    }
    return ViewCalendar::Ptr();
}

void MultiViewCalendar::addCalendar(const ViewCalendar::Ptr &calendar)
{
    if (!mSubCalendars.contains(calendar)) {
        mSubCalendars.append(calendar);
    }
}

void MultiViewCalendar::setETMCalendar(const Akonadi::ETMCalendar::Ptr &calendar)
{
    if (!mETMCalendar) {
        mETMCalendar = AkonadiViewCalendar::Ptr(new AkonadiViewCalendar);
        mETMCalendar->mAgendaView = mAgendaView;
    }
    mETMCalendar->mCalendar = calendar;
    addCalendar(mETMCalendar);
}

QString MultiViewCalendar::displayName(const KCalCore::Incidence::Ptr &incidence) const
{
    ViewCalendar::Ptr cal = findCalendar(incidence);
    if (cal) {
        return cal->displayName(incidence);
    }
    return QString();
}

QString MultiViewCalendar::iconForIncidence(const KCalCore::Incidence::Ptr &incidence) const
{
    ViewCalendar::Ptr cal = findCalendar(incidence);
    if (cal) {
        return cal->iconForIncidence(incidence);
    }
    return QString();
}

bool MultiViewCalendar::isValid(const KCalCore::Incidence::Ptr &incidence) const
{
    ViewCalendar::Ptr cal = findCalendar(incidence);
    return cal;
}

bool MultiViewCalendar::isValid(const QString &incidenceIdentifier) const
{
    ViewCalendar::Ptr cal = findCalendar(incidenceIdentifier);
    return cal;
}

QColor MultiViewCalendar::resourceColor(const KCalCore::Incidence::Ptr &incidence) const
{
    ViewCalendar::Ptr cal = findCalendar(incidence);
    if (cal) {
        return cal->resourceColor(incidence);
    }
    return QColor();
}

Akonadi::Item MultiViewCalendar::item(const KCalCore::Incidence::Ptr &incidence) const
{
    if (mETMCalendar->isValid(incidence)) {
        return mETMCalendar->item(incidence);
    }

    return Akonadi::Item();
}

AkonadiViewCalendar::~AkonadiViewCalendar()
{
}

bool AkonadiViewCalendar::isValid(const KCalCore::Incidence::Ptr &incidence) const
{
    if (!mCalendar) {
        return false;
    }

    if (item(incidence).isValid()) {
        return true;
    }
    return false;
}

bool AkonadiViewCalendar::isValid(const QString &incidenceIdentifier) const
{
    if (!mCalendar) {
        return false;
    }

    return !mCalendar->incidence(incidenceIdentifier).isNull();
}

Akonadi::Item AkonadiViewCalendar::item(const KCalCore::Incidence::Ptr &incidence) const
{
    if (!mCalendar || !incidence) {
        return Akonadi::Item();
    }
    bool ok = false;
    Akonadi::Item::Id id = incidence->customProperty("VOLATILE", "AKONADI-ID").toLongLong(&ok);

    if (id == -1 || !ok) {
        id = mCalendar->item(incidence).id();

        if (id == -1) {
            // Ok, we really don't know the ID, give up.
            qCWarning(CALENDARVIEW_LOG) << "Item is invalid. uid = " << incidence->instanceIdentifier();
            return Akonadi::Item();
        }
        return mCalendar->item(incidence->instanceIdentifier());
    }
    return mCalendar->item(id);
}

QString AkonadiViewCalendar::displayName(const KCalCore::Incidence::Ptr &incidence) const
{
    return CalendarSupport::displayName(mCalendar.data(), item(incidence).parentCollection());
}

QColor AkonadiViewCalendar::resourceColor(const KCalCore::Incidence::Ptr &incidence) const
{
    return EventViews::resourceColor(item(incidence), mAgendaView->preferences());
}

QString AkonadiViewCalendar::iconForIncidence(const KCalCore::Incidence::Ptr &incidence) const
{
    return mAgendaView->iconForItem(item(incidence));
}

KDateTime::Spec AkonadiViewCalendar::timeSpec() const
{
    return mCalendar->timeSpec();
}

KCalCore::Calendar::Ptr AkonadiViewCalendar::getCalendar() const
{
    return mCalendar;
}
