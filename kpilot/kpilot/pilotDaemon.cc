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

static char *id="$Id$";

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

#include <qdir.h>
#include <qlist.h>
#include <qpopmenu.h>
#include <qcursor.h>
#include <qdragobject.h>

#include <kapp.h>
#include <kwin.h>
#include <ksimpleconfig.h>
#include <kurl.h>
#include <ksock.h>
#include <kmessagebox.h>
#include <kstddirs.h>
#include <kpopupmenu.h>
#include <kio/netaccess.h>
#include <kdebug.h>

#include "pilotDaemon.moc"
#include "hotsync.h"
#include "busysync.h"
#include "statusMessages.h"
#include "options.h"
#include "kpilot.h"

static struct option longOptions[]=
{
	{ "debug",1,0L,'d' },
	{ "help",0,0L,1 },
	{ 0L,0,0L,0 }
} ;

void handleOptions(int& argc, char **argv)
{
	FUNCTIONSETUP;

	static const char *banner=
		"\t\tKPilotDaemon v4.0b\n"
		"Copyright (C) 1998,1999 Dan Pilone\n"
		"Copyright (C) 2000 Adriaan de Groot\n\n";

	int c,li;

	while((c=getopt_long(argc,argv,"d:",longOptions,&li))>0)
	{
		switch(c)
		{
		case 'd' : debug_level=atoi(optarg);
			if (debug_level)
			{
				cerr << fname << ": Debug level set to "
					<< debug_level << endl;
			}
			break;
		case 1 : usage(banner,longOptions); exit(0);
		default : usage(banner,longOptions); exit(1);
		}
	}
}


DockingLabel::DockingLabel (PilotDaemon* daemon, QWidget* w) : KDockWindow(), fDaemon(daemon)
{
  FUNCTIONSETUP;
  QPixmap icon(hotsync_icon);
  setPixmap(icon);
  QPopupMenu* menu = contextMenu();
  menu->insertItem(i18n("&About"), daemon, SLOT(slotShowAbout()));
  menu->insertSeparator();
  menu->insertItem(i18n("&Exit"), daemon, SLOT(quitImmediately()));
  setAcceptDrops(true);
}


void 
DockingLabel::mousePressEvent(QMouseEvent *e)
{
  FUNCTIONSETUP;
  if( e->button() == RightButton)
    {
      KPopupMenu *menu = contextMenu();
      contextMenuAboutToShow( menu );
      menu->popup( e->globalPos() );
    }
}

void DockingLabel::dragEnterEvent(QDragEnterEvent* event)
{
  event->accept(QUriDrag::canDecode(event));
}


void
DockingLabel::dropEvent(QDropEvent* drop)
{
  QStrList list;
  QUriDrag::decode(drop, list);
  
  kdDebug() << "DockingLabel::dropEvent() - Got " << list.first() << endl;
  
  if(list.first() != 0L)
    {
      unsigned int i = 0;
      QString dirname = KGlobal::dirs()->saveLocation("data", QString("kpilot/pending_install/"));
      while (i < list.count())
	{
	  KURL srcName = list.at(i);
	  KURL destDir(dirname + "/" + srcName.filename());
	  KIO::NetAccess::copy(srcName, destDir);
	  i++;
	}
    }
}

int PilotDaemon::getPilotSpeed(KConfig *config)
{
	FUNCTIONSETUP;

	int speed = config->readNumEntry("PilotSpeed", 0);

	// Translate the speed entry in the
	// config file to something we can
	// put in the environment (for who?)
	//
	//
	char *speedname=0L;

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

	putenv(speedname);

	return speed;
}

PilotDaemon::PilotDaemon() : 
	KTMainWindow(), 
	fStatus(INIT),
  	fMonitorProcess(0L), fCurrentSocket(0L),
    fCommandSocket(0L), fStatusSocket(0L), fQuit(false), fPilotLink(0L),
    fDockingLabel(0L), fStartKPilot(false), fWaitingForKPilot(false)
{
	FUNCTIONSETUP;

	KConfig* config = KPilotLink::getConfig();
	config->setGroup(QString());
  
  fPilotDevice = config->readEntry("PilotDevice");


	getPilotSpeed(config);

  fStartKPilot = (bool) config->readNumEntry("StartKPilotAtHotSync", 0);
  delete config;
  fStatusConnections.setAutoDelete(true);

  setupWidget();
  setupConnections();
	if (fStatus == ERROR) return;
  setupSubProcesses();
}


void
PilotDaemon::testDir(QString name)
{
	FUNCTIONSETUP;

    DIR *dp = NULL;
    dp = opendir(name.latin1());
    if(dp == 0L)
        ::mkdir (name.latin1(), S_IRWXU);
    else
        closedir( dp );
}

