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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
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
#include <iostream>
#include <fstream.h>
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

	KGlobal::iconLoader()->addAppDir("kpilot");
	icon = KGlobal::iconLoader()->loadIcon("hotsync", KIcon::Toolbar,
		0, KIcon::DefaultState, 0, false);
	busyicon = KGlobal::iconLoader()->loadIcon("busysync", KIcon::Toolbar,
		0, KIcon::DefaultState, 0, false);

	slotShowBusy();
	QTimer::singleShot(2000,this,SLOT(slotShowNormal()));

	KPopupMenu *menu = contextMenu();

	menu->insertItem(i18n("&About"), this, SLOT(slotShowAbout()));
	menuKPilotItem = menu->insertItem(i18n("Start &KPilot"), daemon,
		SLOT(slotRunKPilot()));

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

	QString s("PilotDaemon=");

	switch (status())
	{
	case INIT:
		s.append(QString("Initializing"));
		break;
	case READY:
		s.append(QString("Found device"));
		break;
	case ERROR:
		s.append(QString("Error"));
		break;
	case FILE_INSTALL_REQ:
		s.append(QString("Installing File"));
		break;
	case HOTSYNC_END:
		s.append(QString("End of Hotsync"));
		break;
	case HOTSYNC_START:
		s.append(QString("Syncing"));
		break;
	}

	s.append(" NextSync=");
	s.append(syncTypeString(fNextSyncType));

	s.append(" (");
	if (fPilotLink)
	{
		s.append(fPilotLink->statusString());
	}
	s.append(")");

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
		return QString("Test");
	case PilotDaemonDCOP::HotSync:
		return QString("HotSync");
	case PilotDaemonDCOP::FastSync:
		return QString("FastSync");
	case PilotDaemonDCOP::Backup:
		return QString("Backup");
	case PilotDaemonDCOP::Restore:
		return QString("Restore");
	default:
		return QString("<unknown>");
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
		
		if (conduits.findIndex( "vcal-conduit" ) >= 0 )
			_kroupwareParts |= KroupwareSync::Cal ;
		if (conduits.findIndex( "todo-conduit" ) >= 0 )
			_kroupwareParts |= KroupwareSync::Todo ;
		if (conduits.findIndex( "knotes-conduit" ) >= 0 )
			_kroupwareParts |= KroupwareSync::Notes ;
		if (conduits.findIndex( "abbrowser_conduit" ) >= 0 )
			_kroupwareParts |= KroupwareSync::Address ;
	}
	c.setGroup(QString::null);

	if (_syncWithKMail)
	{
		fSyncStack->addAction(new KroupwareSync(true /* pre-sync */,
			_kroupwareParts,fPilotLink);
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
			_kroupwareParts,fPilotLink);
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
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGDAEMON << fname << ": " << s << endl;
#endif

	getLogger().logMessage(s);
	updateTrayStatus(s);
}

/* slot */ void PilotDaemon::logError(const QString & s)
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGDAEMON << fname << ": " << s << endl;
#endif

	getLogger().logMessage(s);
	updateTrayStatus(s);
}

