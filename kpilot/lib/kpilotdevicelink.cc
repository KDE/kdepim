/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone <dan@kpilot.org>
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
** Copyright (C) 2006-2007 Adriaan de Groot <groot@kde.org>
** Copyright (C) 2007 Jason 'vanRijn' Kasper <vR@movingparts.net>
**
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

// Own headers.
#include "kpilotdevicelink.h"
#include "kpilotdevicelink.moc"
#include "kpilotdevicelinkPrivate.moc"

// KPilot headers
#include "kpilotlink.h"
#include <config-kpilot.h>
#include "options.h"
#include "pilotUser.h"
#include "pilotSysInfo.h"
#include "pilotCard.h"
#include "pilotSerialDatabase.h"
#include "pilotLocalDatabase.h"

// pilot-link headers
#include <pi-source.h>
#include <pi-socket.h>
#include <pi-dlp.h>
#include <pi-file.h>
#include <pi-buffer.h>

// Qt headers
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtCore/QSocketNotifier>
#include <QtCore/QTimer>
#include <QtGui/QApplication>

// KDE headers
#include <kconfig.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kio/netaccess.h>

// Other headers
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

DeviceMap *DeviceMap::mThis = 0L;


DeviceCommThread::DeviceCommThread(KPilotDeviceLink *d) :
	QThread(),
	fDone(true),
   fWorker(NULL),
   fHandle(d)
{
	FUNCTIONSETUP;
}


DeviceCommThread::~DeviceCommThread()
{
	FUNCTIONSETUPL(2);
	KPILOT_DELETE(fWorker);
}

void DeviceCommThread::run()
{
	FUNCTIONSETUP;

	fDone = false;

   fWorker = new DeviceCommWorker(fHandle);

	/*
	 * Start our thread's event loop.  This is necessary to allow our
	 * QTimers to function correctly.  Our thread is now stopped by calling
	 * stop().
	 */
	exec();

	/*
	 * now sleep one last bit to make sure the pthread inside
	 * pilot-link (potentially, if it's libusb) is done before we exit
	 */
	QThread::sleep(5);

	DEBUGKPILOT << ": comm thread now done...";
}

// End of Comm Thread



DeviceCommWorker::DeviceCommWorker(KPilotDeviceLink *d) :
	QObject(),
	fHandle(d),
	fOpenTimer(0L),
	fSocketNotifier(0L),
	fSocketNotifierActive(false),
	fWorkaroundUSBTimer(0L),
	fPilotSocket(-1),
	fTempSocket(-1),
	fAcceptedCount(0)
{
	FUNCTIONSETUP;

	/*
	 * Because our polling is asynchronous, we can't use a normal loop.
	 * We'll fall back into trying to open the device again as a
	 * result of either a failure in opening/binding, or a timeout from our
	 * USB workaround timer.
	 */
	fOpenTimer = new QTimer();
	connect(fOpenTimer, SIGNAL(timeout()), this, SLOT(openDevice()));
	fOpenTimer->setSingleShot(true);
	fOpenTimer->setInterval(5000);

	/**
	 * We _always_ want to set a maximum amount of time that we will wait
	 * for the sync process to start.  In the case where our user
	 * has told us that he has a funky USB device, set the workaround timeout
	 * for shorter than normal.
	 */
	int timeout=20000;
	if (link()->fWorkaroundUSB)
	{
		timeout=5000;
	}

	fWorkaroundUSBTimer = new QTimer();
	connect(fWorkaroundUSBTimer, SIGNAL(timeout()), this, SLOT(workaroundUSB()));
	fWorkaroundUSBTimer->setSingleShot(true);
	fWorkaroundUSBTimer->setInterval(timeout);

	fOpenTimer->start();
}


DeviceCommWorker::~DeviceCommWorker()
{
	FUNCTIONSETUPL(2);

	if (fOpenTimer) {
	   fOpenTimer->stop();
	   fOpenTimer->deleteLater();
	   fOpenTimer = 0L;
	}
	if (fWorkaroundUSBTimer) {
	   fWorkaroundUSBTimer->stop();
	   fWorkaroundUSBTimer->deleteLater();
	   fWorkaroundUSBTimer = 0L;
	}
	close();
}

