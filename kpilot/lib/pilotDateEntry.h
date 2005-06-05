#ifndef _KPILOT_PILOTDATEENTRY_H
#define _KPILOT_PILOTDATEENTRY_H
/* pilotDateEntry.h	-*- C++ -*-	KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** See the .cc file for an explanation of what this file is for.
*/

/** @file pilotDateEntry.h defines a wrapper for datebook entries. */

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

#include <qbitarray.h>

#include <pi-macros.h>
#include <pi-datebook.h>

#include "pilotAppCategory.h"
#include "pilotDatabase.h"

namespace KCal
{
class Event;
}

/** This class is a wrapper for pilot-link's datebook entries (struct Appointment). */
class KDE_EXPORT PilotDateEntry : public PilotAppCategory
{
public:
	/** Constructor. Sets the appinfo structure and zeroes out the appointment. */
	PilotDateEntry(struct AppointmentAppInfo &appInfo);

	/** Constructor. Interprets the given record as an appointment. */
	PilotDateEntry(struct AppointmentAppInfo &appInfo, PilotRecord* rec);

	/** Copy constructor. */
	PilotDateEntry(const PilotDateEntry &e);

	/** Destructor. */
	~PilotDateEntry() { free_Appointment(&fAppointmentInfo); }

	/** Assignment operator. */
	PilotDateEntry& operator=(const PilotDateEntry &e);

	/** Create a textual representation (human-readable) of this appointment.
	* If @p richText is true, then the text representation uses qt style
	* tags as well.
	*/
	virtual QString getTextRepresentation(bool richText=false);

	/** Is this appointment a "floating" appointment?
	*
	* Floating appointments are those that have a day assigned, but no time
	* in that day (birthday appointments are like that).
	*/
	bool doesFloat() const { return !fAppointmentInfo.event; }
	/** Antonym for doesFloat -- if the appointment does not float, then it
	* has a time associated (and the contrapositive, too).
	*/
	inline bool isEvent() const { return !doesFloat(); }
	/** Antonym for doesFloat, deprecated. */
	int KDE_DEPRECATED getEvent() const { return !doesFloat(); }

	/** Sets this appointment's floating status.
	*
	* Floating appointments are those that have a day assigned, but no time
	* in that day (birthday appointments are like that).
	*/
	void setFloats(bool f) { fAppointmentInfo.event = (f ? 0 : 1) /* Force 0 or 1 */ ; }
	/** Synonym for setFloats() */
	void KDE_DEPRECATED setEvent(int event) { setFloats( !event ); }

	/** Get the start time of this appointment. @see dtStart() for caveats. */
	struct tm getEventStart() const { return fAppointmentInfo.begin; }
	/** Get a pointer to the start time of this appointment. @see dtStart() for caveats. */
	const struct tm *getEventStart_p() const { return &fAppointmentInfo.begin; }
	/** Sets the start time of this appointment. */
	void setEventStart(struct tm& start) { fAppointmentInfo.begin = start; }
	/** Get the start time of this appointment. For floating appointments, the
	* time is undefined (perhaps 1 minute past midnight).
	*
	* Floating appointments are those that have a day assigned, but no time
	* in that day (birthday appointments are like that).
	*/
	QDateTime dtStart() const;

	/** Get the end time of this appointment. @see dtEnd() for caveats. */
	struct tm getEventEnd() const { return fAppointmentInfo.end; }
	/** Get a pointer to the end time of this appointment. @see dtEnd() for caveats. */
	const struct tm *getEventEnd_p() const { return &fAppointmentInfo.end; }
	/** Set the end time of this appointment. */
	void setEventEnd(struct tm& end) { fAppointmentInfo.end = end; }
	/** Get the end time of this appointment. For floating appointments, the
	* time is undefined (perhaps 1 minute past midnight).
	*
	* Floating appointments are those that have a day assigned, but no time
	* in that day (birthday appointments are like that).
	*/
	QDateTime dtEnd() const;

