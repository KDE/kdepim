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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/


static const char *kpilot_id =
	"$Id$";


#ifndef _KPILOT_OPTIONS_H
#include "options.h"
#endif


#include <sys/types.h>
#include <dirent.h>
#include <iostream>
#include <fstream.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#ifndef QFILE_H
#include <qfile.h>
#endif

#ifndef QLIST_H
#include <qlist.h>
#endif

#ifndef QSTRING_H
#include <qstring.h>
#endif

#ifndef QLISTBOX_H
#include <qlistbox.h>
#endif

#ifndef QCOMBOBOX_H
#include <qcombobox.h>
#endif

#include <qvbox.h>
#include <kjanuswidget.h>

#ifndef _KURL_H_
#include <kurl.h>
#endif
#ifndef _KMESSAGEBOX_H_
#include <kmessagebox.h>
#endif
#ifndef _KSTATUSBAR_H_
#include <kstatusbar.h>
#endif
#ifndef _KCONFIG_H_
#include <kconfig.h>
#endif
#ifndef _KWIN_H_
#include <kwin.h>
#endif
#ifndef _KPROCESS_H_
#include <kprocess.h>
#endif
#ifndef _KCOMBOBOX_H_
#include <kcombobox.h>
#endif
#ifndef _KMENUBAR_H_
#include <kmenubar.h>
#endif
#ifndef _KSTDDIRS_H_
#include <kstddirs.h>
#endif
#ifndef _KABOUTDATA_H_
#include <kaboutdata.h>
#endif
#ifndef _KCMDLINEARGS_H_
#include <kcmdlineargs.h>
#endif
#ifndef _KICONLOADER_H_
#include <kiconloader.h>
#endif
#ifndef _KDEBUG_H_
#include <kdebug.h>
#endif
#ifndef _KACTION_H_
#include <kaction.h>
#endif
#ifndef _KSTDACTION_H_
#include <kstdaction.h>
#endif
#ifndef _KUNIQUEAPP_H_
#include <kuniqueapp.h>
#endif
#ifndef _KKDEYDIALOG_H_
#include <kkeydialog.h>
#endif
#ifndef _KEDITTOOLBAR_H_
#include <kedittoolbar.h>
#endif

#include <kprogress.h>


#include "kpilotConfigDialog.h"
#include "kpilotConfig.h"

#include "pilotComponent.h"

#include "addressWidget.h"
#include "memoWidget.h"
#include "fileInstallWidget.h"
#include "logWidget.h"

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
	typedef QList<PilotComponent> ComponentList;

private:
	ComponentList  fPilotComponentList;

public:
	ComponentList &list() { return fPilotComponentList; } ;
} ;

KPilotInstaller::KPilotInstaller() :
	KMainWindow(0),
	DCOPObject("KPilotIface"),
	fDaemonStub(new PilotDaemonDCOP_stub("kpilotDaemon",
		"KPilotDaemonIface")),
	fP(new KPilotPrivate),
	fMenuBar(0L),
	fToolBar(0L),
	fQuitAfterCopyComplete(false),
	fManagingWidget(0L),
	fKillDaemonOnExit(false),
	fDaemonWasRunning(true),
	fStatus(Startup),
	fFileInstallWidget(0L),
	fLogWidget(0L)
{
	FUNCTIONSETUP;

	readConfig();
	setupWidget();

	/* NOTREACHED */
	(void) kpilot_id;
}

KPilotInstaller::~KPilotInstaller()
{
	FUNCTIONSETUP;
	killDaemonIfNeeded();
	delete fDaemonStub;
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

	QString daemonError;
	QCString daemonDCOP;
	int daemonPID;

	QString s = getDaemon().statusString();

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Daemon status is " << s << endl;
#endif

	if ((s.isNull()) || (!getDaemon().ok()))
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": Daemon not responding, trying to start it."
			<< endl;
#endif
		fDaemonWasRunning = false;
	}
	else
	{
		fDaemonWasRunning = true;
	}

	if (!fDaemonWasRunning && KApplication::startServiceByDesktopName(
		"kpilotdaemon",
		QString::null, &daemonError, &daemonDCOP, &daemonPID
#if (KDE_VERSION >= 220)
			// Startup notification was added in 2.2
			, "0"
#endif
		))
	{
		kdError() << k_funcinfo << ": Can't start daemon : " << daemonError << endl;
	}
	else
	{
#ifdef DEBUG
		s = getDaemon().statusString();
		DEBUGKPILOT << fname << ": Daemon status is " << s << endl;
#endif
	}
}

