/* kpilot.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This is the main program in KPilot.
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


static const char *kpilot_id =
	"$Id$";


#include "options.h"

#include <qfile.h>
#include <qptrlist.h>
#include <qstring.h>
#include <qvbox.h>
#include <qtimer.h>

#include <kjanuswidget.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kstatusbar.h>
#include <kconfig.h>
#include <kwin.h>
#include <kcombobox.h>
#include <kmenubar.h>
#include <kstddirs.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kuniqueapp.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kcmultidialog.h>
#include <kprogress.h>


#include "kpilotConfigDialog.h"
#include "kpilotConfig.h"

#include "pilotComponent.h"

#include "addressWidget.h"
#include "memoWidget.h"
#include "fileInstallWidget.h"
#include "logWidget.h"
#include "dbviewerWidget.h"
#include "datebookWidget.h"
#include "todoWidget.h"

#include "conduitConfigDialog.h"

#ifndef _KPILOT_PILOTDAEMON_H
#include "pilotDaemonDCOP.h"
#endif

#ifndef __PILOTDAEMONDCOP_STUB__
#include "pilotDaemonDCOP_stub.h"
#endif

#include "kpilot.moc"

class KPilotInstaller::KPilotPrivate
{
public:
	typedef QPtrList<PilotComponent> ComponentList;

private:
	ComponentList  fPilotComponentList;

public:
	ComponentList &list() { return fPilotComponentList; } ;
} ;

KPilotInstaller::KPilotInstaller() :
	DCOPObject("KPilotIface"),
	KMainWindow(0),
	fDaemonStub(new PilotDaemonDCOP_stub("kpilotDaemon",
		"KPilotDaemonIface")),
	fP(new KPilotPrivate),
	fQuitAfterCopyComplete(false),
	fManagingWidget(0L),
	fKillDaemonOnExit(false),
	fDaemonWasRunning(true),
	fAppStatus(Startup),
	fFileInstallWidget(0L),
	fLogWidget(0L)
{
	FUNCTIONSETUP;

	readConfig();
	setupWidget();

#ifdef DEBUG
	PilotRecord::allocationInfo();
#endif
	fConfigureKPilotDialogInUse = false;

	/* NOTREACHED */
	(void) kpilot_id;
}

KPilotInstaller::~KPilotInstaller()
{
	FUNCTIONSETUP;
	killDaemonIfNeeded();
	delete fDaemonStub;
#ifdef DEBUG
	PilotRecord::allocationInfo();
#endif
}

void KPilotInstaller::killDaemonIfNeeded()
{
	FUNCTIONSETUP;
	if (fKillDaemonOnExit)
	{
		if (!fDaemonWasRunning)
		{
#ifdef DEBUG
			DEBUGKPILOT << fname << ": Killing daemon." << endl;
#endif

			getDaemon().quitNow();
		}
	}
}

void KPilotInstaller::startDaemonIfNeeded()
{
	FUNCTIONSETUP;

	fAppStatus=WaitingForDaemon;

	QString daemonError;
	QCString daemonDCOP;
	int daemonPID;

	QString s = getDaemon().statusString();

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Daemon status is " << s.latin1() << endl;
#endif

	if ((s.isEmpty()) || (!getDaemon().ok()))
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": Daemon not responding, trying to start it."
			<< endl;
#endif
		fLogWidget->addMessage(i18n("Starting the KPilot daemon ..."));
		fDaemonWasRunning = false;
	}
	else
	{
		fDaemonWasRunning = true;
	}

	if (!fDaemonWasRunning && KApplication::startServiceByDesktopName(
		CSL1("kpilotdaemon"),
		QString::null, &daemonError, &daemonDCOP, &daemonPID
#if (KDE_VERSION >= 220)
			// Startup notification was added in 2.2
			, "0"
#endif
		))
	{
		kdError() << k_funcinfo
			<< ": Can't start daemon : " << daemonError << endl;
		if (fLogWidget)
		{
			fLogWidget->addMessage(i18n("Could not start the "
				"KPilot daemon. The system error message "
				"was: &quot;%1&quot;").arg(daemonError));
		}
		fAppStatus=Error;
	}
	else
	{
#ifdef DEBUG
		s = getDaemon().statusString();
		DEBUGKPILOT << fname << ": Daemon status is " << s << endl;
#endif
		if (fLogWidget)
		{
			int wordoffset;
			s.remove(0,12);
			wordoffset=s.find(' ');
			if (wordoffset>0) s.remove(wordoffset,60);

			fLogWidget->addMessage(
				i18n("Daemon status is `%1'")
				.arg(s));
		}
		fAppStatus=Normal;
	}
}

