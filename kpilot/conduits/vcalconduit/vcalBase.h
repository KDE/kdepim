/* vcalBase.h			Program
**
** Copyright (C) 2001 by Adriaan de Groot
**
** Program description
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

/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/

#ifndef _VCAL_VCALBASE_H
#define _VCAL_VCALBASE_H

class PilotRecord;
//class VObject;
class QDateTime;

#ifndef _KPILOT_BASECONDUIT_H
#include "baseConduit.h"
#endif

#ifndef __VCC_H__
#include "vcc.h"
#endif

class VCalBaseConduit : public BaseConduit {
 public:
  VCalBaseConduit(BaseConduit::eConduitMode mode);
  virtual ~VCalBaseConduit();
	
  virtual void doSync() = 0;
  virtual void doBackup() = 0;
  virtual QWidget *aboutAndSetup() = 0;
  virtual const char *dbInfo() = 0;

public:
	/**
	* There are a whole bunch of methods that set particular
	* properties on VObjects. Probably they don't belong here
	* but in versit.
	*/
	static void setSummary(VObject *vevent,const char *note);
	static void setNote(VObject *vevent,const char *note);
	static void setSecret(VObject *vevent,bool secret);
	static void setStatus(VObject *vevent,int status) 
	  { setNumProperty(vevent, KPilotStatusProp, status); }	  

	/** 
	 * Find the summary string of vcalendar event @arg
	 * vevent. Returns an empty string if none found.
	 */
	static QString getSummary(VObject *vevent);

	/** Find the description string of vcalendar event @arg
	    vevent. Returns an empty string if none found. */
	static QString getDescription(VObject *vevent);

	/** Check @arg *vevent for KPilotStatus property. Returns 0 or
	    1 if the property exists, 2 otherwise (i.e. 0 if the record
	    is unmodified, non-zero if it is modified or new).  */
	static int getStatus(VObject *vevent);

	/** Add the date property @arg *prop to the vevent.  This
	    *adds* the property as a sub-VObject to the given VObject,
	    so use this primarily when an object is created. The
	    meaning of @arg truncateTime is explained below at
	    TmToISO(). */
	static void addDateProperty(VObject *vevent,
		const char *prop,
		const struct tm *p,
		bool truncateTime=false);
	static void addDateProperty(VObject *vevent,
		const char *prop,
		const QDateTime& dt,
		bool truncateTime=false);

	/** If you already have a date property and you want to change
	    it, use setDateProperty().  The meaning of @arg
	    truncateTime is explained below at TmToISO(). */
	static void setDateProperty(VObject *vevent,
		const struct tm *p,
		bool truncateTime=false);
	static void setDateProperty(VObject *vevent,
		const QDateTime &dt,
		bool truncateTime=false);

	/** General set and reset function for numeric properties. */
	static void setNumProperty(VObject *vevent,
				   const char *property,
				   int num);

	/* More *static* utility functions. Setting truncateTime to
	 true discards the time part and sets the time in the returned
	 string to "000000". This indicates a floating event in
	 vCal. */
	static QString TmToISO(const QDateTime&,
		bool truncateTime=false);
	static QString TmToISO(const struct tm *tm,
		bool truncateTime=false);
	static QString TmToISO(const struct tm &tm,
		bool truncateTime=false) { return TmToISO(&tm,truncateTime); }
	static struct tm ISOToTm(const QString &tStr,int timeZone);
	static int numFromDay(const QString &day);
	static QDateTime tmToQDateTime(const struct tm &t) {
	  return QDateTime(QDate(1900 + t.tm_year, t.tm_mon + 1,
				 t.tm_mday), 
			   QTime(t.tm_hour, t.tm_min, t.tm_sec)); }


	// Inline non-static methods for the above.
	struct tm ISOToTm(const QString &tStr)
	  { return ISOToTm(tStr,fTimeZone); } ;

	/** Read the value of the KPilotID field of vcalendar record
	 * @arg *vevent. Returns 0 if no ID found. */
	static recordid_t getRecordID(VObject *vevent);


	static int getCalendarTimeZone(VObject *vevent);
	
protected:
	/** Copy the value of a Date/Time property of @arg *vevent to
	    @arg *t. Returns true if successful, false if the
	    requested property was not found. */
	bool getDateProperty(struct tm *t,
			     VObject *vevent,
			     const char *prop);

	PilotRecord *findEntryInDB(recordid_t id);
	VObject *findEntryInCalendar(recordid_t id, 
				     const char *entryType);
	void deleteVObject(PilotRecord *rec, const char *type);
	void saveVCal();
	
	bool getCalendar(const QString& group);
	void noCalendarError(const QString& conduitName);
	
	/**
	* Retrieve the time zone set in the vcal file.
	* Returns number of minutes relative to UTC.
	*/
	int getTimeZone() const;
	bool firstTime() const { return fFirstTime; } ;

	/** Time zone offset to GMT in minutes. Set by
	    getCalendar(). */
	int fTimeZone;
	QString calName;

	VObject *calendar() { return fCalendar; } ;

	/** Delete all records from the pilot that are not in the
	    vcalendar. Meant to be run at the end of a hot-sync, after 
	    all new records from both sides have been inserted on the
	    other. @arg entryType should be VCTodoProp or
	    VCEventProp. */
	void deleteFromPilot(const char *entryType);

private:
	VObject *fCalendar;
	bool fFirstTime;
} ;	

#else
#warning "File doubly included."
#endif
