#ifndef _KPILOT_VCAL_CONDUITBASE_H
#define _KPILOT_VCAL_CONDUITBASE_H
/* vcal-conduit.h                       KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the vcal-conduit plugin.
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
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <plugin.h>


#include <options.h>
//#include <unistd.h>

//#include <qdatetime.h>



namespace KCal
{
class Calendar;
// class Event;
class Incidence;
} ;

class PilotRecord;
class PilotSerialDatabase;
class PilotLocalDatabase;
//class PilotDateEntry;
class PilotAppCategory;

QDateTime readTm(const struct tm &t);
struct tm writeTm(const QDateTime &dt);
struct tm writeTm(const QDate &dt);


class VCalConduitPrivateBase
{
protected:
	bool reading;
	KCal::Calendar *fCalendar;
public:
	VCalConduitPrivateBase(KCal::Calendar *buddy):fCalendar(buddy) { reading=false;};

	virtual int updateIncidences()=0;
	virtual void addIncidence(KCal::Incidence*)=0;
	virtual void removeIncidence(KCal::Incidence *)=0;
	virtual KCal::Incidence *findIncidence(recordid_t)=0;
	virtual KCal::Incidence *getNextIncidence()=0;
	virtual KCal::Incidence *getNextModifiedIncidence()=0;
	virtual int count()=0;
} ;



class VCalConduitBase : public ConduitAction
{
Q_OBJECT;
public:
	VCalConduitBase(KPilotDeviceLink *,
		const char *name=0L,
		const QStringList &args = QStringList());
	virtual ~VCalConduitBase();

	virtual void exec();

protected slots:
	/**
	* This function is called to sync modified records from the Pilot to KOrganizer.
	*/
	void syncRecord();
	/**
	* This function goes the other way around: KOrganizer -> Pilot.
	*/
	void syncIncidence();
	void syncDeletedIncidence();
	void cleanup();

	
protected:

	virtual int resolveConflict(KCal::Incidence*e, PilotAppCategory*de);

	// add, change or delete events from the calendar
	virtual void addRecord(PilotRecord *);
	virtual void changeRecord(PilotRecord *,PilotRecord *);
	virtual void deleteRecord(PilotRecord *,PilotRecord *);

	// add, change or delete records from the palm
	virtual void addPalmRecord(KCal::Incidence*e);
	virtual void changePalmRecord(KCal::Incidence*e, PilotRecord*s);
	virtual void deletePalmRecord(KCal::Incidence*e, PilotRecord*s);

	virtual void updateIncidenceOnPalm(KCal::Incidence*e, PilotAppCategory*de);

	// THESE NEED TO BE IMPLEMENTED BY CHILD CLASSES!!!!
	
	// create events from Palm records or vice versa
	virtual PilotRecord*recordFromIncidence(PilotAppCategory*de, const KCal::Incidence*e)=0;
	virtual KCal::Incidence *incidenceFromRecord(KCal::Incidence *e, const PilotAppCategory *de)=0;
	
	virtual PilotAppCategory*newPilotEntry(PilotRecord*r)=0;
	virtual KCal::Incidence*newIncidence()=0;


	// general settings, implemented by child classes for the conduits
	virtual const QString configGroup()=0;
	virtual const QString dbname()=0;
	
	virtual const QString getTitle(PilotAppCategory*de)=0;

protected:
	KCal::Calendar *fCalendar;
	PilotSerialDatabase *fCurrentDatabase;
	PilotLocalDatabase *fBackupDatabase;

	QString fCalendarFile;
	int syncAction, nextSyncAction, conflictResolution;
	bool archive;
	bool fFirstTime, fFullSync;
	int pilotindex;

protected:
//	class VCalPrivateBase;
	VCalConduitPrivateBase *fP;
   virtual VCalConduitPrivateBase* newVCalPrivate(KCal::Calendar *fCalendar)=0;
} ;


// $Log$
// Revision 1.3  2002/05/01 21:18:23  kainhofe
// Reworked the settings dialog, added various different sync options
//
// Revision 1.1.2.2  2002/05/01 21:11:49  kainhofe
// Reworked the settings dialog, added various different sync options
//
// Revision 1.1.2.1  2002/04/28 12:58:54  kainhofe
// Calendar conduit now works, no memory leaks, timezone still shifted. Todo conduit mostly works, for my large list it crashes when saving the calendar file.
//

#endif
