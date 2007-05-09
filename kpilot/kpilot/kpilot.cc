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

#include <q3ptrlist.h>

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
} ;

KPilotInstaller::KPilotInstaller() :
	KXmlGuiWindow(0),
	fP(new KPilotPrivate),
	fQuitAfterCopyComplete(false),
	fManagingWidget(0L),
	fDaemonWasRunning(true),
	fAppStatus(Startup),
	fFileInstallWidget(0L),
	fLogWidget(0L)
{
	new KpilotAdaptor(this);
	QDBusConnection::sessionBus().registerObject("/KPilot", this);
	//TODO verify it
	fDaemonInterface = new OrgKdeKpilotDaemonInterface("org.kde.kpilot.daemon", "/Daemon", QDBusConnection::sessionBus());
	FUNCTIONSETUP;

	readConfig();
	setupWidget();

	PilotRecord::allocationInfo();
	fConfigureKPilotDialogInUse = false;
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
		if (!fDaemonWasRunning)
		{
			DEBUGKPILOT << fname << ": Killing daemon." << endl;
			getDaemon().quitNow();
		}
	}
}

void KPilotInstaller::startDaemonIfNeeded()
{
	FUNCTIONSETUP;

	fAppStatus=WaitingForDaemon;

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
		if (fLogWidget)
		{
			fLogWidget->addMessage(i18n("Starting the KPilot daemon ..."));
		}
		fDaemonWasRunning = false;
	}
	else
	{
		fDaemonWasRunning = true;
	}
	if (!fDaemonWasRunning &&  KToolInvocation::startServiceByDesktopName( CSL1("kpilotdaemon"), QStringList(), &daemonError, &daemonPID ) )
	{
		WARNINGKPILOT << ": Can't start daemon : " << daemonError << endl;
		if (fLogWidget)
		{
			fLogWidget->addMessage(i18n("Could not start the "
				"KPilot daemon. The system error message "
				"was: &quot;%1&quot;",daemonError));
		}
		fAppStatus=Error;
	}
	else
	{
		DEBUGKPILOT << fname << ": Daemon status is " << s << endl;
		if (fLogWidget)
		{
			int wordoffset;
			s.remove(0,12);
			wordoffset=s.indexOf(';');
			if (wordoffset>0)
			{
				s.truncate(wordoffset);
			}

			fLogWidget->addMessage(
				i18n("Daemon status is `%1'",s.isEmpty() ? i18n("not running") : s ));
		}
		fAppStatus=Normal;
	}
}

void KPilotInstaller::readConfig()
{
	FUNCTIONSETUP;

	KPilotSettings::self()->readConfig();

	(void) Pilot::setupPilotCodec(KPilotSettings::encoding());

	if (fLogWidget)
	{
		fLogWidget->addMessage(i18n("Using character set %1 on "
			"the handheld.",Pilot::codecName()));
	}
}


void KPilotInstaller::setupWidget()
{
	FUNCTIONSETUP;

#ifdef __GNUC__
#warning "kde4 port it"
#endif
	setCaption(CSL1("KPilot"));
	setMinimumSize(500, 405);

#if 0
	fManagingWidget = new KPageView(this);
	fManagingWidget->setFaceType(KPageView::List);
	fManagingWidget->setMinimumSize(fManagingWidget->sizeHint());
	fManagingWidget->show();
	setCentralWidget(fManagingWidget);
	connect( fManagingWidget, SIGNAL( currentPageChanged( const QModelIndex &, const QModelIndex &)),
			this, SLOT( slotAboutToShowComponent(const QModelIndex &, const QModelIndex & ) ) );
#endif
	initIcons();
	initMenu();
	initComponents();

	setMinimumSize(sizeHint() + QSize(10,60));

	createGUI(CSL1("kpilotui.rc"));
	setAutoSaveSettings();
}

