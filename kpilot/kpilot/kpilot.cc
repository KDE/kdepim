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
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/


static const char *kpilot_id="$Id$";


#ifndef _KPILOT_OPTIONS_H
#include "options.h"
#endif


#include <sys/types.h>
#include <dirent.h>
#include <iostream.h>
#include <fstream.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
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

#ifndef QWIDGETSTACK_H
#include <qwidgetstack.h>
#endif

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
#ifndef _KSOCK_H_
#include <ksock.h>
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


#ifndef _KPILOT_KPILOTOPTIONS_H
#include "kpilotOptions.h"
#endif

#ifndef _KPILOT_KPILOTCONFIG_H
#include "kpilotConfig.h"
#endif

#ifndef _KPILOT_MESSAGEDIALOG_H
#include "messageDialog.h"
#endif

#ifndef _KPILOT_ADDRESSWIDGET_H
#include "addressWidget.h"
#endif

#ifndef _KPILOT_MEMOWIDGET_H
#include "memoWidget.h"
#endif

#ifndef _KPILOT_FILEINSTALLWIDGET_H
#include "fileInstallWidget.h"
#endif

#ifndef _KPILOT_STATUSMESSAGES_H
#include "statusMessages.h"
#endif

#ifndef _KPILOT_CONDUITSETUP_H
#include "conduitSetup.h"
#endif

#ifndef _KPILOT_PILOTDAEMON_H
#include "pilotDaemon.h"
#endif

#include "kpilot.moc"

// This is an XPM disguised as an include file.
//
//
#include "kpilot_on_pp.h"

KPilotInstaller::KPilotInstaller() : 
	KMainWindow(0), 
	DCOPObject("KPilotIface"),
	fMenuBar(0L), fStatusBar(0L), fToolBar(0L),
    fQuitAfterCopyComplete(false), fManagingWidget(0L), fPilotLink(0L),
    fPilotCommandSocket(0L), fPilotStatusSocket(0L), fKillDaemonOnExit(false),
    fStatus(Startup),
	fFileInstallWidget(0L)
{
	FUNCTIONSETUP;

	KConfig& config = KPilotConfig::getConfig();
	// Log a warning if the config file is too old.
	//
	//
	(void) KPilotOptions::isNewer(config);

	readConfig(config);

    if(config.readNumEntry("NextUniqueID", 0) == 0)
      {
	// Is this an ok value to use??
	config.writeEntry("NextUniqueID", 0xf000);
	config.sync();
      }


    initPilotLink();
	if (!fPilotCommandSocket)
	{
		kdError() << __FUNCTION__ << ": Couldn't connect to daemon -- quitting"
			<< endl;
		fStatus = Error ;
		return;
	}
    setupWidget();
    initStatusLink();  // This is separate to allow components to initialize


    showTitlePage(QString::null,true);
    show();
    }

KPilotInstaller::~KPilotInstaller()
{
	FUNCTIONSETUP;

	if (fPilotStatusSocket) delete fPilotStatusSocket;
	fPilotStatusSocket=0L;

	if(fKillDaemonOnExit && fPilotCommandSocket &&
		(fPilotCommandSocket->socket()>=0))
	{
			kdDebug() << fname
				<< ": Killing daemon."
				<< endl;

		ofstream out(fPilotCommandSocket->socket());
		out << -3 << endl;
	}

	if (fPilotCommandSocket) delete fPilotCommandSocket;
	fPilotCommandSocket=0L;
}

void KPilotInstaller::readConfig(KConfig& config)
{
	config.setGroup(QString::null);
	fKillDaemonOnExit = config.readBoolEntry("StopDaemonAtExit",false);
}


void
KPilotInstaller::setupWidget()
{
	FUNCTIONSETUP;

	DEBUGKPILOT << fname
		<< ": Creating central widget."
		<< endl;

	setCaption("KPilot");
	setMinimumSize(500,405);
	fManagingWidget = new QWidgetStack(this);
	fManagingWidget->setMinimumSize(500,330);
	fManagingWidget->show();
	setCentralWidget(fManagingWidget);

	initIcons();
	initMenu();
	initComponents();

	createGUI("kpilotui.rc",false);
	DEBUGKPILOT << fname
		<< ": Got XML from "
		<< xmlFile()
		<< " and "
		<< localXMLFile()
		<< endl;


	initStatusBar();
}