void DeviceCommWorker::close()
{
	FUNCTIONSETUPL(2);

	static QMutex *mutex = 0L;
	QMutexLocker locker(mutex);

	/*
	 * Note: We don't want to delete these.  Just stop them.  Things seem to
	 * break when we're creating/deleting them from a QThread, so we create
	 * them from our ctor and stop them everywhere until we destruct.
	 */
	if (fOpenTimer)
		fOpenTimer->stop();

	if (fWorkaroundUSBTimer)
		fWorkaroundUSBTimer->stop();

	fSocketNotifierActive=false;
	/*
	 * This one we do delete because we require the socket itself to create
	 * this and we don't have that until we're past the pi_bind phase.
	 */
	fSocketNotifier->deleteLater();
	fSocketNotifier = 0;

	bool closeTemp = (fTempSocket != -1);
	bool closeMainSocket = (fPilotSocket != -1);

	DEBUGKPILOT << "Temp socket: " << fTempSocket << ", main socket: " << fPilotSocket;
	if (closeTemp)
	{
		DEBUGKPILOT << "Closing temp socket: " << fTempSocket;
		pi_close(fTempSocket);
	}
	fTempSocket = (-1);

	if (closeMainSocket)
	{
		DEBUGKPILOT << "Closing main socket: " << fPilotSocket;
		pi_close(fPilotSocket);
	}
	fPilotSocket = (-1);

	DeviceMap::self()->unbindDevice(link()->fRealPilotPath);
}

void DeviceCommWorker::reset()
{
	FUNCTIONSETUP;

	if (link()->fMessages->shouldPrint(Messages::OpenFailMessage))
	{
		QApplication::postEvent(link(), new DeviceCommEvent(EventLogMessage,
				i18n("Could not open device: %1 (will retry)", link()->pilotPath() )));
	}

	link()->fMessages->reset();

	close();

	fOpenTimer->start();

	link()->fLinkStatus = WaitingForDevice;
}

/**
 * This is an asynchronous process.  We try to create a socket with the Palm
 * and then bind to it (in open()).  If we're able to do those 2 things, then
 * we do 2 things:  we set a timeout timer (which will tell us that X amount of
 * time has transpired before we get into the meat of the sync transaction), and
 * we also set up a QSocketNotifier, which will tell us when data is available
 * to be read from the Palm socket.  If we were unable to create a socket
 * and/or bind to the Palm in this method, we'll start our timer again.
 */
void DeviceCommWorker::openDevice()
{
	FUNCTIONSETUPL(2);

	bool deviceOpened = false;

	// This transition (from Waiting to Found) can only be
	// taken once.
	//
	if (link()->fLinkStatus == WaitingForDevice)
	{
		link()->fLinkStatus = FoundDevice;
	}

	if (link()->fMessages->shouldPrint(Messages::OpenMessage))
	{
		QApplication::postEvent(link(), new DeviceCommEvent(EventLogMessage,
				i18n("Trying to open device %1...", link()->fPilotPath)));
	}

	if (link()->fPilotPath.length() > 0)
	{
		DEBUGKPILOT << ": Opening main pilot path: ["
			<< link()->fPilotPath << "].";
		deviceOpened = open(link()->fPilotPath);
	}

	// only try the temp device if our earlier attempt didn't work and the temp
	// device is different than the main device, and it's a non-empty
	// string
	bool tryTemp = !deviceOpened && (link()->fTempDevice.length() > 0) && (link()->fPilotPath != link()->fTempDevice);

	if (tryTemp)
	{
		DEBUGKPILOT << ": Could not open main pilot path. "
			<< "Now trying temp device: ["
			<< link()->fTempDevice << "].";
		deviceOpened = open(link()->fTempDevice);
	}

	// if we couldn't connect, try to connect again...
	if (!deviceOpened)
	{
		DEBUGKPILOT << ": Will try again.";
		reset();
	}
}

