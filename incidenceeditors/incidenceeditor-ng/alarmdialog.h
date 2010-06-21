#ifndef ALARMDIALOG_H
#define ALARMDIALOG_H

#include <KDialog>

namespace KCal {
class Alarm;
}

namespace Ui {
class AlarmDialog;
}

namespace IncidenceEditorsNG {

class AlarmDialog : public KDialog
{
  public:
    AlarmDialog();

    void load( KCal::Alarm *alarm );
    void save( KCal::Alarm *alarm ) const;

  private:
    Ui::AlarmDialog *mUi;
};

}

#endif // ALARMDIALOG_H
