/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/
static const char *kpilotlink_id = "$Id$";

#include "options.h"



#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <iostream>

#include <pi-source.h>
#include <pi-socket.h>
#include <pi-dlp.h>
#include <pi-file.h>
#if !(PILOT_LINK_NUMBER < PILOT_LINK_0_12_0)
#include <pi-buffer.h>
#endif

#include <qdir.h>
#include <qtimer.h>
#include <qdatetime.h>
#include <qsocketnotifier.h>
#include <qthread.h>
#include <qtextcodec.h>

#include <kconfig.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

#include "pilotUser.h"
#include "pilotSysInfo.h"
#include "pilotCard.h"
#include "pilotAppCategory.h"

#include "kpilotlink.moc"


// singleton helper class
class KPilotDeviceLink::KPilotDeviceLinkPrivate
{
public:
	static KPilotDeviceLinkPrivate*self()
	{
		if (!mThis) mThis = new KPilotDeviceLinkPrivate();
		return mThis;
	}

	bool canBind( QString device )
	{
		showList();
		return !mBoundDevices.contains( device );
	}

	void bindDevice( QString device )
	{
		mBoundDevices.append( device );
		showList();
	}

	void unbindDevice( QString device )
	{
		mBoundDevices.remove( device );
		showList();
	}

protected:
	KPilotDeviceLinkPrivate() {}
	~KPilotDeviceLinkPrivate() {}

	QStringList mBoundDevices;
	static KPilotDeviceLinkPrivate*mThis;

private:
	inline void showList() const
	{
#ifdef DEBUG
		FUNCTIONSETUPL(3);
		DEBUGDAEMON << fname << "Bound devices: "
			<< ((mBoundDevices.count() > 0) ? mBoundDevices.join(CSL1(", ")) : CSL1("<none>")) << endl;
#endif
	}
} ;

KPilotDeviceLink::KPilotDeviceLinkPrivate *KPilotDeviceLink::KPilotDeviceLinkPrivate::mThis = 0L;


KPilotDeviceLink::KPilotDeviceLink(QObject * parent, const char *name, const QString &tempDevice) :
	QObject(parent, name),
	fLinkStatus(Init),
	fTickleDone(true),
	fTickleThread(0L),
	fWorkaroundUSB(false),
	fWorkaroundUSBTimer(0L),
	fPilotPath(QString::null),
	fRetries(0),
	fOpenTimer(0L),
	fSocketNotifier(0L),
	fSocketNotifierActive(false),
	fPilotMasterSocket(-1),
	fCurrentPilotSocket(-1),
	fTempDevice(tempDevice),
	fPilotUser(0L),
	fPilotSysInfo(0L)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": Pilot-link version " << PILOT_LINK_NUMBER
		<< endl;
#endif

	messagesMask=0xffffffff;

	(void) kpilotlink_id;
}

KPilotDeviceLink::~KPilotDeviceLink()
{
	FUNCTIONSETUP;
	close();
	KPILOT_DELETE(fWorkaroundUSBTimer);
	KPILOT_DELETE(fPilotSysInfo);
	KPILOT_DELETE(fPilotUser);
}

void KPilotDeviceLink::close()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fWorkaroundUSBTimer);
	KPILOT_DELETE(fOpenTimer);
	KPILOT_DELETE(fSocketNotifier);
	fSocketNotifierActive=false;
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
	KPilotDeviceLinkPrivate::self()->unbindDevice( fRealPilotPath );
	fPilotMasterSocket = (-1);
	fCurrentPilotSocket = (-1);
}

void KPilotDeviceLink::reset(const QString & dP)
{
	FUNCTIONSETUP;

	fLinkStatus = Init;
	fRetries = 0;

	// Release all resources
	//
	close();
	fPilotPath = QString::null;

	fPilotPath = dP;
	if (fPilotPath.isEmpty())
	   fPilotPath = fTempDevice;
	if (fPilotPath.isEmpty())
		return;

	reset();
}

static inline void startOpenTimer(KPilotDeviceLink *dev, QTimer *&t)
{
	if ( !t ){
		t = new QTimer(dev);
		QObject::connect(t, SIGNAL(timeout()),
			dev, SLOT(openDevice()));
	}
	t->start(1000, false);
}

void KPilotDeviceLink::reset()
{
	FUNCTIONSETUP;

	messages=0;
	close();

	checkDevice();

	// Timer already deleted by close() call.
	startOpenTimer(this,fOpenTimer);

	fLinkStatus = WaitingForDevice;
}

