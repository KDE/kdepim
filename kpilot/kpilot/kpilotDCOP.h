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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
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

k_dcop:
	/**
	* This is the method the daemon uses to report
	* changes in its state.
	*/
	virtual ASYNC daemonStatus(int) = 0;
} ;



// $Log$
// Revision 1.7  2002/08/15 21:51:00  kainhofe
// Fixed the error messages (were not printed to the log), finished the categories sync of the todo conduit
//
// Revision 1.6  2001/11/18 16:59:55  adridg
// New icons, DCOP changes
//
// Revision 1.5  2001/09/29 16:26:18  adridg
// The big layout change
//
// Revision 1.4  2001/09/07 20:48:13  adridg
// Stripped away last crufty IPC, added logWidget
//
// Revision 1.3  2001/04/16 13:48:35  adridg
// --enable-final cleanup and #warning reduction
//
// Revision 1.2  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.1  2001/03/04 22:22:29  adridg
// DCOP cooperation between daemon & kpilot for d&d file install
//
#endif
