#ifndef _KPILOT_INITSTATE_H
#define _KPILOT_INITSTATE_H
/* initstate.h                       KPilot
**
** Copyright (C) 2006 Bertjan Broeksema
**
** This file defines the teststate for vcal-conduitbase.
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

#include "conduitstate.h"

class VCalConduitBase;

/**
 * State to test the vcal-conduit. @see vcal-conduitstate.h
 */
class InitState : public ConduitState
{
private:
	ConduitState *fNextState;

public:
	InitState();
	virtual ~InitState();

	virtual void startSync( ConduitAction *vccb );
	virtual void handleRecord( ConduitAction *vccb );
	virtual void finishSync( ConduitAction *vccb );
};

#endif
