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

#include <KCalCore/Event>
#include <KCalCore/FreeBusyPeriod>
#include <KCalCore/MemoryCalendar>
#include <kcalcore/calformat.h>

#include <QStringList>
#include <klocalizedstring.h>
#include <KSystemTimeZones>

#include <KDebug>

FreeBusyCalendar::FreeBusyCalendar(QObject *parent)
: QObject(parent)
, mModel(0)
{
    mCalendar = KCalCore::Calendar::Ptr(new KCalCore::MemoryCalendar(KSystemTimeZones::local()));
    kDebug() << "creating" << this;
}

FreeBusyCalendar::~FreeBusyCalendar()
{
    kDebug() << "deleting" << this;
}


KCalCore::Calendar::Ptr FreeBusyCalendar::calendar() const
{
    return mCalendar;
}

FreeBusyItemModel *FreeBusyCalendar::model() const
{
    return mModel;
}

void FreeBusyCalendar::setModel(FreeBusyItemModel *model)
{
    if (model != mModel) {
        if (mModel) {
            disconnect(mModel, 0, 0, 0);
        }
        mModel = model;
        connect(mModel, SIGNAL(layoutChanged()), SLOT(onLayoutChanged()));
        connect(mModel, SIGNAL(modelReset()), SLOT(onLayoutChanged()));
        connect(mModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                SLOT(onRowsRemoved(QModelIndex,int,int)));
        connect(mModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
                SLOT(onRowsInserted(QModelIndex,int,int)));
        connect(mModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                SLOT(onRowsChanged(QModelIndex,QModelIndex)));
    }
}

void FreeBusyCalendar::onLayoutChanged()
{
    if (mFbEvent.count() > 0) {
        mCalendar->deleteAllEvents();
        mFbEvent.clear();
        for (int i = mModel->rowCount()-1; i>=0; i--) {
            QModelIndex parent = mModel->index(i, 0);
            onRowsInserted(parent, 0, mModel->rowCount(parent)-1);
        }
    }
}

void FreeBusyCalendar::onRowsInserted(const QModelIndex &parent, int first, int last)
{
    if (!parent.isValid()) {
        return;
    }
    for(int i=first; i<=last; i++) {
        QModelIndex index = mModel->index(i, 0, parent);

        const KCalCore::FreeBusyPeriod &period = mModel->data(index, FreeBusyItemModel::FreeBusyPeriodRole).value<KCalCore::FreeBusyPeriod>();
        const KCalCore::Attendee::Ptr &attendee = mModel->data(parent, FreeBusyItemModel::AttendeeRole).value<KCalCore::Attendee::Ptr>();
        const KCalCore::FreeBusy::Ptr &fb = mModel->data(parent, FreeBusyItemModel::FreeBusyRole).value<KCalCore::FreeBusy::Ptr>();

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

        mFbEvent.insert(index, inc);
        mCalendar->addEvent(inc);
    }
}

void FreeBusyCalendar::onRowsRemoved(const QModelIndex &parent, int first, int last)
{
    if (!parent.isValid()) {
        for (int i = first; i<=last; i--) {
            QModelIndex index = mModel->index(i, 0);
            onRowsRemoved(index, 0, mModel->rowCount(index)-1);
        }
    } else {
        for(int i=first; i<=last; i++) {
            QModelIndex index = mModel->index(i, 0, parent);
            KCalCore::Event::Ptr inc = mFbEvent.take(index);
            mCalendar->deleteEvent(inc);
        }
    }
}

void FreeBusyCalendar::onRowsChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (!topLeft.parent().isValid()) {
        return;
    }
    for (int i = topLeft.row(); i <= bottomRight.row(); i++) {
        QModelIndex index = mModel->index(i, 0, topLeft.parent());
        KCalCore::Event::Ptr inc = mFbEvent.value(index);
        mCalendar->beginChange(inc);
        mCalendar->endChange(inc);
    }
}