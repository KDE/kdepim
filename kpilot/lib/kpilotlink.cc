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
** Bug reports and questions can be sent to kde-pim@kde.org
*/
static const char *kpilotlink_id = "$Id$";

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
#include <errno.h>

#include <qdir.h>
#include <qtimer.h>
#include <qdatetime.h>
#include <qsocketnotifier.h>

#include <kconfig.h>
#include <kmessagebox.h>

#include "pilotUser.h"

#include "kpilotlink.moc"



QDateTime readTm(const struct tm &t)
{
	QDateTime dt;
	dt.setDate(QDate(1900 + t.tm_year, t.tm_mon + 1, t.tm_mday));
	dt.setTime(QTime(t.tm_hour, t.tm_min, t.tm_sec));
	return dt;
 }
 
 struct tm writeTm(const QDateTime &dt)
 {
 	struct tm t;
 	t.tm_wday = 0; // unimplemented
	t.tm_yday = 0; // unimplemented
	t.tm_isdst = 0; // unimplemented
 
	t.tm_year = dt.date().year() - 1900;
	t.tm_mon = dt.date().month() - 1;
	t.tm_mday = dt.date().day();
	t.tm_hour = dt.time().hour();
	t.tm_min = dt.time().minute();
	t.tm_sec = dt.time().second();
	return t;
 }


KPilotDeviceLink *KPilotDeviceLink::fDeviceLink = 0L;

KPilotDeviceLink::KPilotDeviceLink(QObject * parent, const char *name) :
	QObject(parent, name),
	fStatus(Init),
	fPilotPath(QString::null),
	fDeviceType(None),
	fRetries(0),
	fOpenTimer(0L),
	fSocketNotifier(0L),
	fPilotMasterSocket(-1), 
	fCurrentPilotSocket(-1)
{
	FUNCTIONSETUP;

	ASSERT(fDeviceLink == 0L);
	fDeviceLink = this;

	(void) kpilotlink_id;
}

KPilotDeviceLink::~KPilotDeviceLink()
{
	FUNCTIONSETUP;
	close();
	fDeviceLink = 0L;
}

KPilotDeviceLink *KPilotDeviceLink::init(QObject * parent, const char *name)
{
	FUNCTIONSETUP;

	ASSERT(!fDeviceLink);

	return new KPilotDeviceLink(parent, name);
}

void KPilotDeviceLink::close()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fOpenTimer);
	KPILOT_DELETE(fSocketNotifier);
#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": Closing sockets "
		<< fCurrentPilotSocket
		<< " and "
		<< fPilotMasterSocket
		<< endl;
#endif
	if (fCurrentPilotSocket != -1)
	{
		pi_close(fCurrentPilotSocket);
		// It seems that pi_close doesn't release
		// the file descriptor, so do that forcibly.
		::close(fCurrentPilotSocket);
	}
	if (fPilotMasterSocket != -1)
	{
		pi_close(fPilotMasterSocket);
		::close(fPilotMasterSocket);
	}
	fPilotMasterSocket = (-1);
	fCurrentPilotSocket = (-1);
}

void KPilotDeviceLink::reset(DeviceType t, const QString & dP)
{
	FUNCTIONSETUP;

	fStatus = Init;
	fRetries = 0;

	// Release all resources
	//
	//
	close();
	fPilotPath = QString::null;

	fDeviceType = t;
	if (t == None)
		return;
	fDeviceType=OldStyleUSB;

	fPilotPath = dP;
	if (fPilotPath.isEmpty())
		return;

	reset();
}

void KPilotDeviceLink::reset()
{
	FUNCTIONSETUP;

	close();

	fOpenTimer = new QTimer(this);
	QObject::connect(fOpenTimer, SIGNAL(timeout()),
		this, SLOT(openDevice()));
	fOpenTimer->start(1000, false);

	fStatus = WaitingForDevice;
}

