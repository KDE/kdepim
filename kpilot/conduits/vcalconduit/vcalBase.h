#ifndef _VCAL_VCALBASE_H
#define _VCAL_VCALBASE_H

/* vcalBase.h			Base class for KOrganizer conduits
**
** Copyright (C) 2001 by Adriaan de Groot, Cornelius Schumacher
**
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
*/

#include "plugin.h"

class PilotRecord;
class PilotDatabase;
class QDateTime;

#if KDE_VERSION < 300
#include <libkcal/calendar.h>
#include <libkcal/incidence.h>
#else
#include <calendar.h>
#include <incidence.h>
#endif

using namespace KCal;

class VCalBaseConduit : public ConduitAction
{
public:
	VCalBaseConduit(KPilotDeviceLink *,
		const char *name=0L,
		const QStringList &args = QStringList() );
	virtual ~VCalBaseConduit();

	/* virtual void exec() from ConduitAction should call these*/
	virtual void doSync() = 0;
	virtual void doBackup() = 0;

	/**
	* There are a whole bunch of methods that set particular
	* properties on Incidences. Probably they don't belong here
	* but in versit.
	*/
	static void setSummary(Incidence *vevent,const char *note);
	static void setNote(Incidence *vevent,const char *note);
	static void setSecret(Incidence *vevent,bool secret);

	/**
	* Find the summary string of vcalendar event @arg
	* vevent. Returns an empty string if none found.
	*/
	static QString getSummary(Incidence *vevent);

	/**
	* Find the description string of vcalendar event @arg
	* vevent. Returns an empty string if none found.
	*/
	static QString getDescription(Incidence *vevent);

	/** Check @arg *vevent for KPilotStatus property. Returns 0 or
	* 1 if the property exists, 2 otherwise (i.e. 0 if the record
	* is unmodified, non-zero if it is modified or new).
	*/
	static int getStatus(Incidence *vevent);

#if 0
	QDate readTm(const struct tm &);
	struct tm writeTm(const QDate &);
#else
	QDateTime readTm(const struct tm &);
	struct tm writeTm(const QDateTime &);
#endif

	// Here are some configuration keys -- which have moved out of the
	// factory -- for all the vcalBase-derived conduits.
	//
	//
	static const char * const calendarFile,
		* const firstTime,
		* const deleteOnPilot;

	// These functions can be used to manipulate the settings in the config file
	// (though make sure to set the group correctly first).
	//
	//
	void setFirstTime(KConfig *,bool);

protected:
	void saveVCal();

	// Deletes a record from the desktop calendar
	void deleteRecord(PilotRecord *rec);

	bool getCalendar(const QString& group);
	void noCalendarError(const QString& conduitName);

	bool isFirstTime() const { return fFirstTime; } ;

	/** Time zone offset to GMT in minutes. Set by
	* getCalendar().
	*/
	int fTimeZone;

	QString calName;

	Calendar *calendar() { return fCalendar; }

	enum { TypeTodo, TypeEvent };

	/** Delete all records from the pilot that are not in the
	* vcalendar. Meant to be run at the end of a hot-sync, after
	* all new records from both sides have been inserted on the
	* other. @arg entryType should be TypeTodo or TypeEvent */
	void deleteFromPilot(int);

	Todo *findTodo(recordid_t id);
	Event *findEvent(recordid_t id);
	PilotDatabase *fDatabase;

	/**
	* Check to make sure that neither korganizer nor alarmd is running,
	* so that we can avoid stepping on their toes for now.
	*/
	bool isKOrganizerRunning();
	
private:
	Calendar *fCalendar;
	bool fFirstTime;
};

// $Log$
// Revision 1.9  2001/12/28 12:56:46  adridg
// Added SyncAction, it may actually do something now.
//
#endif