void KPilotInstaller::readConfig()
{
	FUNCTIONSETUP;

	KPilotSettings::self()->readConfig();

	(void) PilotAppCategory::setupPilotCodec(KPilotSettings::encoding());
	if (fLogWidget)
	{
		fLogWidget->addMessage(i18n("Using character set %1 on "
			"the handheld.")
			.arg(PilotAppCategory::codecName()));
	}
}


void KPilotInstaller::setupWidget()
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Creating central widget." << endl;
#endif

	setCaption(CSL1("KPilot"));
	setMinimumSize(500, 405);


	fManagingWidget = new KJanusWidget(this,"mainWidget",
		KJanusWidget::IconList);
	fManagingWidget->setMinimumSize(500, 330);
	fManagingWidget->show();
	setCentralWidget(fManagingWidget);
	connect( fManagingWidget, SIGNAL( aboutToShowPage ( QWidget* ) ),
			this, SLOT( slotAboutToShowComponent( QWidget* ) ) );

	initIcons();
	initMenu();
	initComponents();

	createGUI(CSL1("kpilotui.rc"), false);
#ifdef DEBUG
	DEBUGKPILOT << fname
		<< ": Got XML from "
		<< xmlFile() << " and " << localXMLFile() << endl;
#endif
	setAutoSaveSettings();
}


void KPilotInstaller::initComponents()
{
	FUNCTIONSETUP;

	QString defaultDBPath = KPilotConfig::getDefaultDBPath();

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Creating component pages." << endl;
#endif

	QString pixfile;
	QWidget *w;

#define VIEWICON(a) KGlobal::dirs()->findResource("data",(a))

#define ADDICONPAGE(a,b) pixfile = VIEWICON(b); \
	w = getManagingWidget()->addVBoxPage(a,QString::null, \
		(pixfile.isEmpty() ? QPixmap() : QPixmap(pixfile))) ;

	ADDICONPAGE(i18n("HotSync"),CSL1("kpilot/kpilot-bhotsync.png"));
	fLogWidget = new LogWidget(w);
	addComponentPage(fLogWidget, i18n("HotSync"));
	fLogWidget->setShowTime(true);

	ADDICONPAGE(i18n("Todo Viewer"),CSL1("kpilot/kpilot-todo.png"));
	addComponentPage(new TodoWidget(w,defaultDBPath),
		i18n("Todo Viewer"));

	ADDICONPAGE(i18n("Address Viewer"),CSL1("kpilot/kpilot-address.png"));
	addComponentPage(new AddressWidget(w,defaultDBPath),
		i18n("Address Viewer"));

	ADDICONPAGE(i18n("Memo Viewer"),CSL1("kpilot/kpilot-knotes.png"));
	addComponentPage(new MemoWidget(w, defaultDBPath),
		i18n("Memo Viewer"));

	ADDICONPAGE(i18n("Generic DB Viewer"),CSL1("kpilot/kpilot-db.png"));
	addComponentPage(new GenericDBWidget(w,defaultDBPath),
		i18n("Generic DB Viewer"));

	ADDICONPAGE(i18n("File Installer"),CSL1("kpilot/kpilot-fileinstaller.png"));
	fFileInstallWidget = new FileInstallWidget(
		w,defaultDBPath);
	addComponentPage(fFileInstallWidget, i18n("File Installer"));

#undef ADDICONPAGE
#undef VIEWICON

	QTimer::singleShot(500,this,SLOT(initializeComponents()));
}



void KPilotInstaller::initIcons()
{
	FUNCTIONSETUP;

}