void KPilotDeviceLink::openDevice()
{
	FUNCTIONSETUP;

	if (isTransient())
	{
		if (!QFile::exists(fPilotPath))
		{
			if (QFile::exists(fPilotPath))
			{
				kdWarning() << k_funcinfo
					<< ": QFile::exists seems to create device node."
					<< endl;
			}
			return;
		}
	}

	// This transition (from Waiting to Found) can only be
	// taken once.
	//
	if (fStatus == WaitingForDevice)
	{
		fStatus = FoundDevice;
	}

	emit logMessage(i18n("Trying to open device..."));

	if (open())
	{
		emit logMessage(i18n("Device link ready."));
	}
	else
	{
		emit logError(i18n("Could not open device: %1").
			arg(fPilotPath));
		if (fStatus != PilotLinkError)
		{
			fOpenTimer->start(1000, false);
		}
	}
}

bool KPilotDeviceLink::open()
{
	FUNCTIONSETUP;

	struct pi_sockaddr addr;
	int ret;
	int e = 0;
	QString msg;

	if (fCurrentPilotSocket != -1)
	{
		// See note in KPilotDeviceLink::close()
		pi_close(fCurrentPilotSocket);
		::close(fCurrentPilotSocket);
	}
	fCurrentPilotSocket = (-1);

	if (fPilotMasterSocket == -1)
	{
		if (fPilotPath.isEmpty())
		{
			kdWarning() << k_funcinfo
				<< ": No point in trying empty device."
				<< endl;

			msg = i18n("The Pilot device is not configured yet.");
			e = 0;
			goto errInit;
		}

#if PILOT_LINK_MAJOR < 10
		if (!(fPilotMasterSocket = pi_socket(PI_AF_SLP,
					PI_SOCK_STREAM, PI_PF_PADP)))
#else
		if (!(fPilotMasterSocket = pi_socket(PI_AF_PILOT,
					PI_SOCK_STREAM, PI_PF_DLP)))
#endif
		{
			e = errno;
			msg = i18n("Cannot create socket for communicating "
				"with the Pilot");
			goto errInit;
		}

#ifdef DEBUG
		DEBUGDAEMON << fname
			<< ": Got master " << fPilotMasterSocket << endl;
#endif

		fStatus = CreatedSocket;
	}

	ASSERT(fStatus == CreatedSocket);

#ifdef DEBUG
	DEBUGDAEMON << fname << ": Binding to path " << fPilotPath << endl;
#endif

#if PILOT_LINK_MAJOR < 10
	addr.pi_family = PI_AF_SLP;
#else
	addr.pi_family = PI_AF_PILOT;
#endif
	strcpy(addr.pi_device, QFile::encodeName(fPilotPath));


	ret = pi_bind(fPilotMasterSocket,
		(struct sockaddr *) &addr, sizeof(addr));

	if (ret >= 0)
	{
		fStatus = DeviceOpen;
		fOpenTimer->stop();

		fSocketNotifier = new QSocketNotifier(fPilotMasterSocket,
			QSocketNotifier::Read, this);
		QObject::connect(fSocketNotifier, SIGNAL(activated(int)),
			this, SLOT(acceptDevice()));
		return true;
	}
	else
	{
#ifdef DEBUG
		DEBUGDAEMON << fname
			<< ": Tried "
			<< addr.pi_device
			<< " and got "
			<< strerror(errno)
			<< endl;
#endif

		if (isTransient() && (fRetries < 5))
		{
			return false;
		}
		e = errno;
		msg = i18n("Cannot open Pilot port \"%1\". ");

		fOpenTimer->stop();

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
		close();
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
	kdError() << k_funcinfo << ": " << msg << endl;
	if (e)
	{
		kdError() << k_funcinfo
			<< ": (" << strerror(e) << ")" << endl;
	}

	fStatus = PilotLinkError;
	emit logError(msg);
	return false;
}

void KPilotDeviceLink::acceptDevice()
{
	FUNCTIONSETUP;

	int ret;

	if (fSocketNotifier)
	{
		fSocketNotifier->setEnabled(false);
	}

#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": Current status "
		<< statusString()
		<< " and master " << fPilotMasterSocket << endl;
#endif

	ret = pi_listen(fPilotMasterSocket, 1);
	if (ret == -1)
	{
		char *s = strerror(errno);

		kdWarning() << "pi_listen: " << s << endl;

		emit logError(i18n("Can't listen on Pilot socket (%1)").
			arg(s));

		return;
	}

	fCurrentPilotSocket = pi_accept(fPilotMasterSocket, 0, 0);
	if (fCurrentPilotSocket == -1)
	{
		char *s = strerror(errno);

		kdWarning() << "pi_accept: " << s << endl;

		emit logError(i18n("Can't accept Pilot (%1)").arg(s));

		fStatus = PilotLinkError;
		return;
	}

	if ((fStatus != DeviceOpen) || (fPilotMasterSocket == -1))
	{
		fStatus = PilotLinkError;
		kdError() << k_funcinfo
			<< ": Already connected or unable to connect!"
			<< endl;
		return;
	}

	emit logProgress(QString::null, 30);

	fPilotUser = new KPilotUser;

	/* Ask the pilot who it is.  And see if it's who we think it is. */
#ifdef DEBUG
	DEBUGDAEMON << fname << ": Reading user info." << endl;
#endif

	dlp_ReadUserInfo(fCurrentPilotSocket, fPilotUser->pilotUser());
	fPilotUser->boundsCheck();

#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": Read user name " << fPilotUser->getUserName() << endl;
#endif

	emit logProgress(i18n("Checking last PC..."), 70);

	/* Tell user (via Pilot) that we are starting things up */
	if (dlp_OpenConduit(fCurrentPilotSocket) < 0)
	{
		fStatus = SyncDone;
		emit logMessage(i18n
			("Exiting on cancel. All data not restored."));
		return;
	}

	fStatus = AcceptedDevice;

	emit logProgress(QString::null, 100);

	emit deviceReady();
}

void KPilotDeviceLink::tickle() const
{
	FUNCTIONSETUP;
	pi_tickle(pilotSocket());
}


int KPilotDeviceLink::installFiles(const QStringList & l)
{
	FUNCTIONSETUP;

	QStringList::ConstIterator i;
	int k = 0;
	int n = 0;

	for (i = l.begin(); i != l.end(); ++i)
	{
		emit logProgress(QString::null,
			(int) ((100.0 / l.count()) * (float) n));

		if (installFile(*i))
			k++;
		n++;
	}
	emit logProgress(QString::null, 100);

	return k;
}

bool KPilotDeviceLink::installFile(const QString & f)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGDAEMON << fname << ": Installing file " << f << endl;
#endif

	if (!QFile::exists(f))
		return false;

	struct pi_file *pf =
		pi_file_open(const_cast < char *>
			((const char *) QFile::encodeName(f)));

	if (!f)
	{
		kdWarning() << k_funcinfo
			<< ": Can't open file " << f << endl;
		emit logError(i18n
			("<qt>Can't install the file &quot;%1&quot;.</qt>").
			arg(f));
		return false;
	}

	if (pi_file_install(pf, fCurrentPilotSocket, 0) < 0)
	{
		kdWarning() << k_funcinfo
			<< ": Can't pi_file_install " << f << endl;
		emit logError(i18n
			("<qt>Can't install the file &quot;%1&quot;.</qt>").
			arg(f));
		return false;
	}

	pi_file_close(pf);
	QFile::remove(f);

	return true;
}


