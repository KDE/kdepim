/* *******************************************************
   KPilot - Hot-Sync Software for Unix.
   Copyright 1998 by Dan Pilone
   This code is released under the GNU PUBLIC LICENSE.
   Please see the file 'COPYING' included in the KPilot
   distribution.
   *******************************************************
 */

#include <qpopmenu.h>
#include <ktopwidget.h>
#include <kprocess.h>
#include <ksock.h>
#include <drag.h>
#include <kfm.h>
#include <kpilotlink.h>

class PilotDaemon;

class DockingLabel : public QLabel
{
  Q_OBJECT

public:
  DockingLabel(PilotDaemon* daemon, QWidget* w);
  void setPopupMenu(QPopupMenu* menu) { fMenu = menu; }

private:
  QPopupMenu* fMenu;
  KFM* fKFM;
  PilotDaemon* fDaemon;

 private slots:
 void mousePressEvent(QMouseEvent* e);
  void slotDropEvent(KDNDDropZone* drop);
  void slotKFMCopyComplete();
};


#define PILOTDAEMON_COMMAND_PORT	31509
#define PILOTDAEMON_STATUS_PORT		31510

// Daemon for Palm Pilot syncing.  Note, if you wish to send commands to
// KPilotLink then you need to connect to the command port _BEFORE_ the 
// hot sync begins. (ie: before the user presses the button...)

class PilotDaemon : public KTopLevelWidget
{
  friend class DockingLabel;

  Q_OBJECT
  
public:
  // This was done as two separate ports so that you can have multiple status
  // listeners
  static const int COMMAND_PORT;
  static const int STATUS_PORT;
  enum DaemonStatus { HOTSYNC_START, HOTSYNC_END, FILE_INSTALL_REQ };

  PilotDaemon();
  ~PilotDaemon();
  void quit(bool yesno) { fQuit = yesno; }

  KProcess* getMonitorProcess() { return fMonitorProcess; }

private:
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
