#ifndef INCIDENCEALARM_H
#define INCIDENCEALARM_H

#include "incidenceeditor-ng.h"

namespace Ui {
class EventOrTodoDesktop;
}

namespace IncidenceEditorsNG {

class IncidenceAlarm : public IncidenceEditor
{
  Q_OBJECT
  public:
    IncidenceAlarm( Ui::EventOrTodoDesktop *ui = 0 );

    virtual void load( KCal::Incidence::ConstPtr incidence );
    virtual void save( KCal::Incidence::Ptr incidence );
    virtual bool isDirty() const;

  private Q_SLOTS:
    void editCurrentAlarm();
    void newAlarm();
    void newAlarmFromPreset();
    void removeCurrentAlarm();
    void updateAlarmList();
    void updateButtons();

  private:
    QString stringForAlarm( KCal::Alarm *alarm );

  private:
    Ui::EventOrTodoDesktop *mUi;

    KCal::Alarm::List mDisabledAlarms;
    KCal::Alarm::List mEnabledAlarms;
};

}

#endif // INCIDENCEALARM_H
