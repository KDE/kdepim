/*
  Copyright (c) 1999 Preston Brown <pbrown@kde.org>
  Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
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
#ifndef EVENTVIEWS_LISTVIEW_H
#define EVENTVIEWS_LISTVIEW_H

#include "eventview.h"

class KConfig;

class QModelIndex;

/**
  This class provides a multi-column list view of events.  It can
  display events from one particular day or several days, it doesn't
  matter.  To use a view that only handles one day at a time, use
  KODayListView.

  @short multi-column list view of various events.
  @author Preston Brown <pbrown@kde.org>
  @see KOBaseView, KODayListView
*/

namespace EventViews
{

class EVENTVIEWS_EXPORT ListView : public EventView
{
    Q_OBJECT
public:
    explicit ListView(const Akonadi::ETMCalendar::Ptr &calendar,
                      QWidget *parent = 0, bool nonInteractive = false);
    ~ListView();

    virtual int currentDateCount() const;
    virtual Akonadi::Item::List selectedIncidences() const;
    virtual KCalCore::DateList selectedIncidenceDates() const;

    // Shows all incidences of the calendar
    void showAll();

    void readSettings(KConfig *config);
    void writeSettings(KConfig *config);

    void clear();
    QSize sizeHint() const Q_DECL_OVERRIDE;

public slots:
    virtual void updateView();

    virtual void showDates(const QDate &start, const QDate &end,
                           const QDate &preferredMonth = QDate());

    virtual void showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date);

    void clearSelection();

    void changeIncidenceDisplay(const Akonadi::Item &, int);

    void defaultItemAction(const QModelIndex &);
    void defaultItemAction(const Akonadi::Item::Id id);

    void popupMenu(const QPoint &);

Q_SIGNALS:
    void showNewEventPopupSignal();
    void showIncidencePopupSignal(const Akonadi::Item &, const QDate &);

protected slots:
    void processSelectionChange();

private:
    class Private;
    Private *const d;
};

}

#endif
