/* pilotDaemon.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** See the .cc file for an explanation of what this file is for.
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

#ifndef _KPILOT_PILOTDAEMON_H
#define _KPILOT_PILOTDAEMON_H

#include <qpixmap.h>
#include <ktmainwindow.h>
#include <ksystemtray.h>

#include "pilotDaemonDCOP.h"

class KConfig;
class KSocket;
class KProcess;
class KAboutApplication;

class PilotRecord;
class KPilotLink;

#define PILOTDAEMON_COMMAND_PORT	31509
#define PILOTDAEMON_STATUS_PORT		31510

// Daemon for Palm Pilot syncing.  Note, if you wish to send commands to
// KPilotLink then you need to connect to the command port _BEFORE_ the 
// hot sync begins. (ie: before the user presses the button...)

class QDragEnterEvent;
class QDropEvent;

class PilotDaemon;
class FileInstaller;

class PilotDaemonTray : public KSystemTray
{
	Q_OBJECT

public:
	PilotDaemonTray(PilotDaemon *p);

	typedef enum { Normal,Busy } IconShape ;
	void changeIcon(IconShape);

	void enableRunKPilot(bool);

	virtual void dragEnterEvent(QDragEnterEvent *);
	virtual void dropEvent(QDropEvent *);

protected:
	void setupWidget();

protected slots:
	void slotShowAbout();

	// "Regular" QT actions
	//
	//
	virtual void mousePressEvent(QMouseEvent* e);
	virtual void closeEvent(QCloseEvent *e);

private:
	QPixmap icon,busyicon;
	PilotDaemon *daemon;

	/**
	* Remember which item in the context menu
	* is "Run KPilot" so we can enable / disable
	* it as necessary.
	*/
	int menuKPilotItem;

	/**
	* Window for the "About KPilot" information.
	*/
	KAboutApplication *kap;

	FileInstaller *fInstaller;
} ;


class PilotDaemon : public QObject, virtual public PilotDaemonDCOP
{
friend class PilotDaemonTray;

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

	/**
	* Display the daemon's system tray icon
	* (if there is one, depending on the DockDaemon
	* setting in the config file)
	*/
	void showTray();

	// The next few functions are the DCOP interface
	//
	//
	virtual ASYNC startHotSync(int);

protected:
	DaemonStatus fStatus;

signals:
  void endHotSync();

private:
	int getPilotSpeed(KConfig&);

  void setupSubProcesses();
  void setupConnections();
  void startHotSync();
  void sendStatus(const int status);
  void reloadSettings();
  void saveProperties(KConfig&);
  void sendRecord(PilotRecord* rec);
  bool quit() { return fQuit; }

#ifdef DEBUG
	// The debugging version of getPilotLink also warns
	// in case of bad link &c.
	//
	//
	KPilotLink *getPilotLink();
#else
	KPilotLink* getPilotLink() { return fPilotLink; }
#endif

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



	/**
	* This is a pointer to the (optional) docked
	* system tray icon for the daemon.
	*/
	PilotDaemonTray *tray;

private slots:
 void slotProcFinished(KProcess*);
  void slotAccepted(KSocket* connection);
  void slotAddStatusConnection(KSocket* connection);
  void slotRemoveStatusConnection(KSocket* connection);
  void slotConnectionClosed(KSocket* connection);
  void slotCommandReceived(KSocket*);
  void slotEndHotSync();
  void quitImmediately();

  void slotSyncingDatabase(char* dbName);
  void slotDBBackupFinished();

	void slotRunKPilot();

protected slots:
	void slotFilesChanged();
};

#else
#warning "File doubly included"
#endif

// $Log$
// Revision 1.19  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.18  2001/03/04 21:24:37  adridg
// Added drag 'n drop file install to daemon
//
// Revision 1.17  2001/03/04 11:23:04  adridg
// Changed for bug 21392
//
// Revision 1.16  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.15  2001/02/06 08:05:20  adridg
// Fixed copyright notices, added CVS log, added surrounding #ifdefs. No code changes.
//
// Revision 1.14  2001/01/04 22:19:38  adridg
// Stuff for Chris and Bug 18072
//
// Revision 1.13  2001/01/03 00:02:45  adridg
// Added Heiko's FastSync
//
