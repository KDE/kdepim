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



#endif
