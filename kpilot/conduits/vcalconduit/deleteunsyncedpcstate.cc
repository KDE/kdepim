/* KPilot
**
** Copyright (C) 2006 by Bertjan Broeksema <b.broeksema@gmail.com>
**
** This file is the implementation of the DeleteUnsyncedPCState.
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
#include <plugin.h>

#include "pilotDatabase.h"
#include "pilotRecord.h"

#include "vcal-conduitbase.h"
#include "deleteunsyncedpcstate.h"
#include "cleanupstate.h"

DeleteUnsyncedPCState::DeleteUnsyncedPCState()
{
	fState = eDeleteUnsyncedPC;
}

DeleteUnsyncedPCState::~DeleteUnsyncedPCState()
{
}

void DeleteUnsyncedPCState::startSync( ConduitAction *ca )
{
	FUNCTIONSETUP;

	VCalConduitBase *vccb = dynamic_cast<VCalConduitBase*>(ca);
	if( !vccb )
	{
		return;
	}

	DEBUGKPILOT << fname << ": Starting DeleteUnsyncedPCState." << endl;
	
	fPilotIndex = 0;
	fNextState = new CleanUpState();
	
	vccb->setHasNextRecord( true );
	fStarted = true;
}

void DeleteUnsyncedPCState::handleRecord( ConduitAction *ca )
{
	FUNCTIONSETUP;

	VCalConduitBase *vccb = dynamic_cast<VCalConduitBase*>(ca);
	if( !vccb )
	{
		return;
	}

	KCal::Incidence *e = 0L;
	e = vccb->privateBase()->getNextIncidence();

	// if we don't have a record, then we can't do anything.  also, if
	// we're copying everything from the PC to our handheld, then we're
	// guaranteed not to have anything extra on our PC that's not on
	// our handheld that needs to get deleted, so we can return in that
	// case too...

	if( !e || ( vccb->syncMode().mode() == ConduitAction::SyncMode::eCopyPCToHH ) )
	{
		vccb->setHasNextRecord( false );
		return;
	}


	// try to find the corresponding index on the palm.  if we can't
	// find it, then we have a pc record that needs to be deleted.
	recordid_t id = e->pilotId();
	
	PilotRecord *s = 0L;

	if( id > 0 )
	{
		s = vccb->database()->readRecordById( id );
	}

	// if we either have a pc record with no palm id or if we can't
	// find a palm record that matches, then we need to delete this PC
	// record.
	if ( id <=0 || !s )
	{
#ifdef DEBUG
		DEBUGKPILOT << fname << ": found PC entry with pilotID: [" << id 
			<< "], Description: [" << e->summary() 
			<< "], Time: ["<< e->dtStart().toString() << "] until: ["
			<< e->dtEnd().toString() << "]. Can't find it on Palm, "
			<< "so I'm deleting it from the local calendar." << endl;
#endif
		vccb->privateBase()->removeIncidence(e);
	}

	KPILOT_DELETE( s );

}

void DeleteUnsyncedPCState::finishSync( ConduitAction *ca )
{
	FUNCTIONSETUP;

	VCalConduitBase *vccb = dynamic_cast<VCalConduitBase*>(ca);
	if( !vccb )
	{
		return;
	}

	DEBUGKPILOT << fname << ": Finishing DeleteUnsyncedPCState." << endl;
	vccb->setState( fNextState );
}
