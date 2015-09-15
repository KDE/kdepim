/*
 * Copyright 2014  Sandro Knauß <knauss@kolabsys.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "freebusycalendar.h"
#include "calendarsupport_debug.h"

#include <KCalCore/Event>
#include <KCalCore/FreeBusyPeriod>
#include <KCalCore/MemoryCalendar>
#include <KCalCore/CalFormat>

#include <KLocalizedString>
#include <KSystemTimeZones>

#include <QStringList>

using namespace KPIM;

class KPIM::FreeBusyCalendarPrivate
{
public:
    FreeBusyCalendarPrivate()
        : mModel(Q_NULLPTR)
    {

    }

    FreeBusyItemModel *mModel;
    KCalCore::Calendar::Ptr mCalendar;
    QMap<QModelIndex, KCalCore::Event::Ptr> mFbEvent;
};

FreeBusyCalendar::FreeBusyCalendar(QObject *parent)
    : QObject(parent)
    , d(new KPIM::FreeBusyCalendarPrivate)
{
    d->mCalendar = KCalCore::Calendar::Ptr(new KCalCore::MemoryCalendar(KSystemTimeZones::local()));
    qCDebug(CALENDARSUPPORT_LOG) << "creating" << this;
}

FreeBusyCalendar::~FreeBusyCalendar()
{
    qCDebug(CALENDARSUPPORT_LOG) << "deleting" << this;
    delete d;
}

KCalCore::Calendar::Ptr FreeBusyCalendar::calendar() const
{
    return d->mCalendar;
}

FreeBusyItemModel *FreeBusyCalendar::model() const
{
    return d->mModel;
}

void FreeBusyCalendar::setModel(FreeBusyItemModel *model)
{
    if (model != d->mModel) {
        if (d->mModel) {
            disconnect(d->mModel, 0, 0, 0);
        }
        d->mModel = model;
        connect(d->mModel, &QAbstractItemModel::layoutChanged, this, &FreeBusyCalendar::onLayoutChanged);
        connect(d->mModel, &QAbstractItemModel::modelReset, this, &FreeBusyCalendar::onLayoutChanged);
        connect(d->mModel, &QAbstractItemModel::rowsAboutToBeRemoved,
                this, &FreeBusyCalendar::onRowsRemoved);
        connect(d->mModel, &QAbstractItemModel::rowsInserted,
                this, &FreeBusyCalendar::onRowsInserted);
        connect(d->mModel, &QAbstractItemModel::dataChanged,
                this, &FreeBusyCalendar::onRowsChanged);
    }
}

void FreeBusyCalendar::deleteAllEvents()
{
    foreach (const KCalCore::Event::Ptr &event, d->mCalendar->events()) {
        d->mCalendar->deleteEvent(event);
    }
}

void FreeBusyCalendar::onLayoutChanged()
{
    if (!d->mFbEvent.isEmpty()) {
        deleteAllEvents();
        d->mFbEvent.clear();
        for (int i = d->mModel->rowCount() - 1; i >= 0; --i) {
            QModelIndex parent = d->mModel->index(i, 0);
            onRowsInserted(parent, 0, d->mModel->rowCount(parent) - 1);
        }
    }
}

void FreeBusyCalendar::onRowsInserted(const QModelIndex &parent, int first, int last)
{
    if (!parent.isValid()) {
        return;
    }
    for (int i = first; i <= last; ++i) {
        QModelIndex index = d->mModel->index(i, 0, parent);

        const KCalCore::FreeBusyPeriod &period = d->mModel->data(index, FreeBusyItemModel::FreeBusyPeriodRole).value<KCalCore::FreeBusyPeriod>();
        const KCalCore::FreeBusy::Ptr &fb = d->mModel->data(parent, FreeBusyItemModel::FreeBusyRole).value<KCalCore::FreeBusy::Ptr>();

        KCalCore::Event::Ptr inc = KCalCore::Event::Ptr(new KCalCore::Event());
        inc->setDtStart(period.start());
        inc->setDtEnd(period.end());
        inc->setUid(QLatin1String("fb-") + fb->uid() + QLatin1String("-") + QString::number(i));

        inc->setCustomProperty("FREEBUSY", "STATUS", QString::number(period.type()));
        QString summary = period.summary();
        if (summary.isEmpty()) {
            switch (period.type()) {
            case KCalCore::FreeBusyPeriod::Free:
                summary = i18n("Free");
                break;
            case KCalCore::FreeBusyPeriod::Busy:
                summary =  i18n("Busy");
                break;
            case KCalCore::FreeBusyPeriod::BusyUnavailable:
                summary = i18n("Unavailable");
                break;
            case KCalCore::FreeBusyPeriod::BusyTentative:
                summary = i18n("Tentative");
                break;
            default:
                summary = i18n("Unknown");
            }
        }
        inc->setSummary(summary);

        d->mFbEvent.insert(index, inc);
        d->mCalendar->addEvent(inc);
    }
}

void FreeBusyCalendar::onRowsRemoved(const QModelIndex &parent, int first, int last)
{
    if (!parent.isValid()) {
        for (int i = first; i <= last; ++i) {
            QModelIndex index = d->mModel->index(i, 0);
            onRowsRemoved(index, 0, d->mModel->rowCount(index) - 1);
        }
    } else {
        for (int i = first; i <= last; ++i) {
            QModelIndex index = d->mModel->index(i, 0, parent);
            KCalCore::Event::Ptr inc = d->mFbEvent.take(index);
            d->mCalendar->deleteEvent(inc);
        }
    }
}

void FreeBusyCalendar::onRowsChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (!topLeft.parent().isValid()) {
        return;
    }
    for (int i = topLeft.row(); i <= bottomRight.row(); ++i) {
        QModelIndex index = d->mModel->index(i, 0, topLeft.parent());
        KCalCore::Event::Ptr inc = d->mFbEvent.value(index);
        d->mCalendar->beginChange(inc);
        d->mCalendar->endChange(inc);
    }
}