void
KPilotInstaller::initComponents()
{
	FUNCTIONSETUP;

	DEBUGKPILOT << fname
		<< ": Creating title screen."
		<< endl;

	QLabel* titleScreen = new QLabel(getManagingWidget());
	titleScreen->setPixmap(QPixmap(kpilot_on_pp));
	titleScreen->setAlignment(AlignCenter);
	titleScreen->setBackgroundColor(QColor("black"));
	titleScreen->setGeometry(0, 0, 
		getManagingWidget()->geometry().width(), 
		getManagingWidget()->geometry().height());
	fManagingWidget->addWidget(titleScreen,0);

	QString defaultDBPath = KPilotConfig::getDefaultDBPath();

	DEBUGKPILOT << fname
		<< ": Creating component pages."
		<< endl;

	addComponentPage(new MemoWidget(getManagingWidget(),defaultDBPath),
		i18n("Memo Viewer"));
	addComponentPage(new AddressWidget(getManagingWidget(),defaultDBPath),
		i18n("Address Viewer"));

	fFileInstallWidget = new FileInstallWidget(getManagingWidget(),
			defaultDBPath);
	addComponentPage(fFileInstallWidget,
		i18n("File Installer"));
}

void
KPilotInstaller::initStatusBar()
{
	FUNCTIONSETUP;
	QString welcomeMessage=i18n("Welcome to KPilot");
	welcomeMessage+=" (";
	welcomeMessage+=version(0);
	welcomeMessage+=')';

	fStatusBar = statusBar();
	fStatusBar->insertItem(welcomeMessage,0);
	fStatusBar->show();
}


void KPilotInstaller::initIcons()
{
	FUNCTIONSETUP;

}



void KPilotInstaller::slotShowTitlePage()
{
	showTitlePage();
}

void KPilotInstaller::showTitlePage(const QString& msg,bool)
{
	FUNCTIONSETUP;

	fManagingWidget->raiseWidget((int) 0);

	if (!msg.isNull())
	{
		fStatusBar->changeItem(msg,0);
	}
}

	



void
KPilotInstaller::initPilotLink()
{
	FUNCTIONSETUP;

	if(fPilotLink)
	{
		kdWarning() << __FUNCTION__ << ": Pilot Link already created?\n" ;
		return;
	}

	fPilotLink = new KPilotLink();
	if (fPilotLink==NULL)
	{
		kdError() << __FUNCTION__ << ": Can't allocate fPilotLink.\n";
		KMessageBox::error(this,
				   i18n("Allocating PilotLink failed."),
				   i18n("Cannot create link to Daemon"));
		return;
	}


	if(fPilotCommandSocket == NULL)
	{
		initCommandSocket();
	}
	else 
	{
		// We were called after a reconfigure
		//
		//
			kdDebug() << fname << ": Reconfiguring daemon.\n" ;

		ofstream out(fPilotCommandSocket->socket());
		out << "-2" << endl;
	}
}

