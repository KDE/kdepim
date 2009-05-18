/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone <dan@kpilot.org>
** Copyright (C) 2001-2007 by Adriaan de Groot <groot@kde.org>
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
** Copyright (C) 2006-2007 Jason 'vanRijn' Kasper <vr@movingparts.net>
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <stdlib.h>

#include <QtCore/QTimer>
#include <QtCore/QProcess>
#include <QtCore/QHash>
#include <QtGui/QAction>
#include <QtGui/QMenu>

#include <kiconloader.h>
#include <khelpmenu.h>
#include <kmenu.h>
#include <kicon.h>
#include <kaboutapplicationdialog.h>
#include <ktoolinvocation.h>
#include <kuniqueapplication.h>
#include <kservice.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include "pilotRecord.h"

#include "fileInstaller.h"
#include "pilotUser.h"
#include "pilotDatabase.h"
#include "kpilotlink.h"
#include "kpilotdevicelink.h"
#include "actionQueue.h"
#include "actions.h"

#include "hotSync.h"
#include "logFile.h"

#include "kpilotConfig.h"
#include "kpilot.h"

#include "daemonadaptor.h"

#include "kpilot_interface.h"
#include "logfile_interface.h"
#include "kpilot_daemon_interface.h"
#include "pilotDaemon.moc"

static KAboutData *aboutData = 0L;

PilotDaemonTray::PilotDaemonTray(PilotDaemon * p) :
	KSystemTrayIcon(0),
	fSyncTypeMenu(0L),
	daemon(p),
	kap(0L),
	fBlinkTimer(0L)
{
	FUNCTIONSETUP;
	setObjectName("pilotDaemon");
	setupWidget();
}

void PilotDaemonTray::setupWidget()
{
	FUNCTIONSETUP;
	KIconLoader::global()->addAppDir( CSL1("kpilot") );

#define L(idx,name) \
	icons[idx]=loadIcon( CSL1(name) ); \
	if (icons[idx].isNull()) { WARNINGKPILOT << "No icon" << name; } \
	else { DEBUGKPILOT << "Loaded icon" << name; }

	L(Normal,"kpilotDaemon")
	L(Busy,"kpilot_busysync")
	L(NotListening,"kpilot_nosync")
#undef L

	slotShowNotListening();
	QTimer::singleShot(2000,this,SLOT(slotShowNormal()));

	QMenu *menu = contextMenu();

	menuKPilotItem = new QAction( i18n( "Start &KPilot" ), this );
	connect( menuKPilotItem, SIGNAL( triggered() ), this, SLOT( slotRunKPilot() ) );
	menu->addAction( menuKPilotItem );

	menuConfigureConduitsItem = new QAction( i18n("&Configure KPilot..."), this );
	connect( menuConfigureConduitsItem, SIGNAL( triggered() ), this
		, SLOT( slotRunConfig() ) );
	menu->addAction( menuConfigureConduitsItem );

	menu->addSeparator();

	// Configured default sync
	menuDefaultSyncActionItem = new QAction( i18n("Default (%1)"
		, SyncAction::SyncMode::name( (SyncAction::SyncMode::Mode)
			KPilotSettings::syncType() ) ), this );
	menuDefaultSyncActionItem->setCheckable( true );
	menuDefaultSyncActionItem->setData( (int) 0 );

	connect( menuDefaultSyncActionItem, SIGNAL( toggled( bool ) ), daemon
		, SLOT( toggleDefaultSync( bool ) ) );

	fSyncTypeMenu = new QMenu( i18n( "Next &Sync" ), menu );
	fSyncTypeMenu->setObjectName( "sync_type_menu" );
	fSyncTypeMenu->addAction( menuDefaultSyncActionItem );
	fSyncTypeMenu->addSeparator();
	QString once = i18nc("Appended to names of sync types to indicate the sync"
		" will happen just one time"," (once)");

	QActionGroup *syncTypeActions = new QActionGroup( this );
	QAction *action;
	QString syncModeName;

#define MI(a,b) \
	syncModeName = SyncAction::SyncMode::name( a ); \
	action = new QAction( syncModeName, this ); \
	action->setCheckable( true ); \
	action->setChecked( b ); \
	action->setData( (int) a ); \
	syncTypeActions->addAction( action ); \
	fSyncTypeMenu->addAction( action ); \

	// Keep this synchronized with kpilotui.rc and kpilot.cc if at all possible.
	MI( SyncAction::SyncMode::eHotSync, true );
	MI( SyncAction::SyncMode::eFullSync, false );
	MI( SyncAction::SyncMode::eCopyPCToHH, false );
	MI( SyncAction::SyncMode::eCopyHHToPC, false );
	MI( SyncAction::SyncMode::eBackup, false );
	MI( SyncAction::SyncMode::eRestore, false );
#undef MI

	connect( syncTypeActions, SIGNAL( triggered( QAction* ) ), daemon
		, SLOT( requestSync( QAction* ) ) );

	// See toggleDefaultSync(). This is only useful now all other action are
	// added to syncTypeActions.
	menuDefaultSyncActionItem->setChecked( true );

	menu->addMenu( fSyncTypeMenu );

	KHelpMenu *helpMenu = new KHelpMenu( menu, aboutData );
	helpMenu->menu()->setIcon( KIcon( CSL1( "system-help" ) ) );
	menu->addMenu( helpMenu->menu() );

	DEBUGKPILOT << "Finished getting icons";
}

