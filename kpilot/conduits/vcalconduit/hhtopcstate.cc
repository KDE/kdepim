/* KPilot
**
** Copyright (C) 2006 by Bertjan Broeksema <b.broeksema@gmail.com>
**
** This file is the implementation of the HHtoPCState
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

#include <options.h>

#include "pilotDatabase.h"
#include "pilotRecord.h"

#include "vcalconduitSettings.h"
#include "vcal-conduitbase.h"
#include "hhtopcstate.h"
#include "pctohhstate.h"
#include "cleanupstate.h"

HHToPCState::HHToPCState()
{
	fState = eHHToPC;
	fPilotindex = 0;
}

HHToPCState::~HHToPCState()
{
}

void HHToPCState::startSync( ConduitAction *ca )
{
	FUNCTIONSETUP;

	VCalConduitBase *vccb = dynamic_cast<VCalConduitBase*>(ca);
	if( !vccb )
	{
		return;
	}

	DEBUGKPILOT << fname << ": Starting HHToPCState." << endl;

	if ( vccb->syncMode() == ConduitAction::SyncMode::eCopyHHToPC )
	{
		fNextState = new CleanUpState();
	}
	else
	{
		fNextState = new PCToHHState();
	}

	fStarted = true;
	vccb->setHasNextRecord( true );
}

void HHToPCState::handleRecord( ConduitAction *ca )
{
	FUNCTIONSETUP;

	VCalConduitBase *vccb = dynamic_cast<VCalConduitBase*>(ca);
	if( !vccb )
	{
		return;
	}

	PilotRecord *r = 0L;
	PilotRecord *s = 0L;

	if ( vccb->isFullSync() )
	{
		r = vccb->database()->readRecordByIndex( fPilotindex++ );
	}
	else
	{
		r = vccb->database()->readNextModifiedRec();
	}

	if (!r)
	{
		vccb->privateBase()->updateIncidences();
		vccb->setHasNextRecord( false );
		return;
	}

	// let subclasses do something with the record before we try to sync
	vccb->preRecord( r );

	bool archiveRecord = ( r->isArchived() );
	s = vccb->localDatabase()->readRecordById( r->id() );
	
	if ( !s || vccb->isFirstSync() )
	{
#ifdef DEBUG
		if ( r->id() > 0 && !s )
		{
			DEBUGKPILOT << "-------------------------------------------------";
			DEBUGKPILOT << "--------------------------" << endl;
			DEBUGKPILOT << fname << ": Could not read palm record with ID ";
			DEBUGKPILOT << r->id() << endl;
		}
#endif
		if ( !r->isDeleted() 
			|| ( vccb->config()->syncArchived() && archiveRecord ) )
		{
			KCal::Incidence *e = vccb->addRecord( r );
			if ( vccb->config()->syncArchived() && archiveRecord )  {
				e->setSyncStatus( KCal::Incidence::SYNCDEL );
			}
		}
	}
	else
	{
		if ( r->isDeleted() )
		{
			if ( vccb->config()->syncArchived() && archiveRecord )
			{
				vccb->changeRecord( r, s );
			}
			else
			{
				vccb->deleteRecord( r, s );
			}
		}
		else
		{
			vccb->changeRecord( r, s );
		}
	}

	KPILOT_DELETE(r);
	KPILOT_DELETE(s);
}

void HHToPCState::finishSync( ConduitAction *ca )
{
	FUNCTIONSETUP;

	VCalConduitBase *vccb = dynamic_cast<VCalConduitBase*>(ca);
	if( !vccb )
	{
		return;
	}

	DEBUGKPILOT << fname << ": Finished HHToPCState." << endl;
	vccb->setState( fNextState );
}

/*
void VCalConduitBase::slotPalmRecToPC()
{
	FUNCTIONSETUP;

	PilotRecord *r;
	if (isFullSync())
	{
		r = fDatabase->readRecordByIndex(pilotindex++);
	}
	else
	{
		r = fDatabase->readNextModifiedRec();
	}
	PilotRecord *s = 0L;

	if (!r)
	{
		fP->updateIncidences();
		if ( syncMode()==SyncMode::eCopyHHToPC )
		{
			emit logMessage(i18n("Cleaning up ..."));
			QTimer::singleShot(0, this, SLOT(cleanup()));
			return;
		}
		else
		{
			emit logMessage(i18n("Copying records to Pilot ..."));
			QTimer::singleShot(0 ,this,SLOT(slotPCRecToPalm()));
			return;
		}
	}

	// let subclasses do something with the record before we try to sync
	preRecord(r);

//	DEBUGKPILOT<<fname<<": Event: "<<e->dtStart()<<" until "<<e->dtEnd()<<endl;
//	DEBUGKPILOT<<fname<<": Time: "<<e->dtStart()<<" until "<<e->dtEnd()<<endl;
	bool archiveRecord=(r->isArchived());

	s = fLocalDatabase->readRecordById(r->id());
	if (!s || isFirstSync())
	{
#ifdef DEBUG
		if (r->id()>0 && !s)
		{
			DEBUGKPILOT<<"---------------------------------------------------------------------------"<<endl;
			DEBUGKPILOT<< fname<<": Could not read palm record with ID "<<r->id()<<endl;
		}
#endif
		if (!r->isDeleted() || (config()->syncArchived() && archiveRecord))
		{
			KCal::Incidence*e=addRecord(r);
			if (config()->syncArchived() && archiveRecord)  {
				e->setSyncStatus(KCal::Incidence::SYNCDEL);
			}
		}
	}
	else
	{
		if (r->isDeleted())
		{
			if (config()->syncArchived() && archiveRecord)
			{
				changeRecord(r,s);
			}
			else
			{
				deleteRecord(r,s);
			}
		}
		else
		{
			changeRecord(r,s);
		}
	}

	KPILOT_DELETE(r);
	KPILOT_DELETE(s);

	QTimer::singleShot(0,this,SLOT(slotPalmRecToPC()));
}
*/
