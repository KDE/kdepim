/* KPilot
**
** Copyright (C) 2006 by Bertjan Broeksema <b.broeksema@gmail.com>
**
** This file is the implementation of the TestState.
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

#include <qdatetime.h>
#include <qfile.h>

#include "pilotSerialDatabase.h"
#include "pilotLocalDatabase.h"
#include "pilotDateEntry.h"

#include "teststate.h"
#include "vcal-conduitbase.h"

TestState::TestState() : fCalendar( QString::null )
{
	fState = eTest;
}

TestState::~TestState()
{
	FUNCTIONSETUP;
}

void TestState::startSync( ConduitAction *ca )
{
	FUNCTIONSETUP;
	
	VCalConduitBase *vccb = dynamic_cast<VCalConduitBase*>(ca);
	if( !vccb )
	{
		return;
	}
	
	DEBUGKPILOT << fname << ": Starting teststate." << endl;

	vccb->setHasNextRecord( true );
	fPilotindex = 0;
	fStarted = true;
}

void TestState::handleRecord( ConduitAction *ca )
{
	FUNCTIONSETUP;

	VCalConduitBase *vccb = dynamic_cast<VCalConduitBase*>(ca);
	if( !vccb )
	{
		return;
	}

	DEBUGKPILOT << fname << ": Handling record " << fPilotindex << endl;

	PilotRecord *record = vccb->readRecordByIndex( fPilotindex );
	
	if( record )
	{
		KCal::Incidence *i = vccb->incidenceFromRecord( record );
		fCalendar.addIncidence( i );
	
		KPILOT_DELETE(record);
	
		// Schedule more work.
		++fPilotindex;
	}
	else
	{
		vccb->setHasNextRecord( false );
	}
}

void TestState::finishSync( ConduitAction *ca )
{
	FUNCTIONSETUP;
	
	VCalConduitBase *vccb = dynamic_cast<VCalConduitBase*>(ca);
	if( !vccb )
	{
		return;
	}

	DEBUGKPILOT << fname << ": finishing teststate." << endl;

	// No more records present on the device so lets dump the
	// readed records in a file.
	QFile f( CSL1("dump.ics") );
	if( !f.exists() )
	{
		f.open( IO_WriteOnly );
		f.close();
	}

	if( !fCalendar.save( CSL1("dump.ics") ) )
	{
		DEBUGKPILOT << fname << ": Can't save calendar file." << endl;
	}

	fCalendar.close();

	vccb->setState( 0L );
}