void KPilotInstaller::slotAboutToShowComponent( QWidget *c )
{
	FUNCTIONSETUP;
	int ix = fManagingWidget->pageIndex( c );
	PilotComponent*compToShow = fP->list().at(ix);
DEBUGKPILOT<<"Index: "<<ix<<", Widget="<<c<<", ComToShow="<<compToShow<<endl;
	for ( PilotComponent *comp = fP->list().first(); comp; comp = fP->list().next() )
	{
DEBUGKPILOT<<"comp="<<comp<<endl;
		// Load/Unload the data needed
		comp->showKPilotComponent( comp == compToShow );
	}
}

void KPilotInstaller::slotSelectComponent(PilotComponent * c)
{
	FUNCTIONSETUP;
	if (!c)
	{
		kdWarning() << k_funcinfo << ": Not a widget." << endl;
		return;
	}

	QObject *o = c->parent();
	if (!o)
	{
		kdWarning() << k_funcinfo << ": No parent." << endl;
		return;
	}

	QWidget *parent = dynamic_cast<QWidget *>(o);
	if (!parent)
	{
		kdWarning() << k_funcinfo << ": No widget parent." << endl;
		return;
	}

	int index = fManagingWidget->pageIndex(parent);

	if (index < 0)
	{
		kdWarning() << k_funcinfo << ": Index " << index << endl;
		return;
	}

	for ( PilotComponent *comp = fP->list().first(); comp; comp = fP->list().next() )
	{
		// Load/Unload the data needed
		comp->showKPilotComponent( comp == c );
	}
	fManagingWidget->showPage(index);
}




void KPilotInstaller::slotBackupRequested()
{
	FUNCTIONSETUP;
	setupSync(PilotDaemonDCOP::Backup,
		i18n("Backing up Pilot. ") +
		i18n("Please press the HotSync button."));
}

void KPilotInstaller::slotRestoreRequested()
{
	FUNCTIONSETUP;
	setupSync(PilotDaemonDCOP::Restore,
		i18n("Restoring Pilot. ") +
		i18n("Please press the HotSync button."));
}

void KPilotInstaller::slotHotSyncRequested()
{
	FUNCTIONSETUP;
	setupSync(PilotDaemonDCOP::HotSync,
		i18n("HotSyncing. ") +
		i18n("Please press the HotSync button."));
}

#if 0
void KPilotInstaller::slotFastSyncRequested()
{
	FUNCTIONSETUP;
	setupSync(PilotDaemonDCOP::FastSync,
		i18n("FastSyncing. ") +
		i18n("Please press the HotSync button."));
}
#endif

void KPilotInstaller::slotListSyncRequested()
{
	FUNCTIONSETUP;
	setupSync(PilotDaemonDCOP::Test,
		QString::fromLatin1("Listing Pilot databases."));
}

/* virtual DCOP */ ASYNC KPilotInstaller::daemonStatus(int i)
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGKPILOT << fname << ": Received daemon message " << i << endl;
#endif

	switch(i)
	{
	case KPilotDCOP::StartOfHotSync :
		if (fAppStatus==Normal)
		{
			fAppStatus=WaitingForDaemon;
			componentPreSync();
		}
		break;
	case KPilotDCOP::EndOfHotSync :
		if (fAppStatus==WaitingForDaemon)
		{
			componentPostSync();
			fAppStatus=Normal;
		}
		break;
	default :
		kdWarning() << k_funcinfo << ": Unhandled status message " << i << endl;
		break;
	}
}

/* virtual DCOP*/ int KPilotInstaller::kpilotStatus()
{
	return status();
}

/* virtual DCOP */ ASYNC KPilotInstaller::configure()
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGKPILOT << fname << ": Daemon requested configure" << endl;
#endif

	if (!fConfigureKPilotDialogInUse)
		slotConfigureKPilot();
}

bool KPilotInstaller::componentPreSync()
{
	FUNCTIONSETUP;

	QString reason;
	QString rprefix(i18n("Can't start a Sync now. %1"));

	for (fP->list().first();
		fP->list().current(); fP->list().next())
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": Pre-sync for builtin "
			<< fP->list().current()->name() << endl;
#endif
		if (!fP->list().current()->preHotSync(reason))
			break;
	}

	if (!reason.isNull())
	{
		KMessageBox::sorry(this,
			rprefix.arg(reason),
			i18n("Can't start Sync"));
		return false;
	}
	return true;
}