/* slot */ void PilotDaemon::logProgress(const QString & s, int i)
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGDAEMON << fname << ": " << s << " (" << i << "%)" << endl;
#endif

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

	if (KApplication::startServiceByDesktopName("kpilot",
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
#ifdef DEBUG
	KCmdLineArgs::addCmdLineOptions(debug_options, "debug");
#endif
	KUniqueApplication::addCmdLineOptions();


	if (!KUniqueApplication::start())
	{
		return 0;
	}
	KUniqueApplication a(true, true);

	// No options besides debug
	KPilotConfig::getDebugLevel(false);

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



// $Log$
// Revision 1.71  2003/01/30 22:25:22  kainhofe
// Style fixes
//
// Revision 1.70  2003/01/18 00:30:18  kainhofe
// Removed the Log: tags from the conduits I maintain.
// Cleanup of the includes.
// Started implementing the other field sync of the addressbookconduit. Still have trouble converting a string to a QDate using a custom format
//
// Revision 1.69  2002/12/31 13:22:07  mueller
// CVS_SILENT fixincludes
//
// Revision 1.68  2002/12/04 10:55:37  thorsen
// Kroupware merge to HEAD. 3.1 will follow when the issues with the patch have been worked out.
//
// Revision 1.66.2.3  2002/11/29 11:12:10  thorsen
// Merged from head
//
// Revision 1.66.2.2  2002/10/14 21:20:35  rogowski
// Added syncing with addressbook of kmail. Fixed some bugs.
//
// Revision 1.66.2.1  2002/10/11 09:16:24  rogowski
// Implemented syncing of kpilot with kmail(only todos and calendars up to now). To enable syncing, choose in the sync config tab the option >sync with kmail<. But be careful with doing this with important data on your pilot: There are still bugs in kmail eating your data!
//
// Revision 1.66  2002/08/30 22:24:55  adridg
// - Improved logging, connected the right signals now
// - Try to handle dlp_ReadUserInfo failures sensibly
// - Trying to sort out failures reading the database list.
//
// Revision 1.65  2002/08/23 22:03:21  adridg
// See ChangeLog - exec() becomes bool, debugging added
//
// Revision 1.64  2002/08/15 21:51:00  kainhofe
// Fixed the error messages (were not printed to the log), finished the categories sync of the todo conduit
//
// Revision 1.63  2002/07/25 15:44:03  kainhofe
// LMB on tray icon starts kpilot, settings are reloaded when kpilot changes them
//
// Revision 1.62  2002/06/24 19:29:11  adridg
// Allow daemon RW access to config file
//
// Revision 1.61  2002/06/08 09:17:07  adridg
// Added tooltip for daemon
//
// Revision 1.60  2002/05/14 22:57:40  adridg
// Merge from _BRANCH
//
// Revision 1.59.2.3  2002/05/09 22:29:33  adridg
// Various small things not important for the release
//
// Revision 1.59.2.2  2002/04/16 19:41:05  adridg
// Make default sync a HotSync instead of Test
//
// Revision 1.59.2.1  2002/04/04 20:28:28  adridg
// Fixing undefined-symbol crash in vcal. Fixed FD leak. Compile fixes
// when using PILOT_VERSION. kpilotTest defaults to list, like the options
// promise. Always do old-style USB sync (also works with serial devices)
// and runs conduits only for HotSync. KPilot now as it should have been
// for the 3.0 release.
//
// Revision 1.59  2002/02/02 11:46:02  adridg
// Abstracting away pilot-link stuff
//
// Revision 1.58  2002/01/25 21:43:12  adridg
// ToolTips->WhatsThis where appropriate; vcal conduit discombobulated - it doesn't eat the .ics file anymore, but sync is limited; abstracted away more pilot-link
//
// Revision 1.57  2002/01/23 08:35:54  adridg
// Remove K-menu dependency
//
// Revision 1.56  2002/01/20 13:53:52  adridg
// Added new sync types
//
// Revision 1.55  2001/12/29 15:49:01  adridg
// SyncStack changes
//
// Revision 1.54  2001/11/18 16:59:55  adridg
// New icons, DCOP changes
//
// Revision 1.53  2001/10/10 13:40:07  cschumac
// Compile fixes.
//
// Revision 1.52  2001/10/08 22:20:18  adridg
// Changeover to libkpilot, prepare for lib-based conduits
//
// Revision 1.51  2001/10/08 12:49:11  cschumac
// kde3 compile fixes.
//
// Revision 1.50  2001/09/30 19:51:56  adridg
// Some last-minute layout, compile, and __FUNCTION__ (for Tru64) changes.
//
// Revision 1.49  2001/09/30 16:58:08  adridg
// Daemon reports name in statusString
//
// Revision 1.48  2001/09/29 16:23:31  adridg
// Layout + icons changed
//
// Revision 1.47  2001/09/24 22:24:06  adridg
// Use new SyncActions
//
// Revision 1.46  2001/09/23 21:44:56  adridg
// Myriad small changes
//
// Revision 1.45  2001/09/23 18:46:11  adridg
// Oops .. needed some extra work on the QStack part
//
// Revision 1.44  2001/09/23 18:24:59  adridg
// New syncing architecture
//
// Revision 1.43  2001/09/16 13:37:48  adridg
// Large-scale restructuring
//
// Revision 1.42  2001/08/27 22:54:27  adridg
// Decruftifying; improve DCOP link between daemon & viewer
//
// Revision 1.41  2001/08/19 19:25:57  adridg
// Removed kpilotlink dependency from kpilot; added DCOP interfaces to make that possible. Also fixed a connect() type mismatch that was harmless but annoying.
//
// Revision 1.40  2001/08/01 20:20:57  adridg
// Fix for bug #29764
//
// Revision 1.39  2001/06/11 07:36:10  adridg
// Cleanup char constant in <<
//
// Revision 1.38  2001/05/25 16:06:52  adridg
// DEBUG breakage
//
// Revision 1.37  2001/04/16 13:54:17  adridg
// --enable-final file inclusion fixups
//
// Revision 1.36  2001/04/01 17:32:52  adridg
// I really don't remember
//
// Revision 1.35  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.34  2001/03/05 23:44:39  adridg
// KPILOT_VERSION added. Fixed double-sync (maybe). Extra monitor debugging.
//
// Revision 1.33  2001/03/04 21:22:00  adridg
// Added drag 'n drop file install to daemon
//
// Revision 1.32  2001/03/04 11:23:04  adridg
// Changed for bug 21392
//
// Revision 1.31  2001/02/26 22:09:49  adridg
// Fixed some exit() calls; extra listener process debugging
//
// Revision 1.30  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.29  2001/02/08 13:17:19  adridg
// Fixed crash when conduits run during a backup and exit after the
// end of that backup (because the event loop is blocked by the backup
// itself). Added better debugging error exit message (no i18n needed).
//
// Revision 1.28  2001/02/05 21:01:07  adridg
// Fixed copyright headers for source releases. No code changed
//
// Revision 1.27  2001/02/05 19:16:32  adridg
// Removing calls to exit() from internal functions
//
// Revision 1.26  2001/01/06 13:20:23  adridg
// Cleaned up DCOP; changed version number
//
// Revision 1.25  2001/01/04 22:19:37  adridg
// Stuff for Chris and Bug 18072
//
// Revision 1.24  2001/01/04 11:33:20  bero
// Fix build
//
// Revision 1.23  2001/01/03 00:02:45  adridg
// Added Heiko's FastSync
//
// Revision 1.22  2001/01/02 15:02:59  bero
// Fix build
//
// Revision 1.21  2000/12/31 16:44:00  adridg
// Patched up the debugging stuff again
//
