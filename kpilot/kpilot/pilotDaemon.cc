/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2001-2004 by Adriaan de Groot
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
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

#include <qtimer.h>
#include <qtooltip.h>
#include <qpixmap.h>

#include <kuniqueapplication.h>
#include <kaboutapplication.h>
#include <kcmdlineargs.h>
#include <kwin.h>
#include <kurl.h>
#include <kpopupmenu.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kprocess.h>
#include <dcopclient.h>
#include <kurldrag.h>
#include <kservice.h>
#include <kapplication.h>
#include <khelpmenu.h>

#include "pilotRecord.h"

#include "fileInstaller.h"
#include "pilotUser.h"
#include "pilotDatabase.h"
#include "kpilotlink.h"
#include "kpilotdevicelink.h"
#include "actionQueue.h"
#include "actions.h"

#include "hotSync.h"
#include "internalEditorAction.h"
#include "logFile.h"

#include "kpilotConfig.h"


#include "kpilotDCOP_stub.h"
#include "kpilotDCOP.h"
#include "loggerDCOP_stub.h"

#include "pilotDaemon.moc"

static KAboutData *aboutData = 0L;

PilotDaemonTray::PilotDaemonTray(PilotDaemon * p) :
	KSystemTray(0, "pilotDaemon"),
	fSyncTypeMenu(0L),
	daemon(p),
	kap(0L),
	fBlinkTimer(0L)
{
	FUNCTIONSETUP;
	setupWidget();
	setAcceptDrops(true);
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

	KGlobal::iconLoader()->addAppDir( CSL1("kpilot") );
	icons[Normal] = loadIcon( CSL1("kpilotDaemon") );
	icons[Busy] = loadIcon( CSL1("busysync") );
	icons[NotListening] = loadIcon( CSL1("nosync") );

	slotShowNotListening();
	QTimer::singleShot(2000,this,SLOT(slotShowNormal()));

	KPopupMenu *menu = contextMenu();

	menuKPilotItem = menu->insertItem(i18n("Start &KPilot"), daemon,
		SLOT(slotRunKPilot()));
	menuConfigureConduitsItem = menu->insertItem(i18n("&Configure KPilot..."),
		daemon, SLOT(slotRunConfig()));
	menu->insertSeparator();

	fSyncTypeMenu = new KPopupMenu(menu,"sync_type_menu");
	QString once = i18n("Appended to names of sync types to indicate the sync will happen just one time"," (once)");
#define MI(a) fSyncTypeMenu->insertItem( \
		SyncAction::SyncMode::name(SyncAction::SyncMode::a) + once, \
		(int)(SyncAction::SyncMode::a));
	fSyncTypeMenu->insertItem(i18n("Default (%1)")
		.arg(SyncAction::SyncMode::name((SyncAction::SyncMode::Mode)KPilotSettings::syncType())),
		0);
	fSyncTypeMenu->insertSeparator();

        // Keep this synchronized with kpilotui.rc and kpilot.cc if at all possible.
	MI(eHotSync);
	MI(eFullSync);
	MI(eBackup);
	MI(eRestore);
	MI(eCopyHHToPC);
	MI(eCopyPCToHH);

	fSyncTypeMenu->setCheckable(true);
	fSyncTypeMenu->setItemChecked(0,true);
#undef MI
	connect(fSyncTypeMenu,SIGNAL(activated(int)),daemon,SLOT(requestSync(int)));
	menu->insertItem(i18n("Next &Sync"),fSyncTypeMenu);

	KHelpMenu *help = new KHelpMenu(menu,aboutData);
	menu->insertItem(
		KGlobal::iconLoader()->loadIconSet(CSL1("help"),KIcon::Small,0,true),
		i18n("&Help"),help->menu(),false /* no whatsthis */);



