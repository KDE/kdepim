// kpilot.cc
//
// Copyright (C) 1998,1999 Dan Pilone
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 

// $Revision$
static const char *id="$Id$";


// REVISION HISTORY 
//
// 3.1b9	By Dan Pilone
// 3.1b10	By Adriaan de Groot: comments added all over the place,
//		added debug-level variable. The error messages sent to
//		the user still need work. Fixed socket bugs thanks
//		to Robert Ambrose. Added dependency on pilotDaemon.h
//		to ensure compatibility.
//



#include <sys/types.h>
#include <dirent.h>
#include <iostream.h>
#include <fstream.h>
#include <qfile.h>
#include <qlist.h>
#include <qstring.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <kurl.h>
#include <kmsgbox.h>
#include <kstatusbar.h>
#include <qlistbox.h>
#include <qcombobox.h>

#include <kwm.h>
#include <kpilotOptions.h>
#include <kprocess.h>
#include "kpilot.moc"
#include "kpilotlink.h"
#include "messageDialog.h"
#include "addressWidget.h"
#include "kpilot_on_pp.h"
#include "hotsync.h"
#include "statusMessages.h"
#include "conduitSetup.h"
#include "pilotDaemon.h"

int debug_level=NO_DEBUG;

const int KPilotInstaller::ID_FILE_QUIT = 1;
const int KPilotInstaller::ID_FILE_SETTINGS = 2;
const int KPilotInstaller::ID_FILE_BACKUP = 3;
const int KPilotInstaller::ID_FILE_RESTORE = 4;
const int KPilotInstaller::ID_HELP_HELP = 5;
const int KPilotInstaller::ID_HELP_ABOUT = 6;
const int KPilotInstaller::ID_CONDUITS_ENABLE = 7;
const int KPilotInstaller::ID_CONDUITS_SETUP = 8;
// Components that go in the "Conduits" menu -- ie.
// address conduit, memo conduit, and file installer --
// get a menu id derived from ID_COMBO+n, where n is
// the number of installed components.
// This is why we choose ID_COMBO a long way
// away from ther ids: so that we have lots
// of ids free for the components.
// Remember to catch this in the menu handler.
const int KPilotInstaller::ID_COMBO = 1000;

KPilotInstaller::KPilotInstaller()
  : KTopLevelWidget(), fMenuBar(0L), fStatusBar(0L), fToolBar(0L), 
    fQuitAfterCopyComplete(false), fManagingWidget(0L), fPilotLink(0L),
    fPilotCommandSocket(0L), fPilotStatusSocket(0L), fKillDaemonOnExit(false)
    {
	FUNCTIONSETUP;

    KConfig* config = kapp->getConfig();
    if(config->readNumEntry("Configured", 0) != 3)
	{
	// If we haven't been configured, force the user to set things up.
	config->writeEntry("Configured", 3);
	KPilotOptions* options = new KPilotOptions(this);
	options->show();
	delete options;
	}
    if(config->readNumEntry("NextUniqueID", 0) == 0)
      {
	// Is this an ok value to use??
	config->writeEntry("NextUniqueID", 0xf000);
	config->sync();
      }
    initPilotLink();
    setupWidget();
    initComponents();
    initStatusLink();  // This is separate to allow components to initialize
    showTitlePage();
    show();
    }

KPilotInstaller::~KPilotInstaller()
    {
	FUNCTIONSETUP;

      delete fPilotStatusSocket;
      if(fKillDaemonOnExit)
	{
	  ofstream out(fPilotCommandSocket->socket());
	  out << -3 << endl;
	}
//      if(fMenuBar)
//    	delete fMenuBar;
//      if(fStatusBar)
//    	delete fStatusBar;
//      if(fToolBar)
//    	delete fToolBar;
    }

void
KPilotInstaller::setupWidget()
    {
	FUNCTIONSETUP;

    KWM::setIcon(winId(), kapp->getIcon());
    setCaption("KPilot");
    setMinimumSize(500,405);
    setMaximumSize(500,405);
    initMenu();
    initStatusBar();
    initToolBar();
    fManagingWidget = new QWidget(this);
    fManagingWidget->setMinimumSize(500,330);
    fManagingWidget->show();
    setView(fManagingWidget, FALSE);
    }

