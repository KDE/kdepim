/* KPilot
**
** Copyright (C) 2006 by Bertjan Broeksema <b.broeksema@gmail.com>
**
** This file is the implementation of the DeleteUnsyncedHHState.
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
#include "deleteunsyncedhhstate.h"
#include "deleteunsyncedpcstate.h"
#include "cleanupstate.h"

DeleteUnsyncedHHState::DeleteUnsyncedHHState()
{
	fState = eDeleteUnsyncedHH;
}

DeleteUnsyncedHHState::~DeleteUnsyncedHHState()
{
}

void DeleteUnsyncedHHState::startSync( ConduitAction *ca )
{
	FUNCTIONSETUP;

	VCalConduitBase *vccb = dynamic_cast<VCalConduitBase*>(ca);
	if( !vccb )
	{
		return;
	}

	DEBUGKPILOT << fname << ": Starting DeleteUnsyncedHHState." << endl;
	
	fPilotIndex = 0;
	fNextState = new DeleteUnsyncedPCState();
	
	vccb->setHasNextRecord( true );
	fStarted = true;
}

void DeleteUnsyncedHHState::handleRecord( ConduitAction *ca )
{
	FUNCTIONSETUP;

	VCalConduitBase *vccb = dynamic_cast<VCalConduitBase*>(ca);
	if( !vccb )
	{
		return;
	}

	PilotRecord *r = vccb->localDatabase()->readRecordByIndex( fPilotIndex++ );
	// if either we don't have a record, or if we're copying everything
	// from the handheld to the pc, then we don't have anything to do
	// here.  the latter is because if we're copying HH->PC, then by
	// definition, we will have everything from the HH on the PC and
	// therefore can't possibly have anything that needs to be deleted
	// from it.
	if ( !r
		|| ( vccb->syncMode().mode() == ConduitAction::SyncMode::eCopyHHToPC ) )
	{
		vccb->setHasNextRecord( false );
		return;
	}

	KCal::Incidence *e = vccb->privateBase()->findIncidence( r->id() );
	if ( !e )
	{
		DEBUGKPILOT << "Didn't find incidence with id = " << r->id()
			<< ", deleting it" << endl;
		vccb->deletePalmRecord( NULL, r );
	}

	KPILOT_DELETE( r );
}

void DeleteUnsyncedHHState::finishSync( ConduitAction *ca )
{
	FUNCTIONSETUP;

	VCalConduitBase *vccb = dynamic_cast<VCalConduitBase*>(ca);
	if( !vccb )
	{
		return;
	}

	DEBUGKPILOT << fname << ": Finishing DeleteUnsyncedHHState." << endl;
	vccb->setState( fNextState );
}