bool DeviceCommWorker::open(const QString &device)
{
	FUNCTIONSETUPL(2);

	int ret;
	int e = 0;
	QString msg;

	link()->fRealPilotPath
			= KStandardDirs::realFilePath(device.isEmpty() ? link()->fPilotPath : device);

	if ( !DeviceMap::self()->canBind(link()->fRealPilotPath) )
	{
		msg = i18n("Already listening on that device");

		WARNINGKPILOT << "Pilot Path: ["
			<< link()->fRealPilotPath << "] already connected.";
		WARNINGKPILOT << msg;

		link()->fLinkStatus = PilotLinkError;

		QApplication::postEvent(link(), new DeviceCommEvent(EventLogError, msg));

		return false;
	}

	DEBUGKPILOT << ": Trying to create socket.";

	fTempSocket = pi_socket(PI_AF_PILOT, PI_SOCK_STREAM, PI_PF_DLP);

	if (fTempSocket < 0)
	{
		e = errno;
		msg = i18n("Cannot create socket for communicating "
				"with the Pilot (%1)", errorMessage(e));
		DEBUGKPILOT << msg;
		DEBUGKPILOT << "(" << strerror(e) << ")";

		link()->fLinkStatus = PilotLinkError;

		QApplication::postEvent(link(), new DeviceCommEvent(EventLogError, msg));

		return false;
	}

	DEBUGKPILOT << ": Got socket: [" << fTempSocket << "]";

	link()->fLinkStatus = CreatedSocket;

	DEBUGKPILOT << ": Binding to path: ["
		<< link()->fRealPilotPath << "]";

	ret = pi_bind(fTempSocket, QFile::encodeName(link()->fRealPilotPath));

	if (ret < 0)
	{
		DEBUGKPILOT
			<< ": pi_bind error: [" << strerror(errno) << "]";

		e = errno;
		msg = i18n("Cannot open Pilot port: [%1].", link()->fRealPilotPath);

		DEBUGKPILOT << msg;
		DEBUGKPILOT << "(" << strerror(e) << ")";

		link()->fLinkStatus = PilotLinkError;

		if (link()->fMessages->shouldPrint(Messages::OpenFailMessage))
		{
			QApplication::postEvent(link(), new DeviceCommEvent(EventLogError, msg));
		}

		return false;
	}

	link()->fLinkStatus = DeviceOpen;
	DeviceMap::self()->bindDevice(link()->fRealPilotPath);

	fSocketNotifier = new QSocketNotifier(fTempSocket,
			QSocketNotifier::Read, this);
	QObject::connect(fSocketNotifier, SIGNAL(activated(int)),
			this, SLOT(acceptDevice()));
	fSocketNotifierActive=true;

	fWorkaroundUSBTimer->start();

	return true;
}

/**
 * We've been notified by our QSocketNotifier that we have data available on the
 * socket.  Try to go through the remaining steps of the connection process.
 * Note: If we return at all from this before the very end without a successful
 * connection, we need to make sure we restart our connection open timer, otherwise
 * it won't be restarted.
 */
