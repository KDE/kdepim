#ifndef _KPILOT_KPILOTDCOP_H
#define _KPILOT_KPILOTDCOP_H
/* kpilotDCOP.h			KPilotDaemon
**
** Copyright (C) 2000 by Adriaan de Groot
**
** This file defines the DCOP interface for KPilot.
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


#include <dcopobject.h>


class KPilotDCOP : virtual public DCOPObject
{
	K_DCOP

public:
	enum DaemonMessages {
		None=0,
		StartOfHotSync=1,
		EndOfHotSync=2,
		DaemonQuit=4 } ;
	enum KPilotStatus {
		Normal,
		Startup,
		WaitingForDaemon,
		UIBusy,
		Error } ;

k_dcop:
	/**
	* This is the method the daemon uses to report
	* changes in its state.
	*/
	virtual ASYNC daemonStatus(int) = 0;

	/**
	* This is the method the daemon uses to popup
	* the configure dialog.
	*/
	virtual ASYNC configure() = 0;

	/**
	* This is the method the daemon uses to popup
	* the configure conduits dialog.
	*/
	virtual ASYNC configureConduits() = 0;

	/**
	* Report KPilot's state back to the daemon.
	*/
	virtual int kpilotStatus() = 0;
} ;



#endif