void PilotDaemonTray::slotShowAbout()
{
	FUNCTIONSETUP;

	if (!kap)
	{
		kap = new KAboutApplicationDialog(aboutData, false);
	}

	kap->show();
}


void PilotDaemonTray::enableRunKPilot(bool b)
{
	FUNCTIONSETUP;

	menuKPilotItem->setEnabled( b );
	menuConfigureConduitsItem->setEnabled( b );
}


void PilotDaemonTray::changeIcon(IconShape i)
{
	FUNCTIONSETUP;
	if (icons[i].isNull())
	{
		WARNINGKPILOT << "Icon #"<<i<< " is NULL!";
	}
	setIcon ( icons[i] );
	fCurrentIcon = i;
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

void PilotDaemonTray::slotShowNotListening()
{
	FUNCTIONSETUP;
	changeIcon( NotListening );
}

void PilotDaemonTray::slotBusyTimer()
{
	if (fCurrentIcon == Busy) changeIcon(Normal);
	else if (fCurrentIcon == Normal) changeIcon(Busy);
}

void PilotDaemonTray::startHotSync()
{
	changeIcon(Busy);
	if (!fBlinkTimer)
	{
		fBlinkTimer = new QTimer(this);
		fBlinkTimer->setObjectName(QLatin1String("blink timer"));
	}
	if (fBlinkTimer)
	{
		connect(fBlinkTimer,SIGNAL(timeout()),
			this,SLOT(slotBusyTimer()));
		fBlinkTimer->setSingleShot(false);
		fBlinkTimer->start(750);
	}
}

void PilotDaemonTray::endHotSync()
{
	changeIcon(Normal);
	if (fBlinkTimer)
	{
		fBlinkTimer->stop();
	}
}

void PilotDaemonTray::slotRunKPilot()
{
	FUNCTIONSETUP;

	QString kpilotError;
	QString kpilotPID;

	if (KToolInvocation::startServiceByDesktopName(CSL1("kpilot"),
			QStringList(), &kpilotError, &kpilotPID
		))
	{
		WARNINGKPILOT << "Couldn't start KPilot!" << kpilotError;
	}
}

void PilotDaemonTray::slotRunConfig()
{
	FUNCTIONSETUP;

	// This function tries to send the raise() call to kpilot.
	// If it succeeds, we can assume kpilot is running and then try
	// to send the configure() call.
	// If it fails (probably because kpilot isn't running) it tries
	// to start kpilot (using a command line switch to
	// only bring up the configure dialog).
	//
	// Implementing the function this way catches all cases.
	// ie 1 KPilot running with configure dialog open (raise())
	//    2 KPilot running with dialog NOT open (configureConduits())
	//    3 KPilot NOT running (start it)

	// This call to kpilot's raise function solves the final case
	// ie when kpilot already has the dialog open

	if ( QDBusConnection::sessionBus().interface()->isServiceRegistered( "org.kde.kpilot.kpilot" ) )
	{
		DEBUGKPILOT << "kpilot running. telling it to raise/configure.";

		OrgKdeKpilotKpilotInterface * kpilot = new OrgKdeKpilotKpilotInterface("org.kde.kpilot.kpilot", "/KPilot",QDBusConnection::sessionBus());
		kpilot->raise();
		kpilot->configure();
	}
	else
	{
		DEBUGKPILOT << "kpilot not running. starting it.";
		// KPilot not running
		QProcess *p = new QProcess;
		QStringList arguments;
		arguments<< "-s";
		p->start("kpilot", arguments);
	}
}


PilotDaemon::PilotDaemon() :
	fDaemonStatus(INIT),
	fPostSyncAction(None),
	fPilotLink(0L),
	fNextSyncType(SyncAction::SyncMode::eHotSync,true),
	fSyncStack(0L),
	fTray(0L),
	fInstaller(0L),
	fLogFile(0L),
	fTempDevice(QString())
{
	FUNCTIONSETUP;

        new DaemonAdaptor(this);
        QDBusConnection::sessionBus().registerObject("/Daemon", this);

	//TODO verify it
        fLogInterface = new OrgKdeKpilotLoggerInterface("org.kde.kpilot.kpilot", "/LoggerGUI", QDBusConnection::sessionBus());
	fLogFileInterface = new OrgKdeKpilotLoggerInterface("org.kde.kpilot.daemon", "/LoggerFile", QDBusConnection::sessionBus());
	fKPilotInterface = new OrgKdeKpilotKpilotInterface("org.kde.kpilot.kpilot", "/KPilot",QDBusConnection::sessionBus());

	setupPilotLink();
	reloadSettings();

	if (fDaemonStatus == ERROR)
	{
		WARNINGKPILOT << "Connecting to device failed.";
		return;
	}

	fInstaller = new FileInstaller;
	fLogFile = new LogFile;

	fNextSyncType.setMode( KPilotSettings::syncType() );

	DEBUGKPILOT << "The daemon is ready with status "
		<< statusString() << " (" << (int) fDaemonStatus << ')';
}

PilotDaemon::~PilotDaemon()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fPilotLink);
	KPILOT_DELETE(fSyncStack);
	KPILOT_DELETE(fInstaller);

	(void) PilotDatabase::instanceCount();
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

	DEBUGKPILOT << "Speed set to "
		<< speedname << " (" << speed << ')';

	putenv((char *) speedname);

	return speed;
}