void KPilotInstaller::initCommandSocket()
{
	FUNCTIONSETUP;

	MessageDialog *messageDialog=0L;

	if (fPilotCommandSocket)
	{
		kdWarning() << __FUNCTION__ 
			<< ": We already have a command socket"
			<< endl;
		return;
	}

		kdDebug() << fname
			<< ": Creating command socket"
			<< endl ;

	fPilotCommandSocket = new KSocket("localhost", 
		PILOTDAEMON_COMMAND_PORT);
	if (fPilotCommandSocket==NULL)
	{
		kdError() << __FUNCTION__ << ": Can't allocate fPilotCommandSocket.\n";
		KMessageBox::error(this,
				   i18n("Allocating fPilotCommandSocket failed."),
				   i18n("Cannot create link to Daemon"));

		delete fPilotLink;
		fPilotLink=NULL;
		return;
	}

		kdDebug() << fname
			<< ": Got socket " 
			<< fPilotCommandSocket->socket()
			<< endl ;

	
	if((fPilotCommandSocket->socket() < 0) ||
		!testSocket(fPilotCommandSocket))
	{
		int i;

			kdDebug() << fname
				<< ": Socket not OK, starting daemon"
				<< endl;

		// It wasn't running...
		messageDialog = 
			new MessageDialog(version(0));

		if (messageDialog!=NULL)
		{
			messageDialog->show();
			kapp->processEvents();
			messageDialog->setMessage(
				i18n("Starting Sync Daemon. "
					"Please Wait."));

			kapp->processEvents();
		}

		delete fPilotCommandSocket;
		fPilotCommandSocket=0L;
	

		KProcess pilotDaemon;
		pilotDaemon << "kpilotDaemon";
#ifdef DEBUG
		if (debug_level)
		{
			QString s;
			s.setNum(debug_level);

			pilotDaemon << "--debug";
			pilotDaemon << s;
		}
#endif
			
		kapp->processEvents();

		pilotDaemon.start(KProcess::DontCare);

		// While the daemon is starting up,
		// we'll do some busy-waiting and
		// keep trying to connect to it.
		//
		//
		for (i=0; i<4; i++)
		{
			kapp->processEvents();
			sleep(1);
			kapp->processEvents();

				kdDebug() << fname
					<< ": Trying to connect"
					<< endl;
			fPilotCommandSocket = new KSocket("localhost",
				PILOTDAEMON_COMMAND_PORT);
			if (!fPilotCommandSocket)
			{
				kdError() << __FUNCTION__
					<< ": Couldn't allocate KSocket"
					<< endl;
				continue;
			}

			if ((fPilotCommandSocket->socket() >= 0) &&
				testSocket(fPilotCommandSocket))
			{
				break;
			}
			else
			{
				delete fPilotCommandSocket;
				fPilotCommandSocket=0L;
			}
		}

			kdDebug() << fname
				<< ": Halfway result "
				<< "PilotCommandSocket="
				<< (int)fPilotCommandSocket
				<< endl;
			
		if(!fPilotCommandSocket)
		{
			kdError() << __FUNCTION__ 
				<< ": Can't connect to daemon"
				<< endl ;

			KMessageBox::error(this, 
					   i18n("Cannot start KPilot Daemon. "
						"Check settings and "
						"restart KPilot manually."),
					   i18n("Cannot connect to Daemon"));
					   

				kdDebug() << fname << ": creating options "
					<< endl;
			KPilotOptions* options = new KPilotOptions(this);
			if (options)
			{
				options->exec();
					kdDebug() << fname << ": User said "
						<< options->result()
						<< endl;
				delete options;
			}
			else
			{
				kdError() << __FUNCTION__
					<< ": Couldn't create options window."
					<< endl;
			}
		}

		fKillDaemonOnExit = true;

		if (messageDialog!=NULL)
		{
				kdDebug() << fname
					<< ": Closing and deleting msg dialog"
					<< endl;
			messageDialog->close();
			delete messageDialog;
		}
	}

	fLinkCommand[0] = 0L;

		kdDebug() << fname
			<< ": End result "
			<< "PilotCommandSocket="
			<< (int)fPilotCommandSocket
			<< endl;
}


int KPilotInstaller::testSocket(KSocket *s)
{
	FUNCTIONSETUP;

	char buf[12];
	int r=0;
	int fd=0;

	if (!s) return 0;

	fd=s->socket();
	if (fd<0) return 0;

	sprintf(buf,"%d\n",KPilotLink::TestConnection);

	signal(SIGPIPE,SIG_IGN);
	r=write(fd,buf,strlen(buf));
	if (r<0)
	{
		int e=errno;
		kdError() << __FUNCTION__  
			<< ": (" << strerror(e) << ")" 
			<< endl;
	}
	signal(SIGPIPE,SIG_DFL);
	
	return (r>=0);
}


