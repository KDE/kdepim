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
#include <qdir.h>
#include "pilotDaemon.moc"
#include <kapp.h>
#include <kwm.h>
#include <ksimpleconfig.h>
#include <qlist.h>
#include <kurl.h>
#include <qpopmenu.h>
#include <qcursor.h>
#include <kmsgbox.h>
#include <signal.h>
#include "hotsync.h"
#include "busysync.h"
#include "statusMessages.h"

const int PilotDaemon::COMMAND_PORT = PILOTDAEMON_COMMAND_PORT;
const int PilotDaemon::STATUS_PORT = PILOTDAEMON_STATUS_PORT;

DockingLabel::DockingLabel (PilotDaemon* daemon, QWidget* w) : QLabel(w), fKFM(0L), fDaemon(daemon)
{
  KDNDDropZone* dropZone = new KDNDDropZone(this, DndURL);
  connect(dropZone, SIGNAL(dropAction(KDNDDropZone*)), this, SLOT(slotDropEvent(KDNDDropZone*)));
}


void 
DockingLabel::mousePressEvent(QMouseEvent *e)
{
  if( e->button() == RightButton)
    {
      int x = e->x() + this->x();
      int y = e->y() + this->y();
      fMenu->popup(QPoint(x, y));
    }
}

void
DockingLabel::slotDropEvent(KDNDDropZone* drop)
{
  QStrList& list = drop->getURLList();
  unsigned int i = 0;
  QString tempFileName;
  QString dest = "file:";
  dest += kapp->localkdedir();
  dest += "/share/apps/kpilot/pending_install/";
  
  if(fKFM)
    {
      KMsgBox::message(this, klocale->translate("Busy"), 
		       klocale->translate("Please wait for current operation to complete."), KMsgBox::STOP);
      return;
    }
  fKFM = new KFM;
  fKFM->allowKFMRestart(true);
  if(!fKFM->isOK())
    {
      KMsgBox::message(this, klocale->translate("Error"), 
		       klocale->translate("Could not start KFM."), KMsgBox::STOP);
      delete fKFM;
      fKFM = 0L;
      return;
    }
  
  while(i < list.count())
    {
      tempFileName += list.at(i);
      if(++i < list.count())
	tempFileName += '\n';
    }
  connect(fKFM, SIGNAL(finished()), this, SLOT(slotKFMCopyComplete()));
  if(list.count() == 1)
    fKFM->copy(tempFileName, dest + strrchr(list.at(0), '/'));
  else
    fKFM->copy(tempFileName, dest);
}

void
DockingLabel::slotKFMCopyComplete()
{
  delete fKFM;
  fKFM = 0L;
  fDaemon->sendStatus(CStatusMessages::FILE_INSTALL_REQUEST);
}

PilotDaemon::PilotDaemon()
  : KTopLevelWidget(), fMonitorProcess(0L), fCurrentSocket(0L),
    fCommandSocket(0L), fStatusSocket(0L), fQuit(false), fPilotLink(0L),
    fDockingLabel(0L), fStartKPilot(false), fWaitingForKPilot(false)
{
  KSimpleConfig* config = new KSimpleConfig(kapp->localconfigdir() + "/kpilotrc");
  config->setGroup(0L);
  
  fPilotDevice = config->readEntry("PilotDevice");
  int speed = config->readNumEntry("PilotSpeed", 0);
  switch(speed)
    {
    case 0:
      putenv("PILOTRATE=9600");
      break;
    case 1:
      putenv("PILOTRATE=19200");
      break;
    case 2:
      putenv("PILOTRATE=38400");
      break;
    case 3:
      putenv("PILOTRATE=57600");
      break;
    case 4:
      putenv("PILOTRATE=115200");
      break;
    }
  fStartKPilot = (bool) config->readNumEntry("StartKPilotAtHotSync", 0);
  delete config;
  fStatusConnections.setAutoDelete(true);

//   cout << "Using: " << fPilotDevice << endl;
  testDir(kapp->localkdedir() + "/share/apps/kpilot");
  testDir(kapp->localkdedir() + "/share/apps/kpilot/DBBackup");
  setupWidget();
  setupConnections();
  setupSubProcesses();
}