void PilotDaemon::showTray()
{
	FUNCTIONSETUP;

	if (!fTray)
	{
		DEBUGKPILOT << "No tray icon to display!";

		return;
	}
	fTray->show();
	DEBUGKPILOT << "Tray icon displayed.";

	updateTrayStatus();
}

void PilotDaemon::setTempDevice(const QString &d)
{
	if ( !d.isEmpty() ){
		fTempDevice = d;
		if (fPilotLink)
			fPilotLink->setTempDevice( fTempDevice );
		reloadSettings();
	}
}

void PilotDaemon::reloadSettings()
{
	FUNCTIONSETUP;

	switch (fDaemonStatus)
	{
	case INIT:
	case HOTSYNC_END:
	case ERROR:
	case READY:
	case NOT_LISTENING:
		// It's OK to reload settings in these states.
		break;
	case HOTSYNC_START:
	case FILE_INSTALL_REQ:
		// Postpone the reload till the sync finishes.
		fPostSyncAction |= ReloadSettings;
		return;
		break;
	}

	KPilotSettings::self()->config()->reparseConfiguration();
	KPilotSettings::self()->readConfig();
	getPilotSpeed();

	(void) Pilot::setupPilotCodec(KPilotSettings::encoding());

	DEBUGKPILOT << "Got configuration "
		<< KPilotSettings::pilotDevice();
	DEBUGKPILOT << "Got conduit list "
		<< (KPilotSettings::installedConduits().join(CSL1(",")));

	requestSync(0);


	if (fPilotLink)
	{
		DEBUGKPILOT << "Resetting with device "
			<< KPilotSettings::pilotDevice();

		fPilotLink->reset( KPilotSettings::pilotDevice() );
		DEBUGKPILOT << "Using workarounds "
			<< KPilotSettings::workarounds();
		if ( KPilotSettings::workarounds() == KPilotSettings::eWorkaroundUSB )
		{
			DEBUGKPILOT << "Using USB connect/disconnect/connect workaround.";
			fPilotLink->setWorkarounds(true);
		}
	}

	if (KPilotSettings::dockDaemon())
	{
		if (!fTray)
		{
			fTray = new PilotDaemonTray(this);
		}
		fTray->show();
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
	logProgress(QString(),0);
}

void PilotDaemon::stopListening()
{
	fIsListening=false;
	fTray->changeIcon(PilotDaemonTray::NotListening);
	fDaemonStatus=NOT_LISTENING;
	fPilotLink->close();
}

void PilotDaemon::startListening()
{
	fIsListening=true;
	fTray->changeIcon(PilotDaemonTray::Normal);
	fDaemonStatus=INIT;
	fPilotLink->reset();
}

QString PilotDaemon::statusString()
{
	FUNCTIONSETUP;

	QString s = CSL1("PilotDaemon=");
	s.append(shortStatusString());

	s.append(CSL1("; NextSync="));
	s.append(fNextSyncType.name());

	s.append(CSL1(" ("));
	if (fPilotLink)
	{
		s.append(fPilotLink->statusString());
	}
	s.append(CSL1(");"));

	return s;
}

QString PilotDaemon::shortStatusString()
{
	QString s;

	switch (status())
	{
	case INIT:
		s.append(CSL1("Waiting for sync"));
		break;
	case READY:
		s.append(CSL1("Listening on device"));
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
	case NOT_LISTENING:
		s.append(CSL1("Not Listening (stopped manually)"));
		break;
	}

	return s;
}



bool PilotDaemon::setupPilotLink()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fPilotLink);
	fPilotLink = new KPilotDeviceLink( 0, 0, fTempDevice );
	if (!fPilotLink)
	{
		WARNINGKPILOT << "Can't get pilot link.";
		return false;
	}

	QObject::connect(fPilotLink, SIGNAL(deviceReady(KPilotLink*)),
		this, SLOT(startHotSync(KPilotLink*)));
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