void
KPilotInstaller::initStatusLink()
{
	FUNCTIONSETUP;

	fPilotStatusSocket = new KSocket("localhost", 
		PILOTDAEMON_STATUS_PORT);
	if (fPilotStatusSocket->socket()!=-1)
	{
			kdDebug() << fname <<
				": Connected socket successfully.\n";

		connect(fPilotStatusSocket, SIGNAL(readEvent(KSocket*)),
			this, SLOT(slotDaemonStatus(KSocket*)));
		fPilotStatusSocket->enableRead(true);
	}
	else
	{
		kdError() << __FUNCTION__ <<
			": Failed to connect socket to daemon.\n";
	}
}





void
KPilotInstaller::slotDaemonStatus(KSocket* daemon)
{
	FUNCTIONSETUP;

	ifstream in(daemon->socket());
	int status;
	in.read(&status, sizeof(int));

	DEBUGKPILOT << fname
		<< ": Got command "
		<< status
		<< endl;

	switch(status)
	{
	case CStatusMessages::SYNC_STARTING :
		if(fLinkCommand[0] == 0L)
		{
			// This only happens when the daemon tells
			// us to sync based on a button push.
			//
			//
			KConfig& c = KPilotConfig::getConfig();

			// Either doFastSync() or doHotSync() sets up
			// fLinkCommand to actually contain the bytes
			// that we need to respond.
			//
			//
			if (c.readBoolEntry("PreferFastSync",false))
			{
				getPilotLink()->setFastSyncRequired(true);
				componentPreSync();
				fStatusBar->changeItem(
					i18n("FastSync in progress..."), 0);
			}
			else
			{
				getPilotLink()->setFastSyncRequired(false);
				componentPreSync();
				fStatusBar->changeItem(
					i18n("HotSync in progress..."), 0);
			}
		}

		// This block writes some stuff to the link and
		// therefore requires its own ofstream object.
		//
		//
		{
		ofstream out(fPilotCommandSocket->socket());
		out << fLinkCommand << flush;
		}
		break;
	case CStatusMessages::SYNC_COMPLETED :
		fStatusBar->changeItem(i18n("HotSync complete."), 0);
		fLinkCommand[0] = 0L;
		for(fPilotComponentList.first(); 
			fPilotComponentList.current(); 
			fPilotComponentList.next())
		{
			fPilotComponentList.current()->initialize();
		}
		break;
	case CStatusMessages::FILE_INSTALL_REQUEST :
		for(fPilotComponentList.first(); 
			fPilotComponentList.current(); 
			fPilotComponentList.next())
		{
			fPilotComponentList.current()->initialize();
		}
		break;
	default :
		kdWarning() << __FUNCTION__
			<< ": Unknown command "
			<< status
			<< " received."
			<< endl;
	}
}

void
KPilotInstaller::slotBackupRequested()
{
	FUNCTIONSETUP;
	showTitlePage();
	fStatusBar->changeItem(
		i18n("Backing up pilot. ")+
		i18n("Please press the hot-sync button."), 
		0);

	sprintf(fLinkCommand, "%d\n", KPilotLink::Backup);
}

void
KPilotInstaller::slotRestoreRequested()
{
	FUNCTIONSETUP;

	showTitlePage();

	fStatusBar->changeItem(
		i18n("Restoring pilot. ")+
		i18n("Please press the hot-sync button."), 0);
	sprintf(fLinkCommand, "%d\n", KPilotLink::Restore);
}

void KPilotInstaller::slotHotSyncRequested() 
{ 
	setupSync(KPilotLink::HotSync,
		i18n("HotSyncing. ")+
		i18n("Please press the HotSync button."));
}

void KPilotInstaller::slotFastSyncRequested()
{ 
	setupSync(KPilotLink::FastSync,
		i18n("FastSyncing. ")+
		i18n("Please press the HotSync button."));
}