void KPilotDeviceLink::checkDevice()
{
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
			emit logError(i18n("Pilot device %1 is not read-write.")
				.arg(fPilotPath));
		}
	}
	else
	{
		// It doesn't exist, mention this in the log
		// (relevant as long as we use only one device type)
		//
		emit logError(i18n("Pilot device %1 does not exist. "
			"Probably it is a USB device and will appear during a HotSync.")
				.arg(fPilotPath));
	}
}

void KPilotDeviceLink::setTempDevice( const QString &d )
{
	fTempDevice = d;
	KPilotDeviceLinkPrivate::self()->bindDevice( fTempDevice );
}

void KPilotDeviceLink::openDevice()
{
	FUNCTIONSETUPL(2);

	// This transition (from Waiting to Found) can only be
	// taken once.
	//
	if (fLinkStatus == WaitingForDevice)
	{
		fLinkStatus = FoundDevice;
	}

	shouldPrint(OpenMessage,i18n("Trying to open device %1...")
		.arg(fPilotPath));

	if (open())
	{
		emit logMessage(i18n("Device link ready."));
	}
	else if (open(fTempDevice))
	{
		emit logMessage(i18n("Device link ready."));
	}
	else
	{
		shouldPrint(OpenFailMessage,i18n("Could not open device: %1 "
				"(will retry)").
				arg(fPilotPath));

		if (fLinkStatus != PilotLinkError)
		{
			startOpenTimer(this,fOpenTimer);
		}
	}
}