void PilotDaemon::quitNow()
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
	case NOT_LISTENING:
		getKPilot().daemonStatus(KPilotInstaller::DaemonQuit);
		qApp->quit();
		break;
	case READY:
	case HOTSYNC_START:
	case FILE_INSTALL_REQ:
		fPostSyncAction |= Quit;
		break;
	}
	QDBusMessage message = QDBusMessage::createSignal( "/Daemon", "org.kde.kpilot.daemon", "kpilotDaemonStatusChanged");
	QDBusConnection::sessionBus().send( message );
}

void PilotDaemon::requestRegularSyncNext()
{
	requestSync(SyncAction::SyncMode::eHotSync);
}

void PilotDaemon::requestSync( QAction *action )
{
	FUNCTIONSETUP;

	if( !action )
	{
		WARNINGKPILOT << "Ignored OL action pointer";
		return;
	}

	unsigned int actionData = action->data().toInt();

	requestSync( actionData );

	if( fTray )
	{
		if( actionData != 0 && actionData != KPilotSettings::syncType() )
		{
			fTray->menuDefaultSyncActionItem->setChecked( false );
		}
	}
}

void PilotDaemon::requestSync( int mode )
{
	FUNCTIONSETUP;

	if ( 0==mode )
	{
		mode = KPilotSettings::syncType();
	}

	if ( !fNextSyncType.setMode(mode) )
	{
		WARNINGKPILOT << "Ignored fake sync type" << mode;
		return;
	}

	updateTrayStatus();

	getLogger().logMessage(i18n("Next HotSync will be: %1. ",fNextSyncType.name()) +
		i18n("Please press the HotSync button."));
}

void PilotDaemon::toggleDefaultSync( bool toggled )
{
	// If the default
	if( toggled )
	{
		if( fTray && (fTray->fSyncTypeMenu) )
		{
			QList<QAction*> actions = fTray->fSyncTypeMenu->actions();

			QListIterator<QAction*> i( actions );
			bool finished = false;
			QAction *a;

			while( i.hasNext() && !finished )
			{
				a = i.next();
				if( a->data().toInt() == (int) SyncAction::SyncMode::eHotSync )
				{
					a->setChecked( true );
					finished = true;
				}
			}
		}
	}
}

