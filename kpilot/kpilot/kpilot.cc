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
#include <kstandarddirs.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kaction.h>
#include <kactionclasses.h>
#include <kstdaction.h>
#include <kuniqueapplication.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kcmultidialog.h>
#include <kprogress.h>
#include <klibloader.h>


#include "kpilotConfigDialog.h"
#include "kpilotConfig.h"
#include "kpilotConfigWizard.h"

#include "pilotComponent.h"
#include "pilotDatabase.h"

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
	(void) PilotDatabase::count();
#endif
}

void KPilotInstaller::killDaemonIfNeeded()
{
	FUNCTIONSETUP;
	if (KPilotSettings::killDaemonAtExit())
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
	DEBUGKPILOT << fname << ": Daemon status is "
		<< ( s.isEmpty() ? CSL1("<none>") : s ) << endl;
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
			, "0" /* no notify */
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
			wordoffset=s.find(';');
			if (wordoffset>0) s.truncate(wordoffset);

			fLogWidget->addMessage(
				i18n("Daemon status is `%1'")
				.arg(s.isEmpty() ? i18n("not running") : s ));
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
	fManagingWidget->setMinimumSize(fManagingWidget->sizeHint());
	fManagingWidget->show();
	setCentralWidget(fManagingWidget);
	connect( fManagingWidget, SIGNAL( aboutToShowPage ( QWidget* ) ),
			this, SLOT( slotAboutToShowComponent( QWidget* ) ) );

	initIcons();
	initMenu();
	initComponents();

	setMinimumSize(sizeHint() + QSize(10,60));

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

	ADDICONPAGE(i18n("To-do Viewer"),CSL1("kpilot/kpilot-todo.png"));
	addComponentPage(new TodoWidget(w,defaultDBPath),
		i18n("To-do Viewer"));

	ADDICONPAGE(i18n("Address Viewer"),CSL1("kpilot/kpilot-address.png"));
	addComponentPage(new AddressWidget(w,defaultDBPath),
		i18n("Address Viewer"));

	ADDICONPAGE(i18n("Memo Viewer"),CSL1("kpilot/kpilot-knotes.png"));
	addComponentPage(new MemoWidget(w, defaultDBPath),
		i18n("Memo Viewer"));

	ADDICONPAGE(i18n("File Installer"),CSL1("kpilot/kpilot-fileinstaller.png"));
	fFileInstallWidget = new FileInstallWidget(
		w,defaultDBPath);
	addComponentPage(fFileInstallWidget, i18n("File Installer"));

	ADDICONPAGE(i18n("Generic DB Viewer"),CSL1("kpilot/kpilot-db.png"));
	addComponentPage(new GenericDBWidget(w,defaultDBPath),
		i18n("Generic DB Viewer"));

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
#ifdef DEBUG
	DEBUGKPILOT << fname
		<< ": Index: " << ix
		<< ", Widget=" << c
		<< ", ComToShow=" << compToShow << endl;
#endif
	for ( PilotComponent *comp = fP->list().first(); comp; comp = fP->list().next() )
	{
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
	setupSync(SyncAction::eBackup,
		i18n("Next sync will be a backup. ") +
		i18n("Please press the HotSync button."));
}

void KPilotInstaller::slotRestoreRequested()
{
	FUNCTIONSETUP;
	setupSync(SyncAction::eRestore,
		i18n("Next sync will restore the Pilot from backup. ") +
		i18n("Please press the HotSync button."));
}

void KPilotInstaller::slotHotSyncRequested()
{
	FUNCTIONSETUP;
	setupSync(SyncAction::eHotSync,
		i18n("Next sync will be a regular HotSync. ") +
		i18n("Please press the HotSync button."));
}

void KPilotInstaller::slotFastSyncRequested()
{
	FUNCTIONSETUP;
	setupSync(SyncAction::eFastSync,
		i18n("Next sync will be a Fast Sync. ") +
		i18n("Please press the HotSync button."));
}

void KPilotInstaller::slotFullSyncRequested()
{
	FUNCTIONSETUP;
	setupSync(SyncAction::eFullSync,
		i18n("Next sync will be a Full Sync. ") +
		i18n("Please press the HotSync button."));
}

void KPilotInstaller::slotHHtoPCRequested()
{
	FUNCTIONSETUP;
	setupSync(SyncAction::eCopyHHToPC,
		i18n("Next sync will copy Handheld data to PC. ") +
		i18n("Please press the HotSync button."));
}

void KPilotInstaller::slotPCtoHHRequested()
{
	FUNCTIONSETUP;
	setupSync(SyncAction::eCopyPCToHH,
		i18n("Next sync will copy PC data to Handheld. ") +
		i18n("Please press the HotSync button."));
}

void KPilotInstaller::slotTestSyncRequested()
{
	FUNCTIONSETUP;
	setupSync(SyncAction::eTest,
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
	case KPilotDCOP::DaemonQuit :
		if (fLogWidget)
		{
			fLogWidget->logMessage(i18n("The daemon has exited."));
			fLogWidget->logMessage(i18n("No further HotSyncs are possible."));
			fLogWidget->logMessage(i18n("Restart the daemon to HotSync again."));
		}
		fAppStatus=WaitingForDaemon;
		break;
	case KPilotDCOP::None :
		kdWarning() << k_funcinfo << ": Unhandled status message " << i << endl;
		break;
	}
}

/* virtual DCOP*/ int KPilotInstaller::kpilotStatus()
{
	return status();
}

bool KPilotInstaller::componentPreSync()
{
	FUNCTIONSETUP;

	QString reason;
	QString rprefix(i18n("Cannot start a Sync now. %1"));

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
			i18n("Cannot start Sync"));
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

	KAction *a;

	KActionMenu *syncPopup;

	syncPopup = new KActionMenu(i18n("HotSync"), CSL1("kpilot"),
		actionCollection(), "popup_hotsync");
	syncPopup->setToolTip(i18n("Select the kind of HotSync to perform next."));
	syncPopup->setWhatsThis(i18n("Select the kind of HotSync to perform next. "
		"This applies only to the next HotSync; to change the default, use "
		"the configuration dialog."));
	connect(syncPopup, SIGNAL(activated()),
		this, SLOT(slotHotSyncRequested()));

	// File actions, keep this list synced with kpilotui.rc and pilotDaemon.cc
	a = new KAction(i18n("&HotSync"), CSL1("hotsync"), 0,
		this, SLOT(slotHotSyncRequested()),
		actionCollection(), "file_hotsync");
	a->setToolTip(i18n("Next HotSync will be normal HotSync."));
	a->setWhatsThis(i18n("Tell the daemon that the next HotSync "
		"should be a normal HotSync."));
	syncPopup->insert(a);

	a = new KAction(i18n("&FastSync"), CSL1("fastsync"), 0,
		this, SLOT(slotFastSyncRequested()),
		actionCollection(), "file_fastsync");
	a->setToolTip(i18n("Next HotSync will be a FastSync."));
	a->setWhatsThis(i18n("Tell the daemon that the next HotSync "
		"should be a FastSync (run conduits only)."));
	syncPopup->insert(a);

	a = new KAction(i18n("Full&Sync"), CSL1("fullsync"), 0,
		this, SLOT(slotFullSyncRequested()),
		actionCollection(), "file_fullsync");
	a->setToolTip(i18n("Next HotSync will be a FullSync."));
	a->setWhatsThis(i18n("Tell the daemon that the next HotSync "
		"should be a FullSync (check data on both sides)."));
	syncPopup->insert(a);

	a = new KAction(i18n("&Backup"), CSL1("backup"), 0,
		this, SLOT(slotBackupRequested()),
		actionCollection(), "file_backup");
	a->setToolTip(i18n("Next HotSync will be backup."));
	a->setWhatsThis(i18n("Tell the daemon that the next HotSync "
		"should back up the Handheld to the PC."));
	syncPopup->insert(a);

	a = new KAction(i18n("&Restore"), CSL1("restore"), 0,
		this, SLOT(slotRestoreRequested()),
		actionCollection(), "file_restore");
	a->setToolTip(i18n("Next HotSync will be restore."));
	a->setWhatsThis(i18n("Tell the daemon that the next HotSync "
		"should restore the Handheld from data on the PC."));
	syncPopup->insert(a);

	a = new KAction(i18n("Copy Handheld to PC"), QString::null, 0,
		this, SLOT(slotHHtoPCRequested()),
		actionCollection(), "file_HHtoPC");
	a->setToolTip(i18n("Next HotSync will be backup."));
	a->setWhatsThis(i18n("Tell the daemon that the next HotSync "
		"should copy all data from the Handheld to the PC, "
		"overwriting entries on the PC."));
	syncPopup->insert(a);

	a = new KAction(i18n("Copy PC to Handheld"), QString::null, 0,
		this, SLOT(slotPCtoHHRequested()),
		actionCollection(), "file_PCtoHH");
	a->setToolTip(i18n("Next HotSync will copy PC to Handheld."));
	a->setWhatsThis(i18n("Tell the daemon that the next HotSync "
		"should copy all data from the PC to the Handheld, "
		"overwriting entries on the Handheld."));
	syncPopup->insert(a);


#ifdef DEBUG
	a = new KAction(i18n("&List Only"),CSL1("listsync"),0,
		this,SLOT(slotTestSyncRequested()),
		actionCollection(), "file_list");
	a->setToolTip(i18n("Next HotSync will list databases."));
	a->setWhatsThis(i18n("Tell the daemon that the next HotSync "
		"should just list the files on the Handheld and do nothing "
		"else."));
	syncPopup->insert(a);
#endif


	a = new KAction(i18n("Rese&t Link"),CSL1("reload"), 0,
		this, SLOT(slotResetLink()),
		actionCollection(),"file_reload");
	a->setToolTip(i18n("Reset the device connection."));
	a->setWhatsThis(i18n("Try to reset the daemon and its connection "
		"to the Handheld."));


	a = KStdAction::quit(this, SLOT(quit()), actionCollection());
	a->setWhatsThis(i18n("Quit KPilot, (and stop the daemon "
		"if configured that way)."));

	// View actions

	// Options actions
	createStandardStatusBarAction();
	setStandardToolBarMenuEnabled(true);

	(void) KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()),
		actionCollection());
	(void) KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()),
		actionCollection());
	(void) KStdAction::preferences(this, SLOT(configure()),
		actionCollection());

	a = new KAction(i18n("Configuration &Wizard..."), CSL1("wizard"), 0,
		this, SLOT(configureWizard()),
		actionCollection(), "options_configure_wizard");
	a->setWhatsThis(i18n("Configure KPilot using the configuration wizard."));

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


