/* pilotDaemon.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This is the KPilot Daemon, which does the actual communication with
** the Pilot and with the conduits.
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/
static const char *pilotdaemon_id =
	"$Id$";

// Heck yeah.
#define ENABLE_KROUPWARE

#ifndef _KPILOT_OPTIONS_H
#include "options.h"
#endif

#include <time.h>

#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

#include <qdir.h>
#include <qptrlist.h>
#include <qcursor.h>
#include <qptrstack.h>
#include <qtimer.h>
#include <qtooltip.h>

#include <kuniqueapplication.h>
#include <kaboutdata.h>
#include <kaboutapplication.h>
#include <kcmdlineargs.h>
#include <kwin.h>
#include <ksimpleconfig.h>
#include <kurl.h>
#include <ksock.h>
#include <kstandarddirs.h>
#include <kpopupmenu.h>
#include <kiconloader.h>
#include <kio/netaccess.h>
#include <kdebug.h>
#include <ktempfile.h>
#include <kprocess.h>
#include <dcopclient.h>
#include <kurldrag.h>

#include "pilotAppCategory.h"

#include "fileInstaller.h"
#include "kpilotConfig.h"
#include "pilotUser.h"

#include "hotSync.h"
#include "interactiveSync.h"
#include "syncStack.h"
#include "internalEditorAction.h"
#include "logFile.h"

#ifdef ENABLE_KROUPWARE
#include "kroupware.h"
#endif


#include "kpilotDCOP_stub.h"
#include "kpilotDCOP.h"
#include "loggerDCOP_stub.h"

#include "pilotDaemon.moc"


PilotDaemonTray::PilotDaemonTray(PilotDaemon * p) :
	KSystemTray(0, "pilotDaemon"),
	daemon(p),
	kap(0L)
{
	FUNCTIONSETUP;
	setupWidget();
	setAcceptDrops(true);


	/* NOTREACHED */
	(void) pilotdaemon_id;
}

/* virtual */ void PilotDaemonTray::dragEnterEvent(QDragEnterEvent * e)
{
	FUNCTIONSETUP;
	e->accept(KURLDrag::canDecode(e));
}

/* virtual */ void PilotDaemonTray::dropEvent(QDropEvent * e)
{
	FUNCTIONSETUP;

	KURL::List list;

	KURLDrag::decode(e, list);

	QStringList files;
	for(KURL::List::ConstIterator it = list.begin(); it != list.end(); ++it)
	{
	   if ((*it).isLocalFile())
	      files << (*it).path();
	}

	daemon->addInstallFiles(files);
}

/* virtual */ void PilotDaemonTray::mousePressEvent(QMouseEvent * e)
{
	FUNCTIONSETUP;

	switch (e->button())
	{
		case RightButton:
			{
				KPopupMenu *menu = contextMenu();
				contextMenuAboutToShow(menu);
				menu->popup(e->globalPos());
			}
			break;
		case LeftButton:
			if (daemon) daemon->slotRunKPilot();
			break;
		default:
			KSystemTray::mousePressEvent(e);
	}
}

/* virtual */ void PilotDaemonTray::closeEvent(QCloseEvent *)
{
	FUNCTIONSETUP;
	daemon->quitNow();
}

void PilotDaemonTray::setupWidget()
{
	FUNCTIONSETUP;

	KGlobal::iconLoader()->addAppDir(CSL1("kpilot"));
	icon = SmallIcon(CSL1("hotsync"));
	busyicon = SmallIcon(CSL1("busysync"));

	slotShowBusy();
	QTimer::singleShot(2000,this,SLOT(slotShowNormal()));

	KPopupMenu *menu = contextMenu();

	menu->insertItem(i18n("&About"), this, SLOT(slotShowAbout()));
	menuKPilotItem = menu->insertItem(i18n("Start &KPilot"), daemon,
		SLOT(slotRunKPilot()));

	menuConfigureConduitsItem = menu->insertItem(i18n("&Configure KPilot..."),
		daemon, SLOT(slotRunConfig()));

#ifdef DEBUG
	DEBUGDAEMON << fname << ": Finished getting icons" << endl;
#endif
}

