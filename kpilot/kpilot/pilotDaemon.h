#ifndef _KPILOT_PILOTDAEMON_H
#define _KPILOT_PILOTDAEMON_H
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/


#include <ksystemtray.h>

#include "kpilotlink.h"
#include "syncAction.h"
#include "pilotDaemonDCOP.h"

class QPixmap;
class QTimer;
class KAboutApplication;
class QPopupMenu;

class PilotDaemon;
class ActionQueue;
class FileInstaller;
class LoggerDCOP_stub;
class KPilotDCOP_stub;
class LogFile;

class PilotDaemonTray : public KSystemTray
{
	Q_OBJECT

friend class PilotDaemon;

public:
	PilotDaemonTray(PilotDaemon *p);

	typedef enum { Normal, Busy, NotListening } IconShape ;
	void changeIcon(IconShape);

	void enableRunKPilot(bool);

	virtual void dragEnterEvent(QDragEnterEvent *);
	virtual void dropEvent(QDropEvent *);


protected:
	void setupWidget();
	/**
	* Menu of sync types.
	*/
	QPopupMenu *fSyncTypeMenu;

protected slots:
	void slotShowAbout();
	void slotShowBusy();
	void slotShowNormal();
	void slotShowNotListening();
	void slotBusyTimer();

	// "Regular" QT actions
	//
	//
	virtual void mousePressEvent(QMouseEvent* e);
	virtual void closeEvent(QCloseEvent *e);

protected:
	void startHotSync();
	void endHotSync();

private:
	QPixmap icons[((int) NotListening) + 1];
	IconShape fCurrentIcon;
	PilotDaemon *daemon;

	/**
	* Remember which item in the context menu
	* is "Run KPilot" so we can enable / disable
	* it as necessary.
	*/
	int menuKPilotItem;

	/**
	* Remember which item in the context menu
	* is "Configure Conduits" so we can enable / disable
	* it as necessary.
	*/
	int menuConfigureConduitsItem;

	/**
	* Window for the "About KPilot" information.
	*/
	KAboutApplication *kap;

	/**
	* Timer for blinking.
	*/
	QTimer *fBlinkTimer;

} ;

class PilotDaemon : public QObject, virtual public PilotDaemonDCOP
{
Q_OBJECT

// The tray must be our friend so that we can let it stop the daemon.
friend class PilotDaemonTray;


public:
	PilotDaemon();
	~PilotDaemon();

	enum DaemonStatus
  	{
		HOTSYNC_START,    // Hotsync is running
		HOTSYNC_END,      // Hotsync is cleaning up
		FILE_INSTALL_REQ, // A file is being saved for installation
		ERROR,
		READY,            // Connected to device and ready for Sync
		INIT,
		NOT_LISTENING
	};

	DaemonStatus status() const { return fDaemonStatus; } ;
	/* DCOP */ virtual QString statusString();
	/* DCOP */ virtual QString shortStatusString();
	QString  syncTypeString(SyncAction::SyncMode i) const;

	/**
	* Display the daemon's system tray icon
	* (if there is one, depending on the DockDaemon
	* setting in the config file)
	*/
	void showTray();
	virtual void addInstallFiles(const QStringList &);

	// The next few functions are the DCOP interface.
	// Some are also slots.
	//
public slots:
	virtual ASYNC requestSync(int);
public:
	virtual ASYNC requestSyncType(QString);
	virtual ASYNC requestFastSyncNext();
	virtual ASYNC requestRegularSyncNext();
	virtual int nextSyncType() const;
	virtual ASYNC quitNow();
	virtual ASYNC reloadSettings();
	virtual ASYNC setTempDevice(QString d);

	virtual void stopListening();
	virtual void startListening();
	virtual bool isListening() { return fIsListening; }
	/**
	* Functions reporting same status data, e.g. for the kontact plugin.
	*/
	virtual QDateTime lastSyncDate();
	virtual QStringList configuredConduitList();
	virtual QString logFileName();
	virtual QString userName();
	virtual QString pilotDevice();
	virtual bool killDaemonOnExit();

protected:
	DaemonStatus fDaemonStatus;

	enum postSyncActions {
		None=0,
		ReloadSettings = 1,
		Quit = 2
		} ;
	int fPostSyncAction;

protected slots:
	void startHotSync( KPilotDeviceLink* lnk );
	void endHotSync();

	void logMessage(const QString &);
	void logError(const QString &);
	void logProgress(const QString &,int);

private:
	int getPilotSpeed();

	bool setupPilotLink();

	KPilotDeviceLink &getPilotLink() { return *fPilotLink; }
	KPilotDeviceLink *fPilotLink;

	// TODO!!! get rid of the next sync type
	SyncAction::SyncMode fNextSyncType;

	ActionQueue *fSyncStack;

	/**
	* This is a pointer to the (optional) docked
	* system tray icon for the daemon.
	*/
	PilotDaemonTray *fTray;

	/**
	* Set or change the tooltip displayed by the tray icon.
	*/
	void updateTrayStatus(const QString &s=QString::null);

	FileInstaller *fInstaller;

protected slots:
	/**
	* Called after a file has been installed to notify any observers, like
	* KPilot, that files have been installed. [Here that means: copied
	* to the pending_install directory and thus *waiting* for
	* installation on the Palm]
	*/
	void slotFilesChanged();

	/**
	* Start up KPilot.
	*/
	void slotRunKPilot();

	/**
	* Run conduit configuration using "kpilot -c"
	*/
	void slotRunConfig();

	/**
	* Provide access to KPilot's DCOP interface through a stub.
	*/
protected:
	LoggerDCOP_stub &getLogger() { return *fLogStub; } ;
	LoggerDCOP_stub &getFileLogger() { return *fLogFileStub; } ;
	KPilotDCOP_stub &getKPilot() { return *fKPilotStub; } ;

	LogFile *fLogFile;
	bool fIsListening;

private:
	LoggerDCOP_stub *fLogStub;
	LoggerDCOP_stub *fLogFileStub;
	KPilotDCOP_stub *fKPilotStub;
	QString fTempDevice;
};


#endif
