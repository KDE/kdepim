#ifndef _KPILOT_PILOTDATEENTRY_H
#define _KPILOT_PILOTDATEENTRY_H
/* pilotDateEntry.h	-*- C++ -*-	KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** See the .cc file for an explanation of what this file is for.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <time.h>
#include <string.h>

#include <qbitarray.h>

#include <pi-macros.h>
#include <pi-datebook.h>

#include "pilotAppCategory.h"
#include "pilotRecord.h"



class PilotDateEntry : public PilotAppCategory
{
public:
  PilotDateEntry(struct AppointmentAppInfo &appInfo);
  PilotDateEntry(struct AppointmentAppInfo &appInfo, PilotRecord* rec);
  ~PilotDateEntry() { free_Appointment(&fAppointmentInfo); }

  PilotDateEntry(const PilotDateEntry &e);

  PilotDateEntry& operator=(const PilotDateEntry &e);
	virtual QString getTextRepresentation(bool richText=false);

  PilotRecord* pack() { return PilotAppCategory::pack(); }

  bool isEvent() const { return fAppointmentInfo.event; }
  int getEvent() const { return fAppointmentInfo.event; }
  void setEvent(int event) { fAppointmentInfo.event = event; }

  struct tm getEventStart() const { return fAppointmentInfo.begin; }
  const struct tm *getEventStart_p() const { return &fAppointmentInfo.begin; }
  void setEventStart(struct tm& start) { fAppointmentInfo.begin = start; }

  struct tm getEventEnd() const { return fAppointmentInfo.end; }
  const struct tm *getEventEnd_p() const { return &fAppointmentInfo.end; }
  void setEventEnd(struct tm& end) { fAppointmentInfo.end = end; }

  int getAlarm() const { return fAppointmentInfo.alarm; }
  void setAlarm(int alarm) { fAppointmentInfo.alarm = alarm; }

  int getAdvance() const { return fAppointmentInfo.advance; }
  void setAdvance(int advance) { fAppointmentInfo.advance = advance; }

  int getAdvanceUnits() const { return fAppointmentInfo.advanceUnits; }
  void setAdvanceUnits(int units) { fAppointmentInfo.advanceUnits = units; }

  // The following need set routines written
  repeatTypes getRepeatType() const { return fAppointmentInfo.repeatType; }
  void setRepeatType(repeatTypes r) { fAppointmentInfo.repeatType = r; }

  int getRepeatForever() const { return fAppointmentInfo.repeatForever; }
  void setRepeatForever(int f = 1) { fAppointmentInfo.repeatForever = f; }

  struct tm getRepeatEnd() const { return fAppointmentInfo.repeatEnd; }
  void setRepeatEnd(struct tm tm) { fAppointmentInfo.repeatEnd = tm; }

  int getRepeatFrequency() const { return fAppointmentInfo.repeatFrequency; }
  void setRepeatFrequency(int f) { fAppointmentInfo.repeatFrequency = f; }

  DayOfMonthType getRepeatDay() const { return fAppointmentInfo.repeatDay; }
  void setRepeatDay(DayOfMonthType rd) { fAppointmentInfo.repeatDay = rd; };

  const int *getRepeatDays() const { return fAppointmentInfo.repeatDays; }
  void setRepeatDays(int *rd) {
    for (int i = 0; i < 7; i++)
      fAppointmentInfo.repeatDays[i] = rd[i];
  }
  void setRepeatDays(QBitArray rba) {
    for (int i = 0; i < 7; i++)
      fAppointmentInfo.repeatDays[i] = (rba[i] ? 1 : 0);
  }

  int getExceptionCount() const { return fAppointmentInfo.exceptions; }
  void setExceptionCount(int e) { fAppointmentInfo.exceptions = e; }

  const struct tm *getExceptions() const { return fAppointmentInfo.exception; }
  void setExceptions(struct tm *e);

	void setDescription(const QString &);
	QString getDescription() const;

	void setNote(const QString &);
	QString getNote() const;

protected:
	void  setDescriptionP(const char* desc, int l=-1);
	const char* getDescriptionP() const { return fAppointmentInfo.description; }

	void  setNoteP(const char* note, int l=-1);
	const char* getNoteP() const { return fAppointmentInfo.note; }

public:
  bool isMultiDay() const {
    return ((fAppointmentInfo.repeatType == repeatDaily) &&
            (fAppointmentInfo.repeatFrequency == 1) &&
            (!fAppointmentInfo.repeatForever) &&
            fAppointmentInfo.event);
  }

  QString getCategoryLabel() const;
  inline bool setCategory(const QString &label) { return setCat(fAppInfo.category,label); } ;
  static const int APP_BUFFER_SIZE;

protected:
  void *pack(void *, int *);
  void unpack(const void *, int = 0) { }

private:
  struct Appointment fAppointmentInfo;
        struct AppointmentAppInfo &fAppInfo;
	void _copyExceptions(const PilotDateEntry &e);
};



#else
#ifdef DEBUG
#warning "File doubly included"
#endif
#endif
