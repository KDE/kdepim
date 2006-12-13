/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
** Copyright (C) 2006 Adriaan de Groot <groot@kde.org>
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
#include <pi-buffer.h>

#include <qdir.h>
#include <qtimer.h>
#include <qdatetime.h>
#include <qsocketnotifier.h>
#include <qthread.h>

#include <kconfig.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kio/netaccess.h>

#include "pilotUser.h"
#include "pilotSysInfo.h"
#include "pilotCard.h"
#include "pilotSerialDatabase.h"
#include "pilotLocalDatabase.h"

#include "kpilotlink.moc"
#include "kpilotdevicelink.moc"
#include "kpilotlocallink.moc"

/** Class that handles periodically tickling the handheld through
*   the virtual tickle() method; deals with cancels through the
*   shared fDone variable.
*/
class TickleThread : public QThread
{
public:
	TickleThread(KPilotLink *d, bool *done, int timeout) :
		QThread(),
		fHandle(d),
		fDone(done),
		fTimeout(timeout)
	{ };
	virtual ~TickleThread();

	virtual void run();

	static const int ChecksPerSecond = 5;
	static const int SecondsPerTickle = 5;
	static const unsigned int TickleTimeoutEvent = 1066;

private:
	KPilotLink *fHandle;
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
	DEBUGLIBRARY << fname << ": Running for " << timeout << " seconds." << endl;
	DEBUGLIBRARY << fname << ": Done @" << (void *) fDone << endl;
#endif
	while (!(*fDone))
	{
		QThread::msleep(1000/ChecksPerSecond);
		if (!(--subseconds))
		{
#ifdef DEBUG_CERR
// Don't dare use kdDebug() here, we're in a separate thread
			DEBUGLIBRARY << fname << ": One second." << endl;
#endif
			if (timeout)
			{
				if (!(--timeout))
				{
					QApplication::postEvent(fHandle, new QEvent(static_cast<QEvent::Type>(TickleTimeoutEvent)));
					break;
				}
			}
			subseconds=ChecksPerSecond;
			if (!(--ticktock))
			{
#ifdef DEBUG_CERR
				DEBUGLIBRARY << fname << ": Kietel kietel!." << endl;
#endif
				ticktock=SecondsPerTickle;
				fHandle->tickle();
			}
		}
	}
#ifdef DEBUG_CERR
	DEBUGLIBRARY << fname << ": Finished." << endl;
#endif
}








KPilotLink::KPilotLink( QObject *parent, const char *name ) :
	QObject( parent, name ),
	fPilotPath(QString::null),
	fPilotUser(0L),
	fPilotSysInfo(0L),
	fTickleDone(true),
	fTickleThread(0L)

{
	FUNCTIONSETUP;

	fPilotUser = new KPilotUser();
	strncpy( fPilotUser->pilotUser()->username, "Henk Westbroek",
		sizeof(fPilotUser->pilotUser()->username)-1);
	fPilotUser->setLastSuccessfulSyncDate( 1139171019 );

	fPilotSysInfo = new KPilotSysInfo();
	memset(fPilotSysInfo->sysInfo()->prodID, 0,
		sizeof(fPilotSysInfo->sysInfo()->prodID));
	strncpy(fPilotSysInfo->sysInfo()->prodID, "LocalLink",
		sizeof(fPilotSysInfo->sysInfo()->prodID)-1);
	fPilotSysInfo->sysInfo()->prodIDLength =
		strlen(fPilotSysInfo->sysInfo()->prodID);
}

KPilotLink::~KPilotLink()
{
	FUNCTIONSETUP;
	KPILOT_DELETE(fPilotUser);
	KPILOT_DELETE(fPilotSysInfo);
}

