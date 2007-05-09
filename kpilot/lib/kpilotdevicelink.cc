/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
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
#include <qthread.h>
//Added by qt3to4:
#include <QCustomEvent>
#include <QApplication>

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

#include "kpilotlink.h"
#include "kpilotdevicelink.moc"


// singleton helper class
class DeviceMap
{
public:
	static DeviceMap *self()
	{
		if (!mThis) mThis = new DeviceMap();
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
	DeviceMap() {}
	~DeviceMap() {}

	QStringList mBoundDevices;
	static DeviceMap *mThis;

private:
	inline void showList() const
	{
		if ( !(mBoundDevices.count() > 0) ) return;
		FUNCTIONSETUPL(3);
		DEBUGKPILOT << fname << ": Bound devices: ["
			<< ((mBoundDevices.count() > 0) ?
			mBoundDevices.join(CSL1(", ")) : CSL1("<none>")) 
			<< "]" << endl;
	}
} ;

DeviceMap *DeviceMap::mThis = 0L;

class Messages
{
public:
	Messages(KPilotDeviceLink *parent) :
		fDeviceLink(parent)
	{
		reset();
	}

	void reset()
	{
		messages = 0;
		messagesMask = ~messageIsError; // Never block errors
	}

	void block(unsigned int m, bool force=false)
	{
		if (force)
		{
			// Force blocking this message, even if it's an error msg.
			messages |= m;
		}
		else
		{
			messages |= (m & messagesMask);
		}
	}

	/**
	* Some messages are only printed once and are suppressed
	* after that. These are indicated by flag bits in
	* messages. The following enum is a bitfield.
	*/
	enum {
		OpenMessage=1, ///< Trying to open device ..
		OpenFailMessage=2 ///< Failed to open device ...
	} ;
	int messages;
	int messagesMask;
	static const int messageIsError = 0;

	/** Determines whether message @p s which has an id of @p msgid (one of
	 *  the enum values mentioned above) should be printed, which is only if that
	 *  message has not been suppressed through messagesMask.
	 *  If return is true, this method also adds it to the messagesMask.
	 */
	bool shouldPrint(int msgid)
	{
		if (!(messages & msgid))
		{
			block(msgid);
			return true;
		}
		else
		{
			return false;
		}
	}
	
protected:
	KPilotDeviceLink *fDeviceLink;
} ;

class DeviceCommEvent : public QEvent
{
public:
	DeviceCommEvent ( DeviceCustomEvents type, QString msg = QString(), int progress = 0)
		: QEvent( (QEvent::Type)type ),
		fMessage( msg ),
		fProgress( progress ),
		fPilotSocket(-1) {}
	QString message() const
	{
		return fMessage;
	}
	int progress()
	{
		return fProgress;
	}

	inline void setCurrentSocket(int i) { fPilotSocket = i; }

	inline int currentSocket() { return fPilotSocket; }
private:
	QString fMessage;
	int fProgress;
	/**
	* Pilot-link library handles for the device once it's opened.
	*/
	int fPilotSocket;
};


/** Class that handles all device communications.  We do this
    in a different thread so that we do not block the main Qt
    Event thread (similar to Swing's AWT event dispatch thread).
*/

class DeviceCommThread : public QThread
{
friend class KPilotDeviceLink;
public:
	DeviceCommThread(KPilotDeviceLink *d) :
		QThread(),
		fDone(true),
		fHandle(d),
		fPilotSocket(-1),
		fTempSocket(-1)
	{ };
	virtual ~DeviceCommThread();

	virtual void run();

	static const int SecondsBetweenPoll = 1;

	void setDone(bool b) 
	{ 
		FUNCTIONSETUP;
		fDone = b; 
	}

private:
	volatile bool fDone;

	KPilotDeviceLink *fHandle;
	inline KPilotDeviceLink *link() 
	{
		if (fHandle) 
		{
			return fHandle;
		}
		else
		{
			FUNCTIONSETUP;
			WARNINGKPILOT << "Link asked for, but either I'm "
				<< "done or I don't have a valid handle.  "
				<< "Shutting down comm thread." << endl;
			QThread::exit();
			return 0;
		}
	}