void
KPilotInstaller::initComponents()
    {
	FUNCTIONSETUP;

    PilotComponent* aComponent;
    
    QLabel* titleScreen = new QLabel(getManagingWidget());
    titleScreen->setPixmap(QPixmap(kpilot_on_pp));
    titleScreen->setAlignment(AlignCenter);
    titleScreen->setBackgroundColor(QColor("black"));
    titleScreen->setGeometry(0, 0, getManagingWidget()->geometry().width(), 
			           getManagingWidget()->geometry().height());
    fVisibleWidgetList.append(titleScreen);
    
    aComponent = new MemoWidget(this, getManagingWidget());
    aComponent->initialize();
    fPilotComponentList.append(aComponent);
    aComponent = new AddressWidget(this, getManagingWidget());
    aComponent->initialize();
    fPilotComponentList.append(aComponent);
    aComponent = new FileInstallWidget(this, getManagingWidget());
    aComponent->initialize();
    fPilotComponentList.append(aComponent);
    }

void 
KPilotInstaller::initStatusBar()
{
	FUNCTIONSETUP;
	QString welcomeMessage=i18n("Welcome to KPilot");
	welcomeMessage+=" (";
	welcomeMessage+=version(0);
	welcomeMessage+=')';

	fStatusBar = new KStatusBar(this, "statusbar");
	fStatusBar->insertItem(welcomeMessage,0);
	fStatusBar->show();
	setStatusBar(fStatusBar);
}

void
KPilotInstaller::initToolBar()
  {
	FUNCTIONSETUP;

  fToolBar = new KToolBar(this, "toolbar");
  QPixmap icon(hotsync_icon);

  fToolBar->insertButton(icon, 0, SIGNAL(clicked()), this, SLOT(doHotSync()),
			 TRUE, "Hot-Sync");
  icon.load(kapp->kde_toolbardir() + "/exit.xpm");
  
  fToolBar->insertButton( icon, 0, SIGNAL(clicked()), this, SLOT(quit()),
			  TRUE, "Quit");

	conduitCombo=new QComboBox(fToolBar,"conduitCombo");
	conduitCombo->insertItem(i18n("Pilot Application"));
	fToolBar->insertWidget(KPilotInstaller::ID_COMBO,140,conduitCombo,0);
	connect(conduitCombo,SIGNAL(activated(int)),
		this,SLOT(slotModeSelected(int)));
	/*
	fToolBar->insertCombo(i18n("Pilot Application"), 
		KPilotInstaller::ID_COMBO, false, 
		SIGNAL(activated(int)), this, 
		SLOT(slotModeSelected(int)), true, 
		i18n("KPilot Mode"), 140, 0);
	*/

  // This only works after Beta 4.. 
  fToolBar->alignItemRight(KPilotInstaller::ID_COMBO);
  addToolBar(fToolBar);
  }


void
KPilotInstaller::slotModeSelected(int selected)
{
	FUNCTIONSETUP;

	if (debug_level>TEDIOUS)
	{
		cerr << fname << ": Responding to callback " << selected
			<< endl;
	}

	if((unsigned int)selected >= fVisibleWidgetList.count())
	{
		cerr << fname << ": Illegal component #" 
			<< selected << " selected.\n" << endl;
		return;
	}


	for (fVisibleWidgetList.first();
		fVisibleWidgetList.current(); 
		fVisibleWidgetList.next())
	{
		fVisibleWidgetList.current()->hide();
	}
		
	fVisibleWidgetList.at(selected)->show();
}

void KPilotInstaller::showTitlePage()
{
	FUNCTIONSETUP;

	slotModeSelected(0);
	conduitCombo->setCurrentItem(0);
}

	



