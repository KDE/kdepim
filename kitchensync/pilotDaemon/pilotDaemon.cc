/* pilotDaemon.cc                   KPilot
**
** Copyright (C) 2001 by Adriaan de Groot
**
** This program fits into the KDE hardware device-sync architecture
** and provides Sync capabilities for Palm platform devices on a
** serial port.
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

static const char *pilotDaemon_id = "$Id:$";

#include <config.h>
#include <lib/debug.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <qsocketnotifier.h>
#include <qtimer.h>
#include <qfile.h>

#include <kdebug.h>

#include <pilot-link/include/pi-socket.h>
#include <pilot-link/include/pi-dlp.h>
#include <syncManager/syncManagerIface_stub.h>

#include "configdialog.h"
#include "backupConduit.h"
#include "pilotSerialDatabase.h"

#include "pilotDaemon.moc"

PilotDaemon::PilotDaemon(Config *c) :
	fSocket(0),
	fSocketNotifier(0L),
	fStub(0L),
	fConfig(c),
	fU(0),
	fSys(0)
{
	fStub = new SyncManagerIface_stub("kitchenSync","SyncManager");
	fState = Unconnected;
}

bool PilotDaemon::connectToManager()
{
	FUNCTIONSETUP;

	// Register with sync manager
	if (!fStub->requestDevice("PilotDaemon",fConfig->device()))
	{
		kdWarning() << "Can't register with sync manager." << endl;
		fStub->addWarning(
			i18n("Can't register with the Sync manager."),
			QString::null);
		return false;
	}

	fState=Connected;

	QTimer::singleShot(1000,this,SLOT(openDevice()));

	return true;
}

void PilotDaemon::openDevice()
{
	if (fConfig->isTransientDevice())
	{
		if (QFile::exists(fConfig->device()))
		{
			if (open())
			{
				fStub->addLog(i18n("PilotDaemon ready."));
			}
		}
		else
		{
			QTimer::singleShot(1000,this,SLOT(openDevice()));
		}
	}
	else
	{
		if (open())
		{
			fStub->addLog(i18n("PilotDaemon ready."));
		}
	}
}

bool PilotDaemon::open()
{
	FUNCTIONSETUP;

	struct pi_sockaddr addr;
	strcpy(addr.pi_device, "/dev/ttyS0");
	if (!(fSocket = pi_socket(PI_AF_SLP, PI_SOCK_STREAM, PI_PF_PADP))) 
	{
		char *s=strerror(errno);

		kdWarning() << "pi_socket: " << s << endl;

		fStub->addWarning(i18n("Can't open Pilot socket."),
			QString(s));
		return false;
	}
	addr.pi_family = PI_AF_SLP;

	int ret = pi_bind(fSocket, (struct sockaddr *) &addr, sizeof(addr));
	if (ret == -1) 
	{
		char *s=strerror(errno);

		kdWarning() << "pi_bind: " << s << endl;

		fStub->addWarning(i18n("Can't bind to Pilot socket."),
			QString(s));
		pi_close(fSocket);
		return false;
	}

	fSocketNotifier = new QSocketNotifier(fSocket,
		QSocketNotifier::Read,
		this);
	QObject::connect(fSocketNotifier,SIGNAL(activated(int)),
		this,SLOT(accept()));

	return true;
}

bool PilotDaemon::accept()
{
	fSocketNotifier->setEnabled(false);

        int ret = pi_listen(fSocket, 1);
        if (ret == -1) 
	{
		char *s=strerror(errno);

		kdWarning() << "pi_listen: " << s << endl;

		fStub->addWarning(i18n("Can't listen on Pilot socket."),
			QString(s));

		return false;
        }
 
        fSocket = pi_accept(fSocket, 0, 0);
        if (fSocket == -1) 
	{
		char *s=strerror(errno);

		kdWarning() << "pi_accept: " << s << endl;

		fStub->addWarning(i18n("Can't accept Pilot."),
			QString(s));

		return false;
        }

	fStub->startSync("PilotDaemon");
 
	readUser();
	readDatabases();

	dlp_EndOfSync(fSocket, 0);
	fStub->endSync("PilotDaemon");

        pi_close(fSocket);

	fStub->releaseDevice("PilotDaemon","/dev/ttyS0");

	return true;
}

void PilotDaemon::readUser()
{
	fStub->setProgress(10,i18n("Reading user information..."));

        /* Ask the pilot who it is. */
	fU = new struct PilotUser;
	fSys = new struct SysInfo;
        dlp_ReadUserInfo(fSocket, fU);

	QString s(i18n("Read user %1...").arg(fU->username));
	fStub->addLog(s);

	fStub->setProgress(50,QString::null);

	dlp_ReadSysInfo(fSocket,fSys);

	fStub->setProgress(100,i18n("Read user information."));
}

void PilotDaemon::readDatabases()
{
	int i;
	int dbindex=0;
	int count=0;
	struct DBInfo db;

	BackupConduit buc(fStub);

	while ((i=dlp_ReadDBList(fSocket,0,dlpDBListRAM,dbindex,&db)) > 0)
	{
		count++;
		dbindex=db.index+1;

		PilotSerialDatabase *sd = 
			new PilotSerialDatabase(fSocket,db.name);
		if (sd)
		{
			buc.exec(sd);
			delete sd;
		}
	}
}
// $Log:$