	/**
	* Pilot-link library handles for the device once it's opened.
	*/
	int fPilotSocket;
	int fTempSocket;

protected:
	/**
	* Attempt to open the device. Called regularly to check
	* if the device exists (to handle USB-style devices).  This does
	* everything necessary to connect to a Palm device.  If the
	* open was successful, this method returns true, otherwise false.
	*/
	bool openDevice();

	void close();

protected:
	/**
	* Does the low-level opening of the device and handles the
	* pilot-link library initialisation.
	*/
	bool open( const QString &device = QString::null );
	
private:
	inline QString errorMessage(int e) 
	{
		switch (e)
		{
		case ENOENT:
			return i18n(" The port does not exist.");
			break;
		case ENODEV:
			return i18n(" There is no such device.");
			break;
		case EPERM:
			return i18n(" You do not have permission to open the "
				"Pilot device.");
			break;
		default:
			return i18n(" Check Pilot path and permissions.");
		}
	}

} ;

DeviceCommThread::~DeviceCommThread()
{
	FUNCTIONSETUPL(2);
	close();
}


bool DeviceCommThread::openDevice()
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
			i18n("Trying to open device %1...",link()->fPilotPath)));
	}

	// if we're not supposed to be done, try to open the main pilot
	// path...
	if (!fDone && link()->fPilotPath.length() > 0)
	{
		DEBUGKPILOT << fname << ": Opening main pilot path: [" 
			<< link()->fPilotPath << "]." << endl;
		deviceOpened = open( link()->fPilotPath ); 
	}

	// only try the temp device if our earlier attempt didn't work and the temp
	// device is different than the main device, and it's a non-empty
	// string
	bool tryTemp = !deviceOpened && 
			(link()->fTempDevice.length() > 0) &&
			(link()->fPilotPath != link()->fTempDevice) ;

	// if we're not supposed to be done, and we should try the temp
	// device, try the temp device...
	if (!fDone && tryTemp)
	{
		DEBUGKPILOT << fname << ": Couldn't open main pilot path. "
			<< "Now trying temp device: [" 
			<< link()->fTempDevice << "]." << endl;
		deviceOpened = open( link()->fTempDevice );
	}

	return deviceOpened;
}