	/** Does this appointment have an alarm set? On the Pilot, an event
	* may have an alarm (or not). If it has one, it is also enabled and
	* causes the Pilot to beep (or whatever is set in the system preferences).
	*/
	bool isAlarmEnabled() const { return fAppointmentInfo.alarm; }
	/** Does this appointment have an alarm set? @see isAlarmEnabled() */
	int KDE_DEPRECATED getAlarm() const { return fAppointmentInfo.alarm; }
	/** Set whether this appointment has an alarm. */
	void KDE_DEPRECATED setAlarm(int alarm) { fAppointmentInfo.alarm = alarm; }
	/** Set whether this appointment has an alarm. */
	void setAlarmEnabled(bool b) { fAppointmentInfo.alarm = (b?1:0) /* Force to known int values */ ; }

	/** Get the numeric part of "alarm: __ (v) minutes" on the pilot -- you
	* set the alarm time in two parts, a number and a unit type to use; unit
	* types are minutes, hours, days and the number is whatever you like.
	*
	* If alarms are not enabled for this appointment, returns garbage.
	*
	* @see alarmLeadTime()
	* @see dtAlarm()
	*/
	int getAdvance() const { return fAppointmentInfo.advance; }
	/** Set the numeric part of the alarm setting. @see getAdvance for details. */
	void setAdvance(int advance) { fAppointmentInfo.advance = advance; }

	/** Returns the units part of the alarm time. @see getAdvance . */
	int getAdvanceUnits() const { return fAppointmentInfo.advanceUnits; }
	/** Sets the unites part of the alarm time. @see getAdvance . */
	void setAdvanceUnits(int units) { fAppointmentInfo.advanceUnits = units; }

	/** Returns the number of @em seconds "lead time" the alarm should sound
	* before the actual appointment. This interprets the advance number and units.
	* The value is always positive, 0 if no alarms are enabled.
	*/
	unsigned int alarmLeadTime() const;

	/** Returns the absolute date and time that the alarm should sound for
	* this appointment.
	*/
	QDateTime dtAlarm() const { return dtStart().addSecs(-alarmLeadTime()); }

  // The following need set routines written
  repeatTypes getRepeatType() const { return fAppointmentInfo.repeatType; }
  void setRepeatType(repeatTypes r) { fAppointmentInfo.repeatType = r; }

  int getRepeatForever() const { return fAppointmentInfo.repeatForever; }
  void setRepeatForever(int f = 1) { fAppointmentInfo.repeatForever = f; }

  struct tm getRepeatEnd() const { return fAppointmentInfo.repeatEnd; }
  void setRepeatEnd(struct tm tm) { fAppointmentInfo.repeatEnd = tm; }
	/** Returns the date and time that the repeat ends. If there is no repeat,
	* returns an invalid date and time.
	*/
	QDateTime dtRepeatEnd() const;

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

	/** Sets the description of the appointment. This is the short string
	* entered in the day view on the handheld, and it is called the summary
	* in libkcal.
	*/
	void setDescription(const QString &);
	/** Gets the description of the appointment. @see setDescription for meaning. */
	QString getDescription() const;

	/** Sets the note for the appointment. The note is the long text entry
	* that is possible - but clumsy - on the handheld. It is called the
	* description in libkcal.
	*/
	void setNote(const QString &);
	/** Gets the note for this appointment. @see setNote for meaning. */
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
            ( !getRepeatForever() ) &&
            !doesFloat() );
  }

  QString getCategoryLabel() const;
  inline bool setCategory(const QString &label) { return PilotAppCategory::setCategory(fAppInfo.category,label); } ;
  static const int KDE_DEPRECATED APP_BUFFER_SIZE;

protected:
  void *pack_(void *buf, int *size);
  void unpack(const void *buf, int size = 0) { }

private:
  struct Appointment fAppointmentInfo;
        struct AppointmentAppInfo &fAppInfo;
	void _copyExceptions(const PilotDateEntry &e);
};


typedef PilotAppInfo<AppointmentAppInfo,unpack_AppointmentAppInfo, pack_AppointmentAppInfo> PilotDateInfo;


#endif