void PilotDaemonTray::slotShowAbout()
{
	FUNCTIONSETUP;

	if (!kap)
	{
		kap = new KAboutApplication(0, "kpdab", false);
	}

	kap->show();
}


void PilotDaemonTray::enableRunKPilot(bool b)
{
	FUNCTIONSETUP;
	contextMenu()->setItemEnabled(menuKPilotItem, b);
	contextMenu()->setItemEnabled(menuConfigureConduitsItem, b);
}


void PilotDaemonTray::changeIcon(IconShape i)
{
	FUNCTIONSETUP;

	switch (i)
	{
	case Normal:
		if (icon.isNull())
		{
			kdWarning() << k_funcinfo
				<< ": Regular icon is NULL!" << endl;
		}
		setPixmap(icon);
		break;
	case Busy:
		if (busyicon.isNull())
		{
			kdWarning() << k_funcinfo
				<< ": Busy icon is NULL!" << endl;
		}
		setPixmap(busyicon);
		break;
	default:
		kdWarning() << k_funcinfo
			<< ": Bad icon number " << (int) i << endl;
	}
}

void PilotDaemonTray::slotShowNormal()
{
	FUNCTIONSETUP;
	changeIcon(Normal);
}

void PilotDaemonTray::slotShowBusy()
{
	FUNCTIONSETUP;
	changeIcon(Busy);
}



PilotDaemon::PilotDaemon() :
	DCOPObject("KPilotDaemonIface"),
	fDaemonStatus(INIT),
	fPostSyncAction(None),
	fPilotLink(0L),
	fNextSyncType(PilotDaemonDCOP::HotSync),
	fSyncStack(0L),
	fTray(0L),
	fInstaller(0L),
	fLogFile(0L),
	fLogStub(new LoggerDCOP_stub("kpilot", "LogIface")),
	fLogFileStub(new LoggerDCOP_stub("kpilotDaemon", "LogIface")),
	fKPilotStub(new KPilotDCOP_stub("kpilot", "KPilotIface"))
{
	FUNCTIONSETUP;

	setupPilotLink();
	reloadSettings();

	if (fDaemonStatus == ERROR)
	{
		kdWarning() << k_funcinfo
			<< ": Connecting to device failed." << endl;
		return;
	}

	fInstaller = new FileInstaller;
	fLogFile = new LogFile;
	connect(fInstaller, SIGNAL(filesChanged()),
		this, SLOT(slotFilesChanged()));


#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": The daemon is ready with status "
		<< statusString() << " (" << (int) fDaemonStatus << ")" << endl;
#endif
}

PilotDaemon::~PilotDaemon()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fPilotLink);
	KPILOT_DELETE(fSyncStack);
	KPILOT_DELETE(fInstaller);
}

void PilotDaemon::addInstallFiles(const QStringList &l)
{
	FUNCTIONSETUP;

	fInstaller->addFiles( l, fTray );
}

int PilotDaemon::getPilotSpeed()
{
	FUNCTIONSETUP;

	int speed = KPilotSettings::pilotSpeed();

	// Translate the speed entry in the
	// config file to something we can
	// put in the environment (for who?)
	//
	//
	const char *speedname = 0L;

	switch (speed)
	{
	case 0:
		speedname = "PILOTRATE=9600";
		break;
	case 1:
		speedname = "PILOTRATE=19200";
		break;
	case 2:
		speedname = "PILOTRATE=38400";
		break;
	case 3:
		speedname = "PILOTRATE=57600";
		break;
	case 4:
		speedname = "PILOTRATE=115200";
		break;
	default:
		speedname = "PILOTRATE=9600";
	}

#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": Speed set to "
		<< speedname << " (" << speed << ")" << endl;
#endif

	putenv((char *) speedname);

	return speed;
}


