// pilotDaemon.cc
//
// Copyright (C) 1998,1999 Dan Pilone
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 



// REVISION HISTORY 
//
// 3.1b10	By Adriaan de Groot: comments added all over the place,
//		moved the definition of the port numbers to the .h file
//		so others can read them without linking to pilotDaemon.o.
//
//		Remaining questions are marked with QADE.

static const char *id="$Id$";

#include "options.h"

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream.h>
#include <fstream.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>

#include <qdir.h>
#include <qlist.h>
#include <qcursor.h>
#include <qdragobject.h>
#include <errno.h>

#include <kapp.h>
#include <kaboutdata.h>
#include <kaboutapplication.h>
#include <kcmdlineargs.h>
#include <kwin.h>
#include <ksimpleconfig.h>
#include <kurl.h>
#include <ksock.h>
#include <kmessagebox.h>
#include <kstddirs.h>
#include <kpopupmenu.h>
#include <kiconloader.h>
#include <kio/netaccess.h>
#include <kdebug.h>

#include "pilotDaemon.moc"
#include "hotsync.h"
#include "busysync.h"
#include "statusMessages.h"
#include "kpilot.h"

PilotDaemonTray::PilotDaemonTray(PilotDaemon *p) :
	KSystemTray(0,"pilotDaemon"), 
	daemon(p)
{
	FUNCTIONSETUP;
	setupWidget();
}

/* virtual */ void PilotDaemonTray::mousePressEvent(QMouseEvent *e)
{
  FUNCTIONSETUP;
  if( e->button() == RightButton)
    {
      KPopupMenu *menu = contextMenu();
      contextMenuAboutToShow( menu );
      menu->popup( e->globalPos() );
    }
    else
	{
		KSystemTray::mousePressEvent(e);
	}
}

/* virtual */ void PilotDaemonTray::closeEvent(QCloseEvent *e)
{
	FUNCTIONSETUP;
	(void) e;
	daemon->quitImmediately();
}

void
PilotDaemonTray::setupWidget()
{
	FUNCTIONSETUP;

	KGlobal::iconLoader()->addAppDir("kpilot");
	icon = KGlobal::iconLoader()->loadIcon("hotsync",KIcon::Toolbar,
		0,KIcon::DefaultState,0,
		true);
	busyicon = KGlobal::iconLoader()->loadIcon("busysync",KIcon::Toolbar,
		0,KIcon::DefaultState,0,true);
	if (icon.isNull())
	{
		kdDebug() << fname << ": HotSync icon not found."
			<< endl;
		icon=QPixmap(hotsync_icon);
	}
	if (busyicon.isNull())
	{
		kdDebug() << fname << ": HotSync-Busy icon not found."
			<< endl;
		busyicon=QPixmap(busysync_icon);
	}

	changeIcon(Normal);

	KPopupMenu* menu = contextMenu();
	menu->insertItem(i18n("&About"), this, SLOT(slotShowAbout()));
	menuKPilotItem=menu->insertItem(i18n("Start &KPilot"),daemon,
		SLOT(slotRunKPilot()));

#ifdef DEBUG
	if (debug_level & UI_TEDIOUS)
	{
		kdDebug() << fname
			<< ": Finished getting icons"
			<< endl;
	}
#endif
}

void PilotDaemonTray::slotShowAbout()
{
	FUNCTIONSETUP;
  
	if (!kap)
	{
		kap=new KAboutApplication(0,"kpdab",false);
	}

	kap->show();
}


void PilotDaemonTray::enableRunKPilot(bool b)
{
	contextMenu()->setItemEnabled(menuKPilotItem,b);
}


void PilotDaemonTray::changeIcon(IconShape i)
{
	FUNCTIONSETUP;

	switch(i)
	{
	case Normal:
		setPixmap(icon);
		break;
	case Busy:
		setPixmap(busyicon);
		break;
	default :
		kdDebug() << fname 
			<< ": Bad icon number "
			<< (int)i
			<< endl;
	}
}







