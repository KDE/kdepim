/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
    Copyright (C) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef INCIDENCEDATETIMEEDITOR_H
#define INCIDENCEDATETIMEEDITOR_H

#include "combinedincidenceeditor.h"

#include <KCalCore/Event>
#include <KCalCore/Todo>

namespace KCalCore {
class ICalTimeZones;
}

namespace Ui {
class EventOrTodoDesktop;
}

namespace IncidenceEditorsNG {


class INCIDENCEEDITORS_NG_EXPORT IncidenceDateTime : public IncidenceEditor
{
  Q_OBJECT
  public:
    IncidenceDateTime( Ui::EventOrTodoDesktop *ui );
    ~IncidenceDateTime();

    virtual void load( const KCalCore::Incidence::Ptr &incidence );
    virtual void save( const KCalCore::Incidence::Ptr &incidence );
    virtual bool isDirty() const;

    /**
     * Sets the active date for the editing session. This defaults to the current
     * date. It should be set <em>before</em> loading a non-empty (i.e. existing
     * incidence).
     */
    void setActiveDate( const QDate &activeDate );

    QDate startDate() const; /// Returns the current start date.
    QTime startTime() const; /// Returns the current start time.
    QDate endDate() const; /// Returns the current end date.
    QTime endTime() const; /// Returns the current endtime.

    /// Created from the values in the widgets
    KDateTime currentStartDateTime() const;
    KDateTime currentEndDateTime() const;

    void setStartTime( const QTime &newTime );
    void setStartDate( const QDate &newDate );

    bool startDateTimeEnabled() const;
    bool endDateTimeEnabled() const;

    bool isValid();

  signals:
    void startDateTimeToggled( bool enabled );
    void startDateChanged( const QDate &newDate );
    void startTimeChanged( const QTime &newTime );
    void endDateTimeToggled( bool enabled );
    void endDateChanged( const QDate &newDate );
    void endTimeChanged( const QTime &newTime );

  private slots: /// General
    void setTimeZonesVisibility( bool visible );
    void toggleTimeZoneVisibility();
    void updateStartTime( const QTime &newTime );
    void updateStartDate( const QDate &newDate );
    void updateStartSpec();

  private slots: /// Todo specific
    void enableStartEdit( bool enable );
    void enableEndEdit( bool enable );
    void enableTimeEdits();
    bool isDirty( const KCalCore::Todo::Ptr &todo ) const;

  private slots: /// Event specific
    bool isDirty( const KCalCore::Event::Ptr &event ) const;

  private:
    void load( const KCalCore::Event::Ptr &event );
    void load( const KCalCore::Todo::Ptr &todo );
    void save( const KCalCore::Event::Ptr &event );
    void save( const KCalCore::Todo::Ptr &todo );
    void setDateTimes( const KDateTime &start, const KDateTime &end );
    void setTimes( const KDateTime &start, const KDateTime &end );

  private:
    KCalCore::ICalTimeZones *mTimeZones;
    Ui::EventOrTodoDesktop *mUi;

    QDate mActiveDate;
    /**
     * These might differ from mLoadedIncidence->(dtStart|dtDue) as these take
     * in account recurrence if needed. The values are calculated once on load().
     * and don't change afterwards.
     */
    KDateTime mInitialStartDT;
    KDateTime mInitialEndDT;

    /**
     * We need to store the current start date/time to be able to update the end
     * time appropriate when the start time changes.
     */
    KDateTime mCurrentStartDateTime;

    /// Remembers state when switching between takes whole day and timed event/to-do.
    bool mTimezoneCombosWhereVisibile;
};

} // IncidenceEditorsNG

#endif // INCIDENCEDATETIMEEDITOR_H
