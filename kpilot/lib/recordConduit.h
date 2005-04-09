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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <qtimer.h>

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
	RecordConduitBase(KPilotDeviceLink *o,
		const char *n,
		const QStringList a = QStringList()) :
		ConduitAction(o,n,a)
	{
	} ;
	virtual ~RecordConduitBase()
	{
	} ;

	typedef enum { HHtoPC=0, PCtoHH=1, Both=2 } SyncDirection;

protected slots:
	virtual void slotPalmRecToPC();
	virtual void slotCleanup();
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

	bool exec()
	{
		FUNCTIONSETUP;
		if (!_prepare()) return false;
		setFirstSync(false);

		bool retrieved = false;
		if (!openDatabases( fDBName, &retrieved))
		{
			emit logError(i18n("Unable to open the %1 database on the handheld.").arg( fDBName ) );
			return false;
		}
		if (retrieved) setFirstSync(true);

		fAppInfo = new HHAppInfo(fDatabase) ;
		fContainer = new PCContainer();
		if (!fContainer->load())
		{
			emit logError(i18n("Unable to load the %1 database on the PC.").arg(fConduitName));
			return false;
		}
		if (fContainer->isEmpty()) setFirstSync(true); /* And leave UID map empty */
		else fContainer->mapToRecords(fUIDMap);

		if (isFirstSync()) fIDList=fDatabase->idList();
		else fIDList=fDatabase->modifiedIDList();
		fIDListIterator = fIDList.begin();

		QTimer::singleShot(0,this,SLOT(slotPalmRecToPC()));
		return true;
	} ;

	virtual void slotPalmRecToPC()
	{
		if ( (getSyncDirection() == SyncAction::eCopyPCToHH) || (fIDListIterator == fIDList.end()) )
		{
			QTimer::singleShot(0,this,SLOT(cleanup()));
			return;
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

		QTimer::singleShot(0,this,SLOT(slotPalmRecToPC()));
	}

	virtual void cleanup()
	{
		delete fAppInfo;
		fContainer->save();
		delete fContainer;
	}

	virtual bool prepare() { return true; } ;
	virtual bool getAppInfo( unsigned char *buffer, int appLen ) { return true; } ;

protected:
	QMap<recordid_t,QString> fUIDMap;
	RecordIDList fIDList;
	RecordIDList::Iterator fIDListIterator;

	HHAppInfo *fAppInfo;
	PCContainer *fContainer;

	QString fDBName;

private:
	bool _prepare()
	{
		return prepare();
	} ;

} ;


#endif

