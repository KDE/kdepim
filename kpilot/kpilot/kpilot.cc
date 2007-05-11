/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** This is the main program in KPilot.
**
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

#include <QCloseEvent>
#include <QFile>
#include <QPixmap>
#include <QTimer>

#include <KShortcutsDialog>

#include <kurl.h>
#include <kmessagebox.h>
#include <kstatusbar.h>
#include <kconfig.h>
#include <kcombobox.h>
#include <kmenubar.h>
#include <kstandarddirs.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kstandardaction.h>
#include <kuniqueapplication.h>
#include <kedittoolbar.h>
#include <kcmultidialog.h>
#include <kprogressdialog.h>
#include <klibloader.h>
#include <ktoolinvocation.h>

#include <KActionMenu>
#include <KXMLGUIFactory>

#include "kpilotConfig.h"

#include "pilotComponent.h"
#include "pilotDatabase.h"
#include "syncAction.h"

#include "addressWidget.h"
#include "memoWidget.h"
#include "fileInstallWidget.h"
#include "logWidget.h"
#include "dbviewerWidget.h"
#include "datebookWidget.h"
#include "todoWidget.h"

#include "config_dialog.h"

#include "kpilotadaptor.h"
#include "daemon_interface.h"

#include "kpilot.moc"

class KPilotInstaller::KPilotPrivate
{
public:
	typedef QList<PilotComponent *> ComponentList;

	ComponentList  fPilotComponentList;

	/** Was the daemon running before KPilot started? If not, then we
	 *  started it and it is our responsibility to stop it on exit (if configured
	 *  that way).
	 */
	bool fDaemonWasRunning;
	/** Flag to indicate that the configure dialog is in use and no sync should
	 *  be started just now.
	 */
	bool fConfigureKPilotDialogInUse;
	/** When the application is created, data is delay-loaded into the widgets.
	 *  This flag asserts that this is the first load and is set to false afterwards.
	 */
	bool fFirstLoad;

	KPilotStatus fAppStatus;
} ;

KPilotInstaller::KPilotInstaller() :
	KXmlGuiWindow(0),
	fP(new KPilotPrivate)
{
	fP->fAppStatus = Startup;
	fP->fDaemonWasRunning = true; // Assume it was
	fP->fConfigureKPilotDialogInUse = false;
	fP->fFirstLoad = true;

	new KpilotAdaptor(this);
	QDBusConnection::sessionBus().registerObject("/KPilot", this);
	//TODO verify it
	fDaemonInterface = new OrgKdeKpilotDaemonInterface("org.kde.kpilot.daemon", "/Daemon", QDBusConnection::sessionBus());
	FUNCTIONSETUP;

	readConfig();
	setupWidget();

	PilotRecord::allocationInfo();

	QTimer::singleShot(500, this, SLOT(componentUpdate()));
}

KPilotInstaller::~KPilotInstaller()
{
	FUNCTIONSETUP;
	killDaemonIfNeeded();
	delete fDaemonInterface;
	PilotRecord::allocationInfo();
	(void) PilotDatabase::instanceCount();
}

void KPilotInstaller::killDaemonIfNeeded()
{
	FUNCTIONSETUP;
	if (KPilotSettings::killDaemonAtExit())
	{
		if (!fP->fDaemonWasRunning)
		{
			DEBUGKPILOT << fname << ": Killing daemon." << endl;
			getDaemon().quitNow();
		}
	}
}