/* virtual */ bool KPilotLink::event(QEvent *e)
{
	if (e->type() == TickleThread::TickleTimeoutEvent)
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
void KPilotLink::startTickle(unsigned int timeout)
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
	DEBUGLIBRARY << fname << ": Done @" << (void *) (&fTickleDone) << endl;
#endif
	fTickleDone = false;
	fTickleThread = new TickleThread(this,&fTickleDone,timeout);
	fTickleThread->start();
}

void KPilotLink::stopTickle()
{
	FUNCTIONSETUP;
	fTickleDone = true;
	if (fTickleThread)
	{
		fTickleThread->wait();
		KPILOT_DELETE(fTickleThread);
	}
}

unsigned int KPilotLink::installFiles(const QStringList & l, const bool deleteFiles)
{
	FUNCTIONSETUP;

	QStringList::ConstIterator i,e;
	unsigned int k = 0;
	unsigned int n = 0;
	unsigned int total = l.count();

	for (i = l.begin(), e = l.end(); i != e; ++i)
	{
		emit logProgress(QString::null,
			(int) ((100.0 / total) * (float) n));

		if (installFile(*i, deleteFiles))
			k++;
		n++;
	}
	emit logProgress(QString::null, 100);

	return k;
}

void KPilotLink::addSyncLogEntry(const QString & entry, bool log)
{
	FUNCTIONSETUP;
	if (entry.isEmpty()) return;

	addSyncLogEntryImpl(entry);
	if (log)
	{
		emit logMessage(entry);
	}
}


/* virtual */ int KPilotLink::openConduit()
{
	return 0;
}

/* virtual */ int KPilotLink::pilotSocket() const
{
	return -1;
}




// singleton helper class
class KPilotDeviceLink::KPilotDeviceLinkPrivate
{
public:
	static KPilotDeviceLinkPrivate*self()
	{
		if (!mThis) mThis = new KPilotDeviceLinkPrivate();
		return mThis;
	}

	bool canBind( const QString &device )
	{
		showList();
		return !mBoundDevices.contains( device );
	}

	void bindDevice( const QString &device )
	{
		mBoundDevices.append( device );
		showList();
	}

	void unbindDevice( const QString &device )
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
		if ( !(mBoundDevices.count() > 0) ) return;
#ifdef DEBUG
		FUNCTIONSETUPL(3);
		DEBUGLIBRARY << fname << ": Bound devices: "
			<< ((mBoundDevices.count() > 0) ? mBoundDevices.join(CSL1(", ")) : CSL1("<none>")) << endl;
#endif
	}
} ;

KPilotDeviceLink::KPilotDeviceLinkPrivate *KPilotDeviceLink::KPilotDeviceLinkPrivate::mThis = 0L;


KPilotDeviceLink::KPilotDeviceLink(QObject * parent, const char *name, const QString &tempDevice) :
	KPilotLink(parent, name),
	fLinkStatus(Init),
	fWorkaroundUSB(false),
	fWorkaroundUSBTimer(0L),
	fRetries(0),
	fOpenTimer(0L),
	fSocketNotifier(0L),
	fSocketNotifierActive(false),
	fPilotMasterSocket(-1),
	fCurrentPilotSocket(-1),
	fTempDevice(tempDevice)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGLIBRARY << fname
		<< ": Pilot-link version " << PILOT_LINK_NUMBER
		<< endl;
#endif

	messagesMask=0xffffffff;

}

KPilotDeviceLink::~KPilotDeviceLink()
{
	FUNCTIONSETUP;
	close();
	KPILOT_DELETE(fWorkaroundUSBTimer);
	KPILOT_DELETE(fPilotSysInfo);
	KPILOT_DELETE(fPilotUser);
}

/* virtual */ bool KPilotDeviceLink::isConnected() const
{
	 return fLinkStatus == AcceptedDevice;
}


void KPilotDeviceLink::close()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fWorkaroundUSBTimer);
	KPILOT_DELETE(fOpenTimer);
	KPILOT_DELETE(fSocketNotifier);
	fSocketNotifierActive=false;