void PilotDaemon::showTray()
{
	FUNCTIONSETUP;

	if (!fTray)
	{
#ifdef DEBUG
		DEBUGDAEMON << fname << ": No tray icon to display!" << endl;
#endif

		return;
	}

	// Copied from Klipper
	KWin::setSystemTrayWindowFor(fTray->winId(), 0);
	fTray->setGeometry(-100, -100, 42, 42);
	fTray->show();

#ifdef DEBUG
	DEBUGDAEMON << fname << ": Tray icon displayed." << endl;
#endif

	updateTrayStatus();
}

/* DCOP ASYNC */ void PilotDaemon::reloadSettings()
{
	FUNCTIONSETUP;

	switch (fDaemonStatus)
	{
	case INIT:
	case HOTSYNC_END:
	case ERROR:
	case READY:
		// It's OK to reload settings in these states.
		break;
	case HOTSYNC_START:
	case FILE_INSTALL_REQ:
		// Postpone the reload till the sync finishes.
		fPostSyncAction |= ReloadSettings;
		return;
		break;
	}

	// TODO: Is this bunch of calls really necessary to reload the settings???
	delete KPilotSettings::self();
	KPilotSettings::self()->config()->reparseConfiguration();
	KPilotSettings::self()->readConfig();
	getPilotSpeed();

//	fPilotDevice = KPilotSettings::pilotDevice();
	fPilotType = KPilotDeviceLink::None;

	(void) PilotAppCategory::setupPilotCodec(KPilotSettings::encoding());

#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": Got configuration "
		<< KPilotSettings::pilotDevice()
		<< " ("
		<< fPilotType
		<< ")"
		<< endl;
	DEBUGDAEMON << fname
		<< ": Got conduit list "
		<< (KPilotSettings::installedConduits().join(","))
		<< endl;
#endif


	/*
	** Override the kind of device, since OldStyleUSB
	** works with everything and it saves the user from
	** havind to choose the right kind.
	*/
	fPilotType = KPilotDeviceLink::OldStyleUSB;

	if (fPilotLink)
	{
#ifdef DEBUG
		DEBUGDAEMON << fname
			<< ": Resetting with device "
			<< KPilotSettings::pilotDevice()
			<< " and type "
			<< fPilotLink->deviceTypeString(fPilotType) << endl;
#endif

		fPilotLink->reset(fPilotType, KPilotSettings::pilotDevice());
	}

	if (KPilotSettings::dockDaemon())
	{
		if (!fTray)
		{
			fTray = new PilotDaemonTray(this);
			fTray->show();
		}
		else
		{
			fTray->show();
		}
	}
	else
	{
		if (fTray)
		{
			fTray->hide();
			delete fTray;

			fTray = 0L;
		}
	}

	updateTrayStatus();
}

/* DCOP */ void PilotDaemon::stopListening()
{
// TODO
	fIsListening=false;
}

/* DCOP */ void PilotDaemon::startListening()
{
// TODO
	fIsListening=true;
}

/* DCOP */ QString PilotDaemon::statusString()
{
	FUNCTIONSETUP;

	QString s = CSL1("PilotDaemon=");

	switch (status())
	{
	case INIT:
		s.append(CSL1("Initializing"));
		break;
	case READY:
		s.append(CSL1("Found device"));
		break;
	case ERROR:
		s.append(CSL1("Error"));
		break;
	case FILE_INSTALL_REQ:
		s.append(CSL1("Installing File"));
		break;
	case HOTSYNC_END:
		s.append(CSL1("End of Hotsync"));
		break;
	case HOTSYNC_START:
		s.append(CSL1("Syncing"));
		break;
	}

	s.append(CSL1(" NextSync="));
	s.append(syncTypeString(fNextSyncType));

	s.append(CSL1(" ("));
	if (fPilotLink)
	{
		s.append(fPilotLink->statusString());
	}
	s.append(CSL1(")"));

	return s;
}

/* DCOP */ QString PilotDaemon::shortStatusString()
{
	FUNCTIONSETUP;

	QString s;

	switch (status())
	{
	case INIT:
	case READY:
		s=CSL1("Waiting for sync");
		break;
	case ERROR:
		s=CSL1("Error");
		break;
	case FILE_INSTALL_REQ:
		s=CSL1("Installing File");
		break;
	case HOTSYNC_END:
		s=CSL1("End of Hotsync");
		break;
	case HOTSYNC_START:
		s=CSL1("Syncing");
		break;
	}

	return s;
}