void KPilotInstaller::readConfig()
{
	FUNCTIONSETUP;

	KPilotConfigSettings & c = KPilotConfig::getConfig();
	fKillDaemonOnExit = c.getKillDaemonOnExit();
}


void KPilotInstaller::setupWidget()
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Creating central widget." << endl;
#endif

	setCaption("KPilot");
	setMinimumSize(500, 405);


	fManagingWidget = new KJanusWidget(this,"mainWidget",
		KJanusWidget::IconList);
	fManagingWidget->setMinimumSize(500, 330);
	fManagingWidget->show();
	setCentralWidget(fManagingWidget);

	initIcons();
	initMenu();
	initComponents();

	createGUI("kpilotui.rc", false);
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

	ADDICONPAGE(i18n("HotSync"),"kpilot/kpilot-hotsync.png");
	fLogWidget = new LogWidget(w);
	addComponentPage(fLogWidget, i18n("HotSync"));
	fLogWidget->setShowTime(true);


	ADDICONPAGE(i18n("Memo Viewer"),"kpilot/kpilot-knotes.png");
	addComponentPage(new MemoWidget(w, defaultDBPath),
		i18n("Memo Viewer"));

	ADDICONPAGE(i18n("Address Viewer"),"kpilot/kpilot-address.png");
	addComponentPage(new AddressWidget(w,defaultDBPath),
		i18n("Address Viewer"));

	ADDICONPAGE(i18n("File Installer"),"kpilot/kpilot-fileinstaller.png");
	fFileInstallWidget = new FileInstallWidget(
		w,defaultDBPath);
	addComponentPage(fFileInstallWidget, i18n("File Installer"));

#undef ADDICONPAGE
#undef VIEWICON
}



void KPilotInstaller::initIcons()
{
	FUNCTIONSETUP;

}



void KPilotInstaller::slotSelectComponent(PilotComponent * c)
{
	FUNCTIONSETUP;

	QWidget *p = static_cast <QWidget *>(c);
	if (!p) { kdWarning() << k_funcinfo << ": Not a widget." << endl; return;}

	QObject *o = p->parent();
	if (!o) { kdWarning() << k_funcinfo << ": No parent." << endl; return; }

	QWidget *parent = dynamic_cast<QWidget *>(o);
	if (!parent) { kdWarning() << k_funcinfo << ": No widget parent." << endl; return; }

	int index = fManagingWidget->pageIndex(parent);

	if (index < 0)
	{
		kdWarning() << k_funcinfo << ": Index " << index << endl;
		return;
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
	case KPilotDCOP::EndOfHotSync :
		componentPostSync();
		break;
	default :
		kdWarning() << k_funcinfo << ": Unhandled status message " << i << endl;
		break;
	}
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

	KAction *p;

	// File actions
	p = new KAction(i18n("&HotSync"), "hotsync", 0,
		this, SLOT(slotHotSyncRequested()),
		actionCollection(), "file_hotsync");
#if 0
	p = new KAction(i18n("&FastSync"), "fastsync", 0,
		this, SLOT(slotHotSyncRequested()),
		actionCollection(), "file_fastsync");
#endif
#ifdef DEBUG
	p = new KAction(QString::fromLatin1("List only"),"list",0,
		this,SLOT(slotListSyncRequested()),
		actionCollection(), "file_list");
#endif
	p = new KAction(i18n("&Backup"), "backup", 0,
		this, SLOT(slotBackupRequested()),
		actionCollection(), "file_backup");
	p = new KAction(i18n("&Restore"), "restore", 0,
		this, SLOT(slotRestoreRequested()),
		actionCollection(), "file_restore");
	p = KStdAction::quit(this, SLOT(quit()), actionCollection());

	// View actions

	// Options actions
	m_toolbarAction =
		KStdAction::showToolbar(this, SLOT(optionsShowToolbar()),
		actionCollection());
	p = KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()),
		actionCollection());
	p = KStdAction::configureToolbars(this, SLOT(optionsConfigureKeys()),
		actionCollection());
	p = KStdAction::preferences(this, SLOT(slotConfigureKPilot()),
		actionCollection());
	p = new KAction(i18n("C&onfigure Conduits..."), "configure", 0, this,
		SLOT(slotConfigureConduits()), actionCollection(),
		"options_configure_conduits");
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
		<< (int) p << " called " << p->name("(none)") << endl;