void PilotDaemon::requestSyncType(const QString &s)
{
	FUNCTIONSETUP;

	// This checks unique prefixes of the names of the various sync types.
	if (s.startsWith(CSL1("H"))) requestSync(SyncAction::SyncMode::eHotSync);
	else if (s.startsWith(CSL1("Fu"))) requestSync(SyncAction::SyncMode::eFullSync);
	else if (s.startsWith(CSL1("B"))) requestSync(SyncAction::SyncMode::eBackup);
	else if (s.startsWith(CSL1("R"))) requestSync(SyncAction::SyncMode::eRestore);
	else if (s.startsWith(CSL1("T"))) { fNextSyncType.setOptions(true,false); }
	else if (s.startsWith(CSL1("CopyHHToPC"))) requestSync(SyncAction::SyncMode::eCopyHHToPC);
	else if (s.startsWith(CSL1("CopyPCToHH"))) requestSync(SyncAction::SyncMode::eCopyPCToHH);
	else if (s.startsWith(CSL1("D"))) requestSync(0);
	else
	{
		WARNINGKPILOT << "Unknown sync type " << ( s.isEmpty() ? CSL1("<none>") : s );
	}
}

void PilotDaemon::requestSyncOptions(bool test, bool local)
{
	if ( !fNextSyncType.setOptions(test,local) )
	{
		WARNINGKPILOT << "Nonsensical request for "
			<< (test ? "test" : "notest")
			<< ' '
			<< (local ? "local" : "nolocal")
			<< " in mode "
			<< fNextSyncType.name();
	}
}

int PilotDaemon::nextSyncType() const
{
	return fNextSyncType.mode();
}

/**
* Functions reporting some status data, e.g. for the kontact plugin.
*/
QDateTime PilotDaemon::lastSyncDate()
{
	return KPilotSettings::lastSyncTime();
}


static QHash<QString, QString*> *conduitNameMap = 0L;

static void fillConduitNameMap()
{
	if ( !conduitNameMap )
	{
		conduitNameMap = new QHash<QString, QString*>;
	}
	conduitNameMap->clear();

	QStringList l = KPilotSettings::installedConduits();
	// Fill with internal settings.
	if ( l.indexOf( CSL1("internal_fileinstall") ) >= 0 ) {
		conduitNameMap->insert( CSL1("internal_fileinstall"),
			new QString(i18n("File Installer")) );
	}

	QStringList::ConstIterator end = l.end();
	for (QStringList::ConstIterator i = l.begin(); i != end; ++i)
	{
		if (!conduitNameMap->value(*i))
		{
			QString readableName = CSL1("<unknown>");
			KService::Ptr o = KService::serviceByDesktopName(*i);
			if (!o)
			{
				WARNINGKPILOT << "No service for" << *i;
			}
			else
			{
				readableName = o->name();
			}
			conduitNameMap->insert( *i, new QString(readableName) );
		}
	}
}