void
KPilotInstaller::initPilotLink()
{
	FUNCTIONSETUP;
	MessageDialog *messageDialog=NULL;

	if(fPilotLink)
	{
		cerr << fname << ": Pilot Link already created?\n" ;
		return;
	}

	fPilotLink = new KPilotLink();
	if (fPilotLink==NULL)
	{
		cerr << fname << ": Can't allocate fPilotLink.\n";
		KMsgBox::message(this,
			klocale->translate("Cannot create link to Daemon"),
			klocale->translate("Allocating PilotLink failed."),
			KMsgBox::STOP);
		return;
	}

	// There's no reference to kconfig in this function
	//
	// KConfig* config = kapp->getConfig();
	// config->setGroup(0L);

if(fPilotCommandSocket == NULL)
{
	fPilotCommandSocket = new KSocket("localhost", 
		PILOTDAEMON_COMMAND_PORT);
	if (fPilotCommandSocket==NULL)
	{
		cerr << fname << ": Can't allocate fPilotCommandSocket.\n";
		KMsgBox::message(this,
			klocale->translate("Cannot create link to Daemon"),
			klocale->translate(
				"Allocating fPilotCommandSocket failed."),
			KMsgBox::STOP);

		delete fPilotLink;
		fPilotLink=NULL;
		return;
	}

	if(fPilotCommandSocket->socket() < 0)
	{
		// It wasn't running...
		messageDialog = 
			new MessageDialog(version(0));

		if (messageDialog!=NULL)
		{
			messageDialog->setMessage(
				klocale->translate("Starting Sync Daemon. "
					"Please Wait."));

			messageDialog->show();
			kapp->processEvents();
		}

		delete fPilotCommandSocket;
		fPilotCommandSocket=NULL;
	

		KProcess pilotDaemon;
		pilotDaemon << "kpilotDaemon";
		pilotDaemon.start(KProcess::DontCare);
		sleep(4);

		fPilotCommandSocket = new KSocket("localhost", 
			PILOTDAEMON_COMMAND_PORT);
		if(fPilotCommandSocket->socket() < 0)
		{
			KMsgBox::message(this, 
				klocale->translate("Cannot connect to Daemon"), 
				klocale->translate("Cannot start KPilot "
					"Daemon.  Check settings."
					"Restart KPilot manually."), 
				KMsgBox::STOP);

			KPilotOptions* options = new KPilotOptions(this);
			options->show(); // It's modal..
			delete options;
	// if(KMsgBox::yesNo(0L, klocale->translate("Restart?"), 
	//	klocale->translate("Restart KPilot?")) == 2)
	// {
	//	exit(1);
	// }
	// else
	// {
	// 	execl("kpilot", "kpilot", 0);
	// 	exit(1);
	// }
		}

		fKillDaemonOnExit = true;

		if (messageDialog!=NULL)
		{
			messageDialog->hide();
			delete messageDialog;
		}
	}
	fLinkCommand[0] = 0L;
}
	else 
	{
		// We were called after a reconfigure
		//
		//
		if (debug_level)
		{
			cerr << fname << ": Reconfiguring daemon.\n" ;
		}

		ofstream out(fPilotCommandSocket->socket());
		out << "-2" << endl;
	}
}
    
