/* syncManager.cc               KitchenSync
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

#include "lib/debug.h"

#include <kmessagebox.h>

#include "systray.h"

#include "syncManager.moc"


SyncManager::SyncManager() :
	QObject(0,"syncManager"),
	DCOPObject("SyncManager"),
	fState(Idle)
{
	FUNCTIONSETUP;
}


/* virtual DCOP */ bool SyncManager::requestDevice(QString daemon,
	QString device)
{
	FUNCTIONSETUP;

	if (fState==Idle)
	{
		DEBUGMANAGER << "Device " << device << " claimed by "
			<< daemon << endl;
		fState=DeviceGranted;
		fDeviceOwner=daemon;
		return true;
	}
	else
	{
		kdWarning() << "Device request rejected for " 
			<< daemon  << endl;
		return false;
	}
}

/* virtual DCOP */ bool SyncManager::startSync(QString daemon)
{
	FUNCTIONSETUP;

	if ((fState==DeviceGranted) && (daemon==fDeviceOwner))
	{
		DEBUGMANAGER << "Sync start accepted." << endl;

		emit syncStarted(fDeviceOwner);
		fState=SyncStarted;
		return true;
	}
	else
	{
		kdWarning() << "Sync start rejected, state="
			<< fState << endl;
		return false;
	}
}

/* virtual DCOP */ bool SyncManager::endSync(QString daemon)
{
	FUNCTIONSETUP;

	if ((fState==SyncStarted) && (daemon==fDeviceOwner))
	{
		DEBUGMANAGER << "Sync end accepted." << endl;

		emit syncEnded(fDeviceOwner);
		fState=SyncEnded;
		return true;
	}
	else
	{
		kdWarning() << "Sync end rejected, state="
			<< fState << endl;
		return false;
	}
}

/* virtual DCOP */ bool SyncManager::releaseDevice(QString daemon,
	QString device)
{
	FUNCTIONSETUP;

	if ((fState==SyncEnded) && (fDeviceOwner==daemon))
	{
		DEBUGMANAGER << "Device " << device << " released by "
			<< daemon << endl;
		fState=Idle;
		fDeviceOwner=QString::null;
		return true;
	}
	else
	{
		kdWarning() << "Device release rejected for " 
			<< daemon  << endl;
		return false;
	}
}

/* virtual DCOP */ void SyncManager::setProgress(int p,QString m)
{
	FUNCTIONSETUP;

	if ((p>=0) && (p<=100))
	{
		emit syncProgress(p);
	}

	if (!m.isEmpty())
	{
		emit syncMessage(m);
	}
}

/* virtual DCOP */ void SyncManager::addLog(QString m)
{
	emit syncLog(m);
}

/* virtual DCOP */ void SyncManager::addWarning(QString m, QString n)
{
	if (n.isEmpty())
	{
		kdWarning() << m << endl;
		KMessageBox::sorry(0, m);
	}
	else
	{
		kdWarning() << m << " (" << n << ")" << endl;
		KMessageBox::detailedSorry(0,m,n);
	}
}

/* virtual DCOP */ QString SyncManager::status()
{
	QString s;

	switch(fState)
	{
	case Idle:
		s=QString("Idle");
		break;
	case DeviceGranted:
		s=QString("DeviceGranted");
		break;
	case SyncStarted:
		s=QString("SyncStarted");
		break;
	case SyncEnded:
		s=QString("SyncEnded");
		break;
	}

	if (fState!=Idle)
	{
		s.append(" ");
		s.append(fDeviceOwner);
	}

	return s;
}