bool PilotDaemon::setupPilotLink()
{
	FUNCTIONSETUP;

	if (fPilotLink)
	{
		delete fPilotLink;

		fPilotLink = 0;
	}

	fPilotLink = KPilotDeviceLink::init();
	if (!fPilotLink)
	{
		kdWarning() << k_funcinfo
			<< ": Can't get pilot link." << endl;
		return false;
	}

	QObject::connect(fPilotLink, SIGNAL(deviceReady()),
		this, SLOT(startHotSync()));
	// connect the signals emitted by the pilotDeviceLink
	QObject::connect(fPilotLink, SIGNAL(logError(const QString &)),
		this, SLOT(logError(const QString &)));
	QObject::connect(fPilotLink, SIGNAL(logMessage(const QString &)),
		this, SLOT(logMessage(const QString &)));
	QObject::connect(fPilotLink,
		SIGNAL(logProgress(const QString &,int)),
		this, SLOT(logProgress(const QString &,int)));


	return true;
}


/* DCOP ASYNC */ void PilotDaemon::quitNow()
{
	FUNCTIONSETUP;
	// Using switch to make sure we cover all the cases.
	//
	//
	switch (fDaemonStatus)
	{
	case INIT:
	case HOTSYNC_END:
	case ERROR:
		getKPilot().daemonStatus(KPilotDCOP::DaemonQuit);
		kapp->quit();
		break;
	case READY:
	case HOTSYNC_START:
	case FILE_INSTALL_REQ:
		fPostSyncAction |= Quit;
		break;
	}
	emitDCOPSignal( "kpilotDaemonStatusChanged()", QByteArray() );
}

/* DCOP ASYNC */ void PilotDaemon::requestRegularSyncNext()
{
	requestSync(PilotDaemonDCOP::HotSync);
}

/* DCOP ASYNC */ void PilotDaemon::requestFastSyncNext()
{
	requestSync(PilotDaemonDCOP::FastSync);
}


/* DCOP ASYNC */ void PilotDaemon::requestSync(int mode)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": Next sync is: "
		<< syncTypeString(mode)
		<< endl ;
#endif

	fNextSyncType = mode;

	updateTrayStatus();
}

/* DCOP */ int PilotDaemon::nextSyncType() const
{
	return fNextSyncType;
}

QString PilotDaemon::syncTypeString(int i) const
{
	FUNCTIONSETUP;
	switch (i)
	{
	case PilotDaemonDCOP::Test:
		return QString(CSL1("Test"));
	case PilotDaemonDCOP::HotSync:
		return QString(CSL1("HotSync"));
	case PilotDaemonDCOP::FastSync:
		return QString(CSL1("FastSync"));
	case PilotDaemonDCOP::Backup:
		return QString(CSL1("Backup"));
	case PilotDaemonDCOP::Restore:
		return QString(CSL1("Restore"));
	default:
		return QString(CSL1("<unknown>"));
	}
}

/**
* DCOP Functions reporting some status data, e.g. for the kontact plugin.
*/
QDateTime PilotDaemon::lastSyncDate()
{
	return KPilotSettings::lastSyncTime();
}
QStringList PilotDaemon::configuredConduitList()
{
	return KPilotSettings::installedConduits();
}
QString PilotDaemon::logFileName()
{
	return KPilotSettings::logFileName();
}

QString PilotDaemon::userName()
{
	return KPilotSettings::userName();
}
QString PilotDaemon::pilotDevice()
{
	return KPilotSettings::pilotDevice();
}

