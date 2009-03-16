/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone <dan@kpilot.org>
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

#include <QtCore/QFile>
#include <QtCore/QTimer>
#include <QtGui/QCloseEvent>
#include <QtGui/QPixmap>

#include <kshortcutsdialog.h>
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
#include <kaction.h>
#include <kactioncollection.h>
#include <kstandardaction.h>
#include <kuniqueapplication.h>
#include <kedittoolbar.h>
#include <kcmultidialog.h>
#include <kprogressdialog.h>
#include <klibloader.h>
#include <ktoolinvocation.h>
#include <kactionmenu.h>
#include <kxmlguifactory.h>

#include "options.h"
#include "kpilotConfig.h"
#include "pilotDatabase.h"
#include "pilotRecord.h"
#include "syncAction.h"
#include "logWidget.h"
#include "fileInstallWidget.h"
#include "config_dialog.h"
#include "kpilotadaptor.h"

#include "daemon_interface.h"
#include "kpilot.moc"

class KPilotInstaller::KPilotPrivate
{
public:
	KPilotPrivate() {
		fDaemonWasRunning = false;
		fConfigureKPilotDialogInUse = false;
		fFirstLoad = true;
		fFileInstallWidget = 0L;
		fLogWidget = 0L;
	}
	PageList fPilotComponentList;

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

	FileInstallWidget *fFileInstallWidget;

	LogWidget *fLogWidget;

	KPilotStatus fAppStatus;
	
	QPoint fLastWindowPos;
} ;

