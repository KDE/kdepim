/*
  Copyright (c) 2008 Bruno Virlet <bruno.virlet@gmail.com>
  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Author: Bertjan Broeksema, broeksema@kde.org

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

#ifndef EVENTVIEWS_MONTHVIEW_H
#define EVENTVIEWS_MONTHVIEW_H

#include "eventview.h"

#include <KDateTime>

class QModelIndex;

namespace EventViews
{

class MonthViewPrivate;

/**
  New month view.
*/
class EVENTVIEWS_EXPORT MonthView : public EventView
{
    Q_OBJECT
public:
    enum NavButtonsVisibility {
        Visible,
        Hidden
    };

    explicit MonthView(NavButtonsVisibility visibility = Visible, QWidget *parent = 0);
    ~MonthView();

    virtual int currentDateCount() const;
    int currentMonth() const;

    virtual Akonadi::Item::List selectedIncidences() const;

    /** Returns dates of the currently selected events */
    virtual KCalCore::DateList selectedIncidenceDates() const;

    virtual QDateTime selectionStart() const;

    virtual QDateTime selectionEnd() const;

    virtual void setDateRange(const KDateTime &start, const KDateTime &end,
                              const QDate &preferredMonth = QDate());

    virtual bool eventDurationHint(QDateTime &startDt, QDateTime &endDt, bool &allDay) const;

    /**
     * Returns the average date in the view
     */
    QDate averageDate() const;

    bool usesFullWindow();

    bool supportsDateRangeSelection()
    {
        return false;
    }

    bool isBusyDay(const QDate &day) const;

    void setCalendar(const Akonadi::ETMCalendar::Ptr &cal);

Q_SIGNALS:
    void showIncidencePopupSignal(const Akonadi::Item &item, const QDate &date);
    void showNewEventPopupSignal();
    void fullViewChanged(bool enabled);

public Q_SLOTS:
    // virtual slots
    virtual void updateConfig();
    virtual void updateView();
    virtual void showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date);

    // non-virtual slots
    void changeIncidenceDisplay(const Akonadi::Item &, int);
    void changeFullView(); /// Display in full window mode
    void moveBackMonth();  /// Shift the view one month back
    void moveBackWeek();   /// Shift the view one week back
    void moveFwdWeek();    /// Shift the view one week forward
    void moveFwdMonth();   /// Shift the view one month forward

protected Q_SLOTS:
    virtual void calendarReset();

private Q_SLOTS:
    // void dataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight );
    // void rowsInserted( const QModelIndex &parent, int start, int end );
    // void rowsAboutToBeRemoved( const QModelIndex &parent, int start, int end );

#ifndef QT_NO_WHEELEVENT
    virtual void wheelEvent(QWheelEvent *event);
#endif
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);

    ///* reimp */void incidencesAdded( const Akonadi::Item::List &incidences );
    ///* reimp */void incidencesAboutToBeRemoved( const Akonadi::Item::List &incidences );
    ///* reimp */void incidencesChanged( const Akonadi::Item::List &incidences );
    /* reimp */QPair<KDateTime, KDateTime> actualDateRange(
        const KDateTime &start, const KDateTime &end, const QDate &preferredMonth = QDate()) const;

    // Compute and update the whole view
    void reloadIncidences();

protected:
    /**
     * @deprecated
     */
    void showDates(const QDate &start, const QDate &end, const QDate &preferedMonth = QDate());

private:
    MonthViewPrivate *const d;
    friend class MonthViewPrivate;
    friend class MonthScene;
};

}

#endif
