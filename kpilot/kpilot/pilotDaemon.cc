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
#include <qdragobject.h>
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
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kpopupmenu.h>
#include <kiconloader.h>
#include <kio/netaccess.h>
#include <kdebug.h>
#include <ktempfile.h>
#include <kprocess.h>
#include <dcopclient.h>

#include "pilotAppCategory.h"

#include "fileInstaller.h"
#include "kpilotConfig.h"

#include "hotSync.h"
#include "interactiveSync.h"
#include "syncStack.h"

#ifdef ENABLE_KROUPWARE
#include "kroupware.h"
#endif

#include "kpilotDCOP_stub.h"
#include "kpilotDCOP.h"
#include "logWidgetDCOP_stub.h"

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
	e->accept(QUriDrag::canDecode(e));
}

/* virtual */ void PilotDaemonTray::dropEvent(QDropEvent * e)
{
	FUNCTIONSETUP;

	QStrList list;

	QUriDrag::decode(e, list);

	daemon->addInstallFiles(list);
//	fInstaller->addFiles(list);
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
	icon = KGlobal::iconLoader()->loadIcon(CSL1("hotsync"), KIcon::Toolbar,
		0, KIcon::DefaultState, 0, false);
	busyicon = KGlobal::iconLoader()->loadIcon(CSL1("busysync"), KIcon::Toolbar,
		0, KIcon::DefaultState, 0, false);

	slotShowBusy();
	QTimer::singleShot(2000,this,SLOT(slotShowNormal()));

	KPopupMenu *menu = contextMenu();

	menu->insertItem(i18n("&About"), this, SLOT(slotShowAbout()));
	menuKPilotItem = menu->insertItem(i18n("Start &KPilot"), daemon,
		SLOT(slotRunKPilot()));

	menuConfigureConduitsItem = menu->insertItem(i18n("&Configure Conduits..."),
		daemon, SLOT(slotRunConduitConfig()));

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
	fStatus(INIT),
	fPostSyncAction(None),
	fPilotLink(0L),
	fPilotDevice(QString::null),
	fNextSyncType(PilotDaemonDCOP::HotSync),
	fSyncStack(0L),
	fTray(0L),
	fInstaller(0L),
	fLogStub(new LoggerDCOP_stub("kpilot", "LogIface")),
	fKPilotStub(new KPilotDCOP_stub("kpilot", "KPilotIface"))
{
	FUNCTIONSETUP;

	fInstaller = new FileInstaller;
	connect(fInstaller, SIGNAL(filesChanged()),
		this, SLOT(slotFilesChanged()));

	setupPilotLink();
	reloadSettings();

	if (fStatus == ERROR)
	{
		kdWarning() << k_funcinfo
			<< ": Connecting to device failed." << endl;
		return;
	}


#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": The daemon is ready with status "
		<< statusString() << " (" << (int) fStatus << ")" << endl;
#endif
}

PilotDaemon::~PilotDaemon()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fPilotLink);
	KPILOT_DELETE(fSyncStack);
	KPILOT_DELETE(fInstaller);
}

void PilotDaemon::addInstallFiles(QStrList l)
{
	FUNCTIONSETUP;

	fInstaller->addFiles(l);
}

int PilotDaemon::getPilotSpeed(KPilotConfigSettings & config)
{
	FUNCTIONSETUP;

	int speed = config.getPilotSpeed();

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

	switch (fStatus)
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

	KPilotConfigSettings & config = KPilotConfig::getConfig();
	config.reparseConfiguration();

	getPilotSpeed(config);

	fPilotDevice = config.getPilotDevice();
	fPilotType = KPilotDeviceLink::None;
	int t = config.getPilotType();
	
	(void) PilotAppCategory::setupPilotCodec(config.getEncoding());

#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": Got configuration "
		<< fPilotDevice
		<< " ("
		<< fPilotType
		<< ")"
		<< endl;
#endif

	switch (t)
	{
	case 0:
		fPilotType = KPilotDeviceLink::Serial;
		break;
	case 1:
		fPilotType = KPilotDeviceLink::OldStyleUSB;
		break;
	case 2:
		fPilotType = KPilotDeviceLink::DevFSUSB;
		break;
	default:
#ifdef DEBUG
		DEBUGDAEMON << fname << ": Unknown device type " << t << endl;
#endif
		break;
	}

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
			<< fPilotDevice
			<< " and type "
			<< fPilotLink->deviceTypeString(fPilotType) << endl;
#endif

		fPilotLink->reset(fPilotType, fPilotDevice);
	}

	if (config.getDockDaemon())
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