QStringList PilotDaemon::configuredConduitList()
{
	fillConduitNameMap();

	QStringList keys = conduitNameMap->keys();
	keys.sort();

	QStringList::ConstIterator end = keys.end();
	QStringList result;
	for (QStringList::ConstIterator i = keys.begin(); i != end; ++i)
	{
		result << *(conduitNameMap->value(*i));
	}

	return result;
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

bool PilotDaemon::killDaemonOnExit()
{
	return KPilotSettings::killDaemonAtExit();
}

typedef enum { KDL_NotLocked=0, KDL_Locked=1, KDL_Error=2 } KDesktopLockStatus;
static KDesktopLockStatus isKDesktopLockRunning()
{
	FUNCTIONSETUP;

	if (!KPilotSettings::screenlockSecure())
	{
		return KDL_NotLocked;
	}

	QDBusInterface screensaver("org.freedesktop.ScreenSaver", "/ScreenSaver",
				   "org.freedesktop.ScreenSaver");
	QDBusReply<bool> reply = screensaver.call( "GetActive" );

	bool locked = true;

	if ( reply.isValid() )
	{
		locked = reply.value();
		DEBUGKPILOT << "isDesktopLockRunning: reply from screensaver.GetActive: [" << locked << "]";
	}
	else
	{
		WARNINGKPILOT << "isDesktopLockRunning: Could not make DBUS connection. "
			<< "Error: [" << reply.error().type() << "], message: ["<< reply.error().message()
			<< "], name: [" << reply.error().name()
			<< "]. Assuming screensaver is active.";
		return KDL_Error;
	}

	return (locked ? KDL_Locked : KDL_NotLocked);

}


static void informOthers(OrgKdeKpilotKpilotInterface &kpilot,
	OrgKdeKpilotLoggerInterface &log,
	OrgKdeKpilotLoggerInterface &filelog)
{
	kpilot.daemonStatus(KPilotInstaller::StartOfHotSync);
	log.logStartSync();
	filelog.logStartSync();
}

static bool isSyncPossible(ActionQueue *fSyncStack,
	KPilotLink *pilotLink,
	OrgKdeKpilotKpilotInterface &kpilot)
{
	FUNCTIONSETUP;
	/**
	* If KPilot is busy with something - like configuring
	* conduit - then we shouldn't run a real sync, but
	* just tell the user that the sync couldn't run because
	* of that.
	*/
	QDBusReply<int> val = kpilot.kpilotStatus();
	int kpilotstatus=0;
	if (!val.isValid())
	{
		DEBUGKPILOT << "Could not call KPilot for status.";
	}
	else
	{
		DEBUGKPILOT << "KPilot status" << kpilotstatus;
	}
	/**
	* If the call fails, then KPilot is probably not running
	* and we can behave normally.
	*/
	if (val.isValid())
	{
		kpilotstatus = val;
		if(kpilotstatus != KPilotInstaller::WaitingForDaemon)
		{
			WARNINGKPILOT << "KPilot returned status" << kpilotstatus;

			fSyncStack->queueInit();
			fSyncStack->addAction(new SorryAction(pilotLink));
			return false;
		}
	}
	switch (isKDesktopLockRunning())
	{
	case KDL_NotLocked :
		break; /* Fall through to return true below */
	case KDL_Locked :
		fSyncStack->queueInit();
		fSyncStack->addAction(new SorryAction(pilotLink,
			i18n("HotSync is disabled while the screen is locked.")));
		return false;
	case KDL_Error :
		fSyncStack->queueInit();
		fSyncStack->addAction(new SorryAction(pilotLink,
			i18n("HotSync is disabled because KPilot could not "
			"determine the state of the screen saver. You "
			"can disable this security feature by unchecking "
			"the 'do not sync when screensaver is active' box "
			"in the HotSync page of the configuration dialog.")));
		return false;
	}
	return true;
}

static void queueInstaller(ActionQueue *fSyncStack,
	KPilotLink *pilotLink,
	FileInstaller *fInstaller,
	const QStringList &c)
{
	if (c.indexOf(CSL1("internal_fileinstall")) >= 0)
	{
		fSyncStack->addAction(new FileInstallAction(pilotLink,fInstaller->dir()));
	}
}

static void queueConduits(ActionQueue *fSyncStack,
	const QStringList &conduits,
	SyncAction::SyncMode e)
{
	if (conduits.count() > 0)
	{
		fSyncStack->queueConduits(conduits,e);
	}
}

bool PilotDaemon::shouldBackup()
{

	FUNCTIONSETUP;

	bool ret = false;
	int backupfreq = KPilotSettings::backupFrequency();

	DEBUGKPILOT << "Backup Frequency is: " << backupfreq;

	if ( (fNextSyncType == SyncAction::SyncMode::eHotSync) ||
		(fNextSyncType == SyncAction::SyncMode::eFullSync) )
	{
		/** If we're doing a Hot or Full sync, see if our user has
		 * configured us to or to not always do a backup.
		 */
		if ( backupfreq == SyncAction::eOnRequestOnly )
		{
	DEBUGKPILOT << "Should not do backup...";
			ret = false;
		}
		else if ( backupfreq == SyncAction::eEveryHotSync )
		{
	DEBUGKPILOT << "Should do backup...";
			ret = true;
		}
	}

	return ret;

}


/* slot */ void PilotDaemon::startHotSync(KPilotLink *pilotLink)
{
	FUNCTIONSETUP;

	bool pcchanged=false; // If last PC to sync was a different one (implies full sync, normally)
	QStringList conduits ; // list of conduits to run
	QString s; // a generic string for stuff

	DEBUGKPILOT << "Starting Sync with type " << fNextSyncType.name();
	DEBUGKPILOT << "Status is " << shortStatusString();
	(void) PilotDatabase::instanceCount();

	fDaemonStatus = HOTSYNC_START ;
	if (fTray)
	{
		fTray->startHotSync();
	}
	informOthers(getKPilot(),getLogger(),getFileLogger());


	// Queue to add all the actions for this sync to.
	fSyncStack = new ActionQueue(pilotLink);

	// Check if the sync is possible at all.
	if (!isSyncPossible(fSyncStack,pilotLink,getKPilot()))
	{
		// Sync is not possible now, sorry action was added to
		// the queue, and we run that -- skipping all the rest of the sync stuff.
		goto launch;
	}

	// Except when the user has requested a Restore, in which case she knows she doesn't
	// want to sync with a blank palm and then back up the result over her stored backup files,
	// do a Full Sync when changing the PC or using a different Palm Desktop app.
	if (fNextSyncType.mode() != SyncAction::SyncMode::eRestore)
	{ // Use gethostid to determine , since JPilot uses 1+(2000000000.0*random()/(RAND_MAX+1.0))
		// as PC_ID, so using JPilot and KPilot is the same as using two different PCs
		KPilotUser &usr = pilotLink->getPilotUser();
		pcchanged = usr.getLastSyncPC() !=(unsigned long) gethostid();

		if (pcchanged)
		{
			DEBUGKPILOT << "PC changed. Last sync PC: " << usr.getLastSyncPC()
				<< ", me: " << (unsigned long) gethostid();
 			if ( KPilotSettings::fullSyncOnPCChange() )
			{
				DEBUGKPILOT << "Setting sync mode to full sync.";
				fNextSyncType = SyncAction::SyncMode( SyncAction::SyncMode::eFullSync );
			}
			else
			{
				DEBUGKPILOT << "Not changing sync mode because of settings.";
			}
		}
	}

	// Normal case: regular sync.
	fSyncStack->queueInit();
	fSyncStack->addAction(new CheckUser(pilotLink));

	conduits = KPilotSettings::installedConduits() ;

	if (fNextSyncType.isTest())
	{
		fSyncStack->addAction(new TestLink(pilotLink));
	}
	else
	{
		switch (fNextSyncType.mode())
		{
		case SyncAction::SyncMode::eBackup:
			if (KPilotSettings::runConduitsWithBackup() && (conduits.count() > 0))
			{
				queueConduits(fSyncStack,conduits,fNextSyncType);
			}
			fSyncStack->addAction(new BackupAction(pilotLink,true));
			break;
		case SyncAction::SyncMode::eRestore:
			fSyncStack->addAction(new RestoreAction(pilotLink));
			queueInstaller(fSyncStack,pilotLink,fInstaller,conduits);
			break;
		case SyncAction::SyncMode::eFullSync:
		case SyncAction::SyncMode::eHotSync:
			// first install the files, and only then do the conduits
			// (conduits might want to sync a database that will be installed
			queueInstaller(fSyncStack,pilotLink,fInstaller,conduits);
			queueConduits(fSyncStack,conduits,fNextSyncType);
			// And sync the remaining databases if needed.
			if (shouldBackup())
			{
				fSyncStack->addAction(new BackupAction(pilotLink, (fNextSyncType == SyncAction::SyncMode::eFullSync)));
			}
			break;
		case SyncAction::SyncMode::eCopyPCToHH:
			queueConduits(fSyncStack,conduits
				,SyncAction::SyncMode(SyncAction::SyncMode::eCopyPCToHH));
			break;
		case SyncAction::SyncMode::eCopyHHToPC:
			queueConduits(fSyncStack,conduits
				,SyncAction::SyncMode(SyncAction::SyncMode::eCopyHHToPC));
			break;
		}
	}

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
		fTray->endHotSync();
	}

	fSyncStack->deleteLater();
	fPilotLink->close();

	getLogger().logProgress(i18n("HotSync Completed.<br/>"), 100);
	getFileLogger().logProgress(i18n("HotSync Completed.<br/>"), 100);
	getLogger().logEndSync();
	getFileLogger().logEndSync();
	getKPilot().daemonStatus(KPilotInstaller::EndOfHotSync);
	KPilotSettings::setLastSyncTime(QDateTime::currentDateTime());
	KPilotSettings::self()->writeConfig();

	fDaemonStatus = HOTSYNC_END;

	if (fPostSyncAction & Quit)
	{
		getKPilot().daemonStatus(KPilotInstaller::DaemonQuit);
		qApp->quit();
	}
	if (fPostSyncAction & ReloadSettings)
	{
		reloadSettings();
	}
	else
	{
		QTimer::singleShot(10000,fPilotLink,SLOT(reset()));
	}

	fPostSyncAction = None;
	requestSync(0);

	(void) PilotDatabase::instanceCount();

	updateTrayStatus();
}