#ifdef DEBUG
	DEBUGKPILOT << fname << ": Finished getting icons" << endl;
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
	if (icons[i].isNull())
	{
		WARNINGKPILOT << "Icon #"<<i<<" is NULL!" << endl;
	}
	setPixmap(icons[i]);
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
		fBlinkTimer = new QTimer(this,"blink timer");
	}
	if (fBlinkTimer)
	{
		connect(fBlinkTimer,SIGNAL(timeout()),
			this,SLOT(slotBusyTimer()));
		fBlinkTimer->start(750,false);
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


PilotDaemon::PilotDaemon() :
	DCOPObject("KPilotDaemonIface"),
	fDaemonStatus(INIT),
	fPostSyncAction(None),
	fPilotLink(0L),
	fNextSyncType(SyncAction::SyncMode::eHotSync,true),
	fSyncStack(0L),
	fTray(0L),
	fInstaller(0L),
	fLogFile(0L),
	fLogStub(new LoggerDCOP_stub("kpilot", "LogIface")),
	fLogFileStub(new LoggerDCOP_stub("kpilotDaemon", "LogIface")),
	fKPilotStub(new KPilotDCOP_stub("kpilot", "KPilotIface")),
	fTempDevice(QString::null)
{
	FUNCTIONSETUP;

	setupPilotLink();
	reloadSettings();

	if (fDaemonStatus == ERROR)
	{
		WARNINGKPILOT << "Connecting to device failed." << endl;
		return;
	}

	fInstaller = new FileInstaller;
	fLogFile = new LogFile;
	connect(fInstaller, SIGNAL(filesChanged()),
		this, SLOT(slotFilesChanged()));

	fNextSyncType.setMode( KPilotSettings::syncType() );

#ifdef DEBUG
	DEBUGKPILOT << fname
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

	(void) PilotDatabase::instanceCount();
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
	DEBUGKPILOT << fname
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
		DEBUGKPILOT << fname << ": No tray icon to display!" << endl;
#endif

		return;
	}

	// Copied from Klipper
	KWin::setSystemTrayWindowFor(fTray->winId(), 0);
	fTray->setGeometry(-100, -100, 42, 42);
	fTray->show();

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Tray icon displayed." << endl;
#endif

	updateTrayStatus();
}

/* DCOP ASYNC */ void PilotDaemon::setTempDevice(QString d)
{
	if ( !d.isEmpty() ){
		fTempDevice = d;
		if (fPilotLink)
			fPilotLink->setTempDevice( fTempDevice );
		reloadSettings();
	}
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

	// TODO: Is this bunch of calls really necessary to reload the settings???
	delete KPilotSettings::self();
	KPilotSettings::self()->config()->reparseConfiguration();
	KPilotSettings::self()->readConfig();
	getPilotSpeed();

	(void) Pilot::setupPilotCodec(KPilotSettings::encoding());

#ifdef DEBUG
	DEBUGKPILOT << fname
		<< ": Got configuration "
		<< KPilotSettings::pilotDevice()
		<< endl;
	DEBUGKPILOT << fname
		<< ": Got conduit list "
		<< (KPilotSettings::installedConduits().join(CSL1(",")))
		<< endl;
#endif

	requestSync(0);


	if (fPilotLink)
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": Resetting with device "
			<< KPilotSettings::pilotDevice()
			<< endl;
#endif

		fPilotLink->reset( KPilotSettings::pilotDevice() );
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": Using workarounds "
			<< KPilotSettings::workarounds()
			<< endl;
#endif
		if ( KPilotSettings::workarounds() == KPilotSettings::eWorkaroundUSB )
		{
#ifdef DEBUG
			DEBUGKPILOT << fname
				<< ": Using Zire31 USB workaround." << endl;
#endif
			fPilotLink->setWorkarounds(true);
		}
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
	logProgress(QString::null,0);
}

/* DCOP */ void PilotDaemon::stopListening()
{
	fIsListening=false;
	fTray->changeIcon(PilotDaemonTray::NotListening);
	fDaemonStatus=NOT_LISTENING;
	fPilotLink->close();
}

/* DCOP */ void PilotDaemon::startListening()
{
	fIsListening=true;
	fTray->changeIcon(PilotDaemonTray::Normal);
	fDaemonStatus=INIT;
	fPilotLink->reset();
}

/* DCOP */ QString PilotDaemon::statusString()
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

/* DCOP */ QString PilotDaemon::shortStatusString()
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
		WARNINGKPILOT << "Can't get pilot link." << endl;
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
	case NOT_LISTENING:
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
	requestSync(SyncAction::SyncMode::eHotSync);
}


/* DCOP ASYNC */ void PilotDaemon::requestSync(int mode)
{
	FUNCTIONSETUP;

	if ( 0==mode )
	{
		mode = KPilotSettings::syncType();
	}

	if ( !fNextSyncType.setMode(mode) )
	{
		WARNINGKPILOT << "Ignored fake sync type " << mode << endl;
		return;
	}

	updateTrayStatus();

	if (fTray && (fTray->fSyncTypeMenu))
	{
		for (int i=((int)SyncAction::SyncMode::eHotSync);
			i<=((int)SyncAction::SyncMode::eRestore) /* Restore */ ;
			++i)
		{
			fTray->fSyncTypeMenu->setItemChecked(i,mode==i);
		}
	}

	getLogger().logMessage(i18n("Next HotSync will be: %1. ").arg(fNextSyncType.name()) +
		i18n("Please press the HotSync button."));
}