void KPilotInstaller::componentPostSync()
{
	FUNCTIONSETUP;

	for (fP->list().first();
		fP->list().current(); fP->list().next())
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": Post-sync for builtin "
			<< fP->list().current()->name() << endl;
#endif
		fP->list().current()->postHotSync();
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

	// File actions
	(void )new KAction(i18n("&HotSync"), CSL1("hotsync"), 0,
		this, SLOT(slotHotSyncRequested()),
		actionCollection(), "file_hotsync");
#if 0
	(void) new KAction(i18n("&FastSync"), CSL1("fastsync"), 0,
		this, SLOT(slotHotSyncRequested()),
		actionCollection(), "file_fastsync");
#endif
#ifdef DEBUG
	(void) new KAction(i18n("List only"),CSL1("list"),0,
		this,SLOT(slotListSyncRequested()),
		actionCollection(), "file_list");
#endif
	(void) new KAction(i18n("&Backup"), CSL1("backup"), 0,
		this, SLOT(slotBackupRequested()),
		actionCollection(), "file_backup");
	(void) new KAction(i18n("&Restore"), CSL1("restore"), 0,
		this, SLOT(slotRestoreRequested()),
		actionCollection(), "file_restore");
	(void) KStdAction::quit(this, SLOT(quit()), actionCollection());

	// View actions

	// Options actions
#if KDE_VERSION >= 0x30180
	createStandardStatusBarAction();
#endif

#if KDE_VERSION >= 0x30080
	setStandardToolBarMenuEnabled(true);
#else
	m_toolbarAction =
		KStdAction::showToolbar(this, SLOT(optionsShowToolbar()),
		actionCollection());
#endif

	(void) KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()),
		actionCollection());
	(void) KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()),
		actionCollection());
	(void) KStdAction::preferences(this, SLOT(slotConfigureKPilot()),
		actionCollection());
}

void KPilotInstaller::fileInstalled(int)
{
	FUNCTIONSETUP;
}