bool DeviceCommThread::open(const QString &device)
{
	FUNCTIONSETUPL(2);

	int ret;
	int e = 0;
	QString msg;

	link()->fRealPilotPath = KStandardDirs::realFilePath(device.isEmpty() ? link()->fPilotPath : device );

	if ( !DeviceMap::self()->canBind( link()->fRealPilotPath ) ) {
		msg = i18n("Already listening on that device");
		
		WARNINGKPILOT << "Pilot Path: ["
			<< link()->fRealPilotPath << "] already connected." << endl;
		WARNINGKPILOT << msg << endl;

		link()->fLinkStatus = PilotLinkError;

		QApplication::postEvent(link(),
			new DeviceCommEvent(EventLogError, msg));

		return false;
	}


	DEBUGKPILOT << fname << ": Trying to create socket." << endl;

	fTempSocket = pi_socket(PI_AF_PILOT, PI_SOCK_STREAM, PI_PF_DLP);

	if (fTempSocket < 0)
	{
		e = errno;
		msg = i18n("Cannot create socket for communicating "
			"with the Pilot (%1)",errorMessage(e));
		WARNINGKPILOT << msg << endl;
		WARNINGKPILOT << "(" << strerror(e) << ")" << endl;
	
		link()->fLinkStatus = PilotLinkError;
		
		QApplication::postEvent(link(),
			new DeviceCommEvent(EventLogError, msg));
	
		return false;
	}

	DEBUGKPILOT << fname << ": Got socket: [" << fTempSocket << "]" << endl;

	link()->fLinkStatus = CreatedSocket;

	DEBUGKPILOT << fname << ": Binding to path: [" 
		<< link()->fRealPilotPath << "]" << endl;

	ret = pi_bind(fTempSocket, QFile::encodeName(link()->fRealPilotPath));

	if (ret < 0)
	{
		DEBUGKPILOT << fname
			<< ": pi_bind error: ["
			<< strerror(errno) << "]" << endl;

		e = errno;
		msg = i18n("Cannot open Pilot port \"%1\". ",link()->fRealPilotPath);
		
		DEBUGKPILOT << msg << " (" << strerror(e) << ')' << endl;
	
		link()->fLinkStatus = PilotLinkError;
		
		if (link()->fMessages->shouldPrint(Messages::OpenFailMessage))
		{
			QApplication::postEvent(link(),
				new DeviceCommEvent(EventLogError, msg));
		}

		return false;
	}

	link()->fLinkStatus = DeviceOpen;
	DeviceMap::self()->bindDevice( link()->fRealPilotPath );

	DEBUGKPILOT << fname
		<< ": Current status: ["
		<< link()->statusString()
		<< "] and socket: [" << fTempSocket << "]" << endl;

	ret = pi_listen(fTempSocket, 1);
	if (ret < 0)
	{
		char *s = strerror(errno);

		WARNINGKPILOT << "pi_listen returned: [" << s << "]" << endl;

		// Presumably, strerror() returns things in
		// local8Bit and not latin1.
		QApplication::postEvent(link(),
			new DeviceCommEvent(EventLogError,
				i18n("Cannot listen on Pilot socket (%1)",QString::fromLocal8Bit(s)))
			);

		return false;
	}

	QApplication::postEvent(link(),
		new DeviceCommEvent(EventLogProgress, QString::null, 10));

	DEBUGKPILOT << fname <<
		": Listening to pilot. Now trying accept..." << endl;

	int timeout=20;
	if (link()->fWorkaroundUSB)
		timeout=10;

	fPilotSocket = pi_accept_to(fTempSocket, 0, 0, timeout);

	if (fPilotSocket < 0)
	{
		char *s = strerror(errno);

		WARNINGKPILOT << "pi_accept returned: [" << s << "]" << endl;

		QApplication::postEvent(link(),
			new DeviceCommEvent(EventLogError, i18n("Cannot accept Pilot (%1)",QString::fromLocal8Bit(s))));

		link()->fLinkStatus = PilotLinkError;
		return false;
	}

	DEBUGKPILOT << fname << ": Link accept done." << endl;

	if ((link()->fLinkStatus != DeviceOpen) || (fPilotSocket == -1))
	{
		link()->fLinkStatus = PilotLinkError;
		WARNINGKPILOT << "Already connected or unable to connect!" << endl;

		QApplication::postEvent(link(),
			new DeviceCommEvent(EventLogError, i18n("Cannot accept Pilot (%1)",i18n("already connected"))));

		return false;
	}

	QApplication::postEvent(link(),
		new DeviceCommEvent(EventLogProgress, QString::null, 30));

	DEBUGKPILOT << fname << ": doing dlp_ReadSysInfo..." << endl;

	struct SysInfo sys_info;
	if (dlp_ReadSysInfo(fPilotSocket, &sys_info) < 0)
	{
		QApplication::postEvent(link(),
			new DeviceCommEvent(EventLogError,
			i18n("Unable to read system information from Pilot")));

		link()->fLinkStatus=PilotLinkError;
		return false;
	}
	else
	{
		DEBUGKPILOT << fname << ": dlp_ReadSysInfo successful..." << endl;

        	KPILOT_DELETE(link()->fPilotSysInfo);
		link()->fPilotSysInfo = new KPilotSysInfo(&sys_info);
		DEBUGKPILOT << fname
			<< ": RomVersion: [" << link()->fPilotSysInfo->getRomVersion()
			<< "] Locale: [" << link()->fPilotSysInfo->getLocale()
			<< "] Product: [" << link()->fPilotSysInfo->getProductID()
			<< "]" << endl;
	}

	QApplication::postEvent(link(),
		new DeviceCommEvent(EventLogProgress, QString::null, 60));

        KPILOT_DELETE(link()->fPilotUser);
	link()->fPilotUser = new KPilotUser;

	DEBUGKPILOT << fname << ": doing dlp_ReadUserInfo..." << endl;

	/* Ask the pilot who it is.  And see if it's who we think it is. */
	dlp_ReadUserInfo(fPilotSocket, link()->fPilotUser->data());

	QString n = link()->getPilotUser().name();
	DEBUGKPILOT << fname
		<< ": Read user name: [" << n << "]" << endl;

	QApplication::postEvent(link(),
		new DeviceCommEvent(EventLogProgress, i18n("Checking last PC..."), 90));

	/* Tell user (via Pilot) that we are starting things up */
	if ((ret=dlp_OpenConduit(fPilotSocket)) < 0)
	{
		DEBUGKPILOT << fname
			<< ": dlp_OpenConduit returned: [" << ret << "]" << endl;

		QApplication::postEvent(link(),
			new DeviceCommEvent(EventLogError,
			i18n("Could not read user information from the Pilot. "
			"Perhaps you have a password set on the device?")));

	}
	link()->fLinkStatus = AcceptedDevice;

	QApplication::postEvent(link(),
		new DeviceCommEvent(EventLogProgress, QString::null, 100));

	return true;
}

