/* syncManager.h                KitchenSync
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

#ifndef _SYNCMANAGER_H
#define _SYNCMANAGER_H

#include <qstring.h>

#include "syncManagerIface.h"

class SyncManager : public QObject, virtual public SyncManagerIface
{
Q_OBJECT

public:
	SyncManager();

public /* DCOP */:
	virtual bool requestDevice(QString,QString);
	virtual bool startSync(QString);
	virtual bool endSync(QString);
	virtual bool releaseDevice(QString,QString);

	virtual void setProgress(int,QString);
	virtual void addLog(QString);
	virtual void addWarning(QString,QString);

	virtual QString status();

signals:
	void syncStarted(const QString &);
	void syncEnded(const QString &);

	void syncProgress(int);
	void syncMessage(const QString &);
	void syncLog(const QString &);

private:
	typedef enum { Idle, DeviceGranted,
		SyncStarted, SyncEnded } State;
	State fState;

	QString fDeviceOwner;
} ;

#endif

// $Log:$