void KPilotInstaller::startDaemonIfNeeded()
{
	FUNCTIONSETUP;
	fP->fAppStatus=Normal;
	WARNINGKPILOT << fname << "Skipping daemon initialization." << endl;
	return;


	fP->fAppStatus=WaitingForDaemon;

	QString daemonError;
	QString daemonPID;

	QString s = getDaemon().statusString();

	DEBUGKPILOT << fname << ": Daemon status is "
		<< ( s.isEmpty() ? CSL1("<none>") : s ) << endl;

	if ((s.isEmpty()) || (!getDaemon().isValid()))
	{
		DEBUGKPILOT << fname
			<< ": Daemon not responding, trying to start it."
			<< endl;
		statusMessage(i18n("Starting the KPilot daemon ..."));
		fP->fDaemonWasRunning = false;
	}
	else
	{
		fP->fDaemonWasRunning = true;
	}
	if (!fP->fDaemonWasRunning && KToolInvocation::startServiceByDesktopName( CSL1("kpilotdaemon"), QStringList(), &daemonError, &daemonPID ) )
	{
		WARNINGKPILOT << ": Can't start daemon : " << daemonError << endl;
		statusMessage(i18n("Could not start the "
			"KPilot daemon. The system error message "
			"was: &quot;%1&quot;",daemonError));
		fP->fAppStatus=Error;
	}
	else
	{
		DEBUGKPILOT << fname << ": Daemon status is " << s << endl;
		int wordoffset;
		s.remove(0,12);
		wordoffset=s.indexOf(';');
		if (wordoffset>0)
		{
			s.truncate(wordoffset);
		}

		statusMessage(
			i18n("Daemon status is `%1'",s.isEmpty() ? i18n("not running") : s ));
		fP->fAppStatus=Normal;
	}
}

void KPilotInstaller::readConfig()
{
	FUNCTIONSETUP;

	KPilotSettings::self()->readConfig();

	(void) Pilot::setupPilotCodec(KPilotSettings::encoding());

	statusMessage(i18n("Using character set %1 on "
		"the handheld.",Pilot::codecName()));
}

static QWidget *initComponents( QWidget *parent, QList<PilotComponent *> &l )
{
	FUNCTIONSETUP;
	QTabWidget *w = new QTabWidget( parent );
	w->setObjectName( "main_tab_widget" );

	QString defaultDBPath = KPilotConfig::getDefaultDBPath();

	PilotComponent *p;

#define ADDPAGE(cls,label) \
	p = new cls(w,defaultDBPath); \
	w->addTab(p, label); \
	l.append(p);

	ADDPAGE(TodoWidget, i18n("To-do Viewer"))
	p->showKPilotComponent(true);
	ADDPAGE(AddressWidget, i18n("Address Viewer"))
	ADDPAGE(MemoWidget, i18n("Memo Viewer"))
	ADDPAGE(GenericDBWidget, i18n("Generic DB Viewer"))

#undef ADDPAGE

	return w;
}

