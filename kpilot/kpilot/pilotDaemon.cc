/* pilotDaemon.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/
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
#include <signal.h>
#include <errno.h>

#include <qdir.h>
#include <qlist.h>
#include <qcursor.h>
#include <qdragobject.h>
#include <errno.h>

#include <kuniqueapp.h>
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
#include <kprocess.h>
#include <dcopclient.h>

#include "pilotDaemon.moc"
#include "hotsync.h"
#include "busysync.h"
#include "statusMessages.h"
#include "fileInstaller.h"
#include "kpilotlink.h"
#include "kpilotConfig.h"



// Define OWN_CRASHHANDLER if you want the daemon to handle crashes
// by itself, like KPilot 3.x did.
//
//
#undef OWN_CRASHHANDLER


PilotDaemonTray::PilotDaemonTray(PilotDaemon *p) :
	KSystemTray(0,"pilotDaemon"), 
	daemon(p)
{
	FUNCTIONSETUP;
	setupWidget();
	setAcceptDrops(true);

	fInstaller = new FileInstaller;
	connect(fInstaller,SIGNAL(filesChanged()),
		p,SLOT(slotFilesChanged()));
}

/* virtual */ void PilotDaemonTray::dragEnterEvent(QDragEnterEvent *e)
{
	e->accept(QUriDrag::canDecode(e));
}

/* virtual */ void PilotDaemonTray::dropEvent(QDropEvent *e)
{
	FUNCTIONSETUP;

	QStrList list;
	QUriDrag::decode(e, list);

	fInstaller->addFiles(list);
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
		if (icon.isNull())
		{
			kdWarning() << __FUNCTION__
				<< ": Regular icon is NULL!"
				<< endl;
		}
		setPixmap(icon);
		break;
	case Busy:
		if (busyicon.isNull())
		{
			kdWarning() << __FUNCTION__
				<< ": Busy icon is NULL!"
				<< endl;
		}
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
#ifdef DEBUG
	{ "debug <level>", I18N_NOOP(""),"0" },
#endif
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

#ifdef DEBUG
	if (debug_level & SYNC_MINOR)
	{
		kdDebug() << fname
			<< ": Speed set to "
			<< speedname << " ("
			<< speed << ')'
			<< endl;
	}
#endif

	putenv((char *)speedname);

	return speed;
#ifdef DEBUG
	/* NOTREACHED */
	(void) id;
#endif
}


PilotDaemon::PilotDaemon() : 
	DCOPObject("KPilotDaemonIface"),
	fStatus(INIT),
  	fMonitorProcess(0L), fCurrentSocket(0L),
    fCommandSocket(0L), fStatusSocket(0L), fQuit(false), fPilotLink(0L),
	fStartKPilot(false), fWaitingForKPilot(false),
	tray(0L)
{
	FUNCTIONSETUP;

	KConfig& config = KPilotConfig::getConfig();
  
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
	FUNCTIONSETUP;

	if (!tray) 
	{
		DEBUGDAEMON << fname
			<< ": No tray icon to display!"
			<< endl;

		return;
	}

	// Copied from Klipper
	KWin::setSystemTrayWindowFor( tray->winId(), 0 );
	tray->setGeometry(-100, -100, 42, 42 );
	tray->show();

	DEBUGDAEMON << fname
		<< ": Tray icon displayed."
		<< endl;
}

void
PilotDaemon::reloadSettings()
{
	FUNCTIONSETUP;

	KConfig& config = KPilotConfig::getConfig();
  
	fPilotDevice = config.readEntry("PilotDevice");
	getPilotSpeed(config);
  fStartKPilot = (bool) config.readNumEntry("StartKPilotAtHotSync", 0);

	if (fMonitorProcess)
	{
		killMonitor();
	}
	else
	{
#ifdef DEBUG
		kdDebug() << fname << ": No listener to kill (which is OK)."
			<< endl;
#endif
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
	if (!fPilotLink)
	{
		e = EACCES;
		goto ErrConn;
	}
	if (fPilotLink->status() != KPilotLink::Normal)
	{
		e = EACCES;
		goto ErrConn;
	}

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
	kdError() << __FUNCTION__
		<< ": While creating socket for daemon: "
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
		if (!fMonitorProcess->start(KProcess::NotifyOnExit))
		{
			kdWarning() << __FUNCTION__
				<< ": Can't start listener process."
				<< endl;
		}
		else
		{
			DEBUGDAEMON << fname
				<< ": Started listener with pid "
				<< fMonitorProcess->pid()
				<< endl;
		}
	}
	else
	{
		kdWarning() << __FUNCTION__ 
			<< ": Can't allocate new listener process."
			<< endl;
	}
}