void PilotDaemon::updateTrayStatus(const QString &s)
{
	if (!fTray) return;

	QString tipText = CSL1("<qt>");
	tipText.append( s );
	tipText.append( CSL1(" ") );
	tipText.append( i18n("Next sync is %1.",fNextSyncType.name() ) );
	tipText.append( CSL1("</qt>") );

	fTray->setToolTip(tipText);
	QDBusMessage message = QDBusMessage::createSignal( "/Daemon", "org.kde.kpilot.daemon", "kpilotDaemonStatusChanged");
	QDBusConnection::sessionBus().send( message );

	// emit the same signal but including the information needed by Kontact to update its kpilot summary widget
	/*
	* not going to finish doing this since we don't have a kontact component yet, but if
	* we want one, here's how to get it working (from the dcop code):
	*/
	/*
	message = QDBusMessage::createSignal( "/Daemon", "org.kde.kpilot.daemon", "kpilotDaemonStatusDetails()");
	QByteArray data;
	QDataStream arg(data, QIODevice::WriteOnly);
	arg << lastSyncDate();
	arg << shortStatusString();
	arg << configuredConduitList();
	arg << logFileName();
	arg << userName();
	arg << pilotDevice();
	arg << killDaemonOnExit();

	emitDCOPSignal( "kpilotDaemonStatusDetails(QDateTime,QString,QStringList,QString,QString,QString,bool)", data );
	*/
}


