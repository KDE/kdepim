#ifndef _KPILOT_LOGWIDGETDCOP_H
#define _KPILOT_LOGWIDGETDCOP_H
/* logWidgetDCOP.h			KPilotDaemon
**
** Copyright (C) 2000 by Adriaan de Groot
**
** This file defines the DCOP interface for the generalized log widget.
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


class LoggerDCOP : virtual public DCOPObject
{
	K_DCOP

k_dcop:
	// Adds a single message to the log, with formatting 
	// showing it's an error message.
	virtual ASYNC logError(QString) = 0L ;
	// Adds a regular message.
	virtual ASYNC logMessage(QString) = 0L ;
	// Adds a message if the string is non-null, and
	// sets the progress bar to @p n%. Limit @p n
	// to the range 0 .. 100.
	virtual ASYNC logProgress(QString,int n) = 0L ;
} ;



// $Log$
// Revision 1.8  2002/11/27 21:29:06  adridg
// See larger ChangeLog entry
//
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
