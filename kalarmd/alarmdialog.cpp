// $Id$

#include <qhbox.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qspinbox.h>

#include <klocale.h>
#include <kprocess.h>
#include <kaudioplayer.h>

#include "event.h"

#include "koeventviewer.h"

#include "alarmdialog.h"
#include "alarmdialog.moc"

AlarmDialog::AlarmDialog(QWidget *parent,const char *name)
  : KDialogBase(parent,name,false,i18n("Alarm"),Ok|User1,Ok,false,
                i18n("Suspend"))
{
  QVBox *topBox = new QVBox(this);
  topBox->setSpacing(spacingHint());
  setMainWidget(topBox);

  (void)new QLabel(i18n("The following events triggered alarms:"),topBox);

  mEvents.setAutoDelete(true);

  mEventViewer = new KOEventViewer(topBox);

  QHBox *suspendBox = new QHBox(topBox);
  suspendBox->setSpacing(spacingHint());

  (void)new QLabel(i18n("Suspend duration (minutes):"),suspendBox);
  mSuspendSpin = new QSpinBox(1,60,1,suspendBox);
  mSuspendSpin->setValue(5);  // default suspend duration

  setMinimumSize(300,200);
}

AlarmDialog::~AlarmDialog()
{
}

void AlarmDialog::appendEvent(const Calendar* calendar, Event *event)
{
  mEventViewer->appendEvent(event);
  mEvents.append(new EventData(calendar, event));
}

int AlarmDialog::clearEvents(const Calendar* calendar)
{
  int remaining = 0;
  mEventViewer->clearEvents();

  if (calendar)
  {
    // Remove all events belonging to the specified calendar,
    // and redisplay remaining events in other calendars
    for (EventData* evdata = mEvents.first();  evdata;  )
    {
      if (evdata->calendar == calendar) {
        mEvents.remove();
        evdata = mEvents.current();
      } else {
        mEventViewer->appendEvent(evdata->event);
        evdata = mEvents.next();
        ++remaining;
      }
    }
  }
  else {
    mEvents.clear();
  }
  return remaining;
}

void AlarmDialog::slotOk()
{
  clearEvents();
  accept();
}

void AlarmDialog::slotUser1()
{
  emit suspendSignal(mSuspendSpin->value());
  accept();
}

void AlarmDialog::eventNotification()
{
  EventData *evdata;

  for (EventData *evdata = mEvents.first();  evdata;  evdata = mEvents.next()) {
    KOAlarm* alarm = evdata->event->alarm();
    if (!alarm->programFile().isEmpty()) {
      KProcess proc;
      proc << alarm->programFile().latin1();
      proc.start(KProcess::DontCare);
    }

    if (!alarm->audioFile().isEmpty()) {
      KAudioPlayer::play(alarm->audioFile().latin1());
    }
  }
}