void KPilotInstaller::componentPreSync(bool expectEmptyLinkCommand)
{
	FUNCTIONSETUP;

	if (fLinkCommand[0] && expectEmptyLinkCommand)
	{
		kdWarning() << __FUNCTION__
			<< ": LinkCommand not empty!"
			<< endl;
	}


	for(fPilotComponentList.first(); 
		fPilotComponentList.current(); 
		fPilotComponentList.next())
	{
			kdDebug() << fname << ": Pre-sync for builtins."
				<< endl;
		fPilotComponentList.current()->preHotSync(fLinkCommand);
	}
}
 
void KPilotInstaller::setupSync(int kind,const QString& message)
{
	FUNCTIONSETUP;

	showTitlePage(message);

	if (fLinkCommand[0] == 0)
	{
		sprintf(fLinkCommand, "%d\n",kind);
		componentPreSync(false);
	}
}


void
KPilotInstaller::closeEvent(QCloseEvent *e)
{
	FUNCTIONSETUP;

  quit();
  e->accept();
}

void
KPilotInstaller::initMenu()
{
	FUNCTIONSETUP;

	KAction *p;
	KToggleAction *pt;

	// File actions
	p = new KAction(i18n("&HotSync"), "hotsync", 0,
			this, SLOT(slotHotSyncRequested()),
			actionCollection(), "file_hotsync"  );
	p = new KAction(i18n("&FastSync"), "fastsync", 0,
			this, SLOT(slotHotSyncRequested()),
			actionCollection(), "file_fastsync"  );
	p = new KAction(i18n("&Backup"), "backup", 0,
			this, SLOT(slotBackupRequested()),
			actionCollection(), "file_backup"  );
	p = new KAction(i18n("&Restore"), "restore", 0,
			this, SLOT(slotRestoreRequested()),
			actionCollection(), "file_restore"  );
	p = KStdAction::quit(this, SLOT(quit()),
			     actionCollection());

	// View actions
	pt = new KToggleAction(i18n("&KPilot"),"kpilot", 0,
		this, SLOT(slotShowTitlePage()),
		actionCollection(),"view_kpilot");
	pt->setExclusiveGroup("view_menu");
	pt->setChecked(true);

	// Options actions
	m_statusbarAction
	    = KStdAction::showStatusbar(this, SLOT(optionsShowStatusbar()),
					actionCollection());
	m_toolbarAction
	    = KStdAction::showToolbar(this, SLOT(optionsShowToolbar()),
				      actionCollection());
	p = KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()),
				    actionCollection());
	p = KStdAction::configureToolbars(this,
		SLOT(optionsConfigureKeys()),
		actionCollection());
	p = KStdAction::preferences(this, SLOT(slotConfigureKPilot()),
				    actionCollection());
	p = new KAction(i18n("C&onfigure Conduits..."), "configure", 0,
			this, SLOT(slotConfigureConduits()),
			actionCollection(), "options_configure_conduits");
}

void
KPilotInstaller::fileInstalled(int )
    {
    }

void
KPilotInstaller::quit()
{
	FUNCTIONSETUP;

	for(fPilotComponentList.first(); 
		fPilotComponentList.current(); 
		fPilotComponentList.next())
	{
		fPilotComponentList.current()->saveData();
	}

	if(fKillDaemonOnExit && fPilotCommandSocket &&
		(fPilotCommandSocket->socket()>=0))
	{
			kdDebug() << fname
				<< ": Killing daemon."
				<< endl;

		ofstream out(fPilotCommandSocket->socket());
		out << -3 << endl;
	}


	if (fPilotStatusSocket) delete fPilotStatusSocket;
	fPilotStatusSocket=0L;

	if (fPilotCommandSocket) delete fPilotCommandSocket;
	fPilotCommandSocket=0L;

	kapp->quit();
}