void KPilotInstaller::initComponents()
{
	FUNCTIONSETUP;
#if 0
	QString defaultDBPath = KPilotConfig::getDefaultDBPath();

	QPixmap pixmap;
	QString pixfile;
	QWidget *w;

#define ADDICONPAGE(a,b) \
	pixmap = KIconLoader::global()->loadIcon(b, K3Icon::Desktop, 64); \
	w = getManagingWidget()->addVBoxPage(a,QString::null, pixmap) ;

	ADDICONPAGE(i18n("HotSync"),CSL1("kpilotbhotsync"));
	fLogWidget = new LogWidget(w);
	addComponentPage(fLogWidget, i18n("HotSync"));
	fLogWidget->setShowTime(true);

	ADDICONPAGE(i18n("To-do Viewer"),CSL1("kpilottodo"));
	addComponentPage(new TodoWidget(w,defaultDBPath),
		i18n("To-do Viewer"));

	ADDICONPAGE(i18n("Address Viewer"),CSL1("kpilotaddress"));
	addComponentPage(new AddressWidget(w,defaultDBPath),
		i18n("Address Viewer"));

	ADDICONPAGE(i18n("Memo Viewer"),CSL1("kpilotknotes"));
	addComponentPage(new MemoWidget(w, defaultDBPath),
		i18n("Memo Viewer"));

	ADDICONPAGE(i18n("File Installer"),CSL1("kpilotfileinstaller"));
	fFileInstallWidget = new FileInstallWidget(
		w,defaultDBPath);
	addComponentPage(fFileInstallWidget, i18n("File Installer"));

	ADDICONPAGE(i18n("Generic DB Viewer"),CSL1("kpilotdb"));
	addComponentPage(new GenericDBWidget(w,defaultDBPath),
		i18n("Generic DB Viewer"));

#undef ADDICONPAGE
#endif
	QTimer::singleShot(500,this,SLOT(initializeComponents()));
}



void KPilotInstaller::initIcons()
{
	FUNCTIONSETUP;

}



void KPilotInstaller::slotAboutToShowComponent( const QModelIndex&, const QModelIndex& )
{
	FUNCTIONSETUP;
#ifdef __GNUC__
#warning "kde4 port it"
#endif
#if 0
	int ix = fManagingWidget->pageIndex( c );
	PilotComponent*compToShow = fP->list().at(ix);
	for ( PilotComponent *comp = fP->list().first(); comp; comp = fP->list().next() )
	{
		// Load/Unload the data needed
		comp->showKPilotComponent( comp == compToShow );
	}
#endif
}

void KPilotInstaller::slotSelectComponent(PilotComponent *c)
{
	FUNCTIONSETUP;
#ifdef __GNUC__
#warning "kde4 port it"
#endif
#if 0
	if (!c)
	{
		WARNINGKPILOT << "Not a widget." << endl;
		return;
	}

	QObject *o = c->parent();
	if (!o)
	{
		WARNINGKPILOT << "Widget has no parent." << endl;
		return;
	}

	QWidget *parent = dynamic_cast<QWidget *>(o);
	if (!parent)
	{
		WARNINGKPILOT << "Widget's parent is not a widget." << endl;
		return;
	}

	int index = fManagingWidget->pageIndex(parent);

	if (index < 0)
	{
		WARNINGKPILOT << "Bogus index " << index << endl;
		return;
	}

	for ( PilotComponent *comp = fP->list().first(); comp; comp = fP->list().next() )
	{
		// Load/Unload the data needed
		comp->showKPilotComponent( comp == c );
	}
	fManagingWidget->showPage(index);
#endif
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
		if (fAppStatus==Normal)
		{
			fAppStatus=WaitingForDaemon;
			componentPreSync();
		}
		break;
	case KPilotInstaller::EndOfHotSync :
		if (fAppStatus==WaitingForDaemon)
		{
			componentPostSync();
			fAppStatus=Normal;
		}
		break;
	case KPilotInstaller::DaemonQuit :
		if (fLogWidget)
		{
			fLogWidget->logMessage(i18n("The daemon has exited."));
			fLogWidget->logMessage(i18n("No further HotSyncs are possible."));
			fLogWidget->logMessage(i18n("Restart the daemon to HotSync again."));
		}
		fAppStatus=WaitingForDaemon;
		break;
	case KPilotInstaller::None :
		WARNINGKPILOT << "Unhandled status message " << i << endl;
		break;
	}
}

int KPilotInstaller::kpilotStatus()
{
	return status();
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
		QString m(message);
		if (fLogWidget)
		{
			fLogWidget->logMessage(m);
		}
	}
	getDaemon().requestSync(kind);
}

void KPilotInstaller::closeEvent(QCloseEvent * e)
{
	FUNCTIONSETUP;

	quit();
	e->accept();
}

