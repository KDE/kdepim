#ifndef ALARMDIALOG_H
#define ALARMDIALOG_H
// $Id$
//
// Alarm dialog.
//

#include <kdialogbase.h>

#include "event.h"
#include "calendarlocal.h"

using namespace KCal;

class KOEventViewer;
class QSpinBox;

struct EventData
{
    EventData(const Calendar* cal, Event* ev)  : calendar(cal), event(ev) { }
    const Calendar* calendar;
    Event*          event;
};

class AlarmDialog : public KDialogBase {
    Q_OBJECT
  public:
    AlarmDialog(QWidget *parent=0L, const char *name=0L);
    virtual ~AlarmDialog();

    void appendEvent(const Calendar*, Event *event);

    void eventNotification();

    int  clearEvents(const Calendar* = 0L);

  public slots:
    void slotOk();
    void slotUser1();

  signals:
    void suspendSignal(int duration);

  private:
    KOEventViewer *mEventViewer;

    QList<EventData> mEvents;

    QSpinBox *mSuspendSpin;
};

#endif