static bool isKDesktopLockRunning()
{
	if (!KPilotSettings::screenlockSecure()) return false;

	DCOPClient *dcopptr = KApplication::kApplication()->dcopClient();

	// Can't tell, very weird, err on the side of safety.
	if (!dcopptr || !dcopptr->isAttached()) return true;

	QByteArray data,returnValue;
	QCString returnType;

	if (!dcopptr->call("kdesktop","KScreensaverIface","isBlanked()",
		data,returnType,returnValue,true))
	{
		kdWarning() << k_funcinfo << ": Check for screensaver failed." << endl;
		// Err on the side of safety again.
		return true;
	}

	if (returnType == "bool")
	{
		bool b;
		QDataStream reply(returnValue,IO_ReadOnly);
		reply >> b;
		return b;
	}
	else
	{
		// Err on the side of safety.
		return true;
	}
}


/* slot */ void PilotDaemon::startHotSync()
{
	FUNCTIONSETUP;


	if (fTray)
	{
#ifdef DEBUG
		DEBUGKPILOT << fname << ": Changing tray icon." << endl;
#endif

		fTray->changeIcon(PilotDaemonTray::Busy);
	}

	// Tell KPilot what's going on.
	getKPilot().daemonStatus(KPilotDCOP::StartOfHotSync);
	getLogger().logStartSync();
	getFileLogger().logStartSync();

	fDaemonStatus = HOTSYNC_START ;
	int mode=0;
	bool pcchanged=false;

#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": Starting Sync with type "
		<< syncTypeString(fNextSyncType)
		<< " (" << fNextSyncType << ")" << endl;
	DEBUGDAEMON << fname << ": Status is " << shortStatusString() << endl;
#endif

	QStringList conduits( KPilotSettings::installedConduits() );
	if ( (conduits.findIndex( CSL1("internal_fileinstall") ) >= 0) &&
		fInstaller) mode |= ActionQueue::WithInstaller;

	// Queue to add all the actions for this sync to.
	fSyncStack = new ActionQueue(fPilotLink);

#ifdef ENABLE_KROUPWARE
	bool _syncWithKMail = false;
	int _kroupwareParts = 0;
#endif

	/**
	* If KPilot is busy with something - like configuring
	* conduit - then we shouldn't run a real sync, but
	* just tell the user that the sync couldn't run because
	* of that.
	*/
	int kpilotstatus = getKPilot().kpilotStatus();
	DCOPStub::Status callstatus = getKPilot().status();

#ifdef DEBUG
	if (callstatus != DCOPStub::CallSucceeded)
	{
		DEBUGDAEMON << fname <<
			": Could not call KPilot for status." << endl;
	}
	else
	{
		DEBUGDAEMON << fname << ": KPilot status " << kpilotstatus << endl;
	}
#endif
	/**
	* If the call fails, then KPilot is probably not running
	* and we can behave normally.
	*/
	if ((callstatus == DCOPStub::CallSucceeded) &&
		(kpilotstatus != KPilotDCOP::WaitingForDaemon))
	{
		kdWarning() << k_funcinfo <<
			": KPilot returned status " << kpilotstatus << endl;

		fSyncStack->queueInit();
		fSyncStack->addAction(new SorryAction(fPilotLink));
		// Near the end of this function - sets up
		// signal/slot connections and fires off the sync.
		goto launch;
	}

	if (isKDesktopLockRunning())
	{
		fSyncStack->queueInit();
		fSyncStack->addAction(new SorryAction(fPilotLink,
			i18n("HotSync is disabled while the screen is locked.")));
		goto launch;
	}

	// Normal case: regular sync.
	fSyncStack->queueInit(ActionQueue::WithUserCheck);


#ifdef ENABLE_KROUPWARE
	if ( conduits.findIndex( CSL1("internal_kroupware") ) >= 0 )
	{
		logMessage( i18n("Kroupware syncing is enabled.") );

		QString errmsg;
		if (!KroupwareSync::startKMail(&errmsg))
		{
			logMessage( i18n("Could not start KMail. The "
				"error message was: %1.").arg(errmsg));
		}

		_syncWithKMail = true;

		if (conduits.findIndex( CSL1("vcal-conduit") ) >= 0 )
			_kroupwareParts |= KroupwareSync::Cal ;
		if (conduits.findIndex( CSL1("todo-conduit") ) >= 0 )
			_kroupwareParts |= KroupwareSync::Todo ;
		if (conduits.findIndex( CSL1("knotes-conduit") ) >= 0 )
			_kroupwareParts |= KroupwareSync::Notes ;
		if (conduits.findIndex( CSL1("abbrowser_conduit") ) >= 0 )
			_kroupwareParts |= KroupwareSync::Address ;
	}

	if (_syncWithKMail)
	{
		fSyncStack->addAction(new KroupwareSync(true /* pre-sync */,
			_kroupwareParts,fPilotLink));
	}
#endif

	switch (fNextSyncType)
	{
	case PilotDaemonDCOP::Test:
		fSyncStack->addAction(new TestLink(fPilotLink));
		// No conduits, nothing.
		break;
	case PilotDaemonDCOP::Backup:
		mode |= ActionQueue::BackupMode | ActionQueue::FlagFull;
		if (KPilotSettings::runConduitsWithBackup() && (conduits.count() > 0))
		{
			fSyncStack->queueConduits(conduits, mode);
		}
		fSyncStack->addAction(new BackupAction(fPilotLink, mode));
		break;
	case PilotDaemonDCOP::Restore:
		mode |= ActionQueue::RestoreMode | ActionQueue::FlagFull;
		fSyncStack->addAction(new RestoreAction(fPilotLink));
		if (mode & ActionQueue::WithInstaller)
		{
			fSyncStack->queueInstaller(fInstaller->dir(),
				fInstaller->fileNames());
		}
		break;
	case PilotDaemonDCOP::FastSync:
	case PilotDaemonDCOP::HotSync:
		// first install the files, and only then do the conduits
		// (conduits might want to sync a database that will be installed
		mode |= ActionQueue::HotSyncMode;
		if (mode & ActionQueue::WithInstaller)
		{
			fSyncStack->queueInstaller(fInstaller->dir(),
				fInstaller->fileNames());
		}

		if (PilotDaemonDCOP::FastSync == fNextSyncType) goto skipExtraSyncSettings;

		switch (KPilotSettings::syncType())
		{
		case SyncAction::eFastSync:
			break;
		case SyncAction::eHotSync:
			mode |= ActionQueue::WithBackup;
			break;
		case SyncAction::eFullSync:
			mode |= ActionQueue::WithBackup | ActionQueue::FlagFull;
			break;
		case SyncAction::eCopyPCToHH:
			mode |= ActionQueue::FlagPCToHH;
			break;
		case SyncAction::eCopyHHToPC:
			mode |= ActionQueue::FlagHHToPC;
			break;
		}
skipExtraSyncSettings:

		if (KPilotSettings::internalEditors() && !(mode & ActionQueue::FlagHHToPC) )
		{
			fSyncStack->addAction(new InternalEditorAction(fPilotLink, mode));
		}
		// Now check for changed PC
		{
		KPilotUser *usr = fPilotLink->getPilotUser();
		// changing the PC or using a different Palm Desktop app causes a full sync
		// Use gethostid for this, since JPilot uses 1+(2000000000.0*random()/(RAND_MAX+1.0))
		// as PC_ID, so using JPilot and KPilot is the same as using two differenc PCs
		pcchanged = usr->getLastSyncPC() !=(unsigned long) gethostid();
		}

		if (conduits.count() > 0)
		{
			fSyncStack->queueConduits( conduits, pcchanged?(mode|ActionQueue::FlagFull):mode);
		}
		if (pcchanged && KPilotSettings::fullSyncOnPCChange())
			mode |=  (ActionQueue::WithBackup | ActionQueue::FlagFull);
#ifdef DEBUG
		DEBUGDAEMON << fname
			<< ": Sync Mode="
			<< mode << ", Sync Type="<< KPilotSettings::syncType()<<endl;
#endif
		if (mode & ActionQueue::WithBackup)
			fSyncStack->addAction(new BackupAction(fPilotLink, mode));
		break;
	default:
#ifdef DEBUG
		DEBUGDAEMON << fname
			<< ": Can't handle sync type "
			<< syncTypeString(fNextSyncType) << endl;
#endif
		break;
	}

#ifdef ENABLE_KROUPWARE
	if (_syncWithKMail)
	{
		fSyncStack->addAction(new KroupwareSync(false /* post-sync */ ,
			_kroupwareParts,fPilotLink));
	}
#endif

// Jump here to finalize the connections to the sync action
// queue and start the actual sync.
launch:
	fSyncStack->queueCleanup();

	QObject::connect(fSyncStack, SIGNAL(logError(const QString &)),
		this, SLOT(logError(const QString &)));
	QObject::connect(fSyncStack, SIGNAL(logMessage(const QString &)),
		this, SLOT(logMessage(const QString &)));
	QObject::connect(fSyncStack,
		SIGNAL(logProgress(const QString &,int)),
		this, SLOT(logProgress(const QString &,int)));

	QObject::connect(fSyncStack, SIGNAL(syncDone(SyncAction *)),
		this, SLOT(endHotSync()));

	QTimer::singleShot(0,fSyncStack,SLOT(execConduit()));

	updateTrayStatus();
}

