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
k_dcop:
	virtual ASYNC logMessage(QString) = 0;
	virtual ASYNC logError(QString)=0;
	virtual ASYNC logProgress(QString,int) = 0;
} ;



#endif