void KPilotDeviceLink::addSyncLogEntry(const QString & entry, bool suppress)
{
	FUNCTIONSETUP;

	QString t(entry);

#if defined(PILOT_LINK_VERSION) && defined(PILOT_LINK_MAJOR) && defined(PILOT_LINK_MINOR)
#if (PILOT_LINK_VERSION * 1000 + PILOT_LINK_MAJOR * 10 + PILOT_LINK_MINOR) < 110
	t.append("X");
#endif
#else
	// Assume that if not defined, it's a (very) old version.
	t.append("X");
#endif
	dlp_AddSyncLogEntry(fCurrentPilotSocket,
		const_cast < char *>(t.latin1()));
	if (!suppress)
	{
		emit logMessage(entry);
	}
}

int KPilotDeviceLink::openConduit()
{
	return dlp_OpenConduit(fCurrentPilotSocket);
}

QString KPilotDeviceLink::deviceTypeString(int i) const
{
	FUNCTIONSETUP;
	switch (i)
	{
	case None:
		return QString("None");
	case Serial:
		return QString("Serial");
	case OldStyleUSB:
		return QString("OldStyleUSB");
	case DevFSUSB:
		return QString("DevFSUSB");
	default:
		return QString("<unknown>");
	}
}

QString KPilotDeviceLink::statusString() const
{
	FUNCTIONSETUP;
	QString s("KPilotDeviceLink=");


	switch (fStatus)
	{
	case Init:
		s.append("Init");
		break;
	case WaitingForDevice:
		s.append("WaitingForDevice");
		break;
	case FoundDevice:
		s.append("FoundDevice");
		break;
	case CreatedSocket:
		s.append("CreatedSocket");
		break;
	case DeviceOpen:
		s.append("DeviceOpen");
		break;
	case AcceptedDevice:
		s.append("AcceptedDevice");
		break;
	case SyncDone:
		s.append("SyncDone");
		break;
	case PilotLinkError:
		s.append("PilotLinkError");
		break;
	}

	return s;
}