/* slot */ void PilotDaemon::logMessage(const QString & s)
{
	FUNCTIONSETUPL(2);

	getLogger().logMessage(s);
	getFileLogger().logMessage(s);
	updateTrayStatus(s);
}

/* slot */ void PilotDaemon::logError(const QString & s)
{
	FUNCTIONSETUP;

	getLogger().logError(s);
	getFileLogger().logError(s);
	updateTrayStatus(s);
}

/* slot */ void PilotDaemon::logProgress(const QString & s, int i)
{
	FUNCTIONSETUPL(2);

	getLogger().logProgress(s, i);
	getFileLogger().logProgress(s, i);
	if (!s.isEmpty()) updateTrayStatus(s);
}

/* slot */ void PilotDaemon::endHotSync()
{
	FUNCTIONSETUP;

	if (fTray)
	{
		QTimer::singleShot(2000,fTray,SLOT(slotShowNormal()));
	}

	KPILOT_DELETE(fSyncStack);
	fPilotLink->close();

	getLogger().logProgress(i18n("HotSync Completed.<br>"), 100);
	getFileLogger().logProgress(i18n("HotSync Completed.<br>"), 100);
	getLogger().logEndSync();
	getFileLogger().logEndSync();
	getKPilot().daemonStatus(KPilotDCOP::EndOfHotSync);
	KPilotSettings::setLastSyncTime(QDateTime::currentDateTime());
	KPilotSettings::self()->writeConfig();

	fDaemonStatus = HOTSYNC_END;

	if (fPostSyncAction & Quit)
	{
		getKPilot().daemonStatus(KPilotDCOP::DaemonQuit);
		kapp->quit();
	}
	if (fPostSyncAction & ReloadSettings)
	{
		reloadSettings();
	}
	else
	{
		QTimer::singleShot(5000,fPilotLink,SLOT(reset()));
	}

	fPostSyncAction = None;
	requestRegularSyncNext();

	updateTrayStatus();
}


