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
#ifndef ALARMDIALOG_H
#define ALARMDIALOG_H
// $Id$
//
// Alarm dialog.
//

#include "compat.h"

#include <kdialogbase.h>

#include <libkcal/event.h>
#include <libkcal/calendarlocal.h>

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

    QPtrList<EventData> mEvents;

    QSpinBox *mSuspendSpin;
};

#endif
