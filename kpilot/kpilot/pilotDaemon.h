// pilotDaemon.h
//
// Copyright (C) 1998,1999 Dan Pilone
// Copyright (C) 2000 Adriaan de Groot
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 

// $Revision$



#include <qpixmap.h>
#include <ktmainwindow.h>
#include <kpilotlink.h>
#include <ksystemtray.h>

class KConfig;
class KSocket;
class KProcess;
class KAboutApplication;

#define PILOTDAEMON_COMMAND_PORT	31509
#define PILOTDAEMON_STATUS_PORT		31510

// Daemon for Palm Pilot syncing.  Note, if you wish to send commands to
// KPilotLink then you need to connect to the command port _BEFORE_ the 
// hot sync begins. (ie: before the user presses the button...)

class PilotDaemon : public KSystemTray
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
	/**
	* Kill the monitor process, if any. Note that
	* it is a bad idea to continue using the
	* daemon without the monitor process, so
	* this will usually be called just before quit().
	*
	* @arg finishsync indicates that the sync-status
	* window should be updated before quitting.
	*/
	void killMonitor(bool finishsync=false);

protected:
	DaemonStatus fStatus;

signals:
  void endHotSync();

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
  bool    fStartKPilot;
  bool    fWaitingForKPilot;

	QPixmap icon,busyicon;
	KAboutApplication *kap;

	/**
	* Remember which item in the context menu
	* is "Run KPilot" so we can enable / disable
	* it as necessary.
	*/
	int menuKPilotItem;

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
	void slotRunKPilot();

  void slotSyncingDatabase(char* dbName);
  void slotDBBackupFinished();

protected:
	// "Regular" QT actions
	//
	//
	virtual void mousePressEvent(QMouseEvent* e);
	virtual void closeEvent(QCloseEvent *e);
};
