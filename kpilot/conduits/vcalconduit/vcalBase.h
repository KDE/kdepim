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

#ifndef _KPILOT_VCALBASE_H
#define _KPILOT_VCALBASE_H

class PilotRecord;
class VObject;

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
	void setNote(VObject *vevent,const char *note);
	void setSecret(VObject *vevent,bool secret);
	void setStatus(VObject *vevent,int status);	

	// More *static* utility functions.
	//
	//
	static QString TmToISO(struct tm tm);
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