#endif

	p->initialize();
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

	pt->setExclusiveGroup("view_menu");

	connect(p, SIGNAL(showComponent(PilotComponent *)),
		this, SLOT(slotSelectComponent(PilotComponent *)));
}



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


void KPilotInstaller::optionsConfigureKeys()
{
	FUNCTIONSETUP;
	KKeyDialog::configureKeys(actionCollection(), xmlFile());
}

void KPilotInstaller::optionsConfigureToolbars()
{
	FUNCTIONSETUP;
	// use the standard toolbar editor
	saveMainWindowSettings( KGlobal::config(), autoSaveGroup() );
	KEditToolbar dlg(actionCollection());
	connect(&dlg, SIGNAL(newToolbarConfig()), this, SLOT(newToolbarConfig()));
	dlg.exec();
}

void KPilotInstaller::slotNewToolbarConfig()
{
	// recreate our GUI
	createGUI();
	applyMainWindowSettings( KGlobal::config(), autoSaveGroup() );
}

void KPilotInstaller::slotConfigureKPilot()
{
	FUNCTIONSETUP;

	// Display the (modal) options page.
	//
	//
	int rememberedSync = getDaemon().nextSyncType();
	getDaemon().requestSync(0);

	KPilotConfig::getConfig().reparseConfiguration();
	KPilotConfigDialog *options = new KPilotConfigDialog(this,
		"configDialog", true);

	if (!options)
	{
		kdError() << k_funcinfo
			<< ": Can't allocate KPilotOptions object" << endl;
		getDaemon().requestSync(rememberedSync);
		return;
	}

	options->exec();

	if (options->result())
	{
#ifdef DEBUG
		DEBUGKPILOT << fname << ": Updating link." << endl;
#endif

		KPilotConfig::getConfig().sync();
		readConfig();

		// Update the daemon to reflect new settings.
		//
		//
		getDaemon().reloadSettings();

		// Update each installed component.
		//
		//
		for (fP->list().first();
			fP->list().current();
			fP->list().next())
		{
			fP->list().current()->initialize();
		}
	}

	KPILOT_DELETE(options);
	getDaemon().requestSync(rememberedSync);

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Done with options." << endl;
#endif
}

void KPilotInstaller::slotConfigureConduits()
{
	FUNCTIONSETUP;

	ConduitConfigDialog *conSetup = 0L;

	conSetup = new ConduitConfigDialog(this,0L,true);
	int r = conSetup->exec();
	if (r)
	{
		KPilotConfig::getConfig().sync();
		getDaemon().reloadSettings();
	}
	delete conSetup;
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
		I18N_NOOP("Setup the Pilot device and other parameters"),
		0L},
	{"c", 0, 0},
	{"conduit-setup", I18N_NOOP("Run conduit setup"), 0L},
	{0, 0, 0}
};




// "Regular" mode == 0
// setup mode == 's'
// setup forced by config change == 'S'
// conduit setup == 'c'
//
// This is only changed by the --setup flag --
// kpilot still does a setup the first time it is run.
//
//
int run_mode = 0;