static KCmdLineOptions kpilotoptions[] =
{
	{ "debug <level>", I18N_NOOP(""),"0" },
	{ 0,0,0 }
} ;



int PilotDaemon::getPilotSpeed(KConfig& config)
{
	FUNCTIONSETUP;

	int speed = config.readNumEntry("PilotSpeed", 0);

	// Translate the speed entry in the
	// config file to something we can
	// put in the environment (for who?)
	//
	//
	const char *speedname=0L;

	switch(speed)
	{
	case 0:
		speedname="PILOTRATE=9600";
		break;
	case 1:
		speedname="PILOTRATE=19200";
		break;
	case 2:
		speedname="PILOTRATE=38400";
		break;
	case 3:
		speedname="PILOTRATE=57600";
		break;
	case 4:
		speedname="PILOTRATE=115200";
		break;
	default :
		speedname="PILOTRATE=9600";
	}

	if (debug_level & SYNC_MINOR)
	{
		cerr << fname
			<< ": Speed set to "
			<< speedname << " ("
			<< speed << ')'
			<< endl;
	}

	putenv((char *)speedname);

	return speed;
#ifdef DEBUG
	/* NOTREACHED */
	(void) id;
#endif
}


PilotDaemon::PilotDaemon() : 
	fStatus(INIT),
  	fMonitorProcess(0L), fCurrentSocket(0L),
    fCommandSocket(0L), fStatusSocket(0L), fQuit(false), fPilotLink(0L),
	fStartKPilot(false), fWaitingForKPilot(false),
	tray(0L)
{
	FUNCTIONSETUP;

	KConfig& config = KPilotLink::getConfig();
  
	fPilotDevice = config.readEntry("PilotDevice");


	getPilotSpeed(config);

	fStartKPilot = (bool) config.readNumEntry("StartKPilotAtHotSync", 0);
	fStatusConnections.setAutoDelete(true);

	setupConnections();
	if (fStatus == ERROR) 
	{
#ifdef DEBUG
		if (debug_level & SYNC_MAJOR)
		{
			kdDebug() << fname
				<< ": Setting up connections failed."
				<< endl;
		}
#endif
		return;
	}
	setupSubProcesses();

	if (config.readBoolEntry("DockDaemon",false))
	{
		tray = new PilotDaemonTray(this);
	}

#ifdef DEBUG
	if (debug_level & UI_TEDIOUS)
	{
		kdDebug() << fname
			<< ": The daemon is ready with status "
			<< (int)fStatus
			<< endl;
	}
#endif
}

void PilotDaemon::showTray()
{
	if (!tray) return;

	// Copied from Klipper
	KWin::setSystemTrayWindowFor( tray->winId(), 0 );
	tray->setGeometry(-100, -100, 42, 42 );
	tray->show();
}

void
PilotDaemon::reloadSettings()
{
	FUNCTIONSETUP;

	KConfig& config = KPilotLink::getConfig();
  
	fPilotDevice = config.readEntry("PilotDevice");
	getPilotSpeed(config);
  fStartKPilot = (bool) config.readNumEntry("StartKPilotAtHotSync", 0);

	if (fMonitorProcess)
	{
		disconnect(fMonitorProcess, SIGNAL(processExited(KProcess*)),
			this, SLOT(slotProcFinished(KProcess*)));
		killMonitor();
	}
	else
	{
		cerr << fname << ": No listener to kill (which is OK)."
			<< endl;
	}

	fPilotLink->changePilotPath(QFile::encodeName(fPilotDevice));
	setupSubProcesses();
}

