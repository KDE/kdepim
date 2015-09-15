/*
  This file is part of KOrganizer.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

// View of Journal entries

#include "journalview.h"
#include "journalframe.h"

#include <CalendarSupport/Utils>

#include <QVBoxLayout>
#include "calendarview_debug.h"
#include <QEvent>
#include <QScrollArea>

using namespace EventViews;

JournalView::JournalView(QWidget *parent) : EventView(parent), mChanger(Q_NULLPTR)
{
    QVBoxLayout *topLayout = new QVBoxLayout(this);
    topLayout->setMargin(0);
    mSA = new QScrollArea(this);
    mCurrentWidget = new QWidget(mSA->viewport());
    QVBoxLayout *mVBoxVBoxLayout = new QVBoxLayout(mCurrentWidget);
    mVBoxVBoxLayout->setMargin(0);
    mSA->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mSA->setWidgetResizable(true);
    mSA->setWidget(mCurrentWidget);
    topLayout->addWidget(mSA);

    installEventFilter(this);
}

JournalView::~JournalView()
{
}

void JournalView::appendJournal(const Akonadi::Item &journal, const QDate &dt)
{
    JournalDateView *entry = Q_NULLPTR;
    if (mEntries.contains(dt)) {
        entry = mEntries[dt];
    } else {
        entry = new JournalDateView(calendar(), mCurrentWidget);
        mCurrentWidget->layout()->addWidget(entry);
        entry->setDate(dt);
        entry->setIncidenceChanger(mChanger);
        entry->show();
        connect(this, &JournalView::flushEntries, entry, &JournalDateView::flushEntries);
        connect(this, &JournalView::setIncidenceChangerSignal,
                entry, &JournalDateView::setIncidenceChanger);
        connect(this, &JournalView::journalEdited,
                entry, &JournalDateView::journalEdited);
        connect(this, &JournalView::journalDeleted,
                entry, &JournalDateView::journalDeleted);

        connect(entry, &JournalDateView::editIncidence,
                this, &EventView::editIncidenceSignal);
        connect(entry, &JournalDateView::deleteIncidence,
                this, &EventView::deleteIncidenceSignal);
        connect(entry, &JournalDateView::newJournal,
                this, &EventView::newJournalSignal);
        connect(entry, &JournalDateView::incidenceSelected,
                this, &EventView::incidenceSelected);
        connect(entry, &JournalDateView::printJournal,
                this, &JournalView::printJournal);
        mEntries.insert(dt, entry);
    }

    if (entry && CalendarSupport::hasJournal(journal)) {
        entry->addJournal(journal);
    }
}

int JournalView::currentDateCount() const
{
    return mEntries.size();
}

Akonadi::Item::List JournalView::selectedIncidences() const
{
    // We don't have a selection in the journal view.
    // FIXME: The currently edited journal is the selected incidence...
    Akonadi::Item::List eventList;
    return eventList;
}

void JournalView::clearEntries()
{
    //qDebug() << "JournalView::clearEntries()";
    QMap<QDate, JournalDateView *>::Iterator it;
    for (it = mEntries.begin(); it != mEntries.end(); ++it) {
        delete it.value();
    }
    mEntries.clear();
}
void JournalView::updateView()
{
    QMap<QDate, JournalDateView *>::Iterator it = mEntries.end();
    while (it != mEntries.begin()) {
        --it;
        it.value()->clear();
        const KCalCore::Journal::List journals = calendar()->journals(it.key());
        qCDebug(CALENDARVIEW_LOG) << "updateview found" << journals.count();
        Q_FOREACH (const KCalCore::Journal::Ptr &journal, journals) {
            Akonadi::Item item = calendar()->item(journal);
            it.value()->addJournal(item);
        }
    }
}

void JournalView::flushView()
{
    Q_EMIT flushEntries();
}

void JournalView::showDates(const QDate &start, const QDate &end, const QDate &)
{
    clearEntries();
    if (end < start) {
        qCWarning(CALENDARVIEW_LOG) << "End is smaller than start. end=" << end << "; start=" << start;
        return;
    }

    KCalCore::Journal::List jnls;
    for (QDate d = end; d >= start; d = d.addDays(-1)) {
        jnls = calendar()->journals(d);
        //qCDebug(CALENDARVIEW_LOG) << "Found" << jnls.count() << "journals on date" << d;
        foreach (const KCalCore::Journal::Ptr &journal, jnls) {
            Akonadi::Item item = calendar()->item(journal);
            appendJournal(item, d);
        }
        if (jnls.isEmpty()) {
            // create an empty dateentry widget
            //updateView();
            //qCDebug(CALENDARVIEW_LOG) << "Appended null journal";
            appendJournal(Akonadi::Item(), d);
        }
    }
}

void JournalView::showIncidences(const Akonadi::Item::List &incidences, const QDate &date)
{
    Q_UNUSED(date);
    clearEntries();
    Q_FOREACH (const Akonadi::Item &i, incidences) {
        if (const KCalCore::Journal::Ptr j = CalendarSupport::journal(i)) {
            appendJournal(i, j->dtStart().date());
        }
    }
}

void JournalView::changeIncidenceDisplay(const Akonadi::Item &incidence,
        Akonadi::IncidenceChanger::ChangeType changeType)
{
    if (KCalCore::Journal::Ptr journal = CalendarSupport::journal(incidence)) {
        switch (changeType) {
        case Akonadi::IncidenceChanger::ChangeTypeCreate:
            appendJournal(incidence, journal->dtStart().date());
            break;
        case Akonadi::IncidenceChanger::ChangeTypeModify:
            Q_EMIT journalEdited(incidence);
            break;
        case Akonadi::IncidenceChanger::ChangeTypeDelete:
            Q_EMIT journalDeleted(incidence);
            break;
        default:
            qCWarning(CALENDARVIEW_LOG) << "Illegal change type" << changeType;
        }
    }
}

void JournalView::setIncidenceChanger(Akonadi::IncidenceChanger *changer)
{
    mChanger = changer;
    Q_EMIT setIncidenceChangerSignal(changer);
}

void JournalView::newJournal()
{
    Q_EMIT newJournalSignal(QDate::currentDate());
}

bool JournalView::eventFilter(QObject *object, QEvent *event)
{
    Q_UNUSED(object);
    switch (event->type()) {
    case QEvent::MouseButtonDblClick:
        Q_EMIT newJournalSignal(QDate());
        return true;
    default:
        return false;
    }
}