int main(int argc, char **argv)
{
	FUNCTIONSETUP;

	KAboutData about("kpilot", I18N_NOOP("KPilot"),
		KPILOT_VERSION,
		"KPilot - HotSync software for KDE\n\n",
		KAboutData::License_GPL, "(c) 1998-2000,2001, Dan Pilone");
	about.addAuthor("Dan Pilone",
		I18N_NOOP("Project Leader"),
		"pilone@slac.com", "http://www.slac.com/pilone/kpilot_home/");
	about.addAuthor("Adriaan de Groot",
		I18N_NOOP("Maintainer"),
		"groot@kde.org", "http://www.cs.kun.nl/~adridg/kpilot/");
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
	about.addCredit("Reinhold Kainhofer",
		I18N_NOOP("vCal and Addressbook conduit ports, time conduit, AvantGo conduit"), "reinhold@kainhofer.com", "http://reinhold.kainhofer.com/");


	KCmdLineArgs::init(argc, argv, &about);
#ifdef DEBUG
	KCmdLineArgs::addCmdLineOptions(debug_options, "debug", "debug");
#endif
	KCmdLineArgs::addCmdLineOptions(kpilotoptions, "kpilot", 0L, "debug");
	KUniqueApplication::addCmdLineOptions();
	KCmdLineArgs *p = KCmdLineArgs::parsedArgs();

	if (p->isSet("setup"))
	{
		run_mode = 's';
	}
	if (p->isSet("conduit-setup"))
	{
		run_mode = 'c';
	}

	if (!KUniqueApplication::start())
	{
		return 0;
	}
	KUniqueApplication a(true, true);

	KPilotConfig::getDebugLevel();

	KPilotConfigSettings & c = KPilotConfig::getConfig();
	if (c.getVersion() < KPilotConfig::ConfigurationVersion)
	{
		run_mode = 'S';
	}

	if (run_mode == 'c')
	{
		ConduitConfigDialog *cs = new ConduitConfigDialog(0L,0L,true);
		int r = cs->exec();

		if (r)
		{
			return 1;	// Dialog cancelled
		}
		else
		{
			return 0;
		}
	}

	if ((run_mode == 's') || (run_mode == 'S'))
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": Running setup first."
			<< " (mode " << run_mode << ")" << endl;
#endif

		KPilotConfigDialog *options = new KPilotConfigDialog(0L,
			"configDialog", true);
		int r = options->exec();

		if (run_mode == 's')
		{
			if (!r)
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}

		if (!r)
			return 1;


		// The options dialog may have changed the group
		// while reading or writing settings (still a
		// bad idea, actually).
		//
		c.resetGroup();
	}

	if (c.getVersion() < KPilotConfig::ConfigurationVersion)
	{
		kdWarning() << k_funcinfo <<
			": Is still not configured for use." << endl;
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
		"share/apps/kpilot/DBBackup");
	tp->show();
	a.setMainWidget(tp);
	return a.exec();
}