void
PilotDaemon::setupConnections()
{
	FUNCTIONSETUP;
	int e=0;

	fPilotLink = 0L;
	fCommandSocket = 0L;
	fStatusSocket = 0L;

	fPilotLink = new KPilotLink(0L, 0L, QFile::encodeName(fPilotDevice));
	fCommandSocket = new KServerSocket(PilotDaemon::COMMAND_PORT);
	if (fCommandSocket && (fCommandSocket->socket() < 0 )) 
	{
		e = errno ; 
		goto ErrConn;
	}
	fStatusSocket = new KServerSocket(PilotDaemon::STATUS_PORT);
	if (fStatusSocket && (fStatusSocket->socket() < 0 )) 
	{
		e = errno ;
		goto ErrConn;
	}

	if (!(fPilotLink && fCommandSocket && fStatusSocket)) goto ErrConn;

	connect(fPilotLink, SIGNAL(databaseSyncComplete()),
		this, SLOT(slotDBBackupFinished()));
	connect(this, SIGNAL(endHotSync()), this, SLOT(slotEndHotSync()));
	connect(fCommandSocket, SIGNAL(accepted(KSocket*)), 
		this, SLOT(slotAccepted(KSocket*)));
	connect(fStatusSocket, SIGNAL(accepted(KSocket*)),
		this, SLOT(slotAddStatusConnection(KSocket*)));

	return;

ErrConn:
	kdDebug() << fname
		<< ": Error creating socket for daemon: "
		<< strerror(e)
		<< endl;

	if (fPilotLink) delete fPilotLink;
	if (fCommandSocket) delete fCommandSocket;
	if (fStatusSocket) delete fStatusSocket;
	fPilotLink=0L;
	fCommandSocket=0L;
	fStatusSocket=0L;

	fStatus=ERROR;
}

void
PilotDaemon::setupSubProcesses()
{
	FUNCTIONSETUP;

	fMonitorProcess = new KProcess();
	if (fMonitorProcess)
	{
		*fMonitorProcess << "pilotListener" << fPilotDevice;
		connect(fMonitorProcess, SIGNAL(processExited(KProcess*)),
			this, SLOT(slotProcFinished(KProcess*)));
		fMonitorProcess->start(KProcess::NotifyOnExit);
	}
	else
	{
		cerr << fname << ": Can't start new listener process."
			<< endl;
	}
}


void PilotDaemon::killMonitor(bool finishsync)
{
	FUNCTIONSETUP;

	if (finishsync)
	{
		getPilotLink()->endHotSync();
	}

	KProcess *m=getMonitorProcess();
	if (m)
	{
		kdDebug() << fname << ": Killing monitor" << endl;
		m->kill();
	}

	delete fMonitorProcess;
	fMonitorProcess=0L;
}


void PilotDaemon::quitImmediately()
{
	FUNCTIONSETUP;
	killMonitor();
	quit(true);
	kapp->quit();
} 
 
void
PilotDaemon::startHotSync()
{
	FUNCTIONSETUP;


	if (tray) tray->changeIcon(PilotDaemonTray::Busy);

  // We need to send the SYNC_STARTING message after the sync
  // has already begun so that if KPilot is running it doesn't start
  // issuing commands before KPilotLink is ready.
  getPilotLink()->startHotSync();
  sendStatus(CStatusMessages::SYNC_STARTING);
	if (debug_level &  SYNC_MAJOR)
	{
		cerr << fname 
			<< ": Requesting KPilotLink::startHotSync()" << endl;
	}

  if(fCurrentSocket == 0L)
    {
	if (debug_level & SYNC_MINOR)
	{
		cerr << fname  <<
			": No KPilot running." << endl;
	}
		
      if(fStartKPilot) // We are supposed to start up kpilot..
	{
		if (debug_level & SYNC_MAJOR)
		{
			cerr << fname << ": Starting KPilot GUI." << endl;
		}

	  fWaitingForKPilot = true;
	  if (fork()==0)
	    {
	      execlp("kpilot", "kpilot", 0);
		cerr << fname << ": Failed to start KPilot." << endl;
	      exit(1);
	    }
	}
      else
	{
		if (debug_level & SYNC_MAJOR)
		{
			cerr  << fname << ": Starting quick sync." << endl;
		}
	  getPilotLink()->quickHotSync();
	}
    }
  // otherwise we just wait for input..
}