void
PilotDaemon::reloadSettings()
{
	FUNCTIONSETUP;

  KConfig* config = KPilotLink::getConfig();
  config->setGroup(QString());
  
  fPilotDevice = config->readEntry("PilotDevice");
	getPilotSpeed(config);
  fStartKPilot = (bool) config->readNumEntry("StartKPilotAtHotSync", 0);
  delete config;

	if (fMonitorProcess)
	{
		disconnect(fMonitorProcess, SIGNAL(processExited(KProcess*)),
			this, SLOT(slotProcFinished(KProcess*)));
		fMonitorProcess->kill();
		delete fMonitorProcess;
		fMonitorProcess=0L;
	}
	else
	{
		cerr << fname << ": No listener to kill (which is OK)."
			<< endl;
	}

	fPilotLink->changePilotPath(fPilotDevice.latin1());
	setupSubProcesses();
}

void
PilotDaemon::setupConnections()
{
	FUNCTIONSETUP;

	fCommandSocket = new KServerSocket(PilotDaemon::COMMAND_PORT);
	connect(fCommandSocket, SIGNAL(accepted(KSocket*)), 
		this, SLOT(slotAccepted(KSocket*)));
	fStatusSocket = new KServerSocket(PilotDaemon::STATUS_PORT);
	connect(fStatusSocket, SIGNAL(accepted(KSocket*)),
		this, SLOT(slotAddStatusConnection(KSocket*)));

	if ((fCommandSocket == 0L) || (fStatusSocket == 0L) ||
		(fCommandSocket->socket() < 0 ) ||
		(fStatusSocket->socket() < 0))
	{
		cerr << fname
			<< ": Couldn't create sockets for daemon"
			<< endl;
		if (fCommandSocket) delete fCommandSocket;
		if (fStatusSocket) delete fStatusSocket;
		fCommandSocket=0L;
		fStatusSocket=0L;

		fStatus=ERROR;
		return;
	}

	fPilotLink = new KPilotLink(0L, 0L, fPilotDevice.latin1());
	connect(fPilotLink, SIGNAL(databaseSyncComplete()),
		this, SLOT(slotDBBackupFinished()));
	connect(this, SIGNAL(endHotSync()), this, SLOT(slotEndHotSync()));
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

void
PilotDaemon::setupWidget()
{
	FUNCTIONSETUP;

  KConfig* config = KPilotLink::getConfig();
  config->setGroup(QString());
  if(config->readNumEntry("DockDaemon", 0))
    {
      fDockingLabel = new DockingLabel(this, (QWidget*)0L);
      fDockingLabel->show();
    }
  delete config;
}

void 
PilotDaemon::slotShowAbout()
{
  FUNCTIONSETUP;
  
  KMessageBox::error(0L, i18n("KPilot Hot-Sync Daemon v4.0 \n"
				 "By: Dan Pilone.\n"
				"Email: pilone@slac.com"),
		       i18n("KPilot Daemon v4.0"));
}

void
PilotDaemon::quitImmediately()
{
	FUNCTIONSETUP;

	getPilotLink()->endHotSync();
	quit(true);
	kapp->quit();
} 
 
void
PilotDaemon::startHotSync()
{
	FUNCTIONSETUP;


	if(fDockingLabel)
	{
		QPixmap icon(busysync_icon);
		fDockingLabel->setPixmap(icon);
	}

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

	KConfig* config = KPilotLink::getConfig();
	config->setGroup(QString());
	if(config->readNumEntry("SyncFiles"))
	{
	  getPilotLink()->installFiles(KGlobal::dirs()->saveLocation("data", QString("kpilot/pending_install/")));
	}
	delete config;
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

	if(fDockingLabel)
	{
		QPixmap icon(hotsync_icon);
		fDockingLabel->setPixmap(icon);
	}

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
}

void
PilotDaemon::slotCommandReceived(KSocket*)
{
	FUNCTIONSETUP;

  ifstream in(fCurrentSocket->socket());
  int command;

  in >> command;
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
PilotDaemon::saveProperties(KConfig*)
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
		KProcess *m=gPilotDaemon->getMonitorProcess();
		if (m)
		{
			cerr << fname << ": Killing monitor" << endl;
			m->kill();
		}
	}


	exit(-1);
}



int main(int argc, char* argv[])
{
	FUNCTIONSETUP;


	KApplication a(argc, argv, "pilotDaemon");
	handleOptions(argc,argv);



	gPilotDaemon = new PilotDaemon();

	if (gPilotDaemon->status()==PilotDaemon::ERROR)
	{
		cerr << fname 
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

	a.setMainWidget(gPilotDaemon);
	return a.exec();
}
