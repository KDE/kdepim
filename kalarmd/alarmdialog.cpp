/*
    This file is part of the KDE alarm daemon.
    Copyright (c) 2000 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// $Id$

#include <qhbox.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qspinbox.h>

#include <klocale.h>
#include <kprocess.h>
#include <kaudioplayer.h>

#include <libkcal/event.h>

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
    Alarm* alarm = evdata->event->alarm();
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