#ifdef DEBUG
	DEBUGLIBRARY << fname
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
		messages |= (OpenMessage | OpenFailMessage);
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

bool KPilotDeviceLink::open(const QString &device)
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

	if (device.isEmpty() && pilotPath().isEmpty())
	{
		kdWarning() << k_funcinfo
			<< ": No point in trying empty device."
			<< endl;

		msg = i18n("The Pilot device is not configured yet.");
		e = 0;
		goto errInit;
	}
	fRealPilotPath = KStandardDirs::realFilePath( device.isEmpty() ? fPilotPath : device );

	if ( !KPilotDeviceLinkPrivate::self()->canBind( fRealPilotPath ) ) {
		msg = i18n("Already listening on that device");
		e=0;
		kdWarning() << k_funcinfo << ": Pilot Path "
			<< fRealPilotPath << " already connected." << endl;
		goto errInit;
	}


	if (fPilotMasterSocket == -1)
	{
		DEBUGLIBRARY << fname << ": Typing to open "
			<< fRealPilotPath << endl;

		fPilotMasterSocket = pi_socket(PI_AF_PILOT,
			PI_SOCK_STREAM, PI_PF_DLP);

		if (fPilotMasterSocket<1)
		{
			e = errno;
			msg = i18n("Cannot create socket for communicating "
				"with the Pilot");
			goto errInit;
		}

		DEBUGLIBRARY << fname
			<< ": Got master " << fPilotMasterSocket << endl;

		fLinkStatus = CreatedSocket;
	}

	Q_ASSERT(fLinkStatus == CreatedSocket);

	DEBUGLIBRARY << fname << ": Binding to path "
		<< fRealPilotPath << endl;

	ret = pi_bind(fPilotMasterSocket, QFile::encodeName(fRealPilotPath));

	if (ret >= 0)
	{
		fLinkStatus = DeviceOpen;
		if( fOpenTimer)
		fOpenTimer->stop();

		KPilotDeviceLinkPrivate::self()->bindDevice( fRealPilotPath );
		fSocketNotifier = new QSocketNotifier(fPilotMasterSocket,
			QSocketNotifier::Read, this);
		QObject::connect(fSocketNotifier, SIGNAL(activated(int)),
			this, SLOT(acceptDevice()));
		fSocketNotifierActive=true;

		if (fWorkaroundUSB)
		{
			DEBUGLIBRARY << fname << ": Adding Z31 workaround." << endl;
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
		DEBUGLIBRARY << fname
			<< ": Tried " << fRealPilotPath << " and got "
			<< strerror(errno) << endl;

		if (fRetries < 5)
		{
			return false;
		}
		e = errno;
		msg = i18n("Cannot open Pilot port \"%1\". ");
		if (fOpenTimer )
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
		if (fRealPilotPath.isEmpty())
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
		else
		{
			msg = msg.arg(fRealPilotPath);
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
	DEBUGLIBRARY << fname
		<< ": Found connection on device "<<pilotPath().latin1()<<endl;
	DEBUGLIBRARY << fname
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
		DEBUGLIBRARY << fname
			<< ": RomVersion=" << fPilotSysInfo->getRomVersion()
			<< " Locale=" << fPilotSysInfo->getLocale()
			<< " Product=" << fPilotSysInfo->getProductID()
			<< endl;
	}
#endif

	emit logProgress(QString::null, 60);
        KPILOT_DELETE(fPilotUser);
	fPilotUser = new KPilotUser;

	/* Ask the pilot who it is.  And see if it's who we think it is. */
#ifdef DEBUG
	DEBUGLIBRARY << fname << ": Reading user info @"
		<< (void *) fPilotUser << endl;
	DEBUGLIBRARY << fname << ": Buffer @"
		<< (void *) fPilotUser->pilotUser() << endl;
#endif

	dlp_ReadUserInfo(fCurrentPilotSocket, fPilotUser->pilotUser());

#ifdef DEBUG
	const char *n = fPilotUser->getUserName();
	DEBUGLIBRARY << fname
		<< ": Read user name "
		<< ( (!n || !*n) ?
			"<empty>" :
			fPilotUser->getUserName() )
		<< endl;
#endif

	emit logProgress(i18n("Checking last PC..."), 90);

	/* Tell user (via Pilot) that we are starting things up */
	if ((ret=dlp_OpenConduit(fCurrentPilotSocket)) < 0)
	{
#ifdef DEBUG
		DEBUGLIBRARY << k_funcinfo
			<< ": dlp_OpenConduit returned " << ret << endl;
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

/* virtual */ bool KPilotDeviceLink::tickle()
{
	// No FUNCTIONSETUP here because it may be called from
	// a separate thread.
	return pi_tickle(pilotSocket()) >= 0;
}

/* virtual */ void KPilotDeviceLink::addSyncLogEntryImpl( const QString &entry )
{
	dlp_AddSyncLogEntry(fCurrentPilotSocket,
		const_cast<char *>((const char *)Pilot::toPilot(entry)));
}

bool KPilotDeviceLink::installFile(const QString & f, const bool deleteFile)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGLIBRARY << fname << ": Installing file " << f << endl;
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

	if (pi_file_install(pf, fCurrentPilotSocket, 0, 0L) < 0)
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


int KPilotDeviceLink::openConduit()
{
	return dlp_OpenConduit(fCurrentPilotSocket);
}

QString KPilotDeviceLink::statusString(LinkStatus l)
{
	FUNCTIONSETUP;
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

void KPilotDeviceLink::endOfSync()
{
	dlp_EndOfSync(pilotSocket(), 0);
	KPILOT_DELETE(fPilotSysInfo);
	KPILOT_DELETE(fPilotUser);
}

void KPilotDeviceLink::finishSync()
{
	FUNCTIONSETUP ;

	getPilotUser().setLastSyncPC((unsigned long) gethostid());
	getPilotUser().setLastSyncDate(time(0));


#ifdef DEBUG
	DEBUGLIBRARY << fname << ": Writing username " << getPilotUser().getUserName() << endl;
#endif
	dlp_WriteUserInfo(pilotSocket(),getPilotUser().pilotUser());
	addSyncLogEntry(i18n("End of HotSync\n"));
	endOfSync();
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

bool KPilotDeviceLink::retrieveDatabase(const QString &fullBackupName,
	DBInfo *info)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGLIBRARY << fname << ": Writing DB <" << info->name << "> "
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

	if (pi_file_retrieve(f, pilotSocket(), 0, 0L) < 0)
	{
		kdWarning() << k_funcinfo
			<< ": Failed, unable to back up database" << endl;

		pi_file_close(f);
		return false;
	}

	pi_file_close(f);
	return true;
}


DBInfoList KPilotDeviceLink::getDBList(int cardno, int flags)
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
		kdWarning() << k_funcinfo << ": Could not get info for card "
			<< card << endl;

		KPILOT_DELETE(cardinfo);
		return 0L;
	};
	return cardinfo;
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

PilotDatabase *KPilotDeviceLink::database( const QString &name )
{
	return new PilotSerialDatabase( this, name );
}


typedef QPair<QString, struct DBInfo> DatabaseDescriptor;
typedef QValueList<DatabaseDescriptor> DatabaseDescriptorList;

class KPilotLocalLink::Private
{
public:
	DatabaseDescriptorList fDBs;
} ;

unsigned int KPilotLocalLink::findAvailableDatabases( KPilotLocalLink::Private &info, const QString &path )
{
	FUNCTIONSETUP;

	info.fDBs.clear();

	QDir d(path);
	if (!d.exists())
	{
		// Perhaps return an error?
		return 0;
	}

	// Use this to fake indexes in the list of DBInfo structs
	unsigned int counter = 0;

	QStringList dbs = d.entryList( CSL1("*.pdb"), QDir::Files | QDir::NoSymLinks | QDir::Readable );
	for ( QStringList::ConstIterator i = dbs.begin(); i != dbs.end() ; ++i)
	{
		struct DBInfo dbi;

		// Remove the trailing 4 characters
		QString dbname = (*i);
		dbname.remove(dbname.length()-4,4);
#ifdef DEBUG
		QString dbnamecheck = (*i).left((*i).findRev(CSL1(".pdb")));
		Q_ASSERT(dbname == dbnamecheck);
#endif
		if (PilotLocalDatabase::infoFromFile( path + CSL1("/") + (*i), &dbi))
		{
			DEBUGLIBRARY << fname << ": Loaded "
				<< dbname << endl;
			dbi.index = counter;
			info.fDBs.append( DatabaseDescriptor(dbname,dbi) );
			++counter;
		}
	}

	DEBUGLIBRARY << fname << ": Total " << info.fDBs.count()
		<< " databases." << endl;
	return info.fDBs.count();
}


KPilotLocalLink::KPilotLocalLink( QObject *parent, const char *name ) :
	KPilotLink(parent,name),
	fReady(false),
	d( new Private )
{
	FUNCTIONSETUP;
}

KPilotLocalLink::~KPilotLocalLink()
{
	FUNCTIONSETUP;
	KPILOT_DELETE(d);
}

/* virtual */ QString KPilotLocalLink::statusString() const
{
	return fReady ? CSL1("Ready") : CSL1("Waiting") ;
}

/* virtual */ bool KPilotLocalLink::isConnected() const
{
	return fReady;
}

/* virtual */ void KPilotLocalLink::reset( const QString &p )
{
	FUNCTIONSETUP;
	fPath = p;
	reset();
}

/* virtual */ void KPilotLocalLink::reset()
{
	FUNCTIONSETUP;
	QFileInfo info( fPath );
	fReady = !fPath.isEmpty() && info.exists() && info.isDir() ;
	if (fReady)
	{
		findAvailableDatabases(*d, fPath);
		QTimer::singleShot(500,this,SLOT(ready()));
	}
	else
	{
		kdWarning() << k_funcinfo << ": The local link path "
			<< fPath
			<< " does not exist or is not a direcotory. No sync will be done."
			<< endl;
	}
}

/* virtual */ void KPilotLocalLink::close()
{
	fReady = false;
}

/* virtual */ bool KPilotLocalLink::tickle()
{
	return true;
}

/* virtual */ const KPilotCard *KPilotLocalLink::getCardInfo(int)
{
	return 0;
}

/* virtual */ void KPilotLocalLink::endOfSync()
{
}

/* virtual */ void KPilotLocalLink::finishSync()
{
}

/* virtual */ int KPilotLocalLink::openConduit()
{
	FUNCTIONSETUP;
	return 0;
}


/* virtual */ int KPilotLocalLink::getNextDatabase( int index, struct DBInfo *info )
{
	FUNCTIONSETUP;

	if ( (index<0) || (index>=(int)d->fDBs.count()) )
	{
		kdWarning() << k_funcinfo << ": Index out of range." << endl;
		return -1;
	}

	DatabaseDescriptor dd = d->fDBs[index];

	DEBUGLIBRARY << fname << ": Getting database " << dd.first << endl;

	if (info)
	{
		*info = dd.second;
	}

	return index+1;
}

/* virtual */ int KPilotLocalLink::findDatabase(const char *name, struct DBInfo*info,
		int index, unsigned long type, unsigned long creator)
{
	FUNCTIONSETUP;

	if ( (index<0) || (index>=(int)d->fDBs.count()) )
	{
		kdWarning() << k_funcinfo << ": Index out of range." << endl;
		return -1;
	}

	if (!name)
	{
		kdWarning() << k_funcinfo << ": NULL name." << endl;
		return -1;
	}

	QString desiredName = Pilot::fromPilot(name);
	DEBUGLIBRARY << fname << ": Looking for DB " << desiredName << endl;
	for ( DatabaseDescriptorList::ConstIterator i = d->fDBs.at(index);
		i != d->fDBs.end(); ++i)
	{
		const DatabaseDescriptor &dd = *i;
		if (dd.first == desiredName)
		{
			if ( (!type || (type == dd.second.type)) &&
				(!creator || (creator == dd.second.creator)) )
			{
				if (info)
				{
					*info = dd.second;
				}
				return index;
			}
		}

		++index;
	}

	return -1;
}

/* virtual */ void KPilotLocalLink::addSyncLogEntryImpl(QString const &s)
{
	FUNCTIONSETUP;
	DEBUGLIBRARY << fname << ": " << s << endl ;
}

/* virtual */ bool KPilotLocalLink::installFile(QString const &path, bool deletefile)
{
	FUNCTIONSETUP;

	QFileInfo srcInfo(path);
	QString canonicalSrcPath = srcInfo.dir().canonicalPath() + CSL1("/") + srcInfo.fileName() ;
	QString canonicalDstPath = fPath + CSL1("/") + srcInfo.fileName();

	if (canonicalSrcPath == canonicalDstPath)
	{
		// That's a cheap copy operation
		return true;
	}

	KURL src = KURL::fromPathOrURL( canonicalSrcPath );
	KURL dst = KURL::fromPathOrURL( canonicalDstPath );

	KIO::NetAccess::file_copy(src,dst,-1,true);

	if (deletefile)
	{
		KIO::NetAccess::del(src, 0L);
	}

	return true;
}

/* virtual */ bool KPilotLocalLink::retrieveDatabase( const QString &path, struct DBInfo *db )
{
	FUNCTIONSETUP;

	QString dbname = Pilot::fromPilot(db->name) + CSL1(".pdb") ;
	QString sourcefile = fPath + CSL1("/") + dbname ;
	QString destfile = path ;

	DEBUGLIBRARY << fname << ": src=" << sourcefile << endl;
	DEBUGLIBRARY << fname << ": dst=" << destfile << endl;

	QFile in( sourcefile );
	if ( !in.exists() )
	{
		kdWarning() << k_funcinfo<< ": Source file " << sourcefile << " doesn't exist." << endl;
		return false;
	}
	if ( !in.open( IO_ReadOnly | IO_Raw ) )
	{
		kdWarning() << k_funcinfo << ": Can't read source file " << sourcefile << endl;
		return false;
	}

	QFile out( destfile );
	if ( !out.open( IO_WriteOnly | IO_Truncate | IO_Raw ) )
	{
		kdWarning() << k_funcinfo << ": Can't write destination file " << destfile << endl;
		return false;
	}

	const Q_ULONG BUF_SIZ = 8192 ;
	char buf[BUF_SIZ];
	Q_LONG r;

	while ( (r=in.readBlock(buf,BUF_SIZ))>0 )
	{
		out.writeBlock(buf,r);
	}
	out.flush();
	in.close();

	return out.exists();
}

/* virtual */ DBInfoList KPilotLocalLink::getDBList( int, int )
{
	FUNCTIONSETUP;
	DBInfoList l;
	for ( DatabaseDescriptorList::ConstIterator i=d->fDBs.begin();
		i != d->fDBs.end(); ++i)
	{
		l.append( (*i).second );
	}
	return l;
}


/* virtual */ PilotDatabase *KPilotLocalLink::database( const QString &name )
{
	FUNCTIONSETUP;
	return new PilotLocalDatabase( fPath, name );
}



/* slot */ void KPilotLocalLink::ready()
{
	if (fReady)
	{
		emit deviceReady(this);
	}
}

