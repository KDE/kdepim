#ifndef INCIDENCERECURRENCE_H
#define INCIDENCERECURRENCE_H

#include "incidenceeditor-ng.h"

namespace Ui {
class EventOrTodoDesktop;
}

namespace IncidenceEditorsNG {

class IncidenceDateTime;

class IncidenceRecurrence : public IncidenceEditor
{
    Q_OBJECT
public:
    IncidenceRecurrence( IncidenceDateTime *dateTime = 0, Ui::EventOrTodoDesktop *ui = 0 );

    virtual void load( KCal::Incidence::ConstPtr incidence );
    virtual void save( KCal::Incidence::Ptr incidence );
    virtual bool isDirty() const;
    virtual bool isValid();

private Q_SLOTS:
    void addException();
    void handleExceptionDateChange( const QDate &currentDate );
    void handleRecurrenceTypeChange( int currentIndex );
    void removeExceptions();
    void updateRemoveExceptionButton();

private:
    void toggleRecurrenceWidgets( bool enable );

private:
    Ui::EventOrTodoDesktop *mUi;
    IncidenceDateTime *mDateTime;
    KCal::DateList mExceptionDates;
};

}

#endif // INCIDENCERECURRENCE_H
