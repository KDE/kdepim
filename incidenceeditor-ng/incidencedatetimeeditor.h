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

#include "incidenceeditor-ng.h"

#include <KCal/Event>
#include <KCal/Todo>

namespace KCal {
class ICalTimeZones;
}

namespace Ui {
class IncidenceDateTimeEditor;
}

namespace IncidenceEditorsNG {

class IncidenceDateTimeEditor : public IncidenceEditor
{
  Q_OBJECT
  public:
    IncidenceDateTimeEditor( QWidget *parent = 0 );
    ~IncidenceDateTimeEditor();

    virtual void load( KCal::Incidence::ConstPtr incidence );
    virtual void save( KCal::Incidence::Ptr incidence );
    virtual bool isDirty() const;

  private slots: /// General
    void editRecurrence();
    void enableAlarm( bool enable );
    void updateRecurrenceSummary( KCal::Incidence::ConstPtr incidence );
    
  private slots: /// Todo specific
    void enableStartEdit( bool enable );
    void enableEndEdit( bool enable );
    void enableTimeEdits( bool enable );
    bool isDirty( KCal::Todo::ConstPtr todo ) const;
//     void slotTodoDateChanged();
    void slotTodoStartDateModified();

  private slots: /// Event specific
    
  private:
    void load( KCal::Event::ConstPtr event );
    void load( KCal::Todo::ConstPtr todo );
    
  private:
    KCal::ICalTimeZones *mTimeZones;
    Ui::IncidenceDateTimeEditor *mUi;

    bool mStartDateModified;
    KDateTime::Spec mStartSpec;
    KDateTime::Spec mEndSpec;
};

} // IncidenceEditorsNG

#endif // INCIDENCEDATETIMEEDITOR_H
