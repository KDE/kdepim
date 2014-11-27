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
#ifndef CALENDARVIEWS_JOURNALVIEW_H
#define CALENDARVIEWS_JOURNALVIEW_H

#include "eventview.h"
#include <Akonadi/Calendar/IncidenceChanger>
#include <KCalCore/Journal>

class QScrollArea;

/**
 * This class provides a journal view.

 * @short View for Journal components.
 * @author Cornelius Schumacher <schumacher@kde.org>, Reinhold Kainhofer <reinhold@kainhofer.com>
 * @see KOBaseView
 */
namespace EventViews
{

class JournalDateView;

class EVENTVIEWS_EXPORT JournalView : public EventView
{
    Q_OBJECT
public:
    explicit JournalView(QWidget *parent = 0);
    ~JournalView();

    /**reimp*/ int currentDateCount() const Q_DECL_OVERRIDE;
    /**reimp*/ Akonadi::Item::List selectedIncidences() const Q_DECL_OVERRIDE;
    KCalCore::DateList selectedIncidenceDates() const
    {
        return KCalCore::DateList();
    }

    void appendJournal(const Akonadi::Item &journal, const QDate &dt);

    /** documentation in baseview.h */
    void getHighlightMode(bool &highlightEvents,
                          bool &highlightTodos,
                          bool &highlightJournals);

    bool eventFilter(QObject *, QEvent *);

public Q_SLOTS:
    // Don't update the view when midnight passed, otherwise we'll have data loss (bug 79145)
    virtual void dayPassed(const QDate &) {}
    void updateView() Q_DECL_OVERRIDE;
    void flushView() Q_DECL_OVERRIDE;

    void showDates(const QDate &start, const QDate &end, const QDate &preferredMonth = QDate()) Q_DECL_OVERRIDE;
    void showIncidences(const Akonadi::Item::List &incidences, const QDate &date) Q_DECL_OVERRIDE;

    void changeIncidenceDisplay(const Akonadi::Item &incidence,
                                Akonadi::IncidenceChanger::ChangeType);
    void setIncidenceChanger(Akonadi::IncidenceChanger *changer) Q_DECL_OVERRIDE;
    void newJournal();
Q_SIGNALS:
    void flushEntries();
    void setIncidenceChangerSignal(Akonadi::IncidenceChanger *);
    void journalEdited(const Akonadi::Item &journal);
    void journalDeleted(const Akonadi::Item &journal);
    void printJournal(const KCalCore::Journal::Ptr &, bool preview);

protected:
    void clearEntries();

private:
    QScrollArea *mSA;
    QWidget *mVBox;
    QMap<QDate, EventViews::JournalDateView *> mEntries;
    Akonadi::IncidenceChanger *mChanger;
//    DateList mSelectedDates;  // List of dates to be displayed
};

}

#endif