void KPilotInstaller::initMenu()
{
	FUNCTIONSETUP;
	QAction *a;

	KActionMenu *syncPopup;

	syncPopup = new KActionMenu(KIcon(CSL1("kpilot")),i18n("HotSync"),
		actionCollection());
	actionCollection()->addAction("popup_hotsync", syncPopup);

	syncPopup->setToolTip(i18n("Select the kind of HotSync to perform next."));
	syncPopup->setWhatsThis(i18n("Select the kind of HotSync to perform next. "
		"This applies only to the next HotSync; to change the default, use "
		"the configuration dialog."));
	connect(syncPopup, SIGNAL(activated()),
		this, SLOT(slotHotSyncRequested()));

        a = actionCollection()->addAction( "file_hotsync");
    	a->setText(i18n("&HotSync"));
	a->setIcon(KIcon(CSL1("hotsync")));
        a->setToolTip(i18n("Next HotSync will be normal HotSync."));
        a->setWhatsThis(i18n("Tell the daemon that the next HotSync "
                "should be a normal HotSync."));
	connect(a, SIGNAL(triggered()), this, SLOT(slotHotSyncRequested()));

        a = actionCollection()->addAction( "file_fullsync");
        a->setText(i18n("Full&Sync"));
        a->setIcon(KIcon(CSL1("fullsync")));
        a->setToolTip(i18n("Next HotSync will be a FullSync."));
        a->setWhatsThis(i18n("Tell the daemon that the next HotSync "
                "should be a FullSync (check data on both sides)."));
        connect(a, SIGNAL(triggered()), this, SLOT(slotFullSyncRequested()));

        a = actionCollection()->addAction( "file_backup");
        a->setText(i18n("&Backup"));
        a->setIcon(KIcon(CSL1("backup")));
        a->setToolTip(i18n("Next HotSync will be backup."));
	a->setWhatsThis(i18n("Tell the daemon that the next HotSync "
                "should back up the Handheld to the PC."));
        connect(a, SIGNAL(triggered()), this, SLOT(slotBackupRequested()));

        a = actionCollection()->addAction( "file_restore");
        a->setText(i18n("&Restore"));
        a->setIcon(KIcon(CSL1("restore")));
        a->setToolTip(i18n("Next HotSync will be restore."));
        a->setWhatsThis(i18n("Tell the daemon that the next HotSync "
                "should restore the Handheld from data on the PC."));
        connect(a, SIGNAL(triggered()), this, SLOT(slotRestoreRequested()));

        a = actionCollection()->addAction( "file_HHtoPC");
        a->setText(i18n("Copy Handheld to PC"));
        a->setToolTip(i18n("Next HotSync will be backup."));
        a->setWhatsThis(i18n("Tell the daemon that the next HotSync "
                "should copy all data from the Handheld to the PC, "
                "overwriting entries on the PC."));
        connect(a, SIGNAL(triggered()), this, SLOT(slotPCtoHHRequested()));

        a = actionCollection()->addAction( "file_reload");
        a->setText(i18n("Rese&t Link"));
	a->setIcon(KIcon(CSL1("reload")));
        a->setToolTip(i18n("Reset the device connection."));
        a->setWhatsThis(i18n("Try to reset the daemon and its connection "
                "to the Handheld."));
        connect(a, SIGNAL(triggered()), this, SLOT(slotResetLink()));

        a = KStandardAction::quit(this, SLOT(quit()), actionCollection());
        a->setWhatsThis(i18n("Quit KPilot, (and stop the daemon "
                "if configured that way)."));

        // Options actions
        createStandardStatusBarAction();
        setStandardToolBarMenuEnabled(true);

        (void) KStandardAction::keyBindings(this, SLOT(optionsConfigureKeys()),
                actionCollection());
        (void) KStandardAction::configureToolbars(this, SLOT(optionsConfigureToolbars()),
                actionCollection());
        (void) KStandardAction::preferences(this, SLOT(configure()),
                actionCollection());
}

void KPilotInstaller::fileInstalled(int)
{
	FUNCTIONSETUP;
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
				<< fP->fPilotComponentList[i]->name()
				<< endl;
		}
	}

	killDaemonIfNeeded();
	kapp->quit();
}

