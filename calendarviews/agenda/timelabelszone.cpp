/*
  Copyright (c) 2007 Bruno Virlet <bruno@virlet.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#include "timelabelszone.h"
#include "agenda.h"
#include "agendaview.h"
#include "prefs.h"
#include "timelabels.h"

#include <KSystemTimeZone>

#include <QHBoxLayout>
#include <QScrollArea>
#include <QScrollBar>

using namespace EventViews;

TimeLabelsZone::TimeLabelsZone(QWidget *parent, const PrefsPtr &preferences, Agenda *agenda)
    : QWidget(parent), mAgenda(agenda), mPrefs(preferences),
      mParent(qobject_cast<AgendaView *>(parent))
{
    mTimeLabelsLayout = new QHBoxLayout(this);
    mTimeLabelsLayout->setMargin(0);
    mTimeLabelsLayout->setSpacing(0);

    init();
}

void TimeLabelsZone::reset()
{
    foreach (QScrollArea *label, mTimeLabelsList) {
        label->hide();
        label->deleteLater();
    }
    mTimeLabelsList.clear();

    init();

    // Update some related geometry from the agenda view
    updateAll();
    if (mParent) {
        mParent->updateTimeBarWidth();
        mParent->createDayLabels(true);
    }
}

void TimeLabelsZone::init()
{
    QStringList seenTimeZones(mPrefs->timeSpec().timeZone().name());

    addTimeLabels(mPrefs->timeSpec());

    foreach (const QString &zoneStr, mPrefs->timeScaleTimezones()) {
        if (!seenTimeZones.contains(zoneStr)) {
            KTimeZone zone = KSystemTimeZones::zone(zoneStr);
            if (zone.isValid()) {
                addTimeLabels(zone);
                seenTimeZones += zoneStr;
            }
        }
    }
}

void TimeLabelsZone::addTimeLabels(const KDateTime::Spec &spec)
{
    QScrollArea *area = new QScrollArea(this);
    TimeLabels *labels = new TimeLabels(spec, 24, this);
    mTimeLabelsList.prepend(area);
    area->setWidgetResizable(true);
    area->setWidget(labels);
    area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    area->setBackgroundRole(QPalette::Window);
    area->setFrameStyle(QFrame::NoFrame);
    area->show();
    mTimeLabelsLayout->insertWidget(0, area);

    setupTimeLabel(area);
}

void TimeLabelsZone::setupTimeLabel(QScrollArea *area)
{
    if (mAgenda && mAgenda->verticalScrollBar()) {
        // Scrolling the agenda will scroll the timelabel
        connect(mAgenda->verticalScrollBar(), SIGNAL(valueChanged(int)),
                area->verticalScrollBar(), SLOT(setValue(int)));
        // and vice-versa. ( this won't loop )
        connect(area->verticalScrollBar(), SIGNAL(valueChanged(int)),
                mAgenda->verticalScrollBar(), SLOT(setValue(int)));

        area->verticalScrollBar()->setValue(mAgenda->verticalScrollBar()->value());

    }

    TimeLabels *timeLabels = static_cast<TimeLabels *>(area->widget());
    timeLabels->setAgenda(mAgenda);

    // timelabel's scroll is just a slave, this shouldn't be here
    // if ( mParent ) {
    //  connect( area->verticalScrollBar(), SIGNAL(valueChanged(int)),
    //           mParent, SLOT(setContentsPos(int)) );
    // }
}

int TimeLabelsZone::preferedTimeLabelsWidth() const
{
    if (mTimeLabelsList.isEmpty()) {
        return 0;
    } else {
        return mTimeLabelsList.first()->widget()->sizeHint().width();
    }
}

void TimeLabelsZone::updateAll()
{
    foreach (QScrollArea *area, mTimeLabelsList) {
        TimeLabels *timeLabel = static_cast<TimeLabels *>(area->widget());
        timeLabel->updateConfig();
    }
}

QList<QScrollArea *> TimeLabelsZone::timeLabels() const
{
    return mTimeLabelsList;
}

void TimeLabelsZone::setAgendaView(AgendaView *agendaView)
{
    mParent = agendaView;
    mAgenda = agendaView ? agendaView->agenda() : 0;

    foreach (QScrollArea *timeLabel, mTimeLabelsList) {
        setupTimeLabel(timeLabel);
    }
}

void TimeLabelsZone::updateTimeLabelsPosition()
{
    if (mAgenda) {
        foreach (QScrollArea *area, timeLabels()) {
            TimeLabels *label = static_cast<TimeLabels *>(area->widget());
            const int adjustment = mAgenda->contentsY();
            // y() is the offset to our parent (QScrollArea)
            // and gets negative as we scroll
            if (adjustment != -label->y()) {
                area->verticalScrollBar()->setValue(adjustment);
            }
        }
    }
}

PrefsPtr TimeLabelsZone::preferences() const
{
    return mPrefs;
}

void TimeLabelsZone::setPreferences(const PrefsPtr &prefs)
{
    if (prefs != mPrefs) {
        mPrefs = prefs;
    }
}