void DeviceCommWorker::acceptDevice()
{
	FUNCTIONSETUP;

	int ret;

	/**
	 * Our socket notifier should be the only reason that we end up here.
	 * If we're here without him being active, we have a problem.  Try to clean
	 * up and get out.
	 */
	if (!fSocketNotifierActive)
	{
		WARNINGKPILOT << ": Accidentally in acceptDevice()";
		if (fSocketNotifier)
		{
			fSocketNotifier->setEnabled(false);
		}
		return;
	}

	if (fSocketNotifier)
	{
		fSocketNotifierActive=false;
		fSocketNotifier->setEnabled(false);
	}

	DEBUGKPILOT << ": Found connection on device: ["
		<< link()->pilotPath() << "].";

	DEBUGKPILOT
		<< ": Current status: ["
		<< link()->statusString()
		<< "] and socket: [" << fTempSocket << "]";

	ret = pi_listen(fTempSocket, 1);
	if (ret < 0)
	{
		char *s = strerror(errno);

		WARNINGKPILOT << "pi_listen returned: [" << s << "]";

		// Presumably, strerror() returns things in
		// local8Bit and not latin1.
		QApplication::postEvent(link(), new DeviceCommEvent(EventLogError,
				i18n("Cannot listen on Pilot socket (%1)", QString::fromLocal8Bit(s))));
		reset();
		return;
	}

	QApplication::postEvent(link(), new DeviceCommEvent(EventLogProgress, QString(), 10));

	DEBUGKPILOT <<
		": Listening to pilot. Now trying accept...";

	int timeout = 20;
	fPilotSocket = pi_accept_to(fTempSocket, 0, 0, timeout);

	if (fPilotSocket < 0)
	{
		char *s = strerror(errno);

		WARNINGKPILOT << "pi_accept returned: [" << s << "]";

		QApplication::postEvent(link(), new DeviceCommEvent(EventLogError,
			i18n("Cannot accept Pilot (%1)", QString::fromLocal8Bit(s))));

		link()->fLinkStatus = PilotLinkError;
		reset();
		return;
	}

	DEBUGKPILOT << ": Link accept done.";

	if ((link()->fLinkStatus != DeviceOpen) || (fPilotSocket == -1))
	{
		link()->fLinkStatus = PilotLinkError;
		WARNINGKPILOT << "Already connected or unable to connect!";

		QApplication::postEvent(link(), new DeviceCommEvent(EventLogError,
			i18n("Cannot accept Pilot (%1)", i18n("already connected"))));

		reset();
		return;
	}

	QApplication::postEvent(link(), new DeviceCommEvent(EventLogProgress, QString(), 30));

	DEBUGKPILOT << ": doing dlp_ReadSysInfo...";

	struct SysInfo sys_info;
	if (dlp_ReadSysInfo(fPilotSocket, &sys_info) < 0)
	{
		WARNINGKPILOT << "Unable to do dlp_ReadSysInfo";

		QApplication::postEvent(link(), new DeviceCommEvent(EventLogError,
			i18n("Unable to read system information from Pilot")));

		link()->fLinkStatus=PilotLinkError;
		reset();
		return;
	}
	else
	{
		DEBUGKPILOT << ": dlp_ReadSysInfo successful...";

		KPILOT_DELETE(link()->fPilotSysInfo);
		link()->fPilotSysInfo = new KPilotSysInfo(&sys_info);
		DEBUGKPILOT
			<< ": RomVersion: [" << link()->fPilotSysInfo->getRomVersion()
			<< "] Locale: [" << link()->fPilotSysInfo->getLocale()
			<< "] Product: [" << link()->fPilotSysInfo->getProductID()
			<< "]";
	}

	// If we've made it this far, make sure our USB workaround timer doesn't fire!
	fWorkaroundUSBTimer->stop();

	QApplication::postEvent(link(), new DeviceCommEvent(EventLogProgress, QString(), 60));

	KPILOT_DELETE(link()->fPilotUser);
	link()->fPilotUser = new KPilotUser;

	DEBUGKPILOT << ": doing dlp_ReadUserInfo...";

	/* Ask the pilot who it is.  And see if it's who we think it is. */
	dlp_ReadUserInfo(fPilotSocket, link()->fPilotUser->data());

	QString n = link()->getPilotUser().name();
	DEBUGKPILOT
		<< ": Read user name: [" << n << "]";

	QApplication::postEvent(link(), new DeviceCommEvent(EventLogProgress,
		i18n("Checking last PC..."), 90));

	/* Tell user (via Pilot) that we are starting things up */
	if ((ret=dlp_OpenConduit(fPilotSocket)) < 0)
	{
		DEBUGKPILOT
			<< ": dlp_OpenConduit returned: [" << ret << "]";

		QApplication::postEvent(link(), new DeviceCommEvent(EventLogError,
			i18n("Could not read user information from the Pilot. "
					"Perhaps you have a password set on the device?")));

	}
	link()->fLinkStatus = AcceptedDevice;

	QApplication::postEvent(link(), new DeviceCommEvent(EventLogProgress, QString(), 100));

	DeviceCommEvent * ev = new DeviceCommEvent(EventDeviceReady);
	ev->setCurrentSocket(fPilotSocket);
	QApplication::postEvent(link(), ev);

}