void KPilotDeviceLink::finishSync()
{
	FUNCTIONSETUP ;

	getPilotUser()->setLastSyncPC((unsigned long) gethostid());
	getPilotUser()->setLastSyncDate(time(0));

	dlp_WriteUserInfo(pilotSocket(),getPilotUser()->pilotUser());
	addSyncLogEntry(i18n("End of HotSync\n"));
	dlp_EndOfSync(pilotSocket(), 0);
}

int KPilotDeviceLink::getNextDatabase(int index,struct DBInfo *dbinfo)
{
	FUNCTIONSETUP;

	return dlp_ReadDBList(pilotSocket(),0,dlpDBListRAM,index,dbinfo);
}

// Find a database with the given name. Info about the DB is stored into dbinfo (e.g. to be used later on with retrieveDatabase).
int KPilotDeviceLink::findDatabase(char*name, struct DBInfo*dbinfo) 
{
	FUNCTIONSETUP;
	return dlp_FindDBInfo(pilotSocket(), 0, 0, name, 0, 0, dbinfo);
}

bool KPilotDeviceLink::retrieveDatabase(const QString &fullBackupName, 
	DBInfo *info)
{
	FUNCTIONSETUP;

	// The casts here look funny because:
	//
	// fullBackupName is a QString
	// QFile::encodeName() gives us a QCString
	// which needs an explicit cast to become a const char *
	// which needs a const cast to become a char *
	//
	//
	struct pi_file *f;
	f = pi_file_create(const_cast < char *>
		((const char *) (QFile::encodeName(fullBackupName))),
		info);

	if (f == 0)
	{
		kdWarning() << k_funcinfo
			<< ": Failed, unable to create file" << endl;
		return false;
	}

	if (pi_file_retrieve(f, pilotSocket(), 0) < 0)
	{
		kdWarning() << k_funcinfo
			<< ": Failed, unable to back up database" << endl;

		pi_file_close(f);
		return false;
	}

	pi_file_close(f);
	return true;
}


QDateTime KPilotDeviceLink::getTime()
{
	QDateTime time;
	time_t palmtime;
	if (dlp_GetSysDateTime(pilotSocket(), &palmtime)) 
	{
		time.setTime_t(palmtime);
	}
	return time;
}

bool KPilotDeviceLink::setTime(const time_t &pctime)
{
//	struct tm time_tm=writeTm(time);
//	time_t pctime=mktime(&time_tm);
	return dlp_SetSysDateTime(pilotSocket(), pctime);
}



unsigned long KPilotDeviceLink::ROMversion() const
{
	unsigned long rom;
	dlp_ReadFeature(pilotSocket(), makelong("psys"), 1, &rom);
	return rom;
}
unsigned long KPilotDeviceLink::majorVersion() const
{
	unsigned long rom=ROMversion();
	return (((rom >> 28) & 0xf) * 10)+ ((rom >> 24) & 0xf);
}
unsigned long KPilotDeviceLink::minorVersion() const
{
	unsigned long int rom=ROMversion();
	return (((rom >> 20) & 0xf) * 10)+ ((rom >> 16) & 0xf);
}


bool operator < (const db & a, const db & b) {
	if (a.creator == b.creator)
	{
		if (a.type != b.type)
		{
			if (a.type == pi_mktag('a', 'p', 'p', 'l'))
				return false;
			else
				return true;
		}
	}

	return a.maxblock < b.maxblock;
}