void initMenu( KXmlGuiWindow *parent )
{
	FUNCTIONSETUP;
	QAction *a;

	KActionMenu *syncPopup;

	syncPopup = new KActionMenu(KIcon(CSL1("kpilot_hotsync")),i18n("HotSync"),
		parent->actionCollection());
	parent->actionCollection()->addAction("popup_hotsync", syncPopup);


	syncPopup->setToolTip(i18n("Select the kind of HotSync to perform next."));
	syncPopup->setWhatsThis(i18n("Select the kind of HotSync to perform next. "
		"This applies only to the next HotSync; to change the default, use "
		"the configuration dialog."));
	QObject::connect(syncPopup, SIGNAL(activated()),
		parent, SLOT(slotHotSyncRequested()));

        a = parent->actionCollection()->addAction( CSL1("file_hotsync"),
		parent, SLOT(slotHotSyncRequested()) );
    	a->setText(i18n("&HotSync"));
	a->setIcon(KIcon(CSL1("kpilot_hotsync")));
        a->setToolTip(i18n("Next HotSync will be normal HotSync."));
        a->setWhatsThis(i18n("Tell the daemon that the next HotSync "
                "should be a normal HotSync."));

        a = parent->actionCollection()->addAction( "file_fullsync");
        a->setText(i18n("Full&Sync"));
        a->setIcon(KIcon(CSL1("kpilot_fullsync")));
        a->setToolTip(i18n("Next HotSync will be a FullSync."));
        a->setWhatsThis(i18n("Tell the daemon that the next HotSync "
                "should be a FullSync (check data on both sides)."));
        QObject::connect(a, SIGNAL(triggered()), parent, SLOT(slotFullSyncRequested()));

        a = parent->actionCollection()->addAction( "file_backup");
        a->setText(i18n("&Backup"));
        a->setIcon(KIcon(CSL1("kpilot_backup")));
        a->setToolTip(i18n("Next HotSync will be backup."));
	a->setWhatsThis(i18n("Tell the daemon that the next HotSync "
                "should back up the Handheld to the PC."));
        QObject::connect(a, SIGNAL(triggered()), parent, SLOT(slotBackupRequested()));

        a = parent->actionCollection()->addAction( "file_restore");
        a->setText(i18n("&Restore"));
        a->setIcon(KIcon(CSL1("kpilot_restore")));
        a->setToolTip(i18n("Next HotSync will be restore."));
        a->setWhatsThis(i18n("Tell the daemon that the next HotSync "
                "should restore the Handheld from data on the PC."));
        QObject::connect(a, SIGNAL(triggered()), parent, SLOT(slotRestoreRequested()));

        a = parent->actionCollection()->addAction( "file_HHtoPC");
        a->setText(i18n("Copy Handheld to PC"));
	a->setIcon(KIcon(CSL1("kpilot_hhtopc")));
        a->setToolTip(i18n("Next HotSync will be backup."));
        a->setWhatsThis(i18n("Tell the daemon that the next HotSync "
                "should copy all data from the Handheld to the PC, "
                "overwriting entries on the PC."));
        QObject::connect(a, SIGNAL(triggered()), parent, SLOT(slotPCtoHHRequested()));

        a = parent->actionCollection()->addAction( "file_reload");
        a->setText(i18n("Rese&t Link"));
	a->setIcon(KIcon(CSL1("kpilot_reset")));
        a->setToolTip(i18n("Reset the device connection."));
        a->setWhatsThis(i18n("Try to reset the daemon and its connection "
                "to the Handheld."));
        QObject::connect(a, SIGNAL(triggered()), parent, SLOT(slotResetLink()));

        a = KStandardAction::quit(parent, SLOT(quit()), parent->actionCollection());
        a->setWhatsThis(i18n("Quit KPilot, (and stop the daemon "
                "if configured that way)."));

	(void) KStandardAction::keyBindings(parent->guiFactory(), SLOT(configureShortcuts()), parent->actionCollection());
	(void) KStandardAction::configureToolbars(parent, SLOT(configureToolbars()), parent->actionCollection());
        (void) KStandardAction::preferences(parent, SLOT(configure()),
                parent->actionCollection());
}



void KPilotInstaller::setupWidget()
{
	FUNCTIONSETUP;

	setCaption(CSL1("KPilot"));
	setMinimumSize(500, 405);

	setStatusBar(0L);

	createStandardStatusBarAction();
	setStandardToolBarMenuEnabled(true);
	initMenu( this );
	setCentralWidget( initComponents( this, fP->fPilotComponentList ) );

	setMinimumSize(sizeHint() + QSize(10,60));

	createGUI(CSL1("kpilotui.rc"));
	setAutoSaveSettings();
}

void KPilotInstaller::slotBackupRequested()
{
	FUNCTIONSETUP;
	setupSync(SyncAction::SyncMode::eBackup,
		i18n("Next sync will be a backup. ") +
		i18n("Please press the HotSync button."));
}

void KPilotInstaller::slotRestoreRequested()
{
	FUNCTIONSETUP;
	setupSync(SyncAction::SyncMode::eRestore,
		i18n("Next sync will restore the Pilot from backup. ") +
		i18n("Please press the HotSync button."));
}

void KPilotInstaller::slotHotSyncRequested()
{
	FUNCTIONSETUP;
	setupSync(SyncAction::SyncMode::eHotSync,
		i18n("Next sync will be a regular HotSync. ") +
		i18n("Please press the HotSync button."));
}

