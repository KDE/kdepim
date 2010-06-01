/*
    Copyright (C) 2010  Bertjan Broeksema b.broeksema@home.nl

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#ifndef INCIDENCEDATETIMEEDITOR_H
#define INCIDENCEDATETIMEEDITOR_H

#include "combinedincidenceeditor.h"

#include <KCal/Event>
#include <KCal/Todo>

namespace KCal {
class ICalTimeZones;
}

namespace Ui {
class IncidenceDateTimeEditor;
}

namespace IncidenceEditorsNG {

class IncidenceRecurrenceDialog;

class INCIDENCEEDITORS_NG_EXPORT IncidenceDateTimeEditor : public CombinedIncidenceEditor
{
  Q_OBJECT
  public:
    IncidenceDateTimeEditor( QWidget *parent = 0 );
    ~IncidenceDateTimeEditor();

    virtual void load( KCal::Incidence::ConstPtr incidence );
    virtual void save( KCal::Incidence::Ptr incidence );
    virtual bool isDirty() const;

    /**
     * Sets the active date for the editing session. This defaults to the current
     * date. It should be set <em>before</em> loading a non-empty (i.e. existing
     * incidence).
     */
    void setActiveDate( const QDate &activeDate );

  private slots: /// General
    void editRecurrence();
    void enableAlarm( bool enable );
    void setDuration();
    void setTimeZonesVisibility( bool visible );
    void toggleTimeZoneVisibility();
    void startTimeChanged( const QTime &newTime );
    void startDateChanged( const QDate &newDate );
    void startSpecChanged();
    void updateRecurrence();
    void updateRecurrenceSummary( KCal::Incidence::ConstPtr incidence );

  private slots: /// Todo specific
    void enableStartEdit( bool enable );
    void enableEndEdit( bool enable );
    void enableTimeEdits( bool enable );
    bool isDirty( KCal::Todo::ConstPtr todo ) const;

  private slots: /// Event specific
    bool isDirty( KCal::Event::ConstPtr event ) const;

  private:
    /// Created from the values in the widgets
    KDateTime currentStartDateTime() const; 
    KDateTime currentEndDateTime() const;

    void load( KCal::Event::ConstPtr event );
    void load( KCal::Todo::ConstPtr todo );
    void save( KCal::Event::Ptr event );
    void save( KCal::Todo::Ptr todo );
    void setDateTimes( const KDateTime &start, const KDateTime &end );
    void setTimes( const KDateTime &start, const KDateTime &end );

  private:
    KCal::ICalTimeZones *mTimeZones;
    Ui::IncidenceDateTimeEditor *mUi;

#ifndef KDEPIM_MOBILE_UI
    IncidenceRecurrenceDialog *mRecurrenceDialog;
#endif

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
};

} // IncidenceEditorsNG

#endif // INCIDENCEDATETIMEEDITOR_H
