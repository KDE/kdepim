/* kpilotlink.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
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
static const char *kpilotlink_id =
	"$Id$";

#include "options.h"

#include <pi-source.h>
#include <pi-socket.h>
#include <pi-dlp.h>
#include <pi-file.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream.h>

#include <qdir.h>
#include <qtimer.h>
#include <qsocketnotifier.h>

#include <kconfig.h>
#include <kmessagebox.h>

#include "kpilotConfig.h"
#include "pilotUser.h"

#include "kpilotlink.moc"

KPilotDeviceLink::KPilotDeviceLink() :
	KPilotLink("deviceLink"),
	fPilotPath(QString::null),
	fTransientDevice(false),
	fOpenTimer(0L),
	fSocketNotifier(0L),
	fCurrentPilotSocket(-1),
	fStatus(Init)
{
	FUNCTIONSETUP;
}

KPilotDeviceLink::~KPilotDeviceLink()
{
	FUNCTIONSETUP;

	close();
}

void KPilotDeviceLink::close()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fOpenTimer);
	KPILOT_DELETE(fSocketNotifier);

	pi_close(fCurrentPilotSocket);
	pi_close(fPilotMasterSocket);
}

void KPilotDeviceLink::reset(const QString &dP)
{
	FUNCTIONSETUP;

	fStatus=Init;

	// Release all resources
	//
	//
	KPILOT_DELETE(fOpenTimer);
	KPILOT_DELETE(fSocketNotifier);

	fPilotPath=dP;
	if (fPilotPath.isEmpty()) return;

	fOpenTimer=new QTimer(this);
	QObject::connect(fOpenTimer,SIGNAL(timeout()),
		this,SLOT(openDevice()));
	fOpenTimer->start(1000,false);

	fStatus=WaitingForDevice;
}


void KPilotDeviceLink::openDevice()
{
	FUNCTIONSETUP;

	if (fTransientDevice)
	{
		if (!QFile::exists(fPilotPath))
		{
			return;
		}
	}

	if (fOpenTimer)
	{
		fOpenTimer->stop();
	}

	fStatus = FoundDevice;

	emit logMessage(i18n("Trying to open device ..."));
	if (open())
	{
		emit logMessage(i18n("Device link ready."));
	}
	else
	{
		emit logError(i18n("Could not open device: %1")
			.arg(fPilotPath));
	}
}

bool
KPilotDeviceLink::open()
{
	FUNCTIONSETUP;

	struct pi_sockaddr addr;
	int ret;
	int e = 0;
	QString msg;

	pi_close(fCurrentPilotSocket);

	fPilotMasterSocket = -1;
	if (fPilotPath.isEmpty())
	{
		kdWarning() << __FUNCTION__
			<< ": No point in trying empty device." << endl;

		msg = i18n("The Pilot device is not configured yet.");
		e = 0;
		goto errInit;
	}

	if (!(fPilotMasterSocket =
			pi_socket(PI_AF_SLP, PI_SOCK_STREAM, PI_PF_PADP)))
	{
		e = errno;
		msg = i18n("Cannot create socket for communicating "
			"with the Pilot");
		goto errInit;
	}

	DEBUGDAEMON << fname
		<< ": Binding to path "
		<< fPilotPath
		<< endl;

	addr.pi_family = PI_AF_SLP;
	strcpy(addr.pi_device, QFile::encodeName(fPilotPath));

	ret = pi_bind(fPilotMasterSocket,
		(struct sockaddr *) &addr, sizeof(addr));

	DEBUGDAEMON << fname
		<< ": Got master "
		<< fPilotMasterSocket
		<< endl;

	if (ret >= 0)
	{
		fStatus = DeviceOpen;

		fSocketNotifier = new QSocketNotifier(fPilotMasterSocket,
			QSocketNotifier::Read,
			this);
		QObject::connect(fSocketNotifier,SIGNAL(activated(int)),
			this,SLOT(acceptDevice()));
		return true;
	}
	else
	{
		e = errno;
		msg = i18n("Cannot open Pilot port \"%1\". ");
		// goto errInit;
	}


// We arrive here when some action for initializing the sockets
// has gone wrong, and we need to log that and alert the user
// that it has gone wrong.
//
//
errInit:
	if (fPilotMasterSocket != -1)
	{
		pi_close(fPilotMasterSocket);
	}

	fPilotMasterSocket = -1;

	if (msg.contains('%'))
	{
		if (fPilotPath.isEmpty())
		{
			msg = msg.arg(i18n("(empty)"));
		}
		else
		{
			msg = msg.arg(fPilotPath);
		}
	}
	switch (e)
	{
	case ENOENT:
		msg += i18n(" The port does not exist.");
		break;
	case ENODEV:
		msg += i18n(" These is no such device.");
		break;
	case EPERM:
		msg += i18n(" You don't have permission to open the "
			"Pilot device.");
		break;
	default:
		msg += i18n(" Check Pilot path and permissions.");
	}

	// OK, so we may have to deal with a translated 
	// error message here. Big deal -- we have the line
	// number as well, right?
	//
	//
	kdError() << __FUNCTION__ << ": " << msg << endl;
	if (e)
	{
		kdError() << __FUNCTION__
			<< ": (" << strerror(e) << ")" << endl;
	}

	fStatus = PilotLinkError;
	emit logError(msg);
}

void KPilotDeviceLink::acceptDevice()
{
	FUNCTIONSETUP;

	int ret;

	if (fSocketNotifier)
	{
		fSocketNotifier->setEnabled(false);
	}

	DEBUGDAEMON << fname
		<< ": Current status "
		<< statusString()
		<< " and master "
		<< fPilotMasterSocket
		<< endl;

	if ((fStatus != DeviceOpen) || (fPilotMasterSocket == -1))
	{
		fStatus=PilotLinkError;
		kdError() << __FUNCTION__
			<< ": Already connected or unable to connect!"
			<< endl;
		return;
	}

	emit logMessage(i18n("Starting HotSync..."));
	emit logProgress(i18n("Reading user information..."), 0);


	ret = pi_listen(fPilotMasterSocket, 1);
	if (ret == -1)
	{
		fStatus=PilotLinkError;
		kdError() << __FUNCTION__
			<< ": pi_listen failed: " << perror << endl;
		return;
	}

	fCurrentPilotSocket = pi_accept(fPilotMasterSocket, 0, 0);
	if (fCurrentPilotSocket == -1)
	{
		fStatus=PilotLinkError;
		kdError() << __FUNCTION__
			<< ": pi_accept failed: " << perror << endl;
		return;
	}

	emit logProgress(QString::null,30);

	fPilotUser = new KPilotUser;

	/* Ask the pilot who it is.  And see if it's who we think it is. */
	dlp_ReadUserInfo(fCurrentPilotSocket, fPilotUser->pilotUser());
	fPilotUser->boundsCheck();

	emit logProgress(i18n("Checking last PC..."),70);

	/* Tell user (via Pilot) that we are starting things up */
	if (dlp_OpenConduit(fCurrentPilotSocket) < 0)
	{
		fStatus=SyncDone;
		emit logMessage(i18n("Exiting on cancel. All data not restored."));
		return;
	}

	fStatus = AcceptedDevice;

	emit logProgress(QString::null,100);

	addSyncLogEntry("Sync started with KPilot-v" KPILOT_VERSION "\n");

	emit deviceReady();
}

void KPilotDeviceLink::addSyncLogEntry(const QString &entry)
{
	dlp_AddSyncLogEntry(fCurrentPilotSocket, const_cast<char *>(entry.latin1()));
	emit logMessage(entry);
}

QString KPilotDeviceLink::statusString() const
{
	QString s("KPilotDeviceLink=");


	switch(fStatus)
	{
	case Init : s.append("Init"); break;
	case WaitingForDevice : s.append("WaitingForDevice"); break;
	case FoundDevice : s.append("FoundDevice"); break;
	case DeviceOpen : s.append("DeviceOpen"); break;
	case AcceptedDevice : s.append("AcceptedDevice"); break;
	case SyncDone : s.append("SyncDone"); break;
	case PilotLinkError : s.append("PilotLinkError"); break;
	}

	return s;
}


// $Log$
