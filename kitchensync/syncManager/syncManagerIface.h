/* syncManagerIface.h           KitchenSync
**
** Copyright (C) 2001 by Adriaan de Groot
**
** This program is part of KitchenSync, the KDE handheld-device
** synchronisation framework.
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

#ifndef _SYNCMANAGERIFACE_H
#define _SYNCMANAGERIFACE_H

#include <dcopobject.h>

class SyncManagerIface : virtual public DCOPObject
{
K_DCOP

k_dcop:
	virtual bool requestDevice(QString,QString) = 0;
	virtual bool startSync(QString) = 0;
	virtual bool endSync(QString) = 0;
	virtual bool releaseDevice(QString,QString) = 0;

// void announceSyncTypes(QString,QStringList);

	virtual ASYNC setProgress(int,QString) = 0;
	virtual ASYNC addLog(QString) = 0;
	virtual ASYNC addWarning(QString,QString) = 0;

	virtual QString status() = 0;
} ;

#endif


// $Log:$