void
PilotDaemon::slotDBBackupFinished()
{
	FUNCTIONSETUP;

	if (debug_level & SYNC_MAJOR)
	{
		cerr << fname << ": DB Syncing finished." << endl;
	}

	KConfig& config = KPilotLink::getConfig();
	if(config.readNumEntry("SyncFiles"))
	{
	  getPilotLink()->installFiles(KGlobal::dirs()->saveLocation("data", QString("kpilot/pending_install/")));
	}
	emit(endHotSync());
}

void 
PilotDaemon::slotProcFinished(KProcess*)
{
	FUNCTIONSETUP;

	startHotSync();
}

void
PilotDaemon::slotEndHotSync()
{
	FUNCTIONSETUP;

	if (tray) { tray -> changeIcon(PilotDaemonTray::Normal); } 

	KPilotLink *p=getPilotLink();

	if (p)
	{
		if (debug_level & SYNC_MAJOR)
		{
			cerr << fname
				<< ": Ending hot-sync now"
				<< endl;
		}

		p->endHotSync();
	}
	else
	{
		cerr << fname
			<< ": No link to pilot!"
			<< endl;
	}

	sendStatus(CStatusMessages::SYNC_COMPLETED);

	if(!quit())
	{
		fMonitorProcess->start(KProcess::NotifyOnExit);
	}
	else
	{
		kapp->quit();
	}
}

void
PilotDaemon::slotAccepted(KSocket* connection)
{
	FUNCTIONSETUP;

	if(fCurrentSocket)
	{
		KMessageBox::error(0L, i18n("Error: Only one command connection at a time."),
				   i18n("Too Many Connections"));			       
		delete connection;
		return;
	}

	if (debug_level & SYNC_MAJOR)
	{
		cerr << fname 
			<< ": Accepted command connection "
			<< (int) connection
			<< endl;
	}

  fCurrentSocket = connection;
  connect(fCurrentSocket, SIGNAL(closeEvent(KSocket*)),
	  this, SLOT(slotConnectionClosed(KSocket*)));
  connect(fCurrentSocket, SIGNAL(readEvent(KSocket*)),
	  this, SLOT(slotCommandReceived(KSocket*)));
  fCurrentSocket->enableRead(true);

	if (tray) { tray->enableRunKPilot(false); } 
}

void
PilotDaemon::slotCommandReceived(KSocket*)
{
	FUNCTIONSETUP;

  ifstream in(fCurrentSocket->socket());
  int command;

  in >> command;

#ifdef DEBUG
	if (debug_level & SYNC_TEDIOUS)
	{
		kdDebug() << fname
			<< ": Received command "
			<< command
			<< endl;
	}
#endif

  switch(command)
    {
      // < 0 means commands to us.
    case -1: // Quit after this sync
      quit(true);
      return;
    case -2: // Reload settings
      reloadSettings();
      return;
    case -3: // Quit immediately
      quitImmediately();
      return;
    case KPilotLink::Backup:
      getPilotLink()->doFullBackup();
      break;
    case KPilotLink::Restore:
      getPilotLink()->doFullRestore();
      break;
    case KPilotLink::HotSync:
      if(getPilotLink()->slowSyncRequired())
	getPilotLink()->doFullBackup();
      else
	getPilotLink()->quickHotSync();
      break;
    case KPilotLink::InstallFile:
      getPilotLink()->installFiles(KGlobal::dirs()->saveLocation("data", QString("kpilot/pending_install/")));
      break;
	case KPilotLink::TestConnection :
		if (debug_level & SYNC_MAJOR)
		{
			cerr  << fname
				<< ": Connection tests OK"
				<< endl ;
		}
		break;
    default:
      cerr << fname << ": Unknown command!" << command << endl;
      break;
    }

	if (debug_level & SYNC_TEDIOUS)
	{
		cerr << fname
			<< ": Done reading." << endl;
	}
}