void DeviceCommWorker::workaroundUSB()
{
	FUNCTIONSETUP;
        if ( link()->fPilotPath == "usb:" ||
             link()->fPilotPath == "net:any" ) {
		DEBUGKPILOT << "usb: or net:any device. not resetting.";
		return;
	}
	reset();
}

// End of Worker Thread



KPilotDeviceLink::KPilotDeviceLink(QObject * parent, const char *name, const QString &tempDevice) :
	KPilotLink(parent, name),
	fLinkStatus(Init),
	fWorkaroundUSB(false),
	fPilotSocket(-1),
	fTempDevice(tempDevice),
	fMessages(new Messages(this)),
	fDeviceCommThread(0L)
{
	FUNCTIONSETUP;

	DEBUGKPILOT << ": Pilot-link version: " << PILOT_LINK_NUMBER;
}

KPilotDeviceLink::~KPilotDeviceLink()
{
	FUNCTIONSETUP;
	close();
	KPILOT_DELETE(fPilotSysInfo);
	KPILOT_DELETE(fPilotUser);
	KPILOT_DELETE(fMessages);
}

/* virtual */ bool KPilotDeviceLink::isConnected() const
{
	 return fLinkStatus == AcceptedDevice;
}

/* virtual */bool KPilotDeviceLink::event(QEvent *e)
{
	FUNCTIONSETUP;

	bool handled = false;

	if ((int)e->type() == EventDeviceReady)
	{
		DeviceCommEvent* t = static_cast<DeviceCommEvent*>(e);
		fPilotSocket = t->currentSocket();
		emit deviceReady( this);
		handled = true;
	}
	else if ((int)e->type() == EventLogMessage)
	{
		DeviceCommEvent* t = static_cast<DeviceCommEvent*>(e);
		emit logMessage(t->message());
		handled = true;
	}
	else if ((int)e->type() == EventLogError)
	{
		DeviceCommEvent* t = static_cast<DeviceCommEvent*>(e);
		emit logError(t->message());
		handled = true;
	}
	else if ((int)e->type() == EventLogProgress)
	{
		DeviceCommEvent* t = static_cast<DeviceCommEvent*>(e);
		emit logProgress(t->message(), t->progress());
		handled = true;
	}
	else
	{
		handled = KPilotLink::event(e);
	}

	return handled;
}

void KPilotDeviceLink::stopCommThread()
{
	FUNCTIONSETUP;
	if (fDeviceCommThread)
	{
		fDeviceCommThread->stop();

		// try to wait for our thread to finish, but don't
		// block the main thread forever
		if (fDeviceCommThread->isRunning())
		{
			DEBUGKPILOT
				<< "comm thread still running. Waiting for it to complete.";
			bool done = fDeviceCommThread->wait(30000);
			if (!done)
			{
				DEBUGKPILOT
					<< "comm thread still running after wait(). "
					<< "going to have to delete it anyway. this may end badly.";
			}
		}
		KPILOT_DELETE(fDeviceCommThread);
	}
}

void KPilotDeviceLink::close()
{
	FUNCTIONSETUPL(2);

	stopCommThread();

	fPilotSocket = (-1);
}

void KPilotDeviceLink::reset(const QString & dP)
{
	FUNCTIONSETUP;

	fLinkStatus = Init;

	// Release all resources
	//
	close();
	fPilotPath.clear();

	fPilotPath = dP;
	if (fPilotPath.isEmpty())
	   fPilotPath = fTempDevice;
	if (fPilotPath.isEmpty())
		return;

	reset();
}

void KPilotDeviceLink::startCommThread()
{
	FUNCTIONSETUP;

	stopCommThread();

	if (fTempDevice.isEmpty() && pilotPath().isEmpty())
	{
		WARNINGKPILOT << "No point in trying empty device.";

		QString msg = i18n("The Pilot device is not configured yet.");
		WARNINGKPILOT << msg;


		fLinkStatus = PilotLinkError;

		emit logError(msg);
		return;
	}

	fDeviceCommThread = new DeviceCommThread(this);
	fDeviceCommThread->start();
}


