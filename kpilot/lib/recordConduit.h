#ifndef _RECORD_CONDUIT_H
#define _RECORD_CONDUIT_H
/* record-conduit.h                           KPilot
**
** Copyright (C) 2005 by Adriaan de Groot
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <qtimer.h>
#include <klocale.h>
#include <QMap>

#include "plugin.h"
#include "pilotAppCategory.h"
#include "pilotDatabase.h"


/** @file
*
* This file defines a generic syncing framework for Palm Pilot oriented data.
* It is a lot like KitchenSync's Syncees and such. Basically, we define a
* generic container for data on the PC side and give that container an API
* for searching for a specific handheld record. Syncing consists of iterating
* through the handheld's records and looking up the PC data for each record,
* and then syncing.
*
*/

/** An intermediate class that introduces the slots we need for our sync
* implementation. This is here _only_ because mixing moc with template
* classes sounds really scary.
*/

class RecordConduitBase : public ConduitAction
{
Q_OBJECT
public:
	/** Constructor. The QStringList @p a sets flags for the ConduitAction.
	*/
	RecordConduitBase(KPilotDeviceLink *o,
		const char *n,
		const QStringList a = QStringList()) :
		ConduitAction(o,n,a),
		fTimer(0L)
	{
	} ;
	/** Destructor. */
	virtual ~RecordConduitBase()
	{
		// delete fTimer; // Timer is a child object
	} ;

	/** The different directions that a pair of records (one on the PC, one
	* on the Pilot) can be synced. These correspond to special sync directions
	* and the most general "sync both ways". Values of this type are passed
	* to the single-record syncer functions.
	*/
	typedef enum { HHtoPC=0, PCtoHH=1, Both=2 } SyncDirection;

	/** Return values for the processing functions. Each should return
	* NotDone if it needs to be called again (e.g. to process another record),
	* Done if it is finished and something else should be done, and
	* Error if the sync cannot be completed.
	*/
	typedef enum { NotDone=0, Done=1, Error=2 } SyncProgress;

protected:
	/** Function called at the beginning of a sync to load data from the PC.
	* @return Done when the load has finished.
	* @see process
	*/
	virtual SyncProgress loadPC() = 0;

	/** Function called repeatedly to fetch the next modified entry from the Palm and
	* sync it with the PC by looking up the record, and calling the syncer for it.
	*
	* @return true when there are no more modified records on the Palm
	* @see process
	*/
	virtual SyncProgress palmRecToPC() = 0;

	/** Function called at the end of this conduit's sync, which should reset DB flags
	* and write changed config data out to disk.
	*
	* @return true if the cleanup succeeds.
	* @see process
	*/
	virtual SyncProgress cleanup() = 0;

protected slots:
	/** Slot used for the implementation of a state machine: calls each of the
	* relevant other slots (above) as needed until they return true.
	*/
	void process();

protected:
	virtual bool exec();

private:
	/** Timer to signal the process() slot. Used to keep the UI responsive. */
	QTimer *fTimer;

	/** State of the conduit's sync. This is changed by process. */
	enum { Initialize, PalmToPC, Cleanup } fState;

	QMap<recordid_t,QString> fUIDMap;
	RecordIDList fIDList;
	RecordIDList::Iterator fIDListIterator;

	QString fDBName;
} ;

template <class PCEntry, class PCContainer, class HHEntry, class HHAppInfo, class Syncer>
class RecordConduit : public RecordConduitBase
{
public:
	/** Construct a record conduit on a given device link. */
	RecordConduit(const QString &name /**< Name presented to user */,
		KPilotDeviceLink *o /**< Connection to HH */,
		const char *n /**< Name for QObject */,
		const QStringList a = QStringList() /**< Flags */) :
		RecordConduitBase(o,n,a)
	{
		fConduitName=name;
	} ;
	virtual ~RecordConduit()
	{
	} ;

	virtual SyncProgress loadPC()
	{
		fAppInfo = new HHAppInfo(fDatabase) ;
		fContainer = new PCContainer();
		if (!fContainer->load())
		{
			emit logError(i18n("Unable to load the %1 database on the PC.").arg(fConduitName));
			return Error;
		}
		if (fContainer->isEmpty()) setFirstSync(true); /* And leave UID map empty */
		else fContainer->mapToRecords(fUIDMap);

		return Done;
	} ;

	virtual SyncProgress palmRecToPC()
	{
		if ( fIDListIterator == fIDList.end() )
		{
			return Done;
		}

		recordid_t currentID = *fIDListIterator++;
		PilotRecord *rec = fDatabase->readRecordById(currentID);
		HHEntry *currentHH = 0;
		PCEntry *currentPC = 0;
		Q_ASSERT(rec);
		if (fUIDMap.contains(currentID))
		{
			QString currentUID = fUIDMap[currentID];
			// This is a modified entry or it is deleted on the HH
			if (rec->isDeleted())
			{
				fContainer->remove(currentUID);
			}
			else
			{
				currentHH = new HHEntry(rec);
				currentPC = fContainer->get(currentUID);
				Syncer::sync(currentPC,currentHH,fAppInfo,HHtoPC);
			}
		}
		else
		{
			// Deleted on HH, unknown on PC -> Ignore it.
			// Not deleted, unknown -> New record.
			if (!rec->isDeleted())
			{
				currentHH = new HHEntry(rec);
				currentPC = new PCEntry();
				Syncer::sync(currentPC,currentHH,fAppInfo,HHtoPC);
				fContainer->insert(currentPC);
			}
		}
		delete rec;
		delete currentHH;
		// delete currentPC; Ownership passed to the container

		return NotDone;
	}

	virtual SyncProgress cleanup()
	{
		delete fAppInfo;
		fContainer->save();
		delete fContainer;
		return Done;
	}

	virtual bool getAppInfo( unsigned char *buffer, int appLen ) { return true; } ;

protected:
	HHAppInfo *fAppInfo;
	PCContainer *fContainer;
} ;


#endif

