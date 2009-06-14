#ifndef KPILOT_PILOTDATEENTRY_H
#define KPILOT_PILOTDATEENTRY_H
/* pilotDateEntry.h	-*- C++ -*-	KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone <dan@kpilot.org>
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

// KPilot headers
#include "pilotRecord.h"
#include "pilotAppInfo.h"

// pilot-link headers
#include <pi-macros.h>
#include <pi-datebook.h>

// Qt headers
#include <QtCore/QBitArray>
#include <QtCore/QDateTime>

namespace KCal
{
class Event;
}

inline int _upDBAI(struct AppointmentAppInfo *m, const unsigned char *b, size_t s)
{
	return unpack_AppointmentAppInfo(m,b,s);
}

inline int _pDBAI(const struct AppointmentAppInfo *m, unsigned char *b, size_t s)
{
	return pack_AppointmentAppInfo(m,b,s);
}

/** Interpreted form of the AppInfo block in the datebook database. */
typedef PilotAppInfo<
	AppointmentAppInfo,
	_upDBAI, 
	_pDBAI> PilotDateInfo_;


class PilotDateInfo : public PilotDateInfo_
{
public:
	PilotDateInfo(PilotDatabase *d) : PilotDateInfo_(d)
	{
	}

	/** This resets the entire AppInfo block to one as it would be
	*   in an English-language handheld, with 3 categories and
	*   default field labels for everything.
	*/
	void resetToDefault();

};

/** This class is a wrapper for pilot-link's datebook entries (struct Appointment). */
class KPILOT_EXPORT PilotDateEntry : public PilotRecordBase
{
public:
	/** Constructor. Zeroes out the appointment. */
	PilotDateEntry();

	/** Constructor. Interprets the given record as an appointment. */
	PilotDateEntry(PilotRecord *rec);

	/** Copy constructor. */
	PilotDateEntry(const PilotDateEntry &e);

	/** Destructor. */
	~PilotDateEntry()
	{
		free_Appointment(&fAppointmentInfo);
	}

	/** Assignment operator. */
	PilotDateEntry& operator=(const PilotDateEntry &e);

	/** Create a textual representation (human-readable) of this appointment.
	* If @p richText is true, then the text representation uses qt style
	* tags as well.
	*/
	QString getTextRepresentation(Qt::TextFormat richText);

	/** Is this appointment a "floating" appointment?
	*
	* Floating appointments are those that have a day assigned, but no time
	*  in that day (birthday appointments are like that).  You can think of these
	*  as "events", which don't have a time associated with them for a given day,
	*  as opposed to a regular "appointment", which does normally have a time
	*  associated with it.
	*/
	inline bool doesFloat() const
	{
		return fAppointmentInfo.event;
	}

	/** Is this a non-time-related event as opposed to an appointment that has a
	* time associated with it?.
	*/
	inline bool isEvent() const
	{
		return doesFloat();
	}

	/** Sets this appointment's floating status.
	*
	* Floating appointments are those that have a day assigned, but no time
	*  in that day (birthday appointments are like that).  You can think of these
	*  as "events", which don't have a time associated with them for a given day,
	*  as opposed to a regular "appointment", which does normally have a time
	*  associated with it.
	*/
	inline void setFloats(bool f)
	{
		fAppointmentInfo.event = (f ? 1 : 0) /* Force 1 or 0 */ ;
	}

	/** Get the start time of this appointment.  See dtStart() for caveats. */
	inline struct tm getEventStart() const { return fAppointmentInfo.begin; }

	/** Get a pointer to the start time of this appointment.  See dtStart() for caveats. */
	inline const struct tm *getEventStart_p() const
	{
		return &fAppointmentInfo.begin;
	}

	/** Sets the start time of this appointment. */
	inline void setEventStart(struct tm& start)
	{
		fAppointmentInfo.begin = start;
	}

	/** Get the start time of this appointment. For floating appointments, the
	* time is undefined (perhaps 1 minute past midnight).
	*
	* Floating appointments are those that have a day assigned, but no time
	* in that day (birthday appointments are like that).
	*/
	QDateTime dtStart() const;

	/** Get the end time of this appointment.  See dtEnd() for caveats. */
	inline struct tm getEventEnd() const
	{
		return fAppointmentInfo.end;
	}

	/** Get a pointer to the end time of this appointment.  See dtEnd() for caveats. */
	inline const struct tm *getEventEnd_p() const
	{
		return &fAppointmentInfo.end;
	}

	/** Set the end time of this appointment. */
	inline void setEventEnd(struct tm& end)
	{
		fAppointmentInfo.end = end;
	}

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
	inline bool isAlarmEnabled() const
	{
		return fAppointmentInfo.alarm;
	}

	/** Set whether this appointment has an alarm. */
	inline void setAlarmEnabled(bool b)
	{
		fAppointmentInfo.alarm = (b?1:0) /* Force to known int values */ ;
	}

