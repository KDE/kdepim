/* *******************************************************
   KPilot - Hot-Sync Software for Unix.
   Copyright 1998 by Dan Pilone, Preston Brown
   This code is released under the GNU PUBLIC LICENSE.
   Please see the file 'COPYING' included in the KPilot
   distribution.
   *******************************************************
 */

#ifndef __KPILOT_DATE_ENTRY_H
#define __KPILOT_DATE_ENTRY_H

#include <qbitarray.h>
#include <time.h>
#include <string.h>

#include <pi-macros.h>
#include <pi-datebook.h>

#include "pilotAppCategory.h"
#include "pilotRecord.h"


class PilotDateEntry : public PilotAppCategory
{
public:
  PilotDateEntry(void) : PilotAppCategory() 
    { memset(&fAppointmentInfo, 0, sizeof(struct Appointment)); }
  PilotDateEntry(PilotRecord* rec);
  ~PilotDateEntry() { free_Appointment(&fAppointmentInfo); }
  
  PilotRecord* pack() { return PilotAppCategory::pack(); }
  
  int getEvent() const { return fAppointmentInfo.event; }
  void setEvent(int event) { fAppointmentInfo.event = event; }
  
  struct tm getEventStart() const { return fAppointmentInfo.begin; }
  void setEventStart(struct tm& start) { fAppointmentInfo.begin = start; }

  struct tm getEventEnd() const { return fAppointmentInfo.end; }
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
  void setExceptions(struct tm *e) { fAppointmentInfo.exception = e; }

  void  setDescription(const char* desc);
  char* getDescription() { return fAppointmentInfo.description; }

  void  setNote(const char* note);
  char* getNote() { return fAppointmentInfo.note; }
  
protected:
  void *pack(void *, int *);
  void unpack(void *, int = 0) { }
  
private:
  struct Appointment fAppointmentInfo;
};



#endif
