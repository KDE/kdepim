/* KPilot
**
** Copyright (C) 2006 by Bertjan Broeksema <b.broeksema@gmail.com>
**
** This file is the implementation of the InitState.
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

#include "vcal-conduitbase.h"
#include "initstate.h"
#include "teststate.h"
#include "pctohhstate.h"
#include "hhtopcstate.h"

InitState::InitState()
{
	fState = eInit;
}

InitState::~InitState()
{
}

void InitState::startSync( ConduitAction *ca )
{
	FUNCTIONSETUP;

	VCalConduitBase *vccb = dynamic_cast<VCalConduitBase*>(ca);
	if( !vccb )
	{
		return;
	}

	DEBUGKPILOT << fname << ": Starting InitState." << endl;

	vccb->addLogMessage( i18n( "Initializing conduit ..." ) );
	vccb->preSync();

	if ( vccb->syncMode().isTest() )
	{
		fNextState = new TestState();
	}
	else
	{
		switch( vccb->syncMode().mode() )
		{
		case ConduitAction::SyncMode::eCopyPCToHH:
			// TODO: Clear the palm and backup database??? Or just add the
			// new items ignore the Palm->PC side and leave the existing items
			// on the palm?
			fNextState = new PCToHHState();
			break;
		case ConduitAction::SyncMode::eCopyHHToPC:
			// TODO: Clear the backup database and the calendar, update fP
			//       or just add the palm items and leave the PC ones there????
			fNextState = new HHToPCState();
			break;
		default:
			fNextState = new HHToPCState();
			break;
		}
	}

	fStarted = true;
	vccb->setHasNextRecord( false );
}

void InitState::handleRecord( ConduitAction *vccb )
{
	FUNCTIONSETUP;
	Q_UNUSED(vccb);
}

void InitState::finishSync( ConduitAction *ca )
{
	FUNCTIONSETUP;

	VCalConduitBase *vccb = dynamic_cast<VCalConduitBase*>(ca);
	if( !vccb )
	{
		return;
	}

	DEBUGKPILOT << fname << ": Finished InitState." << endl;
	vccb->setState( fNextState );
}