void
PilotDaemon::testDir(QString name)
{
    DIR *dp;
    dp = opendir(name);
    if(dp == 0L)
        ::mkdir (name, S_IRWXU);
    else
        closedir( dp );
}

void
PilotDaemon::reloadSettings()
{
  KSimpleConfig* config = new KSimpleConfig(kapp->localconfigdir() + "/kpilotrc");
  config->setGroup(0L);
  
  fPilotDevice = config->readEntry("PilotDevice");
  int speed = config->readNumEntry("PilotSpeed", 0);
  switch(speed)
    {
    case 0:
      putenv("PILOTRATE=9600");
      break;
    case 1:
      putenv("PILOTRATE=19200");
      break;
    case 2:
      putenv("PILOTRATE=38400");
      break;
    case 3:
      putenv("PILOTRATE=57600");
      break;
    case 4:
      putenv("PILOTRATE=115200");
      break;
    }
  fStartKPilot = (bool) config->readNumEntry("StartKPilotAtHotSync", 0);
  delete config;
  disconnect(fMonitorProcess, SIGNAL(processExited(KProcess*)),
	  this, SLOT(slotProcFinished(KProcess*)));
  fMonitorProcess->kill();
  delete fMonitorProcess;
  fPilotLink->changePilotPath(fPilotDevice.data());
  setupSubProcesses();
}

void
PilotDaemon::setupConnections()
{
  fPilotLink = new KPilotLink(0L, 0L, fPilotDevice.data());
  connect(fPilotLink, SIGNAL(databaseSyncComplete()),
	  this, SLOT(slotDBBackupFinished()));
  connect(this, SIGNAL(endHotSync()), this, SLOT(slotEndHotSync()));
  fCommandSocket = new KServerSocket(PilotDaemon::COMMAND_PORT);
  connect(fCommandSocket, SIGNAL(accepted(KSocket*)), 
	  this, SLOT(slotAccepted(KSocket*)));
  fStatusSocket = new KServerSocket(PilotDaemon::STATUS_PORT);
  connect(fStatusSocket, SIGNAL(accepted(KSocket*)),
	  this, SLOT(slotAddStatusConnection(KSocket*)));
}

void
PilotDaemon::setupSubProcesses()
{
  fMonitorProcess = new KProcess();
  *fMonitorProcess << "pilotListener" << fPilotDevice;
  connect(fMonitorProcess, SIGNAL(processExited(KProcess*)),
	  this, SLOT(slotProcFinished(KProcess*)));
  fMonitorProcess->start(KProcess::NotifyOnExit);
}

void
PilotDaemon::setupWidget()
{
  KSimpleConfig* config = new KSimpleConfig(kapp->localconfigdir() + "/kpilotrc");
  config->setGroup(0L);
  if(config->readNumEntry("DockDaemon", 0))
    {
      fDockingLabel = new DockingLabel(this, (QWidget*)0L);
      QPixmap icon(hotsync_icon);
      fDockingLabel->setFixedSize(24,24);
      fDockingLabel->setPixmap(icon);
      KWM::setDockWindow(fDockingLabel->winId());
      QPopupMenu* menu = new QPopupMenu();
      menu->insertItem("&About", this, SLOT(slotShowAbout()));
      menu->insertSeparator();
      menu->insertItem("&Exit", this, SLOT(quitImmediately()));
      fDockingLabel->setPopupMenu(menu);
      fDockingLabel->show();
    }
  delete config;
}

void 
PilotDaemon::slotShowAbout()
{
  KMsgBox::message(0L, klocale->translate("KPilot Daemon v3.0"), 
		   klocale->translate("KPilot Hot-Sync Daemon v3.0 \nBy: Dan Pilone.\nEmail: pilone@slac.com"),
		   KMsgBox::INFORMATION);
}

void
PilotDaemon::quitImmediately()
{
  getPilotLink()->endHotSync();
  quit(true);
  kapp->quit();
} 
 