/* DCOP ASYNC */ void PilotDaemon::requestSyncType(QString s)
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
		WARNINGKPILOT << "Unknown sync type " << ( s.isEmpty() ? CSL1("<none>") : s )
			<< endl;
	}
}

/* DCOP ASYNC */ void PilotDaemon::requestSyncOptions(bool test, bool local)
{
	if ( !fNextSyncType.setOptions(test,local) )
	{
		WARNINGKPILOT << "Nonsensical request for "
			<< (test ? "test" : "notest")
			<< ' '
			<< (local ? "local" : "nolocal")
			<< " in mode "
			<< fNextSyncType.name() << endl;
	}
}

/* DCOP */ int PilotDaemon::nextSyncType() const
{
	return fNextSyncType.mode();
}

/**
* DCOP Functions reporting some status data, e.g. for the kontact plugin.
*/
QDateTime PilotDaemon::lastSyncDate()
{
	return KPilotSettings::lastSyncTime();
}


static QDict<QString> *conduitNameMap = 0L;

static void fillConduitNameMap()
{
	if ( !conduitNameMap )
	{
		conduitNameMap = new QDict<QString>;
		conduitNameMap->setAutoDelete(true);
	}
	conduitNameMap->clear();

	QStringList l = KPilotSettings::installedConduits();
	// Fill with internal settings.
	if ( l.find( CSL1("internal_fileinstall") ) != l.end() ) {
		conduitNameMap->insert( CSL1("internal_fileinstall"),
		                        new QString(i18n("File Installer")) );
	}

	QStringList::ConstIterator end = l.end();
	for (QStringList::ConstIterator i = l.begin(); i != end; ++i)
	{
		if (!conduitNameMap->find(*i))
		{
			QString readableName = CSL1("<unknown>");
			KSharedPtr < KService > o = KService::serviceByDesktopName(*i);
			if (!o)
			{
				WARNINGKPILOT << "No service for " << *i << endl;
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

	QStringList keys;

	QDictIterator<QString> it(*conduitNameMap);
	for ( ; *it; ++it)
	{
		keys << it.currentKey();
	}
	keys.sort();

	QStringList::ConstIterator end = keys.end();
	QStringList result;
	for (QStringList::ConstIterator i = keys.begin(); i != end; ++i)
	{
		result << *(conduitNameMap->find(*i));
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

typedef enum { NotLocked=0, Locked=1, DCOPError=2 } KDesktopLockStatus;
static KDesktopLockStatus isKDesktopLockRunning()
{
	if (!KPilotSettings::screenlockSecure()) return NotLocked;

	DCOPClient *dcopptr = KApplication::kApplication()->dcopClient();

	// Can't tell, very weird, err on the side of safety.
	if (!dcopptr || !dcopptr->isAttached())
	{
		WARNINGKPILOT << "Could not make DCOP connection. "
			<< "Assuming screensaver is active." << endl;
		return DCOPError;
	}

	QByteArray data,returnValue;
	QCString returnType;

	if (!dcopptr->call("kdesktop","KScreensaverIface","isBlanked()",
		data,returnType,returnValue,true))
	{
		WARNINGKPILOT << "Check for screensaver failed."
			<< "Assuming screensaver is active." << endl;
		// Err on the side of safety again.
		return DCOPError;
	}

	if (returnType == "bool")
	{
		bool b;
		QDataStream reply(returnValue,IO_ReadOnly);
		reply >> b;
		return (b ? Locked : NotLocked);
	}
	else
	{
		WARNINGKPILOT << "Strange return value from screensaver. "
			<< "Assuming screensaver is active." << endl;
		// Err on the side of safety.
		return DCOPError;
	}
}


static void informOthers(KPilotDCOP_stub &kpilot,
	LoggerDCOP_stub &log,
	LoggerDCOP_stub &filelog)
{
	kpilot.daemonStatus(KPilotDCOP::StartOfHotSync);
	log.logStartSync();
	filelog.logStartSync();
}

static bool isSyncPossible(ActionQueue *fSyncStack,
	KPilotLink *pilotLink,
	KPilotDCOP_stub &kpilot)
{
	FUNCTIONSETUP;

	/**
	* If KPilot is busy with something - like configuring
	* conduit - then we shouldn't run a real sync, but
	* just tell the user that the sync couldn't run because
	* of that.
	*/
	int kpilotstatus = kpilot.kpilotStatus();
	DCOPStub::Status callstatus = kpilot.status();

#ifdef DEBUG
	if (callstatus != DCOPStub::CallSucceeded)
	{
		DEBUGKPILOT << fname <<
			": Could not call KPilot for status." << endl;
	}
	else
	{
		DEBUGKPILOT << fname << ": KPilot status " << kpilotstatus << endl;
	}
#endif
	/**
	* If the call fails, then KPilot is probably not running
	* and we can behave normally.
	*/
	if ((callstatus == DCOPStub::CallSucceeded) &&
		(kpilotstatus != KPilotDCOP::WaitingForDaemon))
	{
		WARNINGKPILOT << "KPilot returned status " << kpilotstatus << endl;

		fSyncStack->queueInit();
		fSyncStack->addAction(new SorryAction(pilotLink));
		return false;
	}

	switch (isKDesktopLockRunning())
	{
	case NotLocked :
		break; /* Fall through to return true below */
	case Locked :
		fSyncStack->queueInit();
		fSyncStack->addAction(new SorryAction(pilotLink,
			i18n("HotSync is disabled while the screen is locked.")));
		return false;
	case DCOPError :
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
	if (c.findIndex(CSL1("internal_fileinstall")) >= 0)
	{
		fSyncStack->addAction(new FileInstallAction(pilotLink,fInstaller->dir()));
	}
}

static void queueEditors(ActionQueue *fSyncStack, KPilotLink *pilotLink)
{
	if (KPilotSettings::internalEditors())
	{
		fSyncStack->addAction(new InternalEditorAction(pilotLink));
	}
}

static void queueConduits(ActionQueue *fSyncStack,
	const QStringList &conduits,
	SyncAction::SyncMode e)
{
	if (conduits.count() > 0)
	{
		fSyncStack->queueConduits(conduits,e);
		// QString s = i18n("Conduit flags: ");
		// s.append(ConduitProxy::flagsForMode(e).join(CSL1(" ")));
		// logMessage(s);
	}
}

bool PilotDaemon::shouldBackup()
{

	FUNCTIONSETUP;

	bool ret = false;
	int backupfreq = KPilotSettings::backupFrequency();

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Backup Frequency is: [" << backupfreq <<
	"]. " << endl;
#endif

	if ( (fNextSyncType == SyncAction::SyncMode::eHotSync) ||
		(fNextSyncType == SyncAction::SyncMode::eFullSync) )
	{
		/** If we're doing a Hot or Full sync, see if our user has
		 * configured us to or to not always do a backup.
		 */
		if ( backupfreq == SyncAction::eOnRequestOnly )
		{
#ifdef DEBUG
	DEBUGKPILOT << fname << ": Should not do backup..." << endl;
#endif
			ret = false;
		}
		else if ( backupfreq == SyncAction::eEveryHotSync )
		{
#ifdef DEBUG
	DEBUGKPILOT << fname << ": Should do backup..." << endl;
#endif
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

#ifdef DEBUG
	DEBUGKPILOT << fname
		<< ": Starting Sync with type "
		<< fNextSyncType.name() << endl;
	DEBUGKPILOT << fname << ": Status is " << shortStatusString() << endl;
	(void) PilotDatabase::instanceCount();
#endif

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
#ifdef DEBUG
			DEBUGKPILOT << fname << ": PC changed. Last sync PC: [" << usr.getLastSyncPC()
				<< "], me: [" << (unsigned long) gethostid() << "]" << endl;
#endif
 			if ( KPilotSettings::fullSyncOnPCChange() )
			{
#ifdef DEBUG
				DEBUGKPILOT << fname << ": Setting sync mode to full sync. " << endl;
#endif
				fNextSyncType = SyncAction::SyncMode::eFullSync;
			}
			else
			{
#ifdef DEBUG
				DEBUGKPILOT << fname << ": Not changing sync mode because of settings. " << endl;
#endif
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
			queueEditors(fSyncStack,pilotLink);
			queueConduits(fSyncStack,conduits,fNextSyncType);
			// After running the conduits, install new databases
			queueInstaller(fSyncStack,pilotLink,fInstaller,conduits);
			// And sync the remaining databases if needed.
			if (shouldBackup())
			{
				fSyncStack->addAction(new BackupAction(pilotLink, (fNextSyncType == SyncAction::SyncMode::eFullSync)));
			}
			break;
		case SyncAction::SyncMode::eCopyPCToHH:
			queueConduits(fSyncStack,conduits,SyncAction::SyncMode::eCopyPCToHH);
			break;
		case SyncAction::SyncMode::eCopyHHToPC:
			queueConduits(fSyncStack,conduits,SyncAction::SyncMode::eCopyHHToPC);
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
		QTimer::singleShot(10000,fPilotLink,SLOT(reset()));
	}

	fPostSyncAction = None;
	requestSync(0);

	(void) PilotDatabase::instanceCount();

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
		WARNINGKPILOT << "Couldn't start KPilot! " << kpilotError << endl;
	}
	else
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
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
		*p << "kpilot" << "-s";

		p->start();
	}
}

void PilotDaemon::updateTrayStatus(const QString &s)
{
	if (!fTray) return;

	QString tipText = CSL1("<qt>");
	tipText.append( s );
	tipText.append( CSL1(" ") );
	tipText.append( i18n("Next sync is %1.")
		.arg( fNextSyncType.name() ) );
	tipText.append( CSL1("</qt>") );

	QToolTip::remove(fTray);
	QToolTip::add(fTray,tipText);
	emitDCOPSignal( "kpilotDaemonStatusChanged()", QByteArray() );
	// emit the same dcop signal but including the information needed by Kontact to update its kpilot summary widget
	QByteArray data;
	QDataStream arg(data, IO_WriteOnly);
	arg << lastSyncDate();
	arg << shortStatusString();
	arg << configuredConduitList();
	arg << logFileName();
	arg << userName();
	arg << pilotDevice();
	arg << killDaemonOnExit();
	emitDCOPSignal( "kpilotDaemonStatusDetails(QDateTime,QString,QStringList,QString,QString,QString,bool)", data );
}

static KCmdLineOptions daemonoptions[] = {
#ifdef DEBUG
	{"debug <level>", I18N_NOOP("Set debugging level"), "0"},
#endif
	{ "device <device>", I18N_NOOP("Device to try first"), ""},
	{"fail-silently", I18N_NOOP("Exit instead of complaining about bad configuration files"), 0},
	KCmdLineLastOption
} ;


int main(int argc, char **argv)
{
	FUNCTIONSETUP;

	KLocale::setMainCatalogue("kpilot");

	KAboutData about("kpilotDaemon",
		I18N_NOOP("KPilot Daemon"),
		KPILOT_VERSION,
		"KPilot - HotSync software for KDE\n\n",
		KAboutData::License_GPL,
		"(c) 1998-2000,2001, Dan Pilone (c) 2000-2004, Adriaan de Groot",
		0L,
		"http://www.kpilot.org/"
		);
	about.addAuthor("Dan Pilone",
		I18N_NOOP("Project Leader"),
		"pilone@slac.com");
	about.addAuthor("Adriaan de Groot",
		I18N_NOOP("Maintainer"),
		"groot@kde.org", "http://www.kpilot.org/");
	about.addAuthor("Reinhold Kainhofer",
		I18N_NOOP("Developer"),
		"reinhold@kainhofer.com", "http://reinhold.kainhofer.com/Linux/");
	aboutData = &about;


	KCmdLineArgs::init(argc, argv, &about);
	KCmdLineArgs::addCmdLineOptions(daemonoptions,"kpilotconfig");
	KUniqueApplication::addCmdLineOptions();
	KCmdLineArgs *p = KCmdLineArgs::parsedArgs();

#ifdef DEBUG
	KPilotConfig::getDebugLevel(p);
#endif
	if (!KUniqueApplication::start())
	{
		if (p->isSet("device")){
			// tell the running kpilotDaemon to use
			// this device now
			DCOPClient d;
			QString dev(p->getOption("device"));
			QByteArray data;
			QDataStream arg(data, IO_WriteOnly);
			arg << dev;
			if (d.attach()){
				d.send("kpilotDaemon", "KPilotDaemonIface", "setTempDevice(QString)", data );
				d.detach();
			}
		}
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
			WARNINGKPILOT << "Is still not configured for use."
				<< endl;
			if (!p->isSet("fail-silently"))
			{
				KPilotConfig::sorryVersionOutdated(KPilotSettings::configVersion());
			}
			return 1;
		}

#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": Configuration version "
			<< KPilotSettings::configVersion() << endl;
#endif
	}


	PilotDaemon *gPilotDaemon = new PilotDaemon();

	if (p->isSet("device"))
		gPilotDaemon->setTempDevice(p->getOption("device"));

	if (gPilotDaemon->status() == PilotDaemon::ERROR)
	{
		delete gPilotDaemon;

		gPilotDaemon = 0;
		WARNINGKPILOT << "Failed to start up daemon "
			"due to errors constructing it." << endl;
		return 2;
	}

	gPilotDaemon->showTray();

	return a.exec();
}



