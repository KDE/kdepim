#ifndef _KPILOT_VCAL_CONDUITBASE_H
#define _KPILOT_VCAL_CONDUITBASE_H
/* vcal-conduit.h                       KPilot
**
** Copyright (C) 2002-2003 Reinhold Kainhofer
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
	virtual KCal::Incidence *findIncidence(PilotAppCategory*tosearch)=0;
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

protected:
	virtual bool exec();

protected slots:
	/**
	* This function is called to sync modified records from the Pilot to KOrganizer.
	*/
	void syncPalmRecToPC();
	/**
	* This function goes the other way around: KOrganizer -> Pilot.
	*/
	void syncPCRecToPalm();
	void syncDeletedIncidence();
	void cleanup();

	
protected:

	virtual int resolveConflict(KCal::Incidence*e, PilotAppCategory*de);

	// add, change or delete events from the calendar
	virtual KCal::Incidence* addRecord(PilotRecord *);
	virtual KCal::Incidence* changeRecord(PilotRecord *,PilotRecord *);
	virtual KCal::Incidence* deleteRecord(PilotRecord *,PilotRecord *);

	// add, change or delete records from the palm
	virtual void addPalmRecord(KCal::Incidence*e);
	virtual void changePalmRecord(KCal::Incidence*e, PilotRecord*s);
	virtual void deletePalmRecord(KCal::Incidence*e, PilotRecord*s);

	virtual void updateIncidenceOnPalm(KCal::Incidence*e, PilotAppCategory*de);
	
	virtual void readConfig();
	virtual bool openCalendar();

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

	// THESE *CAN* BE IMPLEMTED BY CHILD CLASSES
	// execute something at the beginning or end of the sync.
	virtual void preSync(){};
	virtual void postSync();
	virtual void preRecord(PilotRecord*){};
	virtual void preIncidence(KCal::Incidence *){};

protected:
	KCal::Calendar *fCalendar;

	QString fCalendarFile;
	int syncAction, nextSyncAction, conflictResolution;
	bool archive;
	bool fFirstTime, fFullSync;
	int pilotindex;
	enum eCalendarTypeEnum {
		eCalendarResource=0,
		eCalendarLocal
	} fCalendarType;

protected:
//	class VCalPrivateBase;
	VCalConduitPrivateBase *fP;
   virtual VCalConduitPrivateBase* newVCalPrivate(KCal::Calendar *fCalendar)=0;
} ;

#endif