void KPilotInstaller::optionsConfigureKeys()
{
	FUNCTIONSETUP;
	KKeyDialog::configure( actionCollection() );
}

void KPilotInstaller::optionsConfigureToolbars()
{
	FUNCTIONSETUP;
	// use the standard toolbar editor
	// This was added in KDE 3.1
	saveMainWindowSettings( KGlobal::config(), autoSaveGroup() );
	KEditToolbar dlg(actionCollection());
	connect(&dlg, SIGNAL(newToolbarConfig()), this, SLOT(slotNewToolbarConfig()));
	dlg.exec();
}


void KPilotInstaller::slotNewToolbarConfig()
{
	FUNCTIONSETUP;
	// recreate our GUI
	createGUI();
	applyMainWindowSettings( KGlobal::config(), autoSaveGroup() );
}

void KPilotInstaller::slotResetLink()
{
	FUNCTIONSETUP;
	getDaemon().reloadSettings();
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
		// @TODO: This should also be done when pressing apply without
		// closing the dialog.
		//
		daemon.reloadSettings();
		ret = true;
	}

	KPILOT_DELETE(options);
	daemon.requestSync(rememberedSync);

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Done with options." << endl;
#endif

	KPilotConfig::sync();
	return ret;
}

/*
 * Run the config wizard -- this takes a little library magic, and
 * it might fail entirely; returns false if no wizard could be run,
 * or true if the wizard runs (says nothing about it being OK'ed or
 * canceled, though).
 */