int main(int argc, char **argv)
{
	FUNCTIONSETUP;

	KLocale::setMainCatalog("kpilot");

	// a non-intuitive note about this... now that we're using dbus, the way that the path
	// is constructed to communicate with our objects on the bus are of 2 pieces...
	// in the dbus interface string "org.kde.kpilot.foo",
	// the "org.kde.kpilot" part comes from setOrganizationDomain, found in
	// KPILOT_ABOUT_INIT.  the "foo" part comes in the first argument to KAboutData below...
	KAboutData about("daemon", 0,
		ki18n("KPilot Daemon"),
		KPILOT_VERSION,
		ki18n("KPilot - HotSync software for KDE"),
		KAboutData::License_GPL,
		KPILOT_ABOUT_AUTHORS,
		ki18n(0L),
		"http://www.kpilot.org/"
		);
	KPILOT_ABOUT_INIT(about);

	aboutData = &about;


	KCmdLineArgs::init(argc, argv, &about);

	KCmdLineOptions daemonoptions;
#ifdef DEBUG
	daemonoptions.add("debug <level>", ki18n("Set debugging level"), "0");
#endif
	daemonoptions.add("device <device>", ki18n("Device to try first"));
	daemonoptions.add("fail-silently", ki18n("Exit instead of complaining about bad configuration files"));
	KCmdLineArgs::addCmdLineOptions(daemonoptions, ki18n("kpilotconfig"));
	KUniqueApplication::addCmdLineOptions();
	KCmdLineArgs *p = KCmdLineArgs::parsedArgs();

	KPilotConfig::getDebugLevel(p);

	if (!KUniqueApplication::start())
	{
		if (p->isSet("device")){
			OrgKdeKpilotDaemonInterface interface("org.kde.kpilot.daemon", "/Daemon", QDBusConnection::sessionBus());
			QString dev(p->getOption("device"));
			interface.setTempDevice(dev);
		}
		return 0;
	}
	KUniqueApplication a(true, true);
	a.setQuitOnLastWindowClosed(false);

	// A block just to keep variables local.
	{
		if (KPilotSettings::configVersion() < KPilotConfig::ConfigurationVersion)
		{
			WARNINGKPILOT << "Still not configured for use.";
			if (!p->isSet("fail-silently"))
			{
				KPilotConfig::sorryVersionOutdated(KPilotSettings::configVersion());
			}
			return 1;
		}

		DEBUGKPILOT << "Configuration version "
			<< KPilotSettings::configVersion();
	}


	PilotDaemon *pilotDaemon = new PilotDaemon();

	if (p->isSet("device"))
		pilotDaemon->setTempDevice(p->getOption("device"));

	if (pilotDaemon->status() == PilotDaemon::ERROR)
	{
		delete pilotDaemon;

		pilotDaemon = 0;
		WARNINGKPILOT << "Failed to start up daemon "
			"due to errors constructing it.";
		return 2;
	}

	pilotDaemon->showTray();

	return a.exec();
}