bool KPilotDeviceLink::open(QString device)
{
	FUNCTIONSETUPL(2);

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

	if ( device.isEmpty() )
	{
		device = pilotPath();
	}
	if (device.isEmpty())
	{
		kdWarning() << k_funcinfo
			<< ": No point in trying empty device."
			<< endl;

		msg = i18n("The Pilot device is not configured yet.");
		e = 0;
		goto errInit;
	}
	fRealPilotPath = KStandardDirs::realPath ( device );

	if ( !KPilotDeviceLinkPrivate::self()->canBind( fRealPilotPath ) ) {
		msg = i18n("Already listening on that device");
		e=0;
		kdWarning() << k_funcinfo <<": Pilot Path " << pilotPath().latin1() << " already connected." << endl;
		goto errInit;
	}


	if (fPilotMasterSocket == -1)
	{
#ifdef DEBUG
		DEBUGDAEMON << fname << ": Typing to open " << fRealPilotPath << endl;
#endif

#if PILOT_LINK_NUMBER < PILOT_LINK_0_10_0
		fPilotMasterSocket = pi_socket(PI_AF_SLP,
			PI_SOCK_STREAM, PI_PF_PADP);
#else
		fPilotMasterSocket = pi_socket(PI_AF_PILOT,
			PI_SOCK_STREAM, PI_PF_DLP);
#endif

		if (fPilotMasterSocket<1)
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

		fLinkStatus = CreatedSocket;
	}

	Q_ASSERT(fLinkStatus == CreatedSocket);

#ifdef DEBUG
	DEBUGDAEMON << fname << ": Binding to path " << fPilotPath << endl;
#endif

#if PILOT_LINK_NUMBER < PILOT_LINK_0_12_0
	struct pi_sockaddr addr;
#if PILOT_LINK_NUMBER < PILOT_LINK_0_10_0
	addr.pi_family = PI_AF_SLP;
#else
	addr.pi_family = PI_AF_PILOT;
#endif
	strlcpy(addr.pi_device, QFile::encodeName(device),sizeof(addr.pi_device));

	ret = pi_bind(fPilotMasterSocket,
		(struct sockaddr *) &addr, sizeof(addr));
#else
	ret = pi_bind(fPilotMasterSocket, QFile::encodeName(device));
#endif

	if (ret >= 0)
	{
		fLinkStatus = DeviceOpen;
		fOpenTimer->stop();

		KPilotDeviceLinkPrivate::self()->bindDevice( fRealPilotPath );
		fSocketNotifier = new QSocketNotifier(fPilotMasterSocket,
			QSocketNotifier::Read, this);
		QObject::connect(fSocketNotifier, SIGNAL(activated(int)),
			this, SLOT(acceptDevice()));
		fSocketNotifierActive=true;
		
		if (fWorkaroundUSB)
		{
#ifdef DEBUG
			DEBUGDAEMON << fname << ": Adding Z31 workaround." << endl;
#endif
			// Special case for Zire 31, 72, Tungsten T5,
			// all of which may make a non-HotSync connection
			// to the PC. Must detect this and bail quickly.
			//
			fWorkaroundUSBTimer = new QTimer(this);
			connect(fWorkaroundUSBTimer,SIGNAL(timeout()),
				this,SLOT(workaroundUSB()));
			fWorkaroundUSBTimer->start(5000,true);
		}
		
		return true;
	}
	else
	{
#ifdef DEBUG
#if PILOT_LINK_NUMBER < PILOT_LINK_0_12_0
		DEBUGDAEMON << fname
			<< ": Tried "
			<< addr.pi_device
			<< " and got "
			<< strerror(errno)
			<< endl;
#else
		DEBUGDAEMON << fname
			<< ": Tried " << device << " and got " << strerror(errno) << endl;
#endif
#endif

		if (fRetries < 5)
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
	close();

	if (msg.find('%'))
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
		msg += i18n(" You do not have permission to open the "
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

	fLinkStatus = PilotLinkError;
	emit logError(msg);
	return false;
}

void KPilotDeviceLink::acceptDevice()
{
	FUNCTIONSETUP;

	int ret;

	if (!fSocketNotifierActive)
	{
		if (!fAcceptedCount)
		{
		kdWarning() << k_funcinfo << ": Accidentally in acceptDevice()"
			<< endl;
		}
		fAcceptedCount++;
		if (fAcceptedCount>10)
		{
			// Damn the torpedoes
			KPILOT_DELETE(fSocketNotifier);
		}
		return;
	}

	if (fSocketNotifier)
	{
		// fSocketNotifier->setEnabled(false);
		fSocketNotifierActive=false;
	}

#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": Found connection on device "<<pilotPath().latin1()<<endl;
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

		// Presumably, strerror() returns things in
		// local8Bit and not latin1.
		emit logError(i18n("Cannot listen on Pilot socket (%1)").
			arg(QString::fromLocal8Bit(s)));

		close();
		return;
	}

	KPILOT_DELETE(fWorkaroundUSBTimer);

	emit logProgress(QString::null,10);

	fCurrentPilotSocket = pi_accept(fPilotMasterSocket, 0, 0);
	if (fCurrentPilotSocket == -1)
	{
		char *s = strerror(errno);

		kdWarning() << "pi_accept: " << s << endl;

		emit logError(i18n("Cannot accept Pilot (%1)")
			.arg(QString::fromLocal8Bit(s)));

		fLinkStatus = PilotLinkError;
		close();
		return;
	}

	if ((fLinkStatus != DeviceOpen) || (fPilotMasterSocket == -1))
	{
		fLinkStatus = PilotLinkError;
		kdError() << k_funcinfo
			<< ": Already connected or unable to connect!"
			<< endl;
		emit logError(i18n("Cannot accept Pilot (%1)")
			.arg(i18n("already connected")));
		close();
		return;
	}

	emit logProgress(QString::null, 30);

        KPILOT_DELETE(fPilotSysInfo);
	fPilotSysInfo = new KPilotSysInfo;
	if (dlp_ReadSysInfo(fCurrentPilotSocket, fPilotSysInfo->sysInfo()) < 0)
	{
		emit logError(i18n("Unable to read system information from Pilot"));
		fLinkStatus=PilotLinkError;
		return;
	}
#ifdef DEBUG
	else
	{
		DEBUGDAEMON << fname
			<< ": RomVersion=" << fPilotSysInfo->getRomVersion()
			<< " Locale=" << fPilotSysInfo->getLocale()
#if PILOT_LINK_NUMBER < PILOT_LINK_0_10_0
			/* No prodID member */
#else
			<< " Product=" << fPilotSysInfo->getProductID()
#endif
			<< endl;
	}
#endif
	fPilotSysInfo->boundsCheck();

	emit logProgress(QString::null, 60);
        KPILOT_DELETE(fPilotUser);
	fPilotUser = new KPilotUser;

	/* Ask the pilot who it is.  And see if it's who we think it is. */
#ifdef DEBUG
	DEBUGDAEMON << fname << ": Reading user info @"
		<< (long) fPilotUser << endl;
	DEBUGDAEMON << fname << ": Buffer @"
		<< (long) fPilotUser->pilotUser() << endl;
#endif

	dlp_ReadUserInfo(fCurrentPilotSocket, fPilotUser->pilotUser());

#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": Read user name " << fPilotUser->getUserName() << endl;
#endif

	emit logProgress(i18n("Checking last PC..."), 90);

	/* Tell user (via Pilot) that we are starting things up */
	if ((ret=dlp_OpenConduit(fCurrentPilotSocket)) < 0)
	{
#ifdef DEBUG
		DEBUGDAEMON << k_funcinfo
			<< ": dlp_OpenConduit returned " << ret << endl;
#endif

#if 0
		fLinkStatus = SyncDone;
		emit logMessage(i18n
			("Exiting on cancel. All data not restored."));
		return;
#endif
		emit logError(i18n("Could not read user information from the Pilot. "
			"Perhaps you have a password set on the device?"));
	}
	fLinkStatus = AcceptedDevice;


	emit logProgress(QString::null, 100);
	emit deviceReady( this );
}

void KPilotDeviceLink::workaroundUSB()
{
	FUNCTIONSETUP;
	
	Q_ASSERT((fLinkStatus == DeviceOpen) || (fLinkStatus == WorkaroundUSB));
	if (fLinkStatus == DeviceOpen)
	{
		reset();
	}
	fLinkStatus = WorkaroundUSB;
	
	if (!QFile::exists(fRealPilotPath))
	{
		// Fake connection has vanished again.
		// Resume polling for regular connection.
		startOpenTimer(this,fOpenTimer);
		return;
	}
	if (fOpenTimer)
	{
		fOpenTimer->stop();
	}
	KPILOT_DELETE(fWorkaroundUSBTimer);
	QTimer::singleShot(1000,this,SLOT(workaroundUSB()));
}

bool KPilotDeviceLink::tickle() const
{
	// No FUNCTIONSETUP here because it may be called from
	// a separate thread.
	return pi_tickle(pilotSocket()) >= 0;
}

class TickleThread : public QThread
{
public:
	TickleThread(KPilotDeviceLink *d, bool *done, int timeout) :
		QThread(),
		fHandle(d),
		fDone(done),
		fTimeout(timeout)
	{ };
	virtual ~TickleThread();

	virtual void run();

	static const int ChecksPerSecond = 5;
	static const int SecondsPerTickle = 5;

private:
	KPilotDeviceLink *fHandle;
	bool *fDone;
	int fTimeout;
} ;

TickleThread::~TickleThread()
{
}

void TickleThread::run()
{
	FUNCTIONSETUP;
	int subseconds = ChecksPerSecond;
	int ticktock = SecondsPerTickle;
	int timeout = fTimeout;
#ifdef DEBUG_CERR
	DEBUGDAEMON << fname << ": Running for " << timeout << " seconds." << endl;
	DEBUGDAEMON << fname << ": Done @" << (void *) fDone << endl;
#endif
	while (!(*fDone))
	{
		QThread::msleep(1000/ChecksPerSecond);
		if (!(--subseconds))
		{
#ifdef DEBUG_CERR
// Don't dare use kdDebug() here, we're in a separate thread
			DEBUGDAEMON << fname << ": One second." << endl;
#endif
			if (timeout)
			{
				if (!(--timeout))
				{
					QApplication::postEvent(fHandle, new QEvent((QEvent::Type)(KPilotDeviceLink::TickleTimeoutEvent)));
					break;
				}
			}
			subseconds=ChecksPerSecond;
			if (!(--ticktock))
			{
#ifdef DEBUG_CERR
				DEBUGDAEMON << fname << ": Kietel kietel!." << endl;
#endif
				ticktock=SecondsPerTickle;
				fHandle->tickle();
			}
		}
	}
#ifdef DEBUG_CERR
	DEBUGDAEMON << fname << ": Finished." << endl;
#endif
}

/*
** Deal with events, especially the ones used to signal a timeout
** in a tickle thread.
*/

/* virtual */ bool KPilotDeviceLink::event(QEvent *e)
{
	if (e->type() == TickleTimeoutEvent)
	{
		stopTickle();
		emit timeout();
		return true;
	}
	else	return QObject::event(e);
}

/*
Start a tickle thread with the indicated timeout.
*/
void KPilotDeviceLink::startTickle(unsigned int timeout)
{
	FUNCTIONSETUP;

	Q_ASSERT(fTickleDone);

	/*
	** We've told the thread to finish up, but it hasn't
	** done so yet - so wait for it to do so, should be
	** only 200ms at most.
	*/
	if (fTickleDone && fTickleThread)
	{
		fTickleThread->wait();
		KPILOT_DELETE(fTickleThread);
	}

#ifdef DEBUG
	DEBUGDAEMON << fname << ": Done @" << (void *) (&fTickleDone) << endl;
#endif
	fTickleDone = false;
	fTickleThread = new TickleThread(this,&fTickleDone,timeout);
	fTickleThread->start();
}

void KPilotDeviceLink::stopTickle()
{
	FUNCTIONSETUP;
	fTickleDone = true;
	if (fTickleThread)
	{
		fTickleThread->wait();
		KPILOT_DELETE(fTickleThread);
	}
}


int KPilotDeviceLink::installFiles(const QStringList & l, const bool deleteFiles)
{
	FUNCTIONSETUP;

	QStringList::ConstIterator i;
	int k = 0;
	int n = 0;

	for (i = l.begin(); i != l.end(); ++i)
	{
		emit logProgress(QString::null,
			(int) ((100.0 / l.count()) * (float) n));

		if (installFile(*i, deleteFiles))
			k++;
		n++;
	}
	emit logProgress(QString::null, 100);

	return k;
}

bool KPilotDeviceLink::installFile(const QString & f, const bool deleteFile)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGDAEMON << fname << ": Installing file " << f << endl;
#endif

	if (!QFile::exists(f))
		return false;

	char buffer[PATH_MAX];
	memset(buffer,0,PATH_MAX);
	strlcpy(buffer,QFile::encodeName(f),PATH_MAX);
	struct pi_file *pf =
		pi_file_open(buffer);

	if (!f)
	{
		kdWarning() << k_funcinfo
			<< ": Cannot open file " << f << endl;
		emit logError(i18n
			("<qt>Cannot install the file &quot;%1&quot;.</qt>").
			arg(f));
		return false;
	}

#if PILOT_LINK_NUMBER < PILOT_LINK_0_12_0
	if (pi_file_install(pf, fCurrentPilotSocket, 0) < 0)
#else
	if (pi_file_install(pf, fCurrentPilotSocket, 0, 0L) < 0)
#endif
	{
		kdWarning() << k_funcinfo
			<< ": Cannot pi_file_install " << f << endl;
		emit logError(i18n
			("<qt>Cannot install the file &quot;%1&quot;.</qt>").
			arg(f));
		return false;
	}

	pi_file_close(pf);
	if (deleteFile) QFile::remove(f);

	return true;
}