void KPilotInstaller::slotFullSyncRequested()
{
	FUNCTIONSETUP;
	setupSync(SyncAction::SyncMode::eFullSync,
		i18n("Next sync will be a Full Sync. ") +
		i18n("Please press the HotSync button."));
}

void KPilotInstaller::slotHHtoPCRequested()
{
	FUNCTIONSETUP;
	setupSync(SyncAction::SyncMode::eCopyHHToPC,
		i18n("Next sync will copy Handheld data to PC. ") +
		i18n("Please press the HotSync button."));
}

void KPilotInstaller::slotPCtoHHRequested()
{
	FUNCTIONSETUP;
	setupSync(SyncAction::SyncMode::eCopyPCToHH,
		i18n("Next sync will copy PC data to Handheld. ") +
		i18n("Please press the HotSync button."));
}

void KPilotInstaller::daemonStatus(int i)
{
	FUNCTIONSETUP;
	DEBUGKPILOT << fname << ": Received daemon message " << i << endl;

	switch(i)
	{
	case KPilotInstaller::StartOfHotSync :
		if (kpilotStatus()==Normal)
		{
			fP->fAppStatus=WaitingForDaemon;
			componentPreSync();
		}
		break;
	case KPilotInstaller::EndOfHotSync :
		if (kpilotStatus()==WaitingForDaemon)
		{
			componentPostSync();
			fP->fAppStatus=Normal;
		}
		break;
	case KPilotInstaller::DaemonQuit :
		statusMessage(i18n("The daemon has exited. "
			"No further HotSyncs are possible."));
		fP->fAppStatus=WaitingForDaemon;
		break;
	case KPilotInstaller::None :
		WARNINGKPILOT << "Unhandled status message " << i << endl;
		break;
	}
}

int KPilotInstaller::kpilotStatus() const
{
	return fP->fAppStatus;
}

bool KPilotInstaller::componentPreSync()
{
	FUNCTIONSETUP;

	QString reason;

	int e = fP->fPilotComponentList.size();
	for (int i = 0; i<e; ++i)
	{
		if (!fP->fPilotComponentList[i]->preHotSync(reason))
		{
			break;
		}
	}

	if (!reason.isEmpty())
	{
		KMessageBox::sorry(this,
			i18n("Cannot start a Sync now. %1",reason),
			i18n("Cannot start Sync"));
		return false;
	}
	return true;
}

void KPilotInstaller::componentPostSync()
{
	FUNCTIONSETUP;

	int e = fP->fPilotComponentList.size();
	for (int i = 0; i<e; ++i)
	{
		fP->fPilotComponentList[i]->postHotSync();
	}
}

void KPilotInstaller::setupSync(int kind, const QString & message)
{
	FUNCTIONSETUP;

	if (!componentPreSync())
	{
		return;
	}
	if (!message.isEmpty())
	{
		statusMessage(message);
	}
	getDaemon().requestSync(kind);
}

void KPilotInstaller::quit()
{
	FUNCTIONSETUP;

	int e = fP->fPilotComponentList.size();
	for (int i = 0; i<e; ++i)
	{
		QString reason;
		if (!fP->fPilotComponentList[i]->preHotSync(reason))
		{
			WARNINGKPILOT
				<< "Couldn't save "
				<< fP->fPilotComponentList[i]->objectName()
				<< endl;
		}
	}

	killDaemonIfNeeded();
	kapp->quit();
}

void KPilotInstaller::slotResetLink()
{
	FUNCTIONSETUP;
	statusMessage( i18n("Resetting device connection ...") );
	getDaemon().reloadSettings();
}