void
KPilotInstaller::addComponentPage(PilotComponent *p, const QString &name)
{
	FUNCTIONSETUP;

	if (!p)
	{
		kdWarning() << __FUNCTION__
			<< ": Adding NULL component?"
			<< endl;
		return;
	}

	DEBUGKPILOT << fname
		<< ": Adding component @"
		<< (int) p
		<< " called " 
		<< p->name("(none)")
		<< endl;

	p->initialize();
	fPilotComponentList.append(p);

	// The first component added gets id 1, while the title
	// screen -- added elsewhere -- has id 0.
	//
	fManagingWidget->addWidget(p,fPilotComponentList.count());


	const char *componentname = p->name("(none)");
	char *actionname = 0L;
	if (strncmp(componentname,"component_",10)==0)
	{
		actionname = new char[strlen(componentname)-10+8];
		strcpy(actionname,"view_");
		strcat(actionname,componentname+10);
	}
	else
	{
		actionname = new char[8+strlen(componentname)];
		strcpy(actionname,"view_");
		strcat(actionname,componentname);
	}

	DEBUGKPILOT << fname
		<< ": Using component action name "
		<< name
		<< " for "
		<< actionname
		<< endl;

	KToggleAction *pt =
		new KToggleAction(name, /* "kpilot" -- component icon, */  0,
			p, SLOT(slotShowComponent()),
			actionCollection(),actionname);
	pt->setExclusiveGroup("view_menu");

	connect(p,SIGNAL(showComponent(PilotComponent *)),
		fManagingWidget,SLOT(raiseWidget(QWidget *)));
}



void KPilotInstaller::optionsShowStatusbar()
{
	if (m_statusbarAction->isChecked())
	{
		statusBar()->show();
	}
	else
	{
		statusBar()->hide();
	}

	kapp->processEvents();
	resizeEvent(0);
}


void KPilotInstaller::optionsShowToolbar()
{
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
    KKeyDialog::configureKeys(actionCollection(), xmlFile());
}

void KPilotInstaller::optionsConfigureToolbars()
{
	// use the standard toolbar editor
	KEditToolbar dlg(actionCollection());

	if (dlg.exec())
	{
		// recreate our GUI
		createGUI();
	} 
}


void
KPilotInstaller::slotConfigureKPilot()
{
	FUNCTIONSETUP;

	KPilotOptions* options = 0L;

	// Display the (modal) options page.
	//
	//
	showTitlePage();
	options = new KPilotOptions(this);
	if (options==NULL)
	{
		kdError() << __FUNCTION__ << 
			": Can't allocate KPilotOptions object\n";
		return;
	}

		kdDebug() << fname << ": Running options dialog." 
			<< endl;
	options->exec();
		kdDebug() << fname << ": dialog result "
			<< options->result() << endl;

	if (options->result())
	{
			kdDebug() << fname 
				<< ": Updating link." << endl;

		readConfig(KPilotConfig::getConfig());

		// Update the link to reflect new settings.
		//
		//
		destroyPilotLink(); // Get rid of the old one
		initPilotLink(); // make a new one..

		// Update each installed component.
		//
		//
		for(fPilotComponentList.first(); 
		fPilotComponentList.current();
		fPilotComponentList.next())
		{
				kdDebug() << fname 
					<< ": Updating components." 
					<< endl;

			fPilotComponentList.current()->initialize();
		}
	}

	delete options;
	options=NULL;
		kdDebug() << fname << ": Done with options." << endl;
}

void
KPilotInstaller::slotConfigureConduits()
{
	FUNCTIONSETUP;

	CConduitSetup* conSetup = 0L;

	showTitlePage();
	conSetup = new CConduitSetup(this);
	conSetup->exec();
	delete conSetup;
}


void 
KPilotInstaller::slotSyncDone(KProcess*)
{
	FUNCTIONSETUP;

  fStatusBar->changeItem(i18n("Updating display..."), 0);
  kapp->processEvents();
  for(fPilotComponentList.first(); fPilotComponentList.current(); fPilotComponentList.next())
    fPilotComponentList.current()->postHotSync();
  fStatusBar->changeItem(i18n("Hot-Sync complete."),0);
}