#ifdef DEBUG
KPilotLink *PilotDaemon::getPilotLink()
{
	FUNCTIONSETUP;

	if (fPilotLink)
	{
		DEBUGKPILOT << fname
			<< ": Returning Pilot Link @"
			<< (int) fPilotLink
			<< endl;

		return fPilotLink;
	}
	else
	{
		kdWarning() << __FUNCTION__
			<< ": No Pilot Link!"
			<< endl;
		return 0;
	}
}
#endif

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

	if (fMonitorProcess)
	{
		disconnect(fMonitorProcess, SIGNAL(processExited(KProcess*)),
			this, SLOT(slotProcFinished(KProcess*)));
		delete fMonitorProcess;
		fMonitorProcess=0L;
	}
}


void PilotDaemon::quitImmediately()
{
	FUNCTIONSETUP;
	killMonitor();
	quit(true);
	kapp->quit();
} 
 
void
PilotDaemon::startHotSync(int mode)
{
	FUNCTIONSETUP;

	switch(mode)
	{
	case 1:
		DEBUGDAEMON << fname
			<< ": Starting a normal HotSync"
			<< endl;
		break;
	case 2: 
		DEBUGDAEMON << fname
			<< ": Starting a FastSync"
			<< endl;
		break;
	case 3:
		DEBUGDAEMON << fname
			<< ": Starting a full Backup"
			<< endl;
		break;
	default :
		kdWarning() << __FUNCTION__
			<< ": Unknown mode "
			<< mode
			<< " passed to startHotSync()"
			<< endl;
		return;
	}

	kdWarning() << __FUNCTION__
		<< ": Unimplemented right now."
		<< endl;
}

void
PilotDaemon::startHotSync()
{
	FUNCTIONSETUP;


	if (tray) 
	{
		DEBUGKPILOT << fname
			<< ": Changing tray icon."
			<< endl;

		tray->changeIcon(PilotDaemonTray::Busy);
	}

  // We need to send the SYNC_STARTING message after the sync
  // has already begun so that if KPilot is running it doesn't start
  // issuing commands before KPilotLink is ready.
#ifdef DEBUG
	if (debug_level &  SYNC_MAJOR)
	{
		kdDebug() << fname 
			<< ": Requesting KPilotLink::startHotSync()" << endl;
	}
#endif
  getPilotLink()->startHotSync();
  sendStatus(CStatusMessages::SYNC_STARTING);

  if(fCurrentSocket == 0L)
    {
#ifdef DEBUG
	if (debug_level & SYNC_MINOR)
	{
		kdDebug() << fname  <<
			": No KPilot running." << endl;
	}
#endif
		
      if(fStartKPilot) // We are supposed to start up kpilot..
	{
#ifdef DEBUG
		if (debug_level & SYNC_MAJOR)
		{
			kdDebug() << fname << ": Starting KPilot GUI." << endl;
		}
#endif

	  fWaitingForKPilot = true;
	  if (fork()==0)
	    {
	      execlp("kpilot", "kpilot", 0);
		kdError() << __FUNCTION__ << ": Failed to start KPilot." << endl;
	      exit(1);	// This is a legitimate exit(), since we should never get here.
	    }
	}
      else
	{
#ifdef DEBUG
		if (debug_level & SYNC_MAJOR)
		{
			kdDebug()  << fname << ": Starting quick sync." << endl;
		}
#endif
	  getPilotLink()->quickHotSync();
	}
    }
  // otherwise we just wait for input..
}