void KPilotInstaller::addComponentPage(PilotComponent * p,
	const QString & name)
{
	FUNCTIONSETUP;
#ifdef __GNUC__
#warning "kde4 port it"
#endif
#if 0
	if (!p)
	{
		WARNINGKPILOT << "Adding NULL component?" << endl;
		return;
	}

	fP->list().append(p);

	// The first component added gets id 1, while the title
	// screen -- added elsewhere -- has id 0.
	//
	// fManagingWidget->addWidget(p, fP->list().count());


	const char *componentname = p->name("(none)");
	char *actionname = 0L;
	int actionnameLength = 0;

	if (strncmp(componentname, "component_", 10) == 0)
	{
		actionnameLength = strlen(componentname) - 10 + 8;
		actionname = new char[actionnameLength];

		strlcpy(actionname, "view_", actionnameLength);
		strlcat(actionname, componentname + 10, actionnameLength);
	}
	else
	{
		actionnameLength = strlen(componentname) + 8;
		actionname = new char[actionnameLength];

		strlcpy(actionname, "view_", actionnameLength);
		strlcat(actionname, componentname, actionnameLength);
	}

	KToggleAction *pt =
		new KToggleAction(name, /* "kpilot" -- component icon, */ 0,
		p, SLOT(slotShowComponent()),
		actionCollection(), actionname);

	pt->setExclusiveGroup(CSL1("view_menu"));

	connect(p, SIGNAL(showComponent(PilotComponent *)),
		this, SLOT(slotSelectComponent(PilotComponent *)));
#endif
}

/* slot */ void KPilotInstaller::initializeComponents()
{
	FUNCTIONSETUP;

/*	for (PilotComponent *p = fP->list().first();
		p ; p = fP->list().next())
	{
		p->initialize();
	}*/
}


void KPilotInstaller::optionsConfigureKeys()
{
	FUNCTIONSETUP;
	KShortcutsDialog::configure( actionCollection() );
}

void KPilotInstaller::optionsConfigureToolbars()
{
	FUNCTIONSETUP;
	// use the standard toolbar editor
	// This was added in KDE 3.1
	saveMainWindowSettings( KConfigGroup(KGlobal::config(), autoSaveGroup()) );
	KEditToolBar dlg(actionCollection());
	connect(&dlg, SIGNAL(newToolbarConfig()), this, SLOT(slotNewToolbarConfig()));
	dlg.exec();
}


void KPilotInstaller::slotNewToolbarConfig()
{
	FUNCTIONSETUP;
	// recreate our GUI
	createGUI();
	applyMainWindowSettings( KConfigGroup(KGlobal::config(), autoSaveGroup()) );
}

void KPilotInstaller::slotResetLink()
{
	FUNCTIONSETUP;
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
#ifdef __GNUC__
#warning "kde4 port it"
#endif
#if 0
	QString defaultDBPath = KPilotConfig::getDefaultDBPath();
	bool dbPathChanged = false;

	for (fP->list().first();
		fP->list().current();
		fP->list().next())
	{
// TODO_RK: update the current component to use the new settings
//			fP->list().current()->initialize();
		PilotComponent *p = fP->list().current();
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
	if (fLogWidget)
	{
		fLogWidget->logMessage(i18n("Changed username to `%1'.",PilotSettings::userName()));
		fManagingWidget->showPage(0);
		slotAboutToShowComponent(fLogWidget);
	}
	else
	{
		int ix = fManagingWidget->activePageIndex();
		PilotComponent *component = 0L;
		if (ix>=0)
		{
			component = fP->list().at(ix);
		}
		if (component)
		{
			component->hideComponent(); // Throw away current data
			component->showComponent(); // Reload
		}
	}
#endif
}

void KPilotInstaller::configure()
{
	FUNCTIONSETUP;

	if ( fAppStatus!=Normal || fConfigureKPilotDialogInUse )
	{
		if (fLogWidget)
		{
			fLogWidget->addMessage(i18n("Cannot configure KPilot right now (KPilot's UI is already busy)."));
		}
		return;
	}
	fAppStatus=UIBusy;
	fConfigureKPilotDialogInUse = true;
	if (runConfigure(getDaemon(),this))
	{
		componentUpdate();
	}

	fConfigureKPilotDialogInUse = false;
	fAppStatus=Normal;
}


/* static */ const char *KPilotInstaller::version(int kind)
{
	FUNCTIONSETUP;
	// I don't think the program title needs to be translated. (ADE)
	//
	//
	if (kind)
	{
		return "kpilot.cc";
	}
	else
	{
		return "KPilot v" KPILOT_VERSION;
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

	if (tp->status() == KPilotInstaller::Error)
	{
		KPILOT_DELETE(tp);
		return 1;
	}

	QTimer::singleShot(0,tp,SLOT(startDaemonIfNeeded()));

	KGlobal::dirs()->addResourceType("pilotdbs",
		CSL1("share/apps/kpilot/DBBackup"));
	tp->show();
	a.setMainWidget(tp);
	return a.exec();
}