void
PilotDaemon::slotConnectionClosed(KSocket* connection)
{
	FUNCTIONSETUP;

	delete connection;
	if (fCurrentSocket != connection)
	{
		cerr << fname 
			<< ": Connection other than current was closed?"
			<< endl;
	}
	else
	{
		if (debug_level & SYNC_TEDIOUS)
		{
			cerr << fname 
				<< ": Connection "
				<< (int) connection
				<< " closed"
				<< endl;
		}

		fCurrentSocket = 0L;
		if (tray) { tray->enableRunKPilot(true); } 
	}
	emit(endHotSync());
}

void
PilotDaemon::slotAddStatusConnection(KSocket* connection)
{
	FUNCTIONSETUP;

	if (debug_level & SYNC_MAJOR)
	{
		cerr << fname 
			<< ": Accepted status connection "
			<< (int) connection
			<< endl;
	}
  fStatusConnections.append(connection);
  connect(connection, SIGNAL(closeEvent(KSocket*)),
	  this, SLOT(slotRemoveStatusConnection(KSocket*)));
	connection->enableRead(true);
  if(fWaitingForKPilot)
    {
      fWaitingForKPilot = false;
      sendStatus(CStatusMessages::SYNC_STARTING);
    }
}

void
PilotDaemon::slotRemoveStatusConnection(KSocket* connection)
{
	FUNCTIONSETUP;

	if (debug_level & SYNC_TEDIOUS)
	{
		cerr << fname 
			<< ": Connection "
			<< (int) connection
			<< " closed"
			<< endl;
	}

	fStatusConnections.remove(connection); // will be deleted by list.
}

void
PilotDaemon::sendRecord(PilotRecord* rec)
  {
  int len = rec->getLen();
  int attrib = rec->getAttrib();
  int cat = rec->getCat();
  recordid_t uid = rec->getID();
  char* data = rec->getData();
  
  for(fStatusConnections.first(); fStatusConnections.current(); fStatusConnections.next())
    {
      ofstream out(fStatusConnections.current()->socket());
      out.write(&len, sizeof(int));
      out.write(&attrib, sizeof(int));
      out.write(&cat, sizeof(int));
      out.write(&uid, sizeof(recordid_t));
      out.write(data, len);
    }  
  }


void 
PilotDaemon::slotSyncingDatabase(char* dbName)
{
	FUNCTIONSETUP;

  int len = strlen(dbName);

  sendStatus(CStatusMessages::SYNCING_DATABASE);
  for(fStatusConnections.first(); fStatusConnections.current(); fStatusConnections.next())
    {
      ofstream out(fStatusConnections.current()->socket());
      out.write(&len, sizeof(int));
      out.write(dbName, len);
    }  
}

void
PilotDaemon::sendStatus(const int status)
{
	FUNCTIONSETUP;

	KSocket *c;
	int updateCount=0;

	if (debug_level & SYNC_MINOR)
	{
		cerr << fname 
			<< ": Sending status " << status
			<< endl;
	}

	if (debug_level & SYNC_TEDIOUS)
	{
		for(fStatusConnections.first() ; 
			(c=fStatusConnections.current()) ; 
			fStatusConnections.next())
		{
			cerr << fname << ": Will send to connection "
				<< (int) c
				<< endl;
		}
	}

	for(fStatusConnections.first() ; 
		(c=fStatusConnections.current()) ; 
		fStatusConnections.next())
	{
		if (debug_level & SYNC_TEDIOUS)
		{
			cerr << fname
				<< ": Sending to connection "
				<< (int) c
				<< endl;
		}

		if (c->socket()<0)
		{
			cerr << fname
				<< ": Connection "
				<< (int) c 
				<< " has no valid socket"
				<< endl;
		}
		else
		{
			ofstream out(c->socket());
			out.write(&status, sizeof(int));
			updateCount++;
		}
	}

	if (debug_level & SYNC_MINOR)
	{
		cerr << fname
			<< ": Completed status update for "
			<< updateCount
			<< " connections"
			<< endl;
	}
}