void PilotDaemon::slotFilesChanged()
{
	FUNCTIONSETUP;
}

void PilotDaemon::slotRunKPilot()
{
	FUNCTIONSETUP;

	QString kpilotError;
	QCString kpilotDCOP;
	int kpilotPID;

	if (KApplication::startServiceByDesktopName(CSL1("kpilot"),
			QString::null, &kpilotError, &kpilotDCOP, &kpilotPID
#if (KDE_VERSION >= 220)
			// Startup notification added in 2.2
			, ""
#endif
		))
	{
		kdWarning() << k_funcinfo
			<< ": Couldn't start KPilot! " << kpilotError << endl;
	}
	else
	{
#ifdef DEBUG
		DEBUGDAEMON << fname
			<< ": Started KPilot with DCOP name "
			<< kpilotDCOP << " (pid " << kpilotPID << ")" << endl;
#endif
	}
}

void PilotDaemon::slotRunConfig()
{
	FUNCTIONSETUP;

	// This function tries to send the raise() DCOP call to kpilot.
	// If it succeeds, we can assume kpilot is running and then try
	// to send the configure() DCOP call.
	// If it fails (probably because kpilot isn't running) it tries
	// to call kpilot via KProcess (using a command line switch to
	// only bring up the configure dialog).
	//
	// Implementing the function this way catches all cases.
	// ie 1 KPilot running with configure dialog open (raise())
	//    2 KPilot running with dialog NOT open (configureConduits())
	//    3 KPilot NOT running (KProcess)

	DCOPClient *client = kapp->dcopClient();

	// This DCOP call to kpilot's raise function solves the final case
	// ie when kpilot already has the dialog open

	if ( client->isApplicationRegistered( "kpilot" ) )
	{
		client->send("kpilot", "kpilot-mainwindow#1", "raise()",QString::null);
		client->send("kpilot", "KPilotIface", "configure()", QString::null);
	}
	else
	{
		// KPilot not running
		KProcess *p = new KProcess;
		*p << "kpilot" << "-c";

		p->start();
	}
}