// $Log$
// Revision 1.18  2002/07/30 17:17:48  kainhofe
// added majorVersion, minorVersion and ROMversion functions
//
// Revision 1.17  2002/07/25 22:11:22  kainhofe
// time sync conduit
//
// Revision 1.16  2002/07/25 19:02:20  kainhofe
// Added functions to get/set the time on the handheld
//
// Revision 1.15  2002/07/20 22:08:19  mhunter
// Hot-Sync -> HotSync
//
// Revision 1.14  2002/07/05 00:15:22  kainhofe
// Added KPilotDeviceLink::tickle(), Changelog update, compile fixes
//
// Revision 1.13  2002/05/18 23:28:19  adridg
// Compile fixes
//
// Revision 1.12  2002/05/15 20:16:20  kainhofe
// Wrongfully had a constant where 0 needed to be...
//
// Revision 1.11  2002/05/14 22:57:40  adridg
// Merge from _BRANCH
//
// Revision 1.10  2002/05/03 17:21:51  kainhofe
// Added a method findDatabase to KPilotDeviceLink to look up a single db on the palm
//
// Revision 1.9  2002/04/20 13:03:31  binner
// CVS_SILENT Capitalisation fixes.
//
// Revision 1.8.2.3  2002/05/01 21:37:30  kainhofe
// Added a function findDatabase to get the DBInfo for a given database
//
// Revision 1.8.2.2  2002/04/09 21:51:50  adridg
// Extra debugging, pilot-link 0.10.1 still needs workaround
//
// Revision 1.8.2.1  2002/04/04 20:28:28  adridg
// Fixing undefined-symbol crash in vcal. Fixed FD leak. Compile fixes
// when using PILOT_VERSION. kpilotTest defaults to list, like the options
// promise. Always do old-style USB sync (also works with serial devices)
// and runs conduits only for HotSync. KPilot now as it should have been
// for the 3.0 release.
//
// Revision 1.8  2002/02/10 22:21:33  adridg
// Handle pilot-link 0.10.1; spit 'n polish; m505 now supported?
//
// Revision 1.7  2002/02/02 11:46:03  adridg
// Abstracting away pilot-link stuff
//
// Revision 1.6  2002/01/25 21:43:13  adridg
// ToolTips->WhatsThis where appropriate; vcal conduit discombobulated - it doesn't eat the .ics file anymore, but sync is limited; abstracted away more pilot-link
//
// Revision 1.5  2002/01/21 23:14:03  adridg
// Old code removed; extra abstractions added; utility extended
//
// Revision 1.4  2002/01/18 16:28:57  adridg
// CVS_SILENT: Less compile warnings
//
// Revision 1.3  2001/12/29 15:45:28  adridg
// Lots of little changes for the syncstack
//
// Revision 1.2  2001/10/08 22:25:41  adridg
// Moved to libkpilot and lib-based conduits
//
// Revision 1.1  2001/10/08 21:56:02  adridg
// Start of making a separate KPilot lib
//
// Revision 1.60  2001/09/30 19:51:56  adridg
// Some last-minute layout, compile, and __FUNCTION__ (for Tru64) changes.
//
// Revision 1.59  2001/09/29 16:26:18  adridg
// The big layout change
//
// Revision 1.58  2001/09/24 22:20:28  adridg
// Made exec() pure virtual for SyncActions
//
// Revision 1.57  2001/09/24 19:46:17  adridg
// Made exec() pure virtual for SyncActions, since that makes more sense than having an empty default action.
//
// Revision 1.56  2001/09/23 18:24:59  adridg
// New syncing architecture
//
// Revision 1.55  2001/09/16 12:24:54  adridg
// Added sensible subclasses of KPilotLink, some USB support added.
//
// Revision 1.54  2001/09/06 22:04:27  adridg
// Enforce singleton-ness & retry pi_bind()
//
// Revision 1.53  2001/09/05 21:53:51  adridg
// Major cleanup and architectural changes. New applications kpilotTest
// and kpilotConfig are not installed by default but can be used to test
// the codebase. Note that nothing else will actually compile right now.
//
