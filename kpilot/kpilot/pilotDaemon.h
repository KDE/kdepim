// pilotDaemon.h
//
// Copyright (C) 1998,1999 Dan Pilone
// Copyright (C) 2000 Adriaan de Groot
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 

// $Revision$



#include <qpopmenu.h>
#include <ktmainwindow.h>
#include <kpilotlink.h>
#include <kdockwindow.h>

class KConfig;
class KSocket;
class KProcess;

class PilotDaemon;

class DockingLabel : public KDockWindow
{
  Q_OBJECT

public:
  DockingLabel(PilotDaemon* daemon, QWidget* w);
  void setPopupMenu(QPopupMenu* menu) { fMenu = menu; }

 protected:
  void dropEvent(QDropEvent* drop);
  void dragEnterEvent(QDragEnterEvent* event);

private:
  QPopupMenu* fMenu;
  PilotDaemon* fDaemon;

 private slots:
 void mousePressEvent(QMouseEvent* e);
};


#define PILOTDAEMON_COMMAND_PORT	31509
#define PILOTDAEMON_STATUS_PORT		31510

// Daemon for Palm Pilot syncing.  Note, if you wish to send commands to
// KPilotLink then you need to connect to the command port _BEFORE_ the 
// hot sync begins. (ie: before the user presses the button...)

class PilotDaemon : public KTMainWindow
{
  friend class DockingLabel;

  Q_OBJECT
  
public:
  // This was done as two separate ports so that you can have multiple status
  // listeners
  static const int COMMAND_PORT = PILOTDAEMON_COMMAND_PORT;
  static const int STATUS_PORT = PILOTDAEMON_STATUS_PORT;
  enum DaemonStatus 
  	{ 
		HOTSYNC_START, HOTSYNC_END, FILE_INSTALL_REQ, 
		ERROR, INIT 
	};

	DaemonStatus status() const { return fStatus; } ;

  PilotDaemon();
  ~PilotDaemon();
  void quit(bool yesno) { fQuit = yesno; }

  KProcess* getMonitorProcess() { return fMonitorProcess; }

protected:
	DaemonStatus fStatus;

private:
	int getPilotSpeed(KConfig *);

  void setupWidget();
  void setupSubProcesses();
  void setupConnections();
  void startHotSync();
  void sendStatus(const int status);
  void reloadSettings();
  void saveProperties(KConfig*);
  void sendRecord(PilotRecord* rec);
  void testDir(QString name);
  bool quit() { return fQuit; }
  KPilotLink* getPilotLink() { return fPilotLink; }

  KProcess* fMonitorProcess;
  KSocket* fCurrentSocket;
  KServerSocket* fCommandSocket;
  KServerSocket* fStatusSocket;
  bool fQuit;
  KPilotLink* fPilotLink;
  QString fPilotDevice;
  QList<KSocket> fStatusConnections;
  DockingLabel* fDockingLabel;
  bool    fStartKPilot;
  bool    fWaitingForKPilot;

  signals:
  void endHotSync();

 private slots:
 void slotProcFinished(KProcess*);
  void slotAccepted(KSocket* connection);
  void slotAddStatusConnection(KSocket* connection);
  void slotRemoveStatusConnection(KSocket* connection);
  void slotConnectionClosed(KSocket* connection);
  void slotCommandReceived(KSocket*);
  void slotEndHotSync();
  void quitImmediately();
  void slotShowAbout();

  void slotSyncingDatabase(char* dbName);
  void slotDBBackupFinished();
};