KPilotInstaller::KPilotInstaller() :
	KXmlGuiWindow(0),
	fP(new KPilotPrivate)
{
	FUNCTIONSETUP;
	
	fP->fAppStatus = Startup;
	fP->fDaemonWasRunning = true; // Assume it was
	fP->fConfigureKPilotDialogInUse = false;
	fP->fFirstLoad = true;
	fP->fFileInstallWidget = 0L;

	new KpilotAdaptor(this);
	QDBusConnection::sessionBus().registerObject("/KPilot", this);
	//TODO verify it
	fDaemonInterface = new OrgKdeKpilotDaemonInterface("org.kde.kpilot.daemon", "/Daemon", QDBusConnection::sessionBus());

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

void KPilotInstaller::log(const QString &msg)
{
	if (fP->fLogWidget) 
	{
		fP->fLogWidget->addMessage(msg);
	}
}

void KPilotInstaller::killDaemonIfNeeded()
{
	FUNCTIONSETUP;
	if (KPilotSettings::killDaemonAtExit())
	{
		if (!fP->fDaemonWasRunning)
		{
			DEBUGKPILOT << "Killing daemon.";
			getDaemon().quitNow();
		}
	}
}

void KPilotInstaller::startDaemonIfNeeded()
{
	FUNCTIONSETUP;

	fP->fAppStatus=WaitingForDaemon;

	QString daemonError;
	QString daemonPID;
	bool daemonStarted = false;

	QString s = getDaemon().statusString();

	DEBUGKPILOT << "Daemon status is"
		<< ( s.isEmpty() ? CSL1("<none>") : s );

	if ((s.isEmpty()) || (!getDaemon().isValid()))
	{
		DEBUGKPILOT << "Daemon not responding, trying to start it.";
		log(i18n("Starting the KPilot daemon ..."));
		fP->fDaemonWasRunning = false;

		// success (== 0) or failure (> 0).
		daemonStarted = (KToolInvocation::startServiceByDesktopName( 
		    CSL1("kpilotdaemon"), QStringList(), &daemonError, &daemonPID )) == 0;

		if (!daemonStarted)
		{
			WARNINGKPILOT << "Could not start daemon: " << daemonError;
			log(i18n("Could not start the "
				"KPilot daemon. The system error message "
				"was: &quot;%1&quot;",daemonError));
			fP->fAppStatus=Error;
			return;
		} 
		else
		{
			log(i18n("Daemon started."));
		}
	}
	else
	{
		fP->fDaemonWasRunning = true;
	}

	// if we're here, it means that either the daemon was running or we think
	// that we were able to start it
	s = getDaemon().statusString();

	DEBUGKPILOT << "Daemon status is" << s;
	int wordoffset;
	s.remove(0,12);
	wordoffset=s.indexOf(';');
	if (wordoffset>0)
	{
		s.truncate(wordoffset);
	}

	log(
		i18n("Daemon status is `%1'",s.isEmpty() ? i18n("not running") : s ));

	fP->fAppStatus=Normal;
}

void KPilotInstaller::readConfig()
{
	FUNCTIONSETUP;

	KPilotSettings::self()->readConfig();

	(void) Pilot::setupPilotCodec(KPilotSettings::encoding());

	log(i18n("Using character set %1 on "
		"the handheld.",Pilot::codecName()));
}

QWidget *KPilotInstaller::initPages( QWidget *parent, QList<ComponentPageBase*> &l )
{
	FUNCTIONSETUP;
	KPageWidget *pageWidget = new KPageWidget( parent );
	pageWidget->setObjectName( "main_tab_widget" );

	QString defaultDBPath = KPilotConfig::getDefaultDBPath();

	KPageWidgetItem *item;

#define ADDICONPAGE( widget, label, iconname ) \
    	item = new KPageWidgetItem( widget, label ); \
	item->setIcon( KIcon( iconname ) ); \
	pageWidget->addPage( item ); \
	l.append( widget );

	fP->fLogWidget = new LogWidget( pageWidget );
	fP->fLogWidget->setShowTime( true );
	ADDICONPAGE( fP->fLogWidget, i18n( "HotSync" ), CSL1( "kpilot_bhotsync" ) )
	
	fP->fFileInstallWidget = new FileInstallWidget( pageWidget );
	ADDICONPAGE( fP->fFileInstallWidget, i18n("File Installer")
		, CSL1( "kpilot_fileinstaller" ) )

#undef ADDICONPAGE

	l[0]->showPage();

	QObject::connect( pageWidget
		, SIGNAL( currentPageChanged( KPageWidgetItem*, KPageWidgetItem* ) )
		, parent, SLOT( componentChanged( KPageWidgetItem*, KPageWidgetItem* ) ) );
	
	return pageWidget;
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

        a = parent->actionCollection()->addAction( "file_PCtoHH");
        a->setText(i18n("Copy PC to Handheld"));
	a->setIcon(KIcon(CSL1("kpilot_hhtopc")));
        a->setToolTip(i18n("Next HotSync will copy PC to Handheld."));
        a->setWhatsThis(i18n("Tell the daemon that the next HotSync "
                "should copy all data from the PC to the Handheld, "
                "overwriting entries on the Handheld."));
        QObject::connect(a, SIGNAL(triggered()), parent, SLOT(slotPCtoHHRequested()));

        a = parent->actionCollection()->addAction( "file_HHtoPC");
        a->setText(i18n("Copy Handheld to PC"));
	a->setIcon(KIcon(CSL1("kpilot_hhtopc")));
        a->setToolTip(i18n("Next HotSync will copy Handheld to PC."));
        a->setWhatsThis(i18n("Tell the daemon that the next HotSync "
                "should copy all data from the Handheld to the PC, "
                "overwriting entries on the PC."));
        QObject::connect(a, SIGNAL(triggered()), parent, SLOT(slotHHtoPCRequested()));

        /*
         * TODO: Not sure what the state of these are and we don't have
         *       time/manpower to catch them before 4.2. These have been
			*       taken out of kpilotui.rc for the time being. (Backup and
			*       Restore). We can look at enabling them back later, after
			*       the smoke has cleared.
			*/
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
	setCentralWidget( initPages( this, fP->fPilotComponentList ) );

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
	DEBUGKPILOT << "Received daemon message" << i;

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
		log(i18n("The daemon has exited."));
		log(i18n("No further HotSyncs are possible."));
		log(i18n("Restart the daemon to HotSync again."));

		fP->fAppStatus=WaitingForDaemon;
		break;
	case KPilotInstaller::None :
		WARNINGKPILOT << "Unhandled status message" << i;
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

	// Maybe we should completely remove pre and postsync
	/*
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
	*/
	return true;
}

void KPilotInstaller::componentPostSync()
{
	FUNCTIONSETUP;

	// Maybe we should completely remove pre and postsync
	/*
	int e = fP->fPilotComponentList.size();
	for (int i = 0; i<e; ++i)
	{
		fP->fPilotComponentList[i]->postHotSync();
	}
	*/
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
		log(message);
	}
	getDaemon().requestSync(kind);
}

void KPilotInstaller::quit()
{
	FUNCTIONSETUP;

	// Maybe we should completely remove pre and postsync
	/*
	int e = fP->fPilotComponentList.size();
	for (int i = 0; i<e; ++i)
	{
		QString reason;
		if (!fP->fPilotComponentList[i]->preHotSync(reason))
		{
			WARNINGKPILOT << "Couldn't save ["
				<< fP->fPilotComponentList[i]->objectName() << ']';
		}
	}
	*/

	killDaemonIfNeeded();
	kapp->quit();
}