void KPilotDeviceLink::reset()
{
	FUNCTIONSETUP;

	fMessages->reset();
	close();

	checkDevice();

	fLinkStatus = WaitingForDevice;

	startCommThread();
}

void KPilotDeviceLink::checkDevice()
{

        if ( fPilotPath == "usb:" ||
             fPilotPath == "net:any" )
	{
	  return;
	}

	// If the device exists yet doesn't have the right
	// permissions, complain and then continue anyway.
	//
	QFileInfo fi(fPilotPath);
	if (fi.exists())
	{
		// If it exists, it ought to be RW already.
		//
		if (!(fi.isReadable() && fi.isWritable()))
		{
			emit logError(i18n("Pilot device %1 is not read-write.", fPilotPath));
		}
	}
	else
	{
		// It doesn't exist, mention this in the log
		// (relevant as long as we use only one device type)
		//
		emit logError(i18n("Pilot device %1 does not exist. "
			"Probably it is a USB device and will appear during a HotSync.",
			fPilotPath));
		// Suppress all normal and error messages about opening the device.
		fMessages->block(Messages::OpenMessage | Messages::OpenFailMessage, true);
	}
}

void KPilotDeviceLink::setTempDevice( const QString &d )
{
	fTempDevice = d;
	DeviceMap::self()->bindDevice( fTempDevice );
}


/* virtual */ bool KPilotDeviceLink::tickle()
{
	FUNCTIONSETUPL(5);
	return pi_tickle(pilotSocket()) >= 0;
}

/* virtual */ void KPilotDeviceLink::addSyncLogEntryImpl( const QString &entry )
{
	dlp_AddSyncLogEntry(fPilotSocket,
		const_cast<char *>((const char *)Pilot::toPilot(entry)));
}

bool KPilotDeviceLink::installFile(const QString & f, const bool deleteFile)
{
	FUNCTIONSETUP;

	DEBUGKPILOT << "Installing file [" << f << ']';

	if (!QFile::exists(f))
	{
		return false;
	}

	char buffer[PATH_MAX];
	memset(buffer,0,PATH_MAX);
	strlcpy(buffer,QFile::encodeName(f),PATH_MAX);
	struct pi_file *pf = pi_file_open(buffer);

	if (f.isEmpty())
	{
		WARNINGKPILOT << "Cannot open file [" << f << ']';
		emit logError(i18n
			("<qt>Cannot install the file &quot;%1&quot;.</qt>", f));
		return false;
	}

	if (pi_file_install(pf, fPilotSocket, 0, 0L) < 0)
	{
		WARNINGKPILOT << "Cannot pi_file_install" << f;
		emit logError(i18n
			("<qt>Cannot install the file &quot;%1&quot;.</qt>", f));
		return false;
	}

	pi_file_close(pf);
	if (deleteFile) QFile::remove(f);

	return true;
}


int KPilotDeviceLink::openConduit()
{
	return dlp_OpenConduit(fPilotSocket);
}

QString KPilotDeviceLink::statusString(LinkStatus l)
{
	QString s = CSL1("KPilotDeviceLink=");

	switch (l)
	{
	case Init:
		s.append(CSL1("Init"));
		break;
	case WaitingForDevice:
		s.append(CSL1("WaitingForDevice"));
		break;
	case FoundDevice:
		s.append(CSL1("FoundDevice"));
		break;
	case CreatedSocket:
		s.append(CSL1("CreatedSocket"));
		break;
	case DeviceOpen:
		s.append(CSL1("DeviceOpen"));
		break;
	case AcceptedDevice:
		s.append(CSL1("AcceptedDevice"));
		break;
	case SyncDone:
		s.append(CSL1("SyncDone"));
		break;
	case PilotLinkError:
		s.append(CSL1("PilotLinkError"));
		break;
	case WorkaroundUSB:
		s.append(CSL1("WorkaroundUSB"));
		break;
	}

	return s;
}

QString KPilotDeviceLink::statusString() const
{
	return statusString( status() );
}