/* virtual */ ASYNC KPilotInstaller::filesChanged()
{
	FUNCTIONSETUP;

	fFileInstallWidget->refreshFileInstallList();
}

 
/* static */ const char *KPilotInstaller::version(int kind)
{
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
static KCmdLineOptions kpilotoptions[] =
{
	{ "setup", I18N_NOOP("Setup the Pilot device and other parameters"),0L },
	{ "cs", I18N_NOOP("Run conduit setup"),0L },
#ifdef DEBUG
	{ "debug <level>", I18N_NOOP("Set debug level to <level> (try 1023)"),"0" },
#endif
	{ 0,0,0 }
} ;




// "Regular" mode == 0
// setup mode == 's'
// setup forced by config change == 'S'
// conduit setup == 'c'
//
// This is only changed by the --setup flag --
// kpilot still does a setup the first time it is run.
//
//
int run_mode=0;
			 

int main(int argc, char** argv)
{
	FUNCTIONSETUP;

        KAboutData about("kpilot", I18N_NOOP("KPilot"),
                         KPILOT_VERSION,
                         "KPilot - Hot-sync software for unix\n\n",
                         KAboutData::License_GPL,
                         "(c) 1998-2000, Dan Pilone");
	about.addAuthor("Dan Pilone",
		I18N_NOOP("Project Leader"),
		"pilone@slac.com",
		"http://www.slac.com/pilone/kpilot_home/");
	about.addAuthor("Adriaan de Groot",
		I18N_NOOP("Maintainer"),
		"adridg@cs.kun.nl",
		"http://www.cs.kun.nl/~adridg/kpilot/");
	about.addAuthor("Preston Brown",
		I18N_NOOP("VCal conduit"));
	about.addAuthor("Greg Stern",
		I18N_NOOP("Abbrowser conduit"));
	about.addAuthor("Chris Molnar",
		I18N_NOOP("Expenses conduit"));
	about.addAuthor("Heiko Purnhagen",
		I18N_NOOP("Bugfixer"));
	about.addAuthor("Joerg Habenicht",
		I18N_NOOP("Bugfixer"));
	about.addAuthor("Martin Junius",
		I18N_NOOP("XML GUI"),
		"mj@m-j-s.net", "http://www.m-j-s.net/kde/");



        KCmdLineArgs::init(argc, argv, &about);
	KCmdLineArgs::addCmdLineOptions(kpilotoptions);
	KUniqueApplication::addCmdLineOptions();
	KCmdLineArgs *p=KCmdLineArgs::parsedArgs();

#ifdef DEBUG
	debug_level=atoi(p->getOption("debug"));
#endif
	if (p->isSet("setup")) { run_mode='s'; } 
	if (p->isSet("cs")) { run_mode='c'; }

	if (!KUniqueApplication::start())
	{
		return 0;
	}
	KUniqueApplication a(true,true);

	KConfig& c=KPilotConfig::getConfig();
	(void)KPilotConfig::getDebugLevel(c);
	if (KPilotConfig::getConfigVersion(c)<KPilotConfig::ConfigurationVersion)
	{
		run_mode='S';
	}

	if (run_mode=='c')
	{
		CConduitSetup *cs = new CConduitSetup(0L);
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

	if ((run_mode=='s') || (run_mode=='S'))
	{
#ifdef DEBUG
			kdDebug() << fname << ": Running setup first."
				<< " (mode " << run_mode << ")"
				<< endl ;
#endif

		KPilotOptions* options = new KPilotOptions(0L);
		int r = options->exec();

		if (run_mode=='s')
		{
			if (r)
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}

		if (r) return 1;
			

		// The options dialog may have changed the group
		// while reading or writing settings (still a
		// bad idea, actually).
		//
		c.setGroup(QString::null);
	}

	if (KPilotConfig::getConfigVersion(c)<KPilotConfig::ConfigurationVersion)
	{
		kdWarning() << __FUNCTION__ << ": Is still not configured for use."
			<< endl;
		return 1;
	}

        KPilotInstaller *tp = new KPilotInstaller();

	if (tp->status() == KPilotInstaller::Error)
	{
		delete tp;
		tp=0;
		return 1;
	}

        KGlobal::dirs()->addResourceType("pilotdbs", "share/apps/kpilot/DBBackup");
	a.setMainWidget(tp);
	return a.exec();
}


// $Log$
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