/*
** Can't be a member function because it needs to be called even with no KPilotInstaller.
*/
static bool runConfigure(OrgKdeKpilotDaemonInterface &daemon,QWidget *parent)
{
	FUNCTIONSETUP;
	bool ret = false;
	// Display the (modal) options page.
	//
	//
	int rememberedSync = daemon.nextSyncType();
	daemon.requestSync(0);

	KPilotSettings::self()->readConfig();
	KCMultiDialog *options = new KCMultiDialog(parent);
	options->setModal(true);
	KPageWidgetItem *item = options->addModule( CSL1("kpilot_config.desktop"));
	item->setName(i18n("Configuration"));

	if (!options)
	{
		WARNINGKPILOT << "Can't allocate KPilotOptions object" << endl;
		daemon.requestSync(rememberedSync);
		return false;
	}

	int r = options->exec();

	if ( r && options->result() )
	{
		DEBUGKPILOT << fname << ": Updating settings." << endl;

		// The settings are changed in the external module!!!
		KPilotSettings::self()->config()->sync();
		KPilotSettings::self()->readConfig();

		// Update the daemon to reflect new settings.
		// @TODO: This should also be done when pressing apply without
		// closing the dialog.
		//
		daemon.reloadSettings();
		ret = true;
	}

	KPILOT_DELETE(options);
	daemon.requestSync(rememberedSync);

	KPilotConfig::sync();
	return ret;
}

void KPilotInstaller::componentUpdate()
{
	FUNCTIONSETUP;

	QString defaultDBPath = KPilotConfig::getDefaultDBPath();
	bool dbPathChanged = fP->fFirstLoad;
	fP->fFirstLoad = false;

	int e = fP->fPilotComponentList.size();
	for (int i = 0; i<e; ++i)
	{
		PilotComponent *p = fP->fPilotComponentList[i];
		if (p && (p->dbPath() != defaultDBPath))
		{
			dbPathChanged = true;
			p->setDBPath(defaultDBPath);
		}
	}

	if (!dbPathChanged) // done if the username didn't change
	{
		return;
	}

	// Otherwise, need to re-load the databases
	//
	statusMessage(i18n("Changed username to `%1'.",KPilotSettings::userName()));

	for (int i = 0; i<e; ++i)
	{
		PilotComponent *p = fP->fPilotComponentList[i];
		if (p)
		{
			p->showComponent();
		}
	}
}

void KPilotInstaller::configure()
{
	FUNCTIONSETUP;

	if ( kpilotStatus()!=Normal || fP->fConfigureKPilotDialogInUse )
	{
		statusMessage(i18n("Cannot configure KPilot right now (KPilot's UI is already busy)."));
		return;
	}
	fP->fAppStatus=UIBusy;
	fP->fConfigureKPilotDialogInUse = true;
	if (runConfigure(getDaemon(),this))
	{
		componentUpdate();
	}

	fP->fConfigureKPilotDialogInUse = false;
	fP->fAppStatus=Normal;
}

void KPilotInstaller::statusMessage( const QString &s )
{
	QStatusBar *b = statusBar();
	if (b)
	{
		b->showMessage( s );
	}
}

// Command line options descriptions.
//
//
//
//
static KCmdLineOptions kpilotoptions[] = {
	{"s", 0, 0},
	{"setup",
		I18N_NOOP("Setup the Pilot device, conduits and other parameters"),
		0L},
	{"debug <level>", I18N_NOOP("Set debugging level"), "0"},
	KCmdLineLastOption
};




// "Regular" mode == 0
// setup mode == 's'
//
// This is only changed by the --setup flag --
// kpilot still does a setup the first time it is run.
//
//
KPilotConfig::RunMode run_mode = KPilotConfig::Normal;