void
PilotDaemon::slotDBBackupFinished()
{
	FUNCTIONSETUP;

#ifdef DEBUG
	if (debug_level & SYNC_MAJOR)
	{
		kdDebug() << fname << ": DB Syncing finished." << endl;
	}
#endif

	KConfig& config = KPilotConfig::getConfig();
	if(config.readNumEntry("SyncFiles"))
	{
	  getPilotLink()->installFiles(KGlobal::dirs()->saveLocation("data", 
		QString("kpilot/pending_install/")));
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
#ifdef DEBUG
		if (debug_level & SYNC_MAJOR)
		{
			kdDebug() << fname
				<< ": Ending hot-sync now"
				<< endl;
		}
#endif

		p->endHotSync();
	}
	else
	{
		kdError() << __FUNCTION__
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

#ifdef DEBUG
	if (debug_level & SYNC_MAJOR)
	{
		kdDebug() << fname 
			<< ": Accepted command connection "
			<< (int) connection
			<< endl;
	}
#endif

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
	getPilotLink()->setFastSyncRequired(false);
    case KPilotLink::FastSync:
      if(getPilotLink()->slowSyncRequired())
      {
	getPilotLink()->doFullBackup();
	}
      else
      {
	getPilotLink()->quickHotSync();
	}
      break;
    case KPilotLink::InstallFile:
      getPilotLink()->installFiles(KGlobal::dirs()->saveLocation("data", QString("kpilot/pending_install/")));
      break;
	case KPilotLink::TestConnection :
#ifdef DEBUG
		if (debug_level & SYNC_MAJOR)
		{
			kdDebug()  << fname
				<< ": Connection tests OK"
				<< endl ;
		}
#endif
		break;
    default:
      kdWarning() << __FUNCTION__ << ": Unknown command!" << command << endl;
      break;
    }

#ifdef DEBUG
	if (debug_level & SYNC_TEDIOUS)
	{
		kdDebug() << fname
			<< ": Done reading." << endl;
	}
#endif
}

void
PilotDaemon::slotConnectionClosed(KSocket* connection)
{
	FUNCTIONSETUP;

	delete connection;
	if (fCurrentSocket != connection)
	{
		kdWarning() << __FUNCTION__
			<< ": Connection other than current was closed?"
			<< endl;
	}
	else
	{
#ifdef DEBUG
		if (debug_level & SYNC_TEDIOUS)
		{
			kdDebug() << fname 
				<< ": Connection "
				<< (int) connection
				<< " closed"
				<< endl;
		}
#endif

		fCurrentSocket = 0L;
		if (tray) { tray->enableRunKPilot(true); } 
	}
	emit(endHotSync());
}

void
PilotDaemon::slotAddStatusConnection(KSocket* connection)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	if (debug_level & SYNC_MAJOR)
	{
		kdDebug() << fname 
			<< ": Accepted status connection "
			<< (int) connection
			<< endl;
	}
#endif
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

#ifdef DEBUG
	if (debug_level & SYNC_TEDIOUS)
	{
		kdDebug() << fname 
			<< ": Connection "
			<< (int) connection
			<< " closed"
			<< endl;
	}
#endif

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

#ifdef DEBUG
	// Large debugging block
	//
	//
	if (debug_level & SYNC_MINOR)
	{
		kdDebug() << fname 
			<< ": Sending status " << status
			<< endl;
	}

	if (debug_level & SYNC_TEDIOUS)
	{
		for(fStatusConnections.first() ; 
			(c=fStatusConnections.current()) ; 
			fStatusConnections.next())
		{
			kdDebug() << fname << ": Will send to connection "
				<< (int) c
				<< endl;
		}
	}
