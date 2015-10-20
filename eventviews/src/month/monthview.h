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

    explicit MonthView(NavButtonsVisibility visibility = Visible, QWidget *parent = Q_NULLPTR);
    ~MonthView();

    int currentDateCount() const Q_DECL_OVERRIDE;
    int currentMonth() const;

    Akonadi::Item::List selectedIncidences() const Q_DECL_OVERRIDE;

    /** Returns dates of the currently selected events */
    KCalCore::DateList selectedIncidenceDates() const Q_DECL_OVERRIDE;

    QDateTime selectionStart() const Q_DECL_OVERRIDE;

    QDateTime selectionEnd() const Q_DECL_OVERRIDE;

    virtual void setDateRange(const KDateTime &start, const KDateTime &end,
                              const QDate &preferredMonth = QDate()) Q_DECL_OVERRIDE;

    bool eventDurationHint(QDateTime &startDt, QDateTime &endDt, bool &allDay) const Q_DECL_OVERRIDE;

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

    void setCalendar(const Akonadi::ETMCalendar::Ptr &cal) Q_DECL_OVERRIDE;

Q_SIGNALS:
    void showIncidencePopupSignal(const Akonadi::Item &item, const QDate &date);
    void showNewEventPopupSignal();
    void fullViewChanged(bool enabled);

public Q_SLOTS:
    void updateConfig() Q_DECL_OVERRIDE;
    void updateView() Q_DECL_OVERRIDE;
    void showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date) Q_DECL_OVERRIDE;

    void changeIncidenceDisplay(const Akonadi::Item &, int);
    void changeFullView(); /// Display in full window mode
    void moveBackMonth();  /// Shift the view one month back
    void moveBackWeek();   /// Shift the view one week back
    void moveFwdWeek();    /// Shift the view one week forward
    void moveFwdMonth();   /// Shift the view one month forward

protected Q_SLOTS:
    void calendarReset() Q_DECL_OVERRIDE;

private Q_SLOTS:
    // void dataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight );
    // void rowsInserted( const QModelIndex &parent, int start, int end );
    // void rowsAboutToBeRemoved( const QModelIndex &parent, int start, int end );

#ifndef QT_NO_WHEELEVENT
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
#endif
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

    QPair<KDateTime, KDateTime> actualDateRange(
        const KDateTime &start, const KDateTime &end, const QDate &preferredMonth = QDate()) const Q_DECL_OVERRIDE;

    // Compute and update the whole view
    void reloadIncidences();

protected:
    /**
     * @deprecated
     */
    void showDates(const QDate &start, const QDate &end, const QDate &preferedMonth = QDate()) Q_DECL_OVERRIDE;

private:
    MonthViewPrivate *const d;
    friend class MonthViewPrivate;
    friend class MonthScene;
};

}

#endif
