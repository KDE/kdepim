/* Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
   Author: Sérgio Martins <sergio.martins@kdab.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "memorycalendarmemento.h"
#include <calendarsupport/calendarsingleton.h>
#include <Akonadi/Calendar/FetchJobCalendar>
#include <Akonadi/Calendar/ETMCalendar>
#include "text_calendar_debug.h"

using namespace MessageViewer;
using namespace Akonadi;

MemoryCalendarMemento::MemoryCalendarMemento()
    : QObject(0), mFinished(false)
{

    Akonadi::ETMCalendar::Ptr etmCalendar = CalendarSupport::calendarSingleton(/*createIfNull=*/false);
    if (etmCalendar && etmCalendar->isLoaded()) {
        // Good, either korganizer or kontact summary view are already running, so reuse ETM to save memory
        mCalendar = etmCalendar;
        QMetaObject::invokeMethod(this, "finalize", Qt::QueuedConnection);
    } else {
        FetchJobCalendar::Ptr calendar = FetchJobCalendar::Ptr(new FetchJobCalendar(this));
        mCalendar = calendar;
        connect(calendar.data(), &FetchJobCalendar::loadFinished,
                this, &MemoryCalendarMemento::slotCalendarLoaded);
    }
}

void MemoryCalendarMemento::slotCalendarLoaded(bool success, const QString &errorMessage)
{
    qCDebug(TEXT_CALENDAR_LOG);
    if (!success) {
        qCWarning(TEXT_CALENDAR_LOG) << "Unable to fetch incidences:" << errorMessage;
    }

    finalize();
}

void MemoryCalendarMemento::finalize()
{
    mFinished = true;
    Q_EMIT update(Viewer::Delayed);
}

bool MemoryCalendarMemento::finished() const
{
    return mFinished;
}

KCalCore::MemoryCalendar::Ptr MemoryCalendarMemento::calendar() const
{
    Q_ASSERT(mFinished);
    return mCalendar;
}

void MemoryCalendarMemento::detach()
{
    disconnect(this, SIGNAL(update(MessageViewer::Viewer::UpdateMode)), 0, 0);
}