typedef enum { Failed, OK, Cancel } WizardResult;
static WizardResult runWizard(PilotDaemonDCOP_stub &daemon,QWidget *parent)
{
	FUNCTIONSETUP;
	WizardResult ret = Failed ;
	int rememberedSync = daemon.nextSyncType();
	daemon.requestSync(0);

	KPilotSettings::self()->readConfig();
	// Declarations at top because of goto's in this function
	ConfigWizard *(* f) (QWidget *, int) = 0L ;
	ConfigWizard *w = 0L;
	KLibrary *l = KLibLoader::self()->library("kcm_kpilot");

	if (!l)
	{
		kdWarning() << k_funcinfo << ": Couldn't load library!" << endl;
		goto sorry;
	}

	if (l->hasSymbol("create_wizard"))
	{
		f = ( ConfigWizard * (*) (QWidget *, int) ) (l->symbol("create_wizard")) ;
	}

	if (!f)
	{
		kdWarning() << k_funcinfo << ": No create_wizard() in library." << endl;
		goto sorry;
	}

	w = f(parent,ConfigWizard::Standalone);
	if (!w)
	{
		kdWarning() << k_funcinfo << ": Can't create wizard." << endl;
		goto sorry;
	}

	if (w->exec())
	{
		KPilotSettings::self()->readConfig();
		ret = OK;
	}
	else
	{
		ret = Cancel;
	}
	KPILOT_DELETE(w);

sorry:
	if (Failed == ret)
	{
		KMessageBox::sorry(parent,
			i18n("The library containing the configuration wizard for KPilot "
				"could not be loaded, and the wizard is not available. "
				"Please try to use the regular configuration dialog."),
				i18n("Wizard Not Available"));
	}

	if (OK == ret)
	{
		KPilotConfig::updateConfigVersion();
		KPilotSettings::writeConfig();
		KPilotConfig::sync();
	}

	daemon.requestSync(rememberedSync);
	return ret;
}

