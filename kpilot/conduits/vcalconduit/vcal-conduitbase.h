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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/


#include <qstring.h>

#include <libkcal/calendarlocal.h>

#include <plugin.h>
#include <pilotRecord.h>

namespace KCal
{
class Calendar;
class Incidence;
}

class PilotSerialDatabase;
class PilotLocalDatabase;
class VCalConduitSettings;

class ConduitState;

class VCalConduitPrivateBase
{
protected:
	bool reading;
	KCal::Calendar *fCalendar;
public:
	VCalConduitPrivateBase(KCal::Calendar *buddy) : fCalendar(buddy)
	{
		reading = false;
	};

	virtual ~VCalConduitPrivateBase() { } ;

	virtual int updateIncidences() = 0;
	virtual void addIncidence(KCal::Incidence*) = 0;
	virtual void removeIncidence(KCal::Incidence*) = 0;
	virtual KCal::Incidence *findIncidence(recordid_t) = 0;
	virtual KCal::Incidence *findIncidence(PilotRecordBase *tosearch) = 0;
	virtual KCal::Incidence *getNextIncidence() = 0;
	virtual KCal::Incidence *getNextModifiedIncidence() = 0;
	virtual int count()=0;
} ;

class VCalConduitBase : public ConduitAction
{
        Q_OBJECT
public:
	VCalConduitBase(KPilotLink *,
		const char *name = 0L,
		const QStringList &args = QStringList());
	virtual ~VCalConduitBase();

/*********************************************************************
                D A T A   M E M B E R S ,   S E T T I N G S
 *********************************************************************/
protected:
	KCal::Calendar *fCalendar;
	QString fCalendarFile;
	VCalConduitPrivateBase *fP;
	ConduitState *fState;
	bool hasNextRecord;

	virtual const QString dbname() = 0;
	virtual const QString getTitle(PilotRecordBase *de) = 0;
	virtual void readConfig();

	virtual bool exec();

protected slots:
	/**
	 * This slot is used to execute the actions applicable to this conduit. What
	 * happens in this method is defined by the state the conduit has at the
	 * moment that this method is called. For more information about the actions
	 * that are executed, look at the classes that are implementing ConduitState.
	 */
	void slotProcess();

public:
	/**
	 * Method used by state classes to indicatie if there are more records to
	 * deal with.
	 */
	void setHasNextRecord( bool b ) 
	{
		hasNextRecord = b;
	}

	/**
	 * Change the current state of the conduit. The state that the conduit has
	 * at the moment of the call will be deleted. The last state *must* set the
	 * state to 0L when finished.
	 */
	void setState( ConduitState *s );

	/**
	 * Returns the privatebase, that is used to for accessing the local calendar.
	 */
	VCalConduitPrivateBase *privateBase() const
	{
		return fP;
	}

	/**
	 * Returns the record at index from the palm or 0L if there is no record at
	 * index.
	 */
	PilotRecord *readRecordByIndex( int index );

	/**
	 * Returns a KCal::Incidence constructed from PilotRecord r. If r is 0L the
	 * it will return a KCal::Incidence that is empty.
	 */
	KCal::Incidence *incidenceFromRecord( PilotRecord *r );

	virtual void preIncidence( KCal::Incidence* ) {};

	// Getters
	KCal::Calendar *calendar() const { return fCalendar; };
	QString calendarFile() const { return fCalendarFile; };

	virtual VCalConduitSettings *config() = 0;
	virtual PilotDatabase *database() const { return fDatabase; };
	virtual PilotDatabase *localDatabase() const { return fLocalDatabase; };

	// add, change or delete records from the palm
	virtual void addPalmRecord( KCal::Incidence *e );
	virtual void changePalmRecord( KCal::Incidence *e, PilotRecord *s );
	virtual void deletePalmRecord( KCal::Incidence *e, PilotRecord *s );

	// add, change or delete events from the calendar
	virtual KCal::Incidence* changeRecord( PilotRecord*, PilotRecord* );
	virtual KCal::Incidence* deleteRecord( PilotRecord*, PilotRecord* );
	virtual KCal::Incidence* addRecord( PilotRecord * );

/*********************************************************************
          P R E -   A N D   P O S T S Y N C   F U N C T I O N S
 *********************************************************************/
	virtual void preSync();
	virtual void postSync();
	virtual void preRecord(PilotRecord*) {};

protected:
	virtual void updateIncidenceOnPalm(KCal::Incidence *e, PilotRecordBase *de);

/*********************************************************************
                 	S Y N C   F U N C T I O N S
               for creating events from Palm records or vice versa
 *********************************************************************/
	virtual PilotRecord *recordFromIncidence(PilotRecordBase *de,
		const KCal::Incidence *e) = 0;
	virtual PilotRecordBase *newPilotEntry(PilotRecord *r) = 0;

	virtual KCal::Incidence *newIncidence() = 0;
	virtual KCal::Incidence *incidenceFromRecord(KCal::Incidence *e,
		const PilotRecordBase *de) = 0;

/*********************************************************************
                M I S C   F U N C T I O N S
 *********************************************************************/
	/**
     * Return how to resolve conflicts. For now
	 * PalmOverrides=0=false,
	 * PCOverrides=1=true,
	 * Ask=2-> ask the user using a messagebox
	 */
	virtual int resolveConflict(KCal::Incidence *e, PilotRecordBase *de);
	virtual bool openCalendar();
	virtual VCalConduitPrivateBase *createPrivateCalendarData(KCal::Calendar *fCalendar) = 0;
} ;

#endif