int main(int argc, char **argv)
{
	FUNCTIONSETUP;

	KAboutData about("kpilot", I18N_NOOP("KPilot"),
		KPILOT_VERSION,
		"KPilot - HotSync software for KDE\n\n",
		KAboutData::License_GPL,
		"(c) 1998-2000,2001, Dan Pilone (c) 2000-2006, Adriaan de Groot",
		0L,
		"http://www.kpilot.org/"
		);
	about.addAuthor("Dan Pilone",
		I18N_NOOP("Project Leader"),
		"pilone@slac.com" );
	about.addAuthor("Adriaan de Groot",
		I18N_NOOP("Maintainer"),
		"groot@kde.org", "http://www.kpilot.org/");
	about.addAuthor("Reinhold Kainhofer",
		I18N_NOOP("Core and conduits developer"), "reinhold@kainhofer.com", "http://reinhold.kainhofer.com/Linux/");
	about.addAuthor("Jason 'vanRijn' Kasper",
		I18N_NOOP("Core and conduits developer"),
		"vR@movingparts.net", "http://movingparts.net/");
	about.addCredit("Preston Brown", I18N_NOOP("VCal conduit"));
	about.addCredit("Greg Stern", I18N_NOOP("Abbrowser conduit"));
	about.addCredit("Chris Molnar", I18N_NOOP("Expenses conduit"));
	about.addCredit("Jörn Ahrens", I18N_NOOP("Notepad conduit, Bugfixer"));
	about.addCredit("Heiko Purnhagen", I18N_NOOP("Bugfixer"));
	about.addCredit("Jörg Habenicht", I18N_NOOP("Bugfixer"));
	about.addCredit("Martin Junius",
		I18N_NOOP("XML GUI"),
		"mj@m-j-s.net", "http://www.m-j-s.net/kde/");
	about.addCredit("David Bishop",
		I18N_NOOP(".ui files"));
	about.addCredit("Aaron J. Seigo",
		I18N_NOOP("Bugfixer, coolness"));
	about.addCredit("Bertjan Broeksema",
		I18N_NOOP("VCalconduit state machine, CMake"));
	about.addCredit("Montel Laurent",
		I18N_NOOP("KDE4 port"));
	KCmdLineArgs::init(argc, argv, &about);
	KCmdLineArgs::addCmdLineOptions(kpilotoptions, "kpilot");
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


	if (p->isSet("setup"))
	{
		run_mode = KPilotConfig::ConfigureKPilot;
	}
	else if (KPilotSettings::configVersion() < KPilotConfig::ConfigurationVersion)
	{
		WARNINGKPILOT << "KPilot configuration version "
			<< KPilotConfig::ConfigurationVersion
			<< " newer than stored version "
			<< KPilotSettings::configVersion() << endl;
		// Only force a reconfigure and continue if the
		// user is expecting normal startup. Otherwise,
		// do the configuration they're explicitly asking for.
		run_mode = KPilotConfig::interactiveUpdate();
		if (run_mode == KPilotConfig::Cancel) return 1;
	}


	if ( (run_mode == KPilotConfig::ConfigureKPilot) ||
		(run_mode == KPilotConfig::ConfigureAndContinue) )
	{
		DEBUGKPILOT << fname
			<< ": Running setup first."
			<< " (mode " << run_mode << ")" << endl;
		OrgKdeKpilotDaemonInterface * daemon = new OrgKdeKpilotDaemonInterface("org.kde.kpilot.daemon", "/Daemon", QDBusConnection::sessionBus());
		bool r = runConfigure(*daemon,0L);
		delete daemon;
		if (!r)
		{
			return 1;
		}
		// User expected configure only.
		if (run_mode == KPilotConfig::ConfigureKPilot)
		{
			return 0;
		}
	}

	if (KPilotSettings::configVersion() < KPilotConfig::ConfigurationVersion)
	{
		WARNINGKPILOT << "Still not configured for use." << endl;
		KPilotConfig::sorryVersionOutdated( KPilotSettings::configVersion());
		return 1;
	}


	KPilotInstaller *tp = new KPilotInstaller();

	if (tp->kpilotStatus() == KPilotInstaller::Error)
	{
		KPILOT_DELETE(tp);
		return 1;
	}

	QTimer::singleShot(0,tp,SLOT(startDaemonIfNeeded()));

	KGlobal::dirs()->addResourceType("pilotdbs",
		CSL1("share/apps/kpilot/DBBackup"));
	tp->show();
	//a.setMainWidget(tp);
	return a.exec();
}


