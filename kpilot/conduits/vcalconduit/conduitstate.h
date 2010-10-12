#ifndef _KPILOT_CONDUITSTATE_H
#define _KPILOT_CONDUITSTATE_H
/* vcal-conduitstate.h                       KPilot
**
** Copyright (C) 2006 Bertjan Broeksema
**
** This file defines the vcal-conduitstate.
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

#include "plugin.h"

/**
 * This class defines the current state of the vcal-conduitbase. Subclasses of
 * this class can do the things that are needed, in methods defined here, for 
 * the state that they define.
 */
class ConduitState
{
public:
	enum state_t {
		eTest,
		eInit,
		ePCToHH,
		eHHToPC,
		eDeleteUnsyncedHH,
		eDeleteUnsyncedPC,
		eCleanUp
	};

protected:
	state_t fState;
	bool fStarted;

public:
	ConduitState(){ fState = eInit; fStarted = false; };
	virtual ~ConduitState() {};

	/**
	 * Prepare for a sync in the current state. Don't forget to set fState to 
	 * true in this method. Otherwise the state won't handle records.
	 */
	virtual void startSync( ConduitAction * ) = 0;
	
	/**
	 * Sync the next record in row.
	 */
	virtual void handleRecord( ConduitAction * ) = 0;

	/**
	 * Clean up after all records are synced and enter next state.
	 */
	virtual void finishSync( ConduitAction * ) = 0;

	/**
	 * Returns the state type.
	 */
	state_t state() { return fState; };

	/**
	 * Returns wether or not this state has started.
	 */
	bool started() { return fStarted; };
};

#endif