// $Log$
// Revision 1.84  2003/01/18 00:30:18  kainhofe
// Removed the Log: tags from the conduits I maintain.
// Cleanup of the includes.
// Started implementing the other field sync of the addressbookconduit. Still have trouble converting a string to a QDate using a custom format
//
// Revision 1.83  2002/12/24 11:26:28  adridg
// Forgot that moc can't handle #ifdef
//
// Revision 1.82  2002/12/10 15:54:00  faure
// Mainwindow settings and KEditToolbar fix, as usual. (untested, other than compilation)
//
// Revision 1.81  2002/11/27 21:29:06  adridg
// See larger ChangeLog entry
//
// Revision 1.80  2002/08/24 21:27:32  adridg
// Lots of small stuff to remove warnings
//
// Revision 1.79  2002/08/15 21:51:00  kainhofe
// Fixed the error messages (were not printed to the log), finished the categories sync of the todo conduit
//
// Revision 1.78  2002/08/13 11:57:37  mhunter
// VCal -> vCal (name consistency)
//
// Revision 1.77  2002/08/12 13:07:07  kainhofe
// Added myself to the credits page
//
// Revision 1.76  2002/06/24 19:29:11  adridg
// Allow daemon RW access to config file
//
// Revision 1.75  2002/05/14 22:57:40  adridg
// Merge from _BRANCH
//
// Revision 1.74.2.1  2002/04/14 22:26:12  adridg
// New TODO's for HEAD; cosmetic bugfix in logWidget; rectify misleading debug output when KPilot starts the daemon itself.
//
// Revision 1.74  2002/02/02 11:46:02  adridg
// Abstracting away pilot-link stuff
//
// Revision 1.73  2002/01/31 16:25:28  hollomon
//
// If we can't start the daemon. tell the user why not.
//
// Revision 1.72  2002/01/31 15:36:33  hollomon
//
// KPilotInstaller::startDaemonIfNeeded was trying to start the daemon
// even if it found it running. Calculated fDaemonWasRunning then ignored
// it.
//
// Revision 1.71  2002/01/26 15:00:01  adridg
// An icon for the address viewer
//
// Revision 1.70  2002/01/25 21:43:12  adridg
// ToolTips->WhatsThis where appropriate; vcal conduit discombobulated - it doesn't eat the .ics file anymore, but sync is limited; abstracted away more pilot-link
//
// Revision 1.69  2002/01/23 08:35:54  adridg
// Remove K-menu dependency
//
// Revision 1.68  2001/12/31 15:52:40  adridg
// CVS_SILENT: Spit 'n polish
//
// Revision 1.67  2001/12/31 09:38:09  adridg
// Splash patch by Aaron
//
// Revision 1.66  2001/11/18 16:59:55  adridg
// New icons, DCOP changes
//
// Revision 1.65  2001/11/11 22:10:38  adridg
// Switched to KJanuswidget
//
// Revision 1.64  2001/10/17 08:46:08  adridg
// Minor cleanups
//
// Revision 1.63  2001/09/30 19:51:56  adridg
// Some last-minute layout, compile, and __FUNCTION__ (for Tru64) changes.
//
// Revision 1.62  2001/09/30 16:58:45  adridg
// Cleaned up preHotSync interface, removed extra includes, added private-d-ptr.
//
// Revision 1.61  2001/09/29 16:26:18  adridg
// The big layout change
//
// Revision 1.60  2001/09/23 21:42:35  adridg
// Factored out debugging options
//
// Revision 1.59  2001/09/23 18:25:50  adridg
// New config architecture
//
// Revision 1.58  2001/09/16 13:37:48  adridg
// Large-scale restructuring
//
// Revision 1.57  2001/09/07 20:48:13  adridg
// Stripped away last crufty IPC, added logWidget
//
// Revision 1.56  2001/08/29 08:50:56  cschumac
// Make KPilot compile.
//
// Revision 1.55  2001/08/27 22:54:27  adridg
// Decruftifying; improve DCOP link between daemon & viewer
//
// Revision 1.54  2001/08/19 19:25:57  adridg
// Removed kpilotlink dependency from kpilot; added DCOP interfaces to make that possible. Also fixed a connect() type mismatch that was harmless but annoying.
//
// Revision 1.53  2001/06/13 21:32:35  adridg
// Dead code removal and replacing complicated stuff w/ QWidgetStack
//
// Revision 1.52  2001/05/25 16:06:52  adridg
// DEBUG breakage
//
// Revision 1.51  2001/05/07 19:45:11  adridg
// KToggle actions used now
//
// Revision 1.50  2001/04/26 21:59:00  adridg
// CVS_SILENT B0rkage with previous commit
//
// Revision 1.49  2001/04/23 21:05:39  adridg
// Fixed bug w/ absent conduit executables. Fixed resize bug.
//
// Revision 1.48  2001/04/23 06:30:38  adridg
// XML UI updates
//
// Revision 1.47  2001/04/14 15:21:35  adridg
// XML GUI and ToolTips
//
// Revision 1.46  2001/04/11 21:36:54  adridg
// Added app icons
//
// Revision 1.45  2001/03/27 23:54:43  stern
// Broke baseConduit functionality out into PilotConduitDatabase and added support for local mode in BaseConduit
//
// Revision 1.44  2001/03/09 09:40:52  adridg
// Large-scale #include cleanup; component resizing bug fixed
//
// Revision 1.43  2001/03/05 23:57:53  adridg
// Added KPILOT_VERSION
//
// Revision 1.42  2001/03/04 22:22:29  adridg
// DCOP cooperation between daemon & kpilot for d&d file install
//
// Revision 1.41  2001/03/02 13:07:18  adridg
// Completed switch to KAction
//
// Revision 1.40  2001/03/01 01:02:48  adridg
// Started changing to KAction
//
// Revision 1.39  2001/02/26 22:11:40  adridg
// Removed useless getopt.h; fixes compile prob on Solaris
//
// Revision 1.38  2001/02/25 12:39:35  adridg
// Fixed component names (src incompatible)
//
// Revision 1.37  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.36  2001/02/08 17:59:34  adridg
// Removed spurious #ifdefs, and the #define that goes with it. Make KPilot exit consistently after user-requested setup actions.
//
// Revision 1.35  2001/02/08 13:17:19  adridg
// Fixed crash when conduits run during a backup and exit after the
// end of that backup (because the event loop is blocked by the backup
// itself). Added better debugging error exit message (no i18n needed).
//
// Revision 1.34  2001/02/08 08:13:44  habenich
// exchanged the common identifier "id" with source unique <sourcename>_id for --enable-final build
//
// Revision 1.33  2001/02/05 20:58:48  adridg
// Fixed copyright headers for source releases. No code changed
//
// Revision 1.32  2001/02/05 19:16:32  adridg
// Removing calls to exit() from internal functions
//
// Revision 1.31  2001/02/05 11:19:18  adridg
// Reduced icon-loading code to hard-coded xpms
//
// Revision 1.30  2001/01/19 22:18:43  waba
// KTMainWindow is obsolete. I hope it works because I can't test due to lack of
// pilot. At least it compiles.
//
// Revision 1.29  2001/01/06 13:21:53  adridg
// Updated version number
//
// Revision 1.28  2001/01/03 00:02:45  adridg
// Added Heiko's FastSync
//
