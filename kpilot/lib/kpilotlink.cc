/* kpilotlink.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
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


#include <qdir.h>
#include <qtimer.h>
#include <qdatetime.h>
#include <qsocketnotifier.h>

#include <kconfig.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

#include "pilotUser.h"
#include "pilotSysInfo.h"
#include "pilotCard.h"

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
		FUNCTIONSETUP;
		showList();
		return !mBoundDevices.contains( device );
	}

	void bindDevice( QString device )
	{
		FUNCTIONSETUP;
		mBoundDevices.append( device );
		showList();
	}

	void unbindDevice( QString device )
	{
		FUNCTIONSETUP;
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
		DEBUGDAEMON << "Bound devices: "
			<< ((mBoundDevices.count() > 0) ? mBoundDevices.join(", ") : CSL1("<none>")) << endl;
#endif
	}
} ;

KPilotDeviceLink::KPilotDeviceLinkPrivate *KPilotDeviceLink::KPilotDeviceLinkPrivate::mThis = 0L;


KPilotDeviceLink::KPilotDeviceLink(QObject * parent, const char *name) :
	QObject(parent, name),
	fLinkStatus(Init),
	fPilotPath(QString::null),
	fRetries(0),
	fOpenTimer(0L),
	fSocketNotifier(0L),
	fSocketNotifierActive(false),
	fPilotMasterSocket(-1),
	fCurrentPilotSocket(-1),
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
	KPILOT_DELETE(fPilotSysInfo);
	KPILOT_DELETE(fPilotUser);
}

void KPilotDeviceLink::close()
{
	FUNCTIONSETUP;

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
		return;

	reset();
}

void KPilotDeviceLink::reset()
{
	FUNCTIONSETUP;

	messages=0;
	close();

	checkDevice();

	// Timer already deleted by close() call.
	fOpenTimer = new QTimer(this);
	QObject::connect(fOpenTimer, SIGNAL(timeout()),
		this, SLOT(openDevice()));
	fOpenTimer->start(1000, false);

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
			"Assuming the device uses DevFS.")
				.arg(fPilotPath));
	}
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
	else
	{
		shouldPrint(OpenFailMessage,i18n("Could not open device: %1 "
				"(will retry)").
				arg(fPilotPath));

		if (fLinkStatus != PilotLinkError)
		{
			fOpenTimer->start(1000, false);
		}
	}
}

bool KPilotDeviceLink::open()
{
	FUNCTIONSETUPL(2);

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

	fRealPilotPath = KStandardDirs::realPath ( pilotPath() );
//	if ( dv.exists() && dv.isSymLink() ) {
//		fRealPilotPath = dv.readLink();
//	}

	if ( !KPilotDeviceLinkPrivate::self()->canBind( fRealPilotPath ) ) {
		msg = i18n("Already listening on that device");
		e=0;
		kdWarning() << k_funcinfo <<": Pilot Path " << pilotPath().latin1() << " already connected." << endl;
		goto errInit;
	}


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
#ifdef DEBUG
		DEBUGDAEMON << fname << ": Typing to open " << fPilotPath << endl;
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

#if PILOT_LINK_NUMBER < PILOT_LINK_0_10_0
	addr.pi_family = PI_AF_SLP;
#else
	addr.pi_family = PI_AF_PILOT;
#endif
	strncpy(addr.pi_device, QFile::encodeName(fPilotPath),sizeof(addr.pi_device));


	ret = pi_bind(fPilotMasterSocket,
		(struct sockaddr *) &addr, sizeof(addr));

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
	fPilotUser->boundsCheck();

#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": Read user name " << fPilotUser->getUserName() << endl;
#endif

	emit logProgress(i18n("Checking last PC..."), 90);

	/* Tell user (via Pilot) that we are starting things up */
	if ((ret=dlp_OpenConduit(fCurrentPilotSocket)) < 0)
	{
		DEBUGDAEMON << k_funcinfo
			<< ": dlp_OpenConduit returned " << ret << endl;

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

void KPilotDeviceLink::tickle() const
{
	FUNCTIONSETUP;
	pi_tickle(pilotSocket());
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

	struct pi_file *pf =
		pi_file_open(const_cast < char *>
			((const char *) QFile::encodeName(f)));

	if (!f)
	{
		kdWarning() << k_funcinfo
			<< ": Cannot open file " << f << endl;
		emit logError(i18n
			("<qt>Cannot install the file &quot;%1&quot;.</qt>").
			arg(f));
		return false;
	}

	if (pi_file_install(pf, fCurrentPilotSocket, 0) < 0)
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
		const_cast < char *>(t.latin1()));
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
	QString s = QString::fromLatin1("KPilotDeviceLink=");


	switch (fLinkStatus)
	{
	case Init:
		s.append(QString::fromLatin1("Init"));
		break;
	case WaitingForDevice:
		s.append(QString::fromLatin1("WaitingForDevice"));
		break;
	case FoundDevice:
		s.append(QString::fromLatin1("FoundDevice"));
		break;
	case CreatedSocket:
		s.append(QString::fromLatin1("CreatedSocket"));
		break;
	case DeviceOpen:
		s.append(QString::fromLatin1("DeviceOpen"));
		break;
	case AcceptedDevice:
		s.append(QString::fromLatin1("AcceptedDevice"));
		break;
	case SyncDone:
		s.append(QString::fromLatin1("SyncDone"));
		break;
	case PilotLinkError:
		s.append(QString::fromLatin1("PilotLinkError"));
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

	dlp_WriteUserInfo(pilotSocket(),getPilotUser()->pilotUser());
	addSyncLogEntry(i18n("End of HotSync\n"));
	endOfSync();
}

int KPilotDeviceLink::getNextDatabase(int index,struct DBInfo *dbinfo)
{
	FUNCTIONSETUP;

	return dlp_ReadDBList(pilotSocket(),0,dlpDBListRAM,index,dbinfo);
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


QPtrList<DBInfo> KPilotDeviceLink::getDBList(int cardno, int flags)
{
	bool cont=true;
	QPtrList<DBInfo>dbs;
	int index=0;
	while (cont)
	{
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