void KPilotInstaller::componentUpdate()
{
	FUNCTIONSETUP;

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
		fLogWidget->logMessage(i18n("Changed username to `%1'.")
			.arg(KPilotSettings::userName()));
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
}

/* virtual DCOP */ ASYNC KPilotInstaller::configureWizard()
{
	FUNCTIONSETUP;

	if ( fAppStatus!=Normal || fConfigureKPilotDialogInUse )
	{
		if (fLogWidget)
		{
			fLogWidget->addMessage(i18n("Cannot run KPilot's configuration wizard right now (KPilot's UI is already busy)."));
		}
		return;
	}
	fAppStatus=UIBusy;
	fConfigureKPilotDialogInUse = true;

	if (runWizard(getDaemon(),this) == OK)
	{
		componentUpdate();
	}

	fConfigureKPilotDialogInUse = false;
	fAppStatus=Normal;
}

/* virtual DCOP */ ASYNC KPilotInstaller::configure()
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
#ifdef DEBUG
	{"debug <level>", I18N_NOOP("Set debugging level"), "0"},
#endif
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
		"(c) 1998-2000,2001, Dan Pilone (c) 2000-2004, Adriaan de Groot",
		0L,
		"http://www.kpilot.org/"
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
		kdWarning() << ": KPilot configuration version "
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
		(run_mode == KPilotConfig::ConfigureAndContinue) ||
		(run_mode == KPilotConfig::WizardAndContinue) )
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": Running setup first."
			<< " (mode " << run_mode << ")" << endl;
#endif
		PilotDaemonDCOP_stub *daemon = new PilotDaemonDCOP_stub("kpilotDaemon","KPilotDaemonIface");
		bool r = false;
		if (run_mode == KPilotConfig::WizardAndContinue)
		{
			r = ( runWizard(*daemon,0L) == OK );
		}
		else
		{
			r = runConfigure(*daemon,0L);
		}
		delete daemon;
		if (!r) return 1;
		// User expected configure only.
		if (run_mode == KPilotConfig::ConfigureKPilot)
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