void KPilotDeviceLink::addSyncLogEntry(const QString & entry, bool log)
{
	FUNCTIONSETUP;
	if (entry.isEmpty()) return;

	QString t(entry);

#if PILOT_LINK_NUMBER < PILOT_LINK_0_11_0
	t.append("X");
#endif

	dlp_AddSyncLogEntry(fCurrentPilotSocket,
		const_cast<char *>((const char *)PilotAppCategory::codec()->fromUnicode(entry)));
	if (log)
	{
		emit logMessage(entry);
	}
}

int KPilotDeviceLink::openConduit()
{
	return dlp_OpenConduit(fCurrentPilotSocket);
}

QString KPilotDeviceLink::statusString() const
{
	FUNCTIONSETUP;
	QString s = CSL1("KPilotDeviceLink=");


	switch (fLinkStatus)
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

void KPilotDeviceLink::endOfSync()
{
	dlp_EndOfSync(pilotSocket(), 0);
	KPILOT_DELETE(fPilotSysInfo);
	KPILOT_DELETE(fPilotUser);
}

void KPilotDeviceLink::finishSync()
{
	FUNCTIONSETUP ;

	getPilotUser()->setLastSyncPC((unsigned long) gethostid());
	getPilotUser()->setLastSyncDate(time(0));

	
#ifdef DEBUG	
	DEBUGDAEMON << fname << ": Writing username " << getPilotUser()->getUserName() << endl;
#endif
	dlp_WriteUserInfo(pilotSocket(),getPilotUser()->pilotUser());
	addSyncLogEntry(i18n("End of HotSync\n"));
	endOfSync();
}

int KPilotDeviceLink::getNextDatabase(int index,struct DBInfo *dbinfo)
{
	FUNCTIONSETUP;

#if PILOT_LINK_NUMBER < PILOT_LINK_0_12_0
	return dlp_ReadDBList(pilotSocket(),0,dlpDBListRAM,index,dbinfo);
#else
	pi_buffer_t buf = { 0,0,0 };
	int r = dlp_ReadDBList(pilotSocket(),0,dlpDBListRAM,index,&buf);
	if (r >= 0)
	{
		memcpy(dbinfo,buf.data,sizeof(struct DBInfo));
	}
	return r;
#endif
}

// Find a database with the given name. Info about the DB is stored into dbinfo (e.g. to be used later on with retrieveDatabase).
int KPilotDeviceLink::findDatabase(const char *name, struct DBInfo *dbinfo,
	int index, long type, long creator)
{
	FUNCTIONSETUP;
	return dlp_FindDBInfo(pilotSocket(), 0, index,
		const_cast<char *>(name), type, creator, dbinfo);
}

bool KPilotDeviceLink::retrieveDatabase(const QString &fullBackupName,
	DBInfo *info)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGDAEMON << fname << ": Writing DB <" << info->name << "> "
		<< " to " << fullBackupName << endl;
#endif

	struct pi_file *f;
	if (fullBackupName.isEmpty())
	{
		// Don't even bother trying to convert or retrieve.
		return false;
	}
	QCString encodedName = QFile::encodeName(fullBackupName);
	char filenameBuf[PATH_MAX];
	memset(filenameBuf,0,PATH_MAX);
	strlcpy(filenameBuf,(const char *)encodedName,PATH_MAX);
	f = pi_file_create(filenameBuf,info);

	if (f == 0)
	{
		kdWarning() << k_funcinfo
			<< ": Failed, unable to create file" << endl;
		return false;
	}

#if PILOT_LINK_NUMBER < PILOT_LINK_0_12_0
	if (pi_file_retrieve(f, pilotSocket(), 0) < 0)
#else
	if (pi_file_retrieve(f, pilotSocket(), 0, 0L) < 0)
#endif
	{
		kdWarning() << k_funcinfo
			<< ": Failed, unable to back up database" << endl;

		pi_file_close(f);
		return false;
	}

	pi_file_close(f);
	return true;
}