#endif

	for(fStatusConnections.first() ; 
		(c=fStatusConnections.current()) ; 
		fStatusConnections.next())
	{
#ifdef DEBUG
		if (debug_level & SYNC_TEDIOUS)
		{
			kdDebug() << fname
				<< ": Sending to connection "
				<< (int) c
				<< endl;
		}
#endif

		if (c->socket()<0)
		{
			kdWarning() << __FUNCTION__
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

#ifdef DEBUG
	if (debug_level & SYNC_MINOR)
	{
		kdDebug() << fname
			<< ": Completed status update for "
			<< updateCount
			<< " connections"
			<< endl;
	}
#endif
}

void
PilotDaemon::saveProperties(KConfig&)
{
	FUNCTIONSETUP;

	killMonitor();
}

PilotDaemon::~PilotDaemon()
{
	FUNCTIONSETUP;

	killMonitor();

  delete fPilotLink;
  delete fCommandSocket;
  delete fStatusSocket;
  
}

void PilotDaemon::slotFilesChanged()
{
	FUNCTIONSETUP;

	DCOPClient *dcopptr = KApplication::kApplication()->dcopClient();
	if (!dcopptr)
	{
		kdWarning() << __FUNCTION__
			<< ": Can't get DCOP client pointer!"
			<< endl;
		return;
	}

	QByteArray data;
	if (dcopptr->send("kpilot",
		"KPilotIface",
		"filesChanged()",
		data))
	{
		kdWarning() << __FUNCTION__
			<< ": No KPilot running to warn of changed files."
			<< endl;
	}
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
#ifdef DEBUG
	if (debug_level)
	{
		*k << "--debug" ;
		*k << QString::number(debug_level);
	}
#endif

	k->start(KProcess::DontCare);
}

PilotDaemon* gPilotDaemon=0L;


#ifdef OWN_CRASHHANDLER
int crashFlag=0;

void signalHandler(int s)
{
	FUNCTIONSETUP;


	crashFlag++;

	// In (crashed) cases like this I think we
	// can't use kdError() safely, so this is
	// the one instance of cerr left ...
	//	
	//
	cerr << fname << ": Caught signal " << s << endl;

	// Suppose the daemon crashes. We get here
	// with crashFlag=1. We set the alarm and it goes
	// off, which gets us here with crashFlag=2.
	// This second time around, there's no message,
	// we just try to kill the monitor. If something 
	// goes wrong *there* too, and we get back here
	// a *third* time, give up.
	//
	//
	if ( crashFlag>2 ) exit(3);

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


	exit(3);
}
#endif



int main(int argc, char* argv[])
{
	FUNCTIONSETUP;

        KAboutData about("kpilotDaemon", I18N_NOOP("KPilot"),
                         "4.0b2",
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
	KUniqueApplication::addCmdLineOptions();
	KCmdLineArgs *p=KCmdLineArgs::parsedArgs();

#ifdef DEBUG
	debug_level=atoi(p->getOption("debug"));
	if (debug_level)
	{
		DEBUGDAEMON << fname
			<< ": Debug level set to "
			<< debug_level
			<< endl;
	}
#endif

	if (!KUniqueApplication::start())
	{
		return 0;
	}
	KUniqueApplication a(true,true);

	// A block just to keep variables local.
	//
	//
	{
	KConfig& c=KPilotConfig::getConfig();
	c.setReadOnly(true);
	int v = c.readNumEntry("Configured",0);

	if (v < KPilotConfig::ConfigurationVersion)
	{
		kdError() << __FUNCTION__ 
			<< ": Is still not configured for use."
			<< endl;
		return 1;
	}

	DEBUGDAEMON << fname
		<< ": Configuration version "
		<< v
		<< endl;
	}


	gPilotDaemon = new PilotDaemon();

	if (gPilotDaemon->status()==PilotDaemon::ERROR)
	{
		delete gPilotDaemon;
		gPilotDaemon = 0;
		kdError() << __FUNCTION__ 
			<< ": **\n"
			   ": Failed to start up daemon\n"
			   ": due to errors constructing it.\n"
			   ": **"
			<< endl;
		return 2;
	}
	
#ifdef OWN_CRASHHANDLER
	signal(SIGHUP, signalHandler);
	signal(SIGINT, signalHandler);
	signal(SIGPIPE, signalHandler);
	signal(SIGSEGV, signalHandler);
	signal(SIGQUIT, signalHandler);
	signal(SIGTERM, signalHandler);
#endif

	gPilotDaemon->showTray();

	return a.exec();

	/* NOTREACHED */
	(void) id;
}



// $Log$
// Revision 1.32  2001/03/04 11:23:04  adridg
// Changed for bug 21392
//
// Revision 1.31  2001/02/26 22:09:49  adridg
// Fixed some exit() calls; extra listener process debugging
//
// Revision 1.30  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.29  2001/02/08 13:17:19  adridg
// Fixed crash when conduits run during a backup and exit after the
// end of that backup (because the event loop is blocked by the backup
// itself). Added better debugging error exit message (no i18n needed).
//
// Revision 1.28  2001/02/05 21:01:07  adridg
// Fixed copyright headers for source releases. No code changed
//
// Revision 1.27  2001/02/05 19:16:32  adridg
// Removing calls to exit() from internal functions
//
// Revision 1.26  2001/01/06 13:20:23  adridg
// Cleaned up DCOP; changed version number
//
// Revision 1.25  2001/01/04 22:19:37  adridg
// Stuff for Chris and Bug 18072
//
// Revision 1.24  2001/01/04 11:33:20  bero
// Fix build
//
// Revision 1.23  2001/01/03 00:02:45  adridg
// Added Heiko's FastSync
//
// Revision 1.22  2001/01/02 15:02:59  bero
// Fix build
//
// Revision 1.21  2000/12/31 16:44:00  adridg
// Patched up the debugging stuff again
//