void
PilotDaemon::saveProperties(KConfig&)
{
	FUNCTIONSETUP;

  disconnect(fMonitorProcess, SIGNAL(processExited(KProcess*)),
	  this, SLOT(slotProcFinished(KProcess*)));
  fMonitorProcess->kill();
}

PilotDaemon::~PilotDaemon()
{
	FUNCTIONSETUP;

  //  Should delete this but something goes apeshit
  disconnect(fMonitorProcess, SIGNAL(processExited(KProcess*)),
	     this, SLOT(slotProcFinished(KProcess*)));
  fMonitorProcess->kill();
//   delete fMonitorProcess;
  //   delete fSyncProcess;
  delete fPilotLink;
  delete fCommandSocket;
  delete fStatusSocket;
  
}

void PilotDaemon::slotRunKPilot()
{
	FUNCTIONSETUP;

	if (fCurrentSocket)
	{
		kdDebug() << fname 
			<< ": Only one KPilot at a time."
			<< endl;
		return;
	}

	KProcess *k=new KProcess();

	*k << "kpilot";
	if (debug_level)
	{
		*k << "--debug" ;
		*k << QString::number(debug_level);
	}

	k->start(KProcess::DontCare);
}

PilotDaemon* gPilotDaemon=0L;
int crashFlag=0;

void signalHandler(int s)
{
	FUNCTIONSETUP;


	crashFlag++;

	if (debug_level)
	{
		cerr << fname << ": Caught signal " << s << endl;
	}

	// Suppose the daemon crashes. We get here
	// with crashFlag=1. We set the alarm and it goes
	// off, which gets us here with crashFlag=2.
	// This second time around, there's no message,
	// we just try to kill the monitor. If something 
	// goes wrong *there* too, and we get back here
	// a *third* time, give up.
	//
	//
	if ( crashFlag>2 ) exit(-1);

	// Often popping up the message for the user
	// gets KApplication into a weird state so
	// that the user can't even exit the app
	// properly. We'll *force* the app to exit
	// by causing another signal soon.
	//
	//
	alarm(10);
	signal(SIGALRM,signalHandler);


	// If the user has already seen the message,
	// and we're coming here because the alarm goes
	// off, skip the message.
	//
	//
	if (crashFlag==1)
	{
		QString msg(
			i18n("Warning!  KPilotDaemon is shutting down."
				"  If this is not\nintentional, please send"
				" mail to kpilot-list@slac.com describing\n"
				"what happened.") ) ;

		msg += i18n(" This window will self-destruct in 10 seconds.");

		KMessageBox::error(0L, msg,i18n("Signal Caught"));
	}


	if (gPilotDaemon)
	{
		gPilotDaemon->killMonitor();
	}


	exit(-1);
}



int main(int argc, char* argv[])
{
	FUNCTIONSETUP;

        KAboutData about("kpilot", I18N_NOOP("KPilot"),
                         "4.0b",
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

        KCmdLineArgs::init(argc, argv, &about);
	KCmdLineArgs::addCmdLineOptions(kpilotoptions);
	KApplication::addCmdLineOptions();
	KCmdLineArgs *p=KCmdLineArgs::parsedArgs();

	debug_level=atoi(p->getOption("debug"));

	KApplication a(true,true);

	KConfig& c=KPilotLink::getConfig();

	if (c.readNumEntry("Configured",0)<KPilotLink::ConfigurationVersion)
	{
		cerr << fname << ": Is still not configured for use."
			<< endl;
		return 1;
	}


	gPilotDaemon = new PilotDaemon();

	if (gPilotDaemon->status()==PilotDaemon::ERROR)
	{
		kdError() << fname 
			<< ": Failed to start up daemon"
			<< endl;
		exit(2);
	}
	
	signal(SIGHUP, signalHandler);
	signal(SIGINT, signalHandler);
	signal(SIGPIPE, signalHandler);
	signal(SIGSEGV, signalHandler);
	signal(SIGQUIT, signalHandler);
	signal(SIGTERM, signalHandler);

	gPilotDaemon->showTray();

	return a.exec();
}
