#ifndef PILOTDAEMONDCOP_H
#define PILOTDAEMONDCOP_H
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


#ifndef _DCOPOBJECT_H
#include <dcopobject.h>
#endif


class PilotDaemonDCOP : virtual public DCOPObject
{
	K_DCOP
public:
	enum HotSyncTypes { Test=0,
		HotSync,
		FastSync,
		Backup,
		Restore } ;
k_dcop:
	/**
	* Start a HotSync. What kind of HotSync is determined
	* by the int parameter (use the enum above!):
	*
	* 0 default -> use whatever form of HotSync has most recently been requested
	* 1 regular HotSync -> backup databases and run conduits
	* 2 FastSync -> only backup databases with conduits
	* 3 FullBackup -> backup, no conduits
	* 4 FullRestore -> restore from local databases
	*/
	virtual ASYNC requestSync(int) = 0;
	virtual ASYNC requestFastSyncNext() = 0;
	virtual ASYNC requestRegularSyncNext() = 0;

	/**
	* Functions for the KPilot UI, indicating what the daemon
	* should do.
	*/
	virtual ASYNC quitNow() = 0;
	virtual ASYNC reloadSettings() =0; // Indicate changed config file.

	/**
	* Functions requesting the status of the daemon.
	*/
	virtual QString statusString() = 0;
} ;



// $Log$
// Revision 1.9  2001/09/16 13:37:48  adridg
// Large-scale restructuring
//
// Revision 1.8  2001/09/07 20:48:44  adridg
// New methods, moved #ifdefs. The KPilot Daemon is still broken, though.
//
// Revision 1.7  2001/08/27 22:54:27  adridg
// Decruftifying; improve DCOP link between daemon & viewer
//
// Revision 1.6  2001/08/19 19:25:57  adridg
// Removed kpilotlink dependency from kpilot; added DCOP interfaces to make that possible. Also fixed a connect() type mismatch that was harmless but annoying.
//
// Revision 1.5  2001/04/16 13:48:35  adridg
// --enable-final cleanup and #warning reduction
//
// Revision 1.4  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.3  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.2  2001/02/06 08:05:20  adridg
// Fixed copyright notices, added CVS log, added surrounding #ifdefs. No code changes.
//
#endif