	/** Get the numeric part of "alarm: __ (v) minutes" on the pilot -- you
	* set the alarm time in two parts, a number and a unit type to use; unit
	* types are minutes, hours, days and the number is whatever you like.
	*
	* If alarms are not enabled for this appointment, returns garbage.
	*
	* @see alarmLeadTime()
	* @see dtAlarm()
	*/
	inline int getAdvance() const
	{
		return fAppointmentInfo.advance;
	}

	/** Set the numeric part of the alarm setting.  See getAdvance for details. */
	inline void setAdvance(int advance)
	{
		fAppointmentInfo.advance = advance;
	}

	/** Returns the units part of the alarm time.  See getAdvance . */
	inline int getAdvanceUnits() const
	{
		return fAppointmentInfo.advanceUnits;
	}

	/** Sets the unites part of the alarm time.  See getAdvance . */
	inline void setAdvanceUnits(int units)
	{
		fAppointmentInfo.advanceUnits = units;
	}

	/** Returns the number of @em seconds "lead time" the alarm should sound
	* before the actual appointment. This interprets the advance number and units.
	* The value is always positive, 0 if no alarms are enabled.
	*/
	unsigned int alarmLeadTime() const;

	/** Returns the absolute date and time that the alarm should sound for
	* this appointment.
	*/
	inline QDateTime dtAlarm() const
	{
		return dtStart().addSecs(-alarmLeadTime());
	}

	// The following need set routines written
	inline repeatTypes getRepeatType() const
	{
		return fAppointmentInfo.repeatType;
	}
	inline void setRepeatType(repeatTypes r)
	{
		fAppointmentInfo.repeatType = r;
	}

	inline int getRepeatForever() const
	{
		return fAppointmentInfo.repeatForever;
	}
	inline void setRepeatForever(int f = 1)
	{
		fAppointmentInfo.repeatForever = f;
	}

	inline struct tm getRepeatEnd() const
	{
		return fAppointmentInfo.repeatEnd;
	}
	inline void setRepeatEnd(struct tm tm)
	{
		fAppointmentInfo.repeatEnd = tm;
	}

	/** Returns the date and time that the repeat ends. If there is no repeat,
	* returns an invalid date and time.
	*/
	QDateTime dtRepeatEnd() const;

	inline int getRepeatFrequency() const
	{
		return fAppointmentInfo.repeatFrequency;
	}
	inline void setRepeatFrequency(int f)
	{
		fAppointmentInfo.repeatFrequency = f;
	}

	inline DayOfMonthType getRepeatDay() const
	{
		return fAppointmentInfo.repeatDay;
	}
	inline void setRepeatDay(DayOfMonthType rd)
	{
		fAppointmentInfo.repeatDay = rd;
	};

	inline const int *getRepeatDays() const
	{
		return fAppointmentInfo.repeatDays;
	}
	inline void setRepeatDays(int *rd)
	{
		for (int i = 0; i < 7; i++)
		{
			fAppointmentInfo.repeatDays[i] = rd[i];
		}
	}
	inline void setRepeatDays(const QBitArray& rba)
	{
		for (int i = 0; i < 7; i++)
		{
			fAppointmentInfo.repeatDays[i] = (rba[i] ? 1 : 0);
		}
	}

	inline int getExceptionCount() const
	{
		return fAppointmentInfo.exceptions;
	}
	inline void setExceptionCount(int e)
	{
		fAppointmentInfo.exceptions = e;
	}

	inline const struct tm *getExceptions() const
	{
		return fAppointmentInfo.exception;
	}
	void setExceptions(struct tm *e);

	/** Sets the description of the appointment. This is the short string
	* entered in the day view on the handheld, and it is called the summary
	* in libkcal.
	*/
	void setDescription(const QString &);
	/** Gets the description of the appointment.  See setDescription for meaning. */
	QString getDescription() const;

	/** Sets the note for the appointment. The note is the long text entry
	* that is possible - but clumsy - on the handheld. It is called the
	* description in libkcal.
	*/
	void setNote(const QString &);
	/** Gets the note for this appointment.  See setNote for meaning. */
	QString getNote() const;

	/**
	 * Sets the location for the appointment. For now it will be placed within
	 * the notes on the handheld. It will be placed on one line and starts with:
	 * Location: {location}. Everything on that line will be counted as location.
	 * TODO: Make distinguish between handhelds that support the location field
	 * and the ones that don't. (Shouldn't this be done in the pilot-link lib?)
	 */
	void setLocation(const QString &);

	/** Gets the location for this appointment.  See setNote for meaning. */
	QString getLocation() const;

protected:
	void  setDescriptionP(const char* desc, int l=-1);
	const char* getDescriptionP() const
	{
		return fAppointmentInfo.description;
	}

	void  setNoteP(const char* note, int l=-1);
	const char* getNoteP() const
	{
		return fAppointmentInfo.note;
	}

public:
	bool isMultiDay() const
	{
	return ((fAppointmentInfo.repeatType == repeatDaily) &&
		(fAppointmentInfo.repeatFrequency == 1) &&
		( !getRepeatForever() ) &&
		!doesFloat() );
	}

	PilotRecord *pack() const;

private:
	struct Appointment fAppointmentInfo;
	void _copyExceptions(const PilotDateEntry &e);
};



#endif