void DeviceCommThread::close()
{
	FUNCTIONSETUPL(4);

	pi_close(fTempSocket);

	if (fPilotSocket != -1)
	{
		DEBUGKPILOT << fname
			<< ": device comm thread closing socket: ["
			<< fPilotSocket << "]" << endl;

		pi_close(fPilotSocket);
	}
	fPilotSocket = (-1);

	DeviceMap::self()->unbindDevice( link()->fRealPilotPath );
}


void DeviceCommThread::run()
{
	FUNCTIONSETUP;
	int sleepBetweenPoll = SecondsBetweenPoll;
	fDone = false;

	DEBUGKPILOT << fname << ": Polling every: ["
		<< sleepBetweenPoll << "] seconds." << endl;

	while (!fDone)
	{
		openDevice();

		if (link()->fLinkStatus == AcceptedDevice)
		{
			DeviceCommEvent * ev = new DeviceCommEvent(EventDeviceReady);
			ev->setCurrentSocket(fPilotSocket);
			QApplication::postEvent(link(), ev);

			break;
		}
		else
		{
			if (link()->fMessages->shouldPrint(Messages::OpenFailMessage))
			{
				QApplication::postEvent(link(), 
					new DeviceCommEvent(EventLogMessage, 
						i18n("Could not open device: %1 (will retry)",link()->fPilotPath)));
			}
			close();

			// sleep before trying again
			QThread::sleep(sleepBetweenPoll);
		}
	}

	DEBUGKPILOT << fname << ": comm thread waiting to be done..." << endl;

	// keep the thread alive until we're supposed to be done
	while (!fDone)
	{
		QThread::sleep(sleepBetweenPoll);
	}

	close();
	// now sleep one last bit to make sure the pthread inside
	// pilot-link (potentially, if it's libusb) is done before we exit
	QThread::sleep(1);

	DEBUGKPILOT << fname << ": comm thread now done..." << endl;
}




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

	DEBUGKPILOT << fname
		<< ": Pilot-link version: [" << PILOT_LINK_NUMBER
		<< "]" << endl;
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

