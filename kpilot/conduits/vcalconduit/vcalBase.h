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
class VObject;
class QDateTime;

#ifndef _KPILOT_BASECONDUIT_H
#include "baseConduit.h"
#endif

class VCalBaseConduit : public BaseConduit
{
public:
	VCalBaseConduit(BaseConduit::eConduitMode mode);
	virtual ~VCalBaseConduit();
	
	// Subclasses should define
	//
	// virtual void doSync();
	// virtual void doBackup();
	// virtual QWidget *aboutAndSetup();
	// virtual void doBackup();
	//
	// virtual const char *dbInfo() { return "<none>"; };

public:
	/**
	* There are a whole bunch of methods that set particular
	* properties on VObjects. Probably they don't belong here
	* but in versit.
	*/
	static void setSummary(VObject *vevent,const char *note);
	static void setNote(VObject *vevent,const char *note);
	static void setSecret(VObject *vevent,bool secret);
	static void setStatus(VObject *vevent,int status);	
	// Add the date property "prop" to the vevent.
	// This *adds* the property as a sub-VObject to
	// the given VObject, so use this primarily when
	// an object is created. The meaning of truncateTime
	// is explained below at TmToISO().
	//
	//
	static void addDateProperty(VObject *vevent,
		const char *prop,
		const struct tm *p,
		bool truncateTime=false);
	static void addDateProperty(VObject *vevent,
		const char *prop,
		const QDateTime& dt,
		bool truncateTime=false);
	// If you already have a date property and you
	// want to change it, use setDateProperty(). 
	// The meaning of truncateTime
	// is explained below at TmToISO().
	//
	//
	static void setDateProperty(VObject *vevent,
		const struct tm *p,
		bool truncateTime=false);
	static void setDateProperty(VObject *vevent,
		const QDateTime &dt,
		bool truncateTime=false);

	// More *static* utility functions. Setting
	// truncateTime to true discards the time
	// part and sets the time in the returned
	// string to "000000". This indicates a
	// floaring event in vCal.
	//
	//
	static QString TmToISO(const QDateTime&,
		bool truncateTime=false);
	static QString TmToISO(const struct tm *tm,
		bool truncateTime=false);
	static QString TmToISO(const struct tm tm,
		bool truncateTime=false) { return TmToISO(&tm,truncateTime); }
	static struct tm ISOToTm(const QString &tStr,int timeZone);
	static int numFromDay(const QString &day);

	// Inline non-static methods for the above.
	//
	//
	struct tm ISOToTm(const QString &tStr) { return ISOToTm(tStr,fTimeZone); } ;

	
protected:
	PilotRecord *findEntryInDB(unsigned int id);
	VObject *findEntryInCalendar(unsigned int id);
	void deleteVObject(PilotRecord *rec);
	void saveVCal();
	
	bool getCalendar(const QString& group);
	void noCalendarError(const QString& conduitName);
	
	/**
	* Retrieve the time zone set in the vcal file.
	* Returns number of minutes relative to UTC.
	*/
	int getTimeZone() const;
	bool firstTime() const { return fFirstTime; } ;

	int fTimeZone;
	QString calName;

	VObject *calendar() { return fCalendar; } ;

private:
	VObject *fCalendar;
	bool fFirstTime;
} ;	

#else
#warning "File doubly included."
#endif