/* DCOP */ QString PilotDaemon::statusString()
{
	FUNCTIONSETUP;

	QString s = CSL1("PilotDaemon=");

	switch (status())
	{
	case INIT:
		s.append(QString(CSL1("Initializing")));
		break;
	case READY:
		s.append(QString(CSL1("Found device")));
		break;
	case ERROR:
		s.append(QString(CSL1("Error")));
		break;
	case FILE_INSTALL_REQ:
		s.append(QString(CSL1("Installing File")));
		break;
	case HOTSYNC_END:
		s.append(QString(CSL1("End of Hotsync")));
		break;
	case HOTSYNC_START:
		s.append(QString(CSL1("Syncing")));
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
	switch (fStatus)
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

	getKPilot().daemonStatus(KPilotDCOP::StartOfHotSync);
	
	fStatus = HOTSYNC_START ;

#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": Starting Sync with type "
		<< syncTypeString(fNextSyncType)
		<< " (" << fNextSyncType << ")" << endl;
#endif

	KPilotConfigSettings &c = KPilotConfig::getConfig();
	QStringList conduits ;
	bool installFiles = false;

	if ((fNextSyncType == PilotDaemonDCOP::HotSync)
		/* || other sync types */
		)
	{
		conduits = c.getInstalledConduits();
		installFiles = c.getSyncFiles();
	}

	fSyncStack = new ActionQueue(fPilotLink);
	fSyncStack->queueInit(ActionQueue::WithUserCheck);

#ifdef ENABLE_KROUPWARE
	bool _syncWithKMail = false;
	int _kroupwareParts = 0;
	
	c.setGroup(QString::null);
	if ( c.getSyncWithKMail() ) 
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
	c.setGroup(QString::null);

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
		if (conduits.count() > 0)
		{
			fSyncStack->queueConduits(&KPilotConfig::getConfig(),
				conduits,
				ActionQueue::Backup);
		}
		fSyncStack->addAction(new BackupAction(fPilotLink));
		break;
	case PilotDaemonDCOP::Restore:
		fSyncStack->addAction(new RestoreAction(fPilotLink));
		break;
	case PilotDaemonDCOP::HotSync:
		if (conduits.count() > 0)
		{
			fSyncStack->queueConduits(&KPilotConfig::getConfig(),
				conduits,
				ActionQueue::HotSync);
		}
		if (installFiles && fInstaller)
		{
			fSyncStack->queueInstaller(fInstaller->dir(),
				fInstaller->fileNames());
		}
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
	updateTrayStatus(s);
}

/* slot */ void PilotDaemon::logError(const QString & s)
{
	FUNCTIONSETUP;

	getLogger().logMessage(s);
	updateTrayStatus(s);
}

/* slot */ void PilotDaemon::logProgress(const QString & s, int i)
{
	FUNCTIONSETUPL(2);

	getLogger().logProgress(s, i);
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
	getKPilot().daemonStatus(KPilotDCOP::EndOfHotSync);
	
	fStatus = HOTSYNC_END;

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
		QTimer::singleShot(2000,fPilotLink,SLOT(reset()));
	}

	fPostSyncAction = None;
	
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

void PilotDaemon::slotRunConduitConfig()
{
	FUNCTIONSETUP;

	KProcess *p = new KProcess;
	*p << "kpilot" << "-c";

	p->start();
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
}


int main(int argc, char **argv)
{
	FUNCTIONSETUP;

	KAboutData about("kpilotDaemon",
		I18N_NOOP("KPilot Daemon"),
		KPILOT_VERSION,
		"KPilot - HotSync software for KDE\n\n",
		KAboutData::License_GPL, "(c) 1998-2001, Dan Pilone");
	about.addAuthor("Dan Pilone",
		I18N_NOOP("Project Leader"),
		"pilone@slac.com", "http://www.slac.com/pilone/kpilot_home/");
	about.addAuthor("Adriaan de Groot",
		I18N_NOOP("Maintainer"),
		"groot@kde.org", "http://www.cs.kun.nl/~adridg/kpilot/");

	KCmdLineArgs::init(argc, argv, &about);
	KUniqueApplication::addCmdLineOptions();
#ifdef DEBUG
	DEBUGDAEMON << fname 
		<< ": Adding debug options." << endl;
	// KCmdLineArgs::addCmdLineOptions(debug_options); // , "debug");



	DEBUGDAEMON << fname << ": Starting app." << endl;
#endif
	if (!KUniqueApplication::start())
	{
		return 0;
	}
	KUniqueApplication a(true, true);

	// No options besides debug
	// KPilotConfig::getDebugLevel(false);

	// A block just to keep variables local.
	//
	//
	{
		KPilotConfigSettings & c = KPilotConfig::getConfig();
		c.setReadOnly(false);

		if (c.getVersion() < KPilotConfig::ConfigurationVersion)
		{
			kdError() << k_funcinfo
				<< ": Is still not configured for use."
				<< endl;
			return 1;
		}

#ifdef DEBUG
		DEBUGDAEMON << fname
			<< ": Configuration version "
			<< c.getVersion() << endl;
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