void KPilotInstaller::quit()
{
	FUNCTIONSETUP;

	for (fP->list().first();
		fP->list().current(); fP->list().next())
	{
		QString reason;
		if (!fP->list().current()->preHotSync(reason))
		{
			kdWarning() << k_funcinfo
				<< ": Couldn't save "
				<< fP->list().current()->name()
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

	if (!p)
	{
		kdWarning() << k_funcinfo
			<< ": Adding NULL component?" << endl;
		return;
	}

#ifdef DEBUG
	DEBUGKPILOT << fname
		<< ": Adding component @"
		<< (unsigned long) p << " called " << p->name("(none)") << endl;
#endif

	fP->list().append(p);

	// The first component added gets id 1, while the title
	// screen -- added elsewhere -- has id 0.
	//
	// fManagingWidget->addWidget(p, fP->list().count());


	const char *componentname = p->name("(none)");
	char *actionname = 0L;

	if (strncmp(componentname, "component_", 10) == 0)
	{
		actionname = new char[strlen(componentname) - 10 + 8];

		strcpy(actionname, "view_");
		strcat(actionname, componentname + 10);
	}
	else
	{
		actionname = new char[8 + strlen(componentname)];

		strcpy(actionname, "view_");
		strcat(actionname, componentname);
	}

#ifdef DEBUG
	DEBUGKPILOT << fname
		<< ": Using component action name "
		<< name << " for " << actionname << endl;
#endif

	KToggleAction *pt =
		new KToggleAction(name, /* "kpilot" -- component icon, */ 0,
		p, SLOT(slotShowComponent()),
		actionCollection(), actionname);

	pt->setExclusiveGroup(CSL1("view_menu"));

	connect(p, SIGNAL(showComponent(PilotComponent *)),
		this, SLOT(slotSelectComponent(PilotComponent *)));
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

#if KDE_VERSION >= 0x30080
// Included in kdelibs in KDE 3.1, but we can't #ifdef slots,
// so include a dummy implementation.
void KPilotInstaller::optionsShowToolbar()
{
}
#else
void KPilotInstaller::optionsShowToolbar()
{
	FUNCTIONSETUP;
	if (m_toolbarAction->isChecked())
	{
		toolBar()->show();
	}
	else
	{
		toolBar()->hide();
	}

	kapp->processEvents();
	resizeEvent(0);
}
#endif

void KPilotInstaller::optionsConfigureKeys()
{
	FUNCTIONSETUP;
	KKeyDialog::configure( actionCollection() );
}

void KPilotInstaller::optionsConfigureToolbars()
{
	FUNCTIONSETUP;
	// use the standard toolbar editor
#if KDE_VERSION >= 0x030100
	// This was added in KDE 3.1
	saveMainWindowSettings( KGlobal::config(), autoSaveGroup() );
#endif
	KEditToolbar dlg(actionCollection());
	connect(&dlg, SIGNAL(newToolbarConfig()), this, SLOT(slotNewToolbarConfig()));
	dlg.exec();
}


void KPilotInstaller::slotNewToolbarConfig()
{
	FUNCTIONSETUP;
	// recreate our GUI
	createGUI();
#if KDE_VERSION >= 0x030100
	applyMainWindowSettings( KGlobal::config(), autoSaveGroup() );
#endif
}

/*
** Can't be a member function because it needs to be called even with no KPilotInstaller.
*/
static bool runConfigure(PilotDaemonDCOP_stub &daemon,QWidget *parent)
{
	FUNCTIONSETUP;
	bool ret = false;

	// Display the (modal) options page.
	//
	//
	int rememberedSync = daemon.nextSyncType();
	daemon.requestSync(0);

	KPilotSettings::self()->readConfig();

	KCMultiDialog *options = new KCMultiDialog( KDialogBase::Plain, i18n("Configuration"), parent, "KPilotPreferences", true );
	options->addModule( "kpilot_config.desktop" );

	if (!options)
	{
		kdError() << k_funcinfo
			<< ": Can't allocate KPilotOptions object" << endl;
		daemon.requestSync(rememberedSync);
		return false;
	}

	int r = options->exec();

	if ( r && options->result() )
	{
#ifdef DEBUG
		DEBUGKPILOT << fname << ": Updating link." << endl;
#endif

		// The settings are changed in the external module!!!
		KPilotSettings::self()->config()->sync();
		KPilotSettings::self()->readConfig();

		// Update the daemon to reflect new settings.
		//
		//
		daemon.reloadSettings();
		ret = true;
	}

	KPILOT_DELETE(options);
	daemon.requestSync(rememberedSync);

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Done with options." << endl;
#endif

	return ret;
}

void KPilotInstaller::slotConfigureKPilot()
{
	FUNCTIONSETUP;

	if (fAppStatus!=Normal)
	{
		if (fLogWidget)
		{
			fLogWidget->addMessage(i18n("Cannot configure KPilot right now."));
		}
		return;
	}
	fAppStatus=UIBusy;
	fConfigureKPilotDialogInUse = true;
	if (runConfigure(getDaemon(),this))
	{
		// Update each installed component.
		for (fP->list().first();
			fP->list().current();
			fP->list().next())
		{
			// TODO_RK: update the current component to use the new settings
//			fP->list().current()->initialize();
		}
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
		return ::kpilot_id;
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
	{"c", 0, 0},
	{"conduit-setup", I18N_NOOP("Deprecated, use \"setup\" instead!"), 0L},
#ifdef DEBUG
	{"debug <level>", I18N_NOOP("Set debugging level"), "0"},
#endif
	KCmdLineLastOption
};




// "Regular" mode == 0
// setup mode == 's'
// conduit setup == 'c'
//
// This is only changed by the --setup flag --
// kpilot still does a setup the first time it is run.
//
//
enum { Normal, ConfigureKPilot, ConfigureConduits, ConfigureAndContinue } run_mode = Normal;


int main(int argc, char **argv)
{
	FUNCTIONSETUP;

	KAboutData about("kpilot", I18N_NOOP("KPilot"),
		KPILOT_VERSION,
		"KPilot - HotSync software for KDE\n\n",
		KAboutData::License_GPL,
		"(c) 1998-2000,2001, Dan Pilone (c) 2000-2004, Adriaan de Groot",
		0L,
		"http://www.slac.com/~pilone/kpilot_home/"
		);
	about.addAuthor("Dan Pilone",
		I18N_NOOP("Project Leader"),
		"pilone@slac.com" );
	about.addAuthor("Adriaan de Groot",
		I18N_NOOP("Maintainer"),
		"groot@kde.org", "http://www.cs.kun.nl/~adridg/");
	about.addAuthor("Reinhold Kainhofer",
		I18N_NOOP("Core and conduits developer"), "reinhold@kainhofer.com", "http://reinhold.kainhofer.com/Linux/");
	about.addCredit("Preston Brown", I18N_NOOP("VCal conduit"));
	about.addCredit("Greg Stern", I18N_NOOP("Abbrowser conduit"));
	about.addCredit("Chris Molnar", I18N_NOOP("Expenses conduit"));
	about.addCredit("Heiko Purnhagen", I18N_NOOP("Bugfixer"));
	about.addCredit("Joerg Habenicht", I18N_NOOP("Bugfixer"));
	about.addCredit("Martin Junius",
		I18N_NOOP("XML GUI"),
		"mj@m-j-s.net", "http://www.m-j-s.net/kde/");
	about.addCredit("David Bishop",
		I18N_NOOP(".ui files"));
	about.addCredit("Aaron J. Seigo",
		I18N_NOOP("Bugfixer, coolness"));


	KCmdLineArgs::init(argc, argv, &about);
	KCmdLineArgs::addCmdLineOptions(kpilotoptions, "kpilot");
	KUniqueApplication::addCmdLineOptions();
	KCmdLineArgs *p = KCmdLineArgs::parsedArgs();

#ifdef DEBUG
	KPilotConfig::getDebugLevel(p);
#endif

	if (p->isSet("setup") || p->isSet("conduit-setup"))
	{
		run_mode = ConfigureKPilot;
	}

	if (!KUniqueApplication::start())
	{
		return 0;
	}
	KUniqueApplication a(true, true);

	if (KPilotSettings::configVersion() < KPilotConfig::ConfigurationVersion)
	{
		kdWarning() << ": KPilot configuration version "
			<< KPilotConfig::ConfigurationVersion
			<< " newer than stored version "
			<< KPilotSettings::configVersion() << endl;
		// Only force a reconfigure and continue if the
		// user is expecting normal startup. Otherwise,
		// do the configuration they're explicitly asking for.
		if (Normal==run_mode) run_mode = ConfigureAndContinue;
	}

	if ((run_mode == ConfigureKPilot) || (run_mode == ConfigureAndContinue))
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": Running setup first."
			<< " (mode " << run_mode << ")" << endl;
#endif
		bool outdated = false;
		if (KPilotSettings::configVersion() < KPilotConfig::ConfigurationVersion)
		{
			outdated = true;
			KPilotConfig::interactiveUpdate();
		}
		PilotDaemonDCOP_stub *daemon = new PilotDaemonDCOP_stub("kpilotDaemon","KPilotDaemonIface");
		bool r = runConfigure(*daemon,0L);
		delete daemon;
		if (!r) return 1;
		// User expected configure only.
		if (run_mode == ConfigureKPilot)
		{
			return 0;
		}
	}

	if (KPilotSettings::configVersion() < KPilotConfig::ConfigurationVersion)
	{
		kdWarning() << k_funcinfo <<
			": Is still not configured for use." << endl;
		KPilotConfig::sorryVersionOutdated( KPilotSettings::configVersion());
		return 1;
	}


	KPilotInstaller *tp = new KPilotInstaller();

	if (tp->status() == KPilotInstaller::Error)
	{
		delete tp;

		tp = 0;
		return 1;
	}

	tp->startDaemonIfNeeded();

	KGlobal::dirs()->addResourceType("pilotdbs",
		CSL1("share/apps/kpilot/DBBackup"));
	tp->show();
	a.setMainWidget(tp);
	return a.exec();
}