void KPilotDeviceLink::endSync( EndOfSyncFlags f )
{
	FUNCTIONSETUP;

	if ( UpdateUserInfo == f )
	{
		getPilotUser().setLastSyncPC((unsigned long) gethostid());
		getPilotUser().setLastSyncDate(time(0));

		DEBUGKPILOT << "Writing username" << getPilotUser().name();

		dlp_WriteUserInfo(pilotSocket(),getPilotUser().data());
		addSyncLogEntry(i18n("End of HotSync\n"));
	}
	dlp_EndOfSync(pilotSocket(), 0);
	KPILOT_DELETE(fPilotSysInfo);
	KPILOT_DELETE(fPilotUser);
}


int KPilotDeviceLink::getNextDatabase(int index,struct DBInfo *dbinfo)
{
	FUNCTIONSETUP;

	pi_buffer_t buf = { 0,0,0 };
	int r = dlp_ReadDBList(pilotSocket(),0,dlpDBListRAM,index,&buf);
	if (r >= 0)
	{
		memcpy(dbinfo,buf.data,sizeof(struct DBInfo));
	}
	return r;
}

// Find a database with the given name. Info about the DB is stored into dbinfo (e.g. to be used later on with retrieveDatabase).
int KPilotDeviceLink::findDatabase(const char *name, struct DBInfo *dbinfo,
	int index, unsigned long type, unsigned long creator)
{
	FUNCTIONSETUP;
	return dlp_FindDBInfo(pilotSocket(), 0, index,
		const_cast<char *>(name), type, creator, dbinfo);
}

bool KPilotDeviceLink::retrieveDatabase(const QString &fullBackupName,	DBInfo *info)
{
	FUNCTIONSETUP;


	if (fullBackupName.isEmpty() || !info)
	{
		// Don't even bother trying to convert or retrieve.
		return false;
	}

	DEBUGKPILOT << "Writing DB [" << info->name << ']'
		<< " to [" << fullBackupName << ']';

	QByteArray encodedName = QFile::encodeName(fullBackupName);
	struct pi_file *f = pi_file_create(encodedName,info);

	if (!f)
	{
		WARNINGKPILOT << "Failed, unable to create file.";
		return false;
	}

	if (pi_file_retrieve(f, pilotSocket(), 0, 0L) < 0)
	{
		WARNINGKPILOT << "Failed, unable to back up database.";

		pi_file_close(f);
		return false;
	}

	pi_file_close(f);
	return true;
}


KPilotLink::DBInfoList KPilotDeviceLink::getDBList(int cardno, int flags)
{
	bool cont=true;
	DBInfoList dbs;
	int index=0;
	while (cont)
	{
		pi_buffer_t buf = { 0,0,0 };
		pi_buffer_clear(&buf);
		// DBInfo*dbi=new DBInfo();
		if (dlp_ReadDBList(pilotSocket(), cardno, flags | dlpDBListMultiple, index, &buf)<0)
		{
			cont=false;
		}
		else
		{
			DBInfo db_n;
			DBInfo *db_it = (DBInfo *)buf.data;
			int info_count = buf.used / sizeof(struct DBInfo);

			while(info_count>0)
			{
				memcpy(&db_n,db_it,sizeof(struct DBInfo));
				++db_it;
				info_count--;
				dbs.append(db_n);
			}
			index=db_n.index+1;
		}
	}
	return dbs;
}

const KPilotCard *KPilotDeviceLink::getCardInfo(int card)
{
	KPilotCard *cardinfo=new KPilotCard();
	if (dlp_ReadStorageInfo(pilotSocket(), card, cardinfo->cardInfo())<0)
	{
		WARNINGKPILOT << "Could not get info for card " << card;

		KPILOT_DELETE(cardinfo);
		return 0L;
	};
	return cardinfo;
}

PilotDatabase *KPilotDeviceLink::database( const QString &name )
{
	return new PilotSerialDatabase( this, name );
}

PilotDatabase *KPilotDeviceLink::database( const DBInfo *info )
{
	return new PilotSerialDatabase( this, info );
}