void KPilotInstaller::slotResetLink()
{
	FUNCTIONSETUP;
	log( i18n("Resetting device connection ...") );
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
		WARNINGKPILOT << "Cannot allocate KPilotOptions object";
		daemon.requestSync(rememberedSync);
		return false;
	}

	int r = options->exec();

	if ( r && options->result() )
	{
		DEBUGKPILOT << "Updating settings.";

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
	/*
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
	log(i18n("Changed username to `%1'.",KPilotSettings::userName()));

	for (int i = 0; i<e; ++i)
	{
		PilotComponent *p = fP->fPilotComponentList[i];
		if (p)
		{
			p->showComponent();
		}
	}
	*/
}

void KPilotInstaller::componentChanged( KPageWidgetItem *current,
	KPageWidgetItem * before )
{
	FUNCTIONSETUP;
	if (!current)
	{
		WARNINGKPILOT << "Current NULL. nothing to do.";
		return;
	}
	
	DEBUGKPILOT << "Selected component [" << current->name() << ']';
	
	int e = fP->fPilotComponentList.size();
	for( int i = 0; i < e; ++i )
	{
		ComponentPageBase *p = fP->fPilotComponentList[i];
		if( current->widget() == p )
		{
			p->showPage();
		}
		else if( before && before->widget() == p  )
		{
			p->hidePage();
		}
	}
}

void KPilotInstaller::toggleVisibility()
{
	FUNCTIONSETUP;
	
	if( isVisible() )
	{
		fP->fLastWindowPos = pos();
		QWidget::setVisible( false );
	}
	else
	{
		QWidget::setVisible( true );
		QWidget::move( fP->fLastWindowPos );
		QWidget::raise();
	}
}

void KPilotInstaller::configure()
{
	FUNCTIONSETUP;

	if ( kpilotStatus()!=Normal || fP->fConfigureKPilotDialogInUse )
	{
		DEBUGKPILOT << "cannot configure. in use.";
		log(i18n("Cannot configure KPilot right now (KPilot's UI is already busy)."));
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

// Command line options descriptions.
//
//
//
//



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

	// a non-intuitive note about this... now that we're using dbus, the way that the path 
	// is constructed to communicate with our objects on the bus are of 2 pieces...
	// in the dbus interface string "org.kde.kpilot.foo",
	// the "org.kde.kpilot" part comes from setOrganizationDomain, found in
	// KPILOT_ABOUT_INIT.  the "foo" part comes in the first argument to KAboutData below...
	KAboutData about("kpilot", 0, ki18n("KPilot"),
		KPILOT_VERSION,
		ki18n("KPilot - HotSync software for KDE"),
		KAboutData::License_GPL,
		KPILOT_ABOUT_AUTHORS,
		ki18n(0L),
		"http://www.kpilot.org/"
		);
	KPILOT_ABOUT_INIT(about);

	KCmdLineArgs::init(argc, argv, &about);

	KCmdLineOptions kpilotoptions;
	kpilotoptions.add("s");
	kpilotoptions.add("setup", ki18n("Setup the Pilot device, conduits and other parameters"), 0L);
#ifdef DEBUG
	kpilotoptions.add("debug <level>", ki18n("Set debugging level"), "0");
#endif
	KCmdLineArgs::addCmdLineOptions(kpilotoptions, ki18n("kpilot"));
	KUniqueApplication::addCmdLineOptions();
	KCmdLineArgs *p = KCmdLineArgs::parsedArgs();

	KPilotConfig::getDebugLevel(p);


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
			<< KPilotSettings::configVersion();
		// Only force a reconfigure and continue if the
		// user is expecting normal startup. Otherwise,
		// do the configuration they're explicitly asking for.
		run_mode = KPilotConfig::interactiveUpdate();
		if (run_mode == KPilotConfig::Cancel) return 1;
	}


	if ( (run_mode == KPilotConfig::ConfigureKPilot) ||
		(run_mode == KPilotConfig::ConfigureAndContinue) )
	{
		DEBUGKPILOT << "Running setup first."
			<< " (mode " << run_mode << ')';
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
		WARNINGKPILOT << "Still not configured for use.";
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

	KGlobal::dirs()->addResourceType("pilotdbs", "data",
		CSL1("share/apps/kpilot/DBBackup"));
	tp->show();
	return a.exec();
}