void PilotDaemon::updateTrayStatus(const QString &s)
{
	if (!fTray) return;

	QToolTip::remove(fTray);
	QToolTip::add(fTray,
		i18n("<qt>%1<br/>%2</qt>")
			.arg(statusString())
			.arg(s)
		);
	emitDCOPSignal( "kpilotDaemonStatusChanged()", QByteArray() );
}

static KCmdLineOptions daemonoptions[] = {
#ifdef DEBUG
	{"debug <level>", I18N_NOOP("Set debugging level"), "0"},
#endif
	KCmdLineLastOption
} ;


int main(int argc, char **argv)
{
	FUNCTIONSETUP;

	KAboutData about("kpilotDaemon",
		I18N_NOOP("KPilot Daemon"),
		KPILOT_VERSION,
		"KPilot - HotSync software for KDE\n\n",
		KAboutData::License_GPL,
		"(c) 1998-2000,2001, Dan Pilone (c) 2000-2004, Adriaan de Groot",
		0L,
		"http://www.slac.com/~pilone/kpilot_home/"
		);
	about.addAuthor("Dan Pilone",
		I18N_NOOP("Project Leader"),
		"pilone@slac.com");
	about.addAuthor("Adriaan de Groot",
		I18N_NOOP("Maintainer"),
		"groot@kde.org", "http://www.cs.kun.nl/~adridg/");
	about.addAuthor("Reinhold Kainhofer",
		I18N_NOOP("Developer"),
		"reinhold@kainhofer.com", "http://reinhold.kainhofer.com/Linux/");

	KCmdLineArgs::init(argc, argv, &about);
	KCmdLineArgs::addCmdLineOptions(daemonoptions,"kpilotconfig");
	KUniqueApplication::addCmdLineOptions();
	KCmdLineArgs *p = KCmdLineArgs::parsedArgs();

#ifdef DEBUG
	KPilotConfig::getDebugLevel(p);
#endif

	if (!KUniqueApplication::start())
	{
		return 0;
	}
	KUniqueApplication a(true, true);

	// A block just to keep variables local.
	//
	//
	{
//		KPilotSettings::self()->config()->setReadOnly(false);

		if (KPilotSettings::configVersion() < KPilotConfig::ConfigurationVersion)
		{
			kdError() << k_funcinfo
				<< ": Is still not configured for use."
				<< endl;
			KPilotConfig::sorryVersionOutdated(KPilotSettings::configVersion());
			return 1;
		}

#ifdef DEBUG
		DEBUGDAEMON << fname
			<< ": Configuration version "
			<< KPilotSettings::configVersion() << endl;
#endif
	}


	PilotDaemon *gPilotDaemon = new PilotDaemon();

	if (gPilotDaemon->status() == PilotDaemon::ERROR)
	{
		delete gPilotDaemon;

		gPilotDaemon = 0;
		kdError() << k_funcinfo
			<< ": **\n"
			": Failed to start up daemon\n"
			": due to errors constructing it.\n" ": **" << endl;
		return 2;
	}

	gPilotDaemon->showTray();

	return a.exec();

	/* NOTREACHED */
	(void) pilotdaemon_id;
}



