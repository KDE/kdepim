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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/


#include <qpixmap.h>
#include <ktmainwindow.h>
#include <ksystemtray.h>

#include "kpilotlink.h"
#include "pilotDaemonDCOP.h"

class KConfig;
class KSocket;
class KProcess;
class KAboutApplication;

class PilotRecord;
class KPilotDCOP_stub;

// Daemon for Palm Pilot syncing.  Note, if you wish to send commands to
// KPilotLink then you need to connect to the command port _BEFORE_ the 
// hot sync begins. (ie: before the user presses the button...)

class QDragEnterEvent;
class QDropEvent;

class KServerSocket;

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

	/**
	* Methods to get information from the FileInstaller
	* in the tray -- which files are there, and where?
	*/
	QStringList installFiles();
	const QString &installDir();

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
		INIT 
	};

	DaemonStatus status() const { return fStatus; } ;
	/* DCOP */ virtual QString statusString();
	QString  syncTypeString(int i) const;

	/**
	* Display the daemon's system tray icon
	* (if there is one, depending on the DockDaemon
	* setting in the config file)
	*/
	void showTray();

	// The next few functions are the DCOP interface
	//
	//
	virtual ASYNC requestSync(int);
	virtual ASYNC requestFastSyncNext();
	virtual ASYNC requestRegularSyncNext();
	virtual ASYNC quitNow();
	virtual ASYNC reloadSettings();

protected:
	DaemonStatus fStatus;
	bool fQuitAfterSync;

protected slots:
	void startHotSync();
	void nextSyncAction(SyncAction *);
	void endHotSync();

	void logMessage(const QString &);
	void logProgress(const QString &,int);

private:
	int getPilotSpeed(KPilotConfigSettings &);

	bool setupPilotLink();

	KPilotDeviceLink &getPilotLink() { return *fPilotLink; }
	KPilotDeviceLink *fPilotLink;

	QString fPilotDevice;
	KPilotDeviceLink::DeviceType fPilotType;
	int fNextSyncType;

	class PilotDaemonPrivate;
	PilotDaemonPrivate *fP;


	/**
	* This is a pointer to the (optional) docked
	* system tray icon for the daemon.
	*/
	PilotDaemonTray *fTray;

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
	* Provide access to KPilot's DCOP interface through a stub.
	*/
protected:
	KPilotDCOP_stub &getKPilot() { return *fKPilotStub; } ;
private:
	KPilotDCOP_stub *fKPilotStub;
};


// $Log$
// Revision 1.27  2001/09/24 22:24:07  adridg
// Use new SyncActions
//
// Revision 1.26  2001/09/23 18:46:11  adridg
// Oops .. needed some extra work on the QStack part
//
// Revision 1.25  2001/09/23 18:24:59  adridg
// New syncing architecture
//
// Revision 1.24  2001/09/16 13:37:48  adridg
// Large-scale restructuring
//
// Revision 1.23  2001/08/27 22:54:27  adridg
// Decruftifying; improve DCOP link between daemon & viewer
//
// Revision 1.22  2001/08/19 19:25:57  adridg
// Removed kpilotlink dependency from kpilot; added DCOP interfaces to make that possible. Also fixed a connect() type mismatch that was harmless but annoying.
//
// Revision 1.21  2001/04/16 13:48:35  adridg
// --enable-final cleanup and #warning reduction
//
// Revision 1.20  2001/04/01 17:32:51  adridg
// I really don't remember
//
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
#endif