void
PilotDaemon::startHotSync()
{

  QPixmap icon(busysync_icon);
  if(fDockingLabel)
    fDockingLabel->setPixmap(icon);
  // We need to send the SYNC_STARTING message after the sync
  // has already begun so that if KPilot is running it doesn't start
  // issuing commands before KPilotLink is ready.
  getPilotLink()->startHotSync();
  sendStatus(CStatusMessages::SYNC_STARTING);
  cout << "Requesting KPilotLink::startHotSync()" << endl;
  if(fCurrentSocket == 0L)
    {
      if(fStartKPilot) // We are supposed to start up kpilot..
	{
	  fWaitingForKPilot = true;
	  if (fork()==0)
	    {
	      execlp("kpilot", "kpilot", 0);
	      exit(1);
	    }
	}
      else
	{
	  cout << "Starting sync next DB" << endl;
	  getPilotLink()->quickHotSync();
	}
    }
  // otherwise we just wait for input..
}

void
PilotDaemon::slotDBBackupFinished()
{
  cout << "DB Syncing finished." << endl;
  KSimpleConfig* config = new KSimpleConfig(kapp->localconfigdir() + "/kpilotrc");
  config->setGroup(0L);
  if(config->readNumEntry("SyncFiles"))
    getPilotLink()->installFiles(kapp->localkdedir() + "/share/apps/kpilot/pending_install/");
  delete config;
  emit(endHotSync());
}

void 
PilotDaemon::slotProcFinished(KProcess*)
{
  startHotSync();
}

void
PilotDaemon::slotEndHotSync()
{
  QPixmap icon(hotsync_icon);
  if(fDockingLabel)
    fDockingLabel->setPixmap(icon);
  getPilotLink()->endHotSync();
  sendStatus(CStatusMessages::SYNC_COMPLETED);
  if(!quit())
    fMonitorProcess->start(KProcess::NotifyOnExit);
  else
    kapp->quit();
}

void
PilotDaemon::slotAccepted(KSocket* connection)
{
  if(fCurrentSocket)
    {
      KMsgBox::message(0L, klocale->translate("Too Many Connections"),
		       klocale->translate("Error: Only one command connection at a time."), 
		       KMsgBox::STOP);
      delete connection;
      return;
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
      getPilotLink()->installFiles(kapp->localkdedir() + "/share/apps/kpilot/pending_install/");
      break;
    default:
      cerr << "Unknown command!" << endl;
      break;
    }
  cerr << "PilotDaemon: Done reading." << endl;
}

void
PilotDaemon::slotConnectionClosed(KSocket* connection)
{
  delete connection;
  fCurrentSocket = 0L;
  emit(endHotSync());
}

void
PilotDaemon::slotAddStatusConnection(KSocket* connection)
{
  fStatusConnections.append(connection);
  connect(connection, SIGNAL(closeEvent(KSocket*)),
	  this, SLOT(slotRemoveStatusConnection(KSocket*)));
  if(fWaitingForKPilot)
    {
      fWaitingForKPilot = false;
      sendStatus(CStatusMessages::SYNC_STARTING);
    }
}

void
PilotDaemon::slotRemoveStatusConnection(KSocket* connection)
{
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
  for(fStatusConnections.first(); fStatusConnections.current(); fStatusConnections.next())
    {
      ofstream out(fStatusConnections.current()->socket());
      out.write(&status, sizeof(int));
    }
}

void
PilotDaemon::saveProperties(KConfig*)
{
  disconnect(fMonitorProcess, SIGNAL(processExited(KProcess*)),
	  this, SLOT(slotProcFinished(KProcess*)));
  fMonitorProcess->kill();
}

PilotDaemon::~PilotDaemon()
{
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

PilotDaemon* gPilotDaemon;

void signalHandler(int)
{
  KMsgBox::message(0L, klocale->translate("Signal Caught"), 
      	       klocale->translate("Warning!  KPilotDaemon is shutting down."
				  "  If this is not\nintentional, please send"
       				  " mail to kpilot-list@slac.com describing\n"
       				  "what happened."), KMsgBox::EXCLAMATION);
  gPilotDaemon->getMonitorProcess()->kill();
  exit(-1);
}


int main(int argc, char* argv[])
{
  KApplication a(argc, argv, "pilotDaemon");
  gPilotDaemon = new PilotDaemon();

  signal(SIGHUP, signalHandler);
  signal(SIGINT, signalHandler);
  signal(SIGPIPE, signalHandler);
  signal(SIGSEGV, signalHandler);
  signal(SIGQUIT, signalHandler);
  signal(SIGTERM, signalHandler);

  a.setMainWidget(gPilotDaemon);
  a.enableSessionManagement(true);
  return a.exec();
}