void
KPilotInstaller::initStatusLink()
{
	FUNCTIONSETUP;

	fPilotStatusSocket = new KSocket("localhost", 
		PILOTDAEMON_STATUS_PORT);
	if (fPilotStatusSocket->socket()!=-1)
	{
		if (debug_level>TEDIOUS)
		{
			cerr << fname <<
				": Connected socket successfully.\n";
		}

		connect(fPilotStatusSocket, SIGNAL(readEvent(KSocket*)),
			this, SLOT(slotDaemonStatus(KSocket*)));
		fPilotStatusSocket->enableRead(true);
	}
	else
	{
		cerr << fname <<
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
  if(status == CStatusMessages::SYNC_STARTING)
    {
      fLastWidgetSelected = fToolBar->getCombo(KPilotInstaller::ID_COMBO)->currentItem();
      fToolBar->getCombo(KPilotInstaller::ID_COMBO)->setCurrentItem(0);
      slotModeSelected(0);
      fToolBar->getCombo(KPilotInstaller::ID_COMBO)->setEnabled(false);
      if(fLinkCommand[0] == 0L)
	doHotSync();
      fStatusBar->changeItem(klocale->translate("Hot-Sync in progress..."), 0);
      ofstream out(fPilotCommandSocket->socket());
      out << fLinkCommand << flush;
    }
  else if(status == CStatusMessages::SYNC_COMPLETED)
    {
      fToolBar->getCombo(KPilotInstaller::ID_COMBO)->setEnabled(true);
      fToolBar->getCombo(KPilotInstaller::ID_COMBO)->setCurrentItem(fLastWidgetSelected);
      slotModeSelected(fLastWidgetSelected);
      fStatusBar->changeItem(klocale->translate("Hot-Sync complete."), 0);
      fLinkCommand[0] = 0L;
      for(fPilotComponentList.first(); fPilotComponentList.current(); fPilotComponentList.next())
	//fPilotComponentList.current()->postHotSync();
	fPilotComponentList.current()->initialize();
    }
  else if(status == CStatusMessages::FILE_INSTALL_REQUEST)
    {
      for(fPilotComponentList.first(); fPilotComponentList.current(); fPilotComponentList.next())
	fPilotComponentList.current()->initialize();
    }
}

void
KPilotInstaller::doBackup()
{
	FUNCTIONSETUP;

  fStatusBar->changeItem(klocale->translate("Backing up pilot. Please press the hot-sync button."), 0);
  sprintf(fLinkCommand, "%d\n", KPilotLink::Backup);
}

void
KPilotInstaller::doRestore()
{
	FUNCTIONSETUP;

  fStatusBar->changeItem(klocale->translate("Restoring pilot. Please press the hot-sync button."), 0);
  sprintf(fLinkCommand, "%d\n", KPilotLink::Restore);
}
  
void
KPilotInstaller::doHotSync()
{
	FUNCTIONSETUP;

	showTitlePage();
	fStatusBar->changeItem(i18n(
		"Hot-Syncing.  "
		"Please press the hot-sync button."), 0);
	if (fLinkCommand[0] == 0)
	{
		sprintf(fLinkCommand, "%d\n", KPilotLink::HotSync);
		for(fPilotComponentList.first(); 
			fPilotComponentList.current(); 
			fPilotComponentList.next())
		{
			if (debug_level & SYNC_MINOR)
			{
				cerr << fname << ": Pre-sync for builtins."
					<< endl;
			}
			fPilotComponentList.current()->preHotSync(fLinkCommand);
		}
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

    QPopupMenu* fileMenu = new QPopupMenu;
    fileMenu->insertItem(klocale->translate("&Settings"), KPilotInstaller::ID_FILE_SETTINGS);
    fileMenu->insertSeparator(-1);
    fileMenu->insertItem(klocale->translate("&Backup"), KPilotInstaller::ID_FILE_BACKUP);
    fileMenu->insertItem(klocale->translate("&Restore"), KPilotInstaller::ID_FILE_RESTORE);
    fileMenu->insertSeparator(-1);
    fileMenu->insertItem(klocale->translate("&Quit"), KPilotInstaller::ID_FILE_QUIT);
    connect(fileMenu, SIGNAL (activated(int)), SLOT (menuCallback(int)));
    
	conduitMenu = new QPopupMenu;
	conduitMenu->insertItem(i18n("&External"), 
		KPilotInstaller::ID_CONDUITS_SETUP);
	conduitMenu->insertSeparator(-1);
	connect(conduitMenu, SIGNAL(activated(int)), 
		SLOT(menuCallback(int)));

	QPopupMenu *helpMenu = kapp->getHelpMenu(true,
		QString(version(0)) +
		i18n("\n\nCopyright (C) 1998-2000 Dan Pilone") +
		i18n("\n\nProgramming by:\n") +
		authors()) ;
    
    this->fMenuBar = new KMenuBar(this);
    this->fMenuBar->insertItem(klocale->translate("&File"), fileMenu);
    this->fMenuBar->insertItem(klocale->translate("&Conduits"), conduitMenu);
    this->fMenuBar->insertSeparator();
    this->fMenuBar->insertItem(klocale->translate("&Help"), helpMenu);
    this->fMenuBar->show();
    setMenu(this->fMenuBar);
    }

void
KPilotInstaller::fileInstalled(int )
    {
    }

void
KPilotInstaller::quit()
    {
	FUNCTIONSETUP;

    for(fPilotComponentList.first(); fPilotComponentList.current(); fPilotComponentList.next())
	fPilotComponentList.current()->saveData();
    kapp->quit();
    }

// Adds 'name' to the pull down menu of components
void
KPilotInstaller::addComponentPage(QWidget* widget, QString name)
{
	FUNCTIONSETUP;

	/*
	fToolBar->insertComboItem(KPilotInstaller::ID_COMBO, name,
		fVisibleWidgetList.count());
	*/
	conduitCombo->insertItem(name,fVisibleWidgetList.count());
	conduitMenu->insertItem(name,
		KPilotInstaller::ID_COMBO+fVisibleWidgetList.count());
	fVisibleWidgetList.append(widget);
}

void KPilotInstaller::menuCallback(int item)
{
	FUNCTIONSETUP;

	KPilotOptions* options = 0L;
	CConduitSetup* conSetup = 0L;

	if (debug_level>TEDIOUS)
	{
		cerr << fname << ": Responding to callback " << item
			<< endl;
	}

	if ((item>=ID_COMBO) && 
		(item<ID_COMBO+(int)fVisibleWidgetList.count()))
	{
		conduitCombo->setCurrentItem(item-ID_COMBO);
		slotModeSelected(item-ID_COMBO);
		return;
	}
		
	switch(item)
	{
	case KPilotInstaller::ID_HELP_ABOUT:
		KMsgBox::message(0L, version(0), 
			i18n("Hot-Sync Software for Unix\n"
				"By: Dan Pilone\n"
				"Email: pilone@slac.com"), 
			KMsgBox::INFORMATION);
		break;

	case KPilotInstaller::ID_HELP_HELP:
		kapp->invokeHTMLHelp("kpilot/index.html", "");
		break;
	case KPilotInstaller::ID_FILE_QUIT:
		quit();
		break;
	case KPilotInstaller::ID_FILE_SETTINGS:
		// Display the (modal) options page.
		//
		//
		showTitlePage();
		options = new KPilotOptions(this);
		if (options==NULL)
		{
			cerr << fname << 
				": Can't allocate KPilotOptions object\n";
			break;
		}

		if (debug_level>UI_ACTIONS)
		{
			cerr << fname << ": Running options dialog." 
			<< endl;
		}
		options->show();
		if (debug_level>UI_ACTIONS)
		{
			cerr << fname << ": dialog result "
			<< options->result() << endl;
		}

		if (options->result())
		{
			if (debug_level>TEDIOUS)
			{
				cerr << fname << ": Updating link." << endl;
			}

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
				if (debug_level>TEDIOUS)
				{
					cerr << fname 
						<< ": Updating components." 
						<< endl;
				}

				fPilotComponentList.current()->initialize();
			}
		}

		delete options;
		options=NULL;
		if (debug_level>UI_ACTIONS)
		{
			cerr << fname << ": Done with options." << endl;
		}
		break;

	case KPilotInstaller::ID_FILE_BACKUP:
		showTitlePage();
		doBackup();
		break;
	case KPilotInstaller::ID_FILE_RESTORE:
		showTitlePage();
		doRestore();
		break;
	case KPilotInstaller::ID_CONDUITS_SETUP:
		showTitlePage();
		conSetup = new CConduitSetup(this);
		conSetup->show();
		delete conSetup;
		break;
	}

	if (debug_level>TEDIOUS)
	{
		cerr << fname << ": Done responding to item " << item
			<< endl;
	}
}

void 
KPilotInstaller::slotSyncDone(KProcess*)
{
	FUNCTIONSETUP;

  fStatusBar->changeItem(klocale->translate("Updating display..."), 0);
  kapp->processEvents();
  for(fPilotComponentList.first(); fPilotComponentList.current(); fPilotComponentList.next())
    fPilotComponentList.current()->postHotSync();
  fStatusBar->changeItem(klocale->translate("Hot-Sync complete."),0);
}

void 
KPilotInstaller::testDir(QString name)
    {
	FUNCTIONSETUP;

    DIR *dp;
    dp = opendir(name);
    if(dp == 0L)
	::mkdir (name, S_IRWXU);
    else
	closedir( dp );
    }
    
/* static */ const char *KPilotInstaller::version(int kind)
{
	// I don't think the program title needs to be translated. (ADE)
	//
	//
	if (kind) return id;
	else return "KPilot v3.1b12";
}

static char authorsbuf[256]={0};
/* static */ const char *KPilotInstaller::authors()
{
	if (!authorsbuf[0])
	{
		sprintf(authorsbuf,"%s%s",
			"Dan Pilone, Adriaan de Groot\n",
			i18n("and many others."));
	}

	return authorsbuf;
}

int main(int argc, char** argv)
{
	FUNCTIONSETUP;

	// QStrList fileList;

	// This is an ugly hack - --debug must be the first argument.
	// But until KApplication supports general --debug flags, we'll
	// have to work this way.
	//
	//
	if (argv[1]!=NULL && strcmp(argv[1],"--debug")==0 && argv[2]!=NULL)
	{
		debug_level=atoi(argv[2]);
		if (debug_level)
		{
			cerr << fname << ":Debug level set to " << 
				debug_level << '\n' ;
		}
	}

	KApplication a(argc, argv, "kpilot");
	KPilotInstaller *tp = new KPilotInstaller();

	a.setMainWidget(tp);
	return a.exec();
}

