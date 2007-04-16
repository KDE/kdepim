/* KPilot
**
** Copyright (C) 2006 by Bertjan Broeksema <b.broeksema@gmail.com>
**
** This file is the implementation of the PCToHHState.
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

#include "vcal-conduitbase.h"
#include "pctohhstate.h"
#include "cleanupstate.h"
#include "deleteunsyncedhhstate.h"

PCToHHState::PCToHHState()
{
	fState = ePCToHH;
}

PCToHHState::~PCToHHState()
{
}

void PCToHHState::startSync( ConduitAction *ca )
{
	FUNCTIONSETUP;
	VCalConduitBase *vccb = dynamic_cast<VCalConduitBase*>(ca);
	if( !vccb )
	{
		return;
	}
	
	DEBUGKPILOT << fname << ": Starting PCToHHState." << endl;

	// if we are asked to copy HH to PC, we shouldn't look for deleted records
	// on the Palm, since we've just copied them all.  =:)  Otherwise, look for
	// data on the palm that shouldn't be there and delete it if we find it....
	if ( vccb->syncMode() == ConduitAction::SyncMode::eCopyHHToPC )
	{
		fNextState = new CleanUpState();
	}
	else
	{
		fNextState = new DeleteUnsyncedHHState();
	}

	vccb->addLogMessage( i18n( "Copying records to Pilot ..." ) );

	fStarted = true;
	vccb->setHasNextRecord( true );
}

void PCToHHState::handleRecord( ConduitAction *ca )
{
	FUNCTIONSETUP;
	VCalConduitBase *vccb = dynamic_cast<VCalConduitBase*>(ca);
	if( !vccb )
	{
		return;
	}

	KCal::Incidence *e = 0L;

	if( vccb->isFullSync() )
	{
		e = vccb->privateBase()->getNextIncidence();
	}
	else
	{
		e = vccb->privateBase()->getNextModifiedIncidence();
	}

	// No more incidences to sync
	if( !e )
	{
		vccb->setHasNextRecord( false );
		return;
	}
	
	// let subclasses do something with the event
	vccb->preIncidence( e );

	// find the corresponding index on the palm and sync. If there is none, 
	// create it.
	recordid_t id = e->pilotId();
	
	DEBUGKPILOT << fname << ": found PC entry with pilotID " << id <<endl;
	DEBUGKPILOT << fname << ": Description: " << e->summary() << endl;

	QDateTime start_time = e->dtStart();
	QDateTime end_time = e->dtEnd();
	DEBUGKPILOT << fname << ": Time: "<< start_time.toString() << " until "
		<< end_time.toString() << endl;

	PilotRecord *s = 0L;

	if( id > 0 && ( s = vccb->database()->readRecordById( id ) ) )
	{
		if( e->syncStatus() == KCal::Incidence::SYNCDEL )
		{
			vccb->deletePalmRecord( e, s );
		}
		else
		{
			vccb->changePalmRecord( e, s );
		}

		KPILOT_DELETE( s );
	} else {
#ifdef DEBUG
		if (id > 0 )
		{
			DEBUGKPILOT << "-------------------------------------------------"
				<< "--------------------------" << endl;
			DEBUGKPILOT << fname << ": Could not read palm record with ID "
				<< id << endl;
		}
#endif
		vccb->addPalmRecord( e );
	}
}

void PCToHHState::finishSync( ConduitAction *ca )
{
	FUNCTIONSETUP;
	
	VCalConduitBase *vccb = dynamic_cast<VCalConduitBase*>(ca);
	if( !vccb )
	{
		return;
	}

	DEBUGKPILOT << fname << ": Finished PCToHHState." << endl;
	vccb->setState( fNextState );
}