QPtrList<DBInfo> KPilotDeviceLink::getDBList(int cardno, int flags)
{
	bool cont=true;
	QPtrList<DBInfo>dbs;
	int index=0;
	while (cont)
	{
#if PILOT_LINK_NUMBER < PILOT_LINK_0_12_0
		DBInfo*dbi=new DBInfo();
		if (dlp_ReadDBList(pilotSocket(), cardno, flags, index, dbi)<0)
		{
			KPILOT_DELETE(dbi);
			cont=false;
		}
		else
		{
			index=dbi->index+1;
			dbs.append(dbi);
		}
#else
		pi_buffer_t buf = { 0,0,0 };
		pi_buffer_clear(&buf);
		// DBInfo*dbi=new DBInfo();
		if (dlp_ReadDBList(pilotSocket(), cardno, flags | dlpDBListMultiple, index, &buf)<0)
		{
			cont=false;
		}
		else
		{
			DBInfo *db_n = 0L;
			DBInfo *db_it = (DBInfo *)buf.data;
			int info_count = buf.used / sizeof(struct DBInfo);

			while(info_count>0)
			{
				db_n = new DBInfo();
				memcpy(db_n,db_it,sizeof(struct DBInfo));
				++db_it;
				info_count--;
				dbs.append(db_n);
			}
			if (db_n)
			{
				index=db_n->index+1;
			}
		}
#endif
	}
	return dbs;
}

KPilotCard *KPilotDeviceLink::getCardInfo(int card)
{
	KPilotCard *cardinfo=new KPilotCard();
	if (dlp_ReadStorageInfo(pilotSocket(), card, cardinfo->cardInfo())<0)
	{
		kdWarning() << k_funcinfo << ": Could not get info for card "
			<< card << endl;

		KPILOT_DELETE(cardinfo);
		return 0L;
	};
	return cardinfo;
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
	dlp_ReadFeature(pilotSocket(),
		makelong(const_cast<char *>("psys")), 1, &rom);
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

/* static */ const int KPilotDeviceLink::messagesType=
	(int)OpenFailMessage ;

void KPilotDeviceLink::shouldPrint(int m,const QString &s)
{
	if (!(messages & m))
	{
		if (messagesType & m) { emit logError(s); }
		else { emit logMessage(s); }
		messages |= (m & messagesMask);
	}
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
