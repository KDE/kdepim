/*
  This file is part of KOrganizer.

  Copyright (c) 2007 Bruno Virlet <bruno.virlet@gmail.com>

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

#ifndef CALENDARVIEWS_TIMESPENTVIEW_H
#define CALENDARVIEWS_TIMESPENTVIEW_H

#include "eventview.h"
#include <Akonadi/Calendar/IncidenceChanger>

namespace EventViews
{

class TimeSpentWidget;

/**
  This view show the time spent on each category.
*/
class EVENTVIEWS_EXPORT TimeSpentView : public EventViews::EventView
{
    Q_OBJECT
public:
    explicit TimeSpentView(QWidget *parent = Q_NULLPTR);
    ~TimeSpentView();

    int currentDateCount() const Q_DECL_OVERRIDE;

    Akonadi::Item::List selectedIncidences() const
    {
        return Akonadi::Item::List();
    }

    KCalCore::DateList selectedIncidenceDates() const
    {
        return KCalCore::DateList();
    }

public Q_SLOTS:
    virtual void updateView();
    virtual void showDates(const QDate &start, const QDate &end,
                           const QDate &preferredMonth = QDate());
    virtual void showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date);

    void changeIncidenceDisplay(const Akonadi::Item &, Akonadi::IncidenceChanger::ChangeType);

private:
    TimeSpentWidget *mView;
    QDate mStartDate;
    QDate mEndDate;

    friend class EventViews::TimeSpentWidget;
};

}

#endif