void KPilotDeviceLink::customEvent(QCustomEvent *e)
{
	FUNCTIONSETUP;

	if ((int)e->type() == EventDeviceReady)
	{
		DeviceCommEvent* t = dynamic_cast<DeviceCommEvent*>(e);
		if (t)
		{
			fPilotSocket = t->currentSocket();
			emit deviceReady( this );
		}
		else
		{
			DEBUGKPILOT << fname
				<< ": unable to cast event: [" << e << "]" << endl;
		}
	}
	else if ((int)e->type() == EventLogMessage)
	{
		DeviceCommEvent* t = dynamic_cast<DeviceCommEvent*>(e);
		if (t)
		{
			emit logMessage(t->message());
		}
		else
		{
			DEBUGKPILOT << fname
				<< ": unable to cast event: [" << e << "]" << endl;
		}
	}
	else if ((int)e->type() == EventLogError)
	{
		DeviceCommEvent* t = dynamic_cast<DeviceCommEvent*>(e);
		if (t)
		{
			emit logError(t->message());
		}
		else
		{
			DEBUGKPILOT << fname
				<< ": unable to cast event: [" << e << "]" << endl;
		}
	}
	else if ((int)e->type() == EventLogProgress)
	{
		DeviceCommEvent* t = dynamic_cast<DeviceCommEvent*>(e);
		if (t)
		{
			emit logProgress(t->message(), t->progress());
		}
		else
		{
			DEBUGKPILOT << fname
				<< ": unable to cast event: [" << e << "]" << endl;
		}
	}
	else
	{
		KPilotLink::customEvent(e);
	}
}

void KPilotDeviceLink::stopCommThread()
{
	FUNCTIONSETUP;
	if (fDeviceCommThread)
	{
		fDeviceCommThread->setDone(true);

		// try to wait for our thread to finish, but don't
		// block the main thread forever
		if (fDeviceCommThread->running()) 
		{
			DEBUGKPILOT << fname
				<< ": comm thread still running. "
				<< "waiting for it to complete." << endl;
			bool done = fDeviceCommThread->wait(30000);
			if (!done)
			{
				DEBUGKPILOT << fname
					<< ": comm thread still running "
					<< "after wait(). "
					<< "going to have to terminate it."
					<< endl;
				// not normally to be done, but we must make sure
				// that this device doesn't come back alive
				fDeviceCommThread->terminate();
				fDeviceCommThread->wait();
			}
		}

		fDeviceCommThread->close();

		KPILOT_DELETE(fDeviceCommThread);
	}
}

void KPilotDeviceLink::close()
{
	FUNCTIONSETUP;

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
	fPilotPath = QString::null;

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
		WARNINGKPILOT << "No point in trying empty device."
			<< endl;

		QString msg = i18n("The Pilot device is not configured yet.");
		WARNINGKPILOT << msg << endl;


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
	// No FUNCTIONSETUP here because it may be called from
	// a separate thread.
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

	DEBUGKPILOT << fname << ": Installing file " << f << endl;

	if (!QFile::exists(f))
		return false;

	char buffer[PATH_MAX];
	memset(buffer,0,PATH_MAX);
	strlcpy(buffer,QFile::encodeName(f),PATH_MAX);
	struct pi_file *pf =
		pi_file_open(buffer);

	if (f.isEmpty())
	{
		WARNINGKPILOT << "Cannot open file " << f << endl;
		emit logError(i18n
			("<qt>Cannot install the file &quot;%1&quot;.</qt>").
			arg(f));
		return false;
	}

	if (pi_file_install(pf, fPilotSocket, 0, 0L) < 0)
	{
		WARNINGKPILOT << "Cannot pi_file_install " << f << endl;
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

		DEBUGKPILOT << fname << ": Writing username " << getPilotUser().name() << endl;

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

	DEBUGKPILOT << fname << ": Writing DB <" << info->name << "> "
		<< " to " << fullBackupName << endl;

	QByteArray encodedName = QFile::encodeName(fullBackupName);
	struct pi_file *f = pi_file_create(encodedName,info);

	if (!f)
	{
		WARNINGKPILOT << "Failed, unable to create file" << endl;
		return false;
	}

	if (pi_file_retrieve(f, pilotSocket(), 0, 0L) < 0)
	{
		WARNINGKPILOT << "Failed, unable to back up database" << endl;

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
		WARNINGKPILOT << "Could not get info for card "
			<< card << endl;

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

