#ifndef _KPILOT_PILOTDAEMON_H
#define _KPILOT_PILOTDAEMON_H
/* pilotDaemon.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/



#include "kpilotlink.h"
#include "syncAction.h"
//Added by qt3to4:
#include <QCloseEvent>
#include <QDropEvent>
#include <QPixmap>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <KSystemTrayIcon>
class QMenu;
class QPixmap;
class QTimer;
class K3AboutApplication;

class PilotDaemon;
class ActionQueue;
class FileInstaller;
class OrgKdeKpilotLoggerInterface;
class OrgKdeKpilotKpilotInterface;

class LogFile;

class KPilotLink;
class KPilotDeviceLink;

class PilotDaemonTray : public KSystemTrayIcon
{
	Q_OBJECT

friend class PilotDaemon;

public:
	PilotDaemonTray(PilotDaemon *p);

	typedef enum { Normal, Busy, NotListening } IconShape ;
	void changeIcon(IconShape);

	void enableRunKPilot(bool);

protected:
	void setupWidget();
	/**
	* Menu of sync types.
	*/
	QMenu *fSyncTypeMenu;

protected slots:
	void slotShowAbout();
	void slotShowBusy();
	void slotShowNormal();
	void slotShowNotListening();
	void slotBusyTimer();

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
	K3AboutApplication *kap;

	/**
	* Timer for blinking.
	*/
	QTimer *fBlinkTimer;

} ;

class PilotDaemon : public QObject
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
	/* D-Bus */ virtual QString statusString();
	/* D-Bus */ virtual QString shortStatusString();

	/**
	* Display the daemon's system tray icon
	* (if there is one, depending on the DockDaemon
	* setting in the config file)
	*/
	void showTray();
	virtual void addInstallFiles(const QStringList &);

	// The next few functions are the D-Bus interface.
	// Some are also slots.
	//
public slots:
	virtual void requestSync(int);
public:
	virtual void requestSyncType(QString);
	virtual void requestRegularSyncNext();
	virtual int nextSyncType() const;
	virtual void requestSyncOptions(bool,bool);

	virtual void quitNow();
	virtual void reloadSettings();
	virtual void setTempDevice(QString d);

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
	void startHotSync( KPilotLink* lnk );
	void endHotSync();

	void logMessage(const QString &);
	void logError(const QString &);
	void logProgress(const QString &,int);

private:
	int getPilotSpeed();

	/**
	* Check whether we should do a backup.  This is based on the
	* KPilotSettings::backupFrequency and uses
	* SyncAction::BackupFrequency.  This will be expanded, hopefully,
	* to provide backup scheduling at some point.
	*/
	bool shouldBackup();

	bool setupPilotLink();

	KPilotDeviceLink &getPilotLink() { return *fPilotLink; }
	KPilotDeviceLink *fPilotLink;

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
	OrgKdeKpilotLoggerInterface &getLogger() { return *fLogInterface; } ;
	OrgKdeKpilotLoggerInterface &getFileLogger() { return *fLogFileInterface; } ;
	OrgKdeKpilotKpilotInterface &getKPilot() { return *fKPilotInterface; } ;

	LogFile *fLogFile;
	bool fIsListening;

private:
	OrgKdeKpilotLoggerInterface *fLogInterface;
	OrgKdeKpilotLoggerInterface *fLogFileInterface;
	OrgKdeKpilotKpilotInterface *fKPilotInterface;
	QString fTempDevice;
};


#endif
