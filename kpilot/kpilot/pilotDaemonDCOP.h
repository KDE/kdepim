/* pilotDaemonDCOP.h			KPilotDaemon
**
** Copyright (C) 2000 by Adriaan de Groot
**
** This file defines the DCOP interface for
** the KPilotDaemon. The daemon has *two* interfaces:
** one belonging with KUniqueApplication and this one.
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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/

#ifndef PILOTDAEMONDCOP_H
#define PILOTDAEMONDCOP_H

#include <dcopobject.h>


class PilotDaemonDCOP : virtual public DCOPObject
{
	K_DCOP
k_dcop:
	/**
	* Start a HotSync. What kind of HotSync is determined
	* by the int parameter:
	*
	* 1 regular HotSync -> backup databases and run conduits
	* 2 FastSync -> only backup databases with conduits
	* 3 FullBackup -> backup, no conduits
	*/
	virtual ASYNC startHotSync(int) = 0;
} ;

#endif


// $Log$
// Revision 1.2  2001/02/06 08:05:20  adridg
// Fixed copyright notices, added CVS log, added surrounding #ifdefs. No code changes.
//
