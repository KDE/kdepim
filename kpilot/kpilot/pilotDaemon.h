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
#include <ksystemtray.h>

#include "kpilotlink.h"
#include "pilotDaemonDCOP.h"

class QDragEnterEvent;
class QDropEvent;

class KServerSocket;
class KConfig;
class KSocket;
class KProcess;
class KAboutApplication;

class PilotRecord;
class KPilotDCOP_stub;
class LoggerDCOP_stub;


class PilotDaemon;
class FileInstaller;
class ActionQueue;

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
	void slotShowBusy();
	void slotShowNormal();
	
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
	void addInstallFiles(QStrList);

	// The next few functions are the DCOP interface
	//
	//
	virtual ASYNC requestSync(int);
	virtual ASYNC requestFastSyncNext();
	virtual ASYNC requestRegularSyncNext();
	virtual int nextSyncType() const;
	virtual ASYNC quitNow();
	virtual ASYNC reloadSettings();

protected:
	DaemonStatus fStatus;

	enum postSyncActions {
		None=0,
		ReloadSettings = 1,
		Quit = 2
		} ;
	int fPostSyncAction;

protected slots:
	void startHotSync();
	void endHotSync();

	void logMessage(const QString &);
	void logError(const QString &);
	void logProgress(const QString &,int);

private:
	int getPilotSpeed(KPilotConfigSettings &);

	bool setupPilotLink();

	KPilotDeviceLink &getPilotLink() { return *fPilotLink; }
	KPilotDeviceLink *fPilotLink;

	QString fPilotDevice;
	KPilotDeviceLink::DeviceType fPilotType;
	int fNextSyncType;

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
	* Provide access to KPilot's DCOP interface through a stub.
	*/
protected:
	LoggerDCOP_stub &getLogger() { return *fLogStub; } ;
	KPilotDCOP_stub &getKPilot() { return *fKPilotStub; } ;

private:
	LoggerDCOP_stub *fLogStub;
	KPilotDCOP_stub *fKPilotStub;
};


// $Log$
// Revision 1.38  2002/12/04 10:55:37  thorsen
// Kroupware merge to HEAD. 3.1 will follow when the issues with the patch have been worked out.
//
// Revision 1.36.2.3  2002/11/29 11:12:10  thorsen
// Merged from head
//
// Revision 1.36.2.2  2002/10/14 21:20:35  rogowski
// Added syncing with addressbook of kmail. Fixed some bugs.
//
// Revision 1.36.2.1  2002/10/11 09:16:24  rogowski
// Implemented syncing of kpilot with kmail(only todos and calendars up to now). To enable syncing, choose in the sync config tab the option >sync with kmail<. But be careful with doing this with important data on your pilot: There are still bugs in kmail eating your data!
//
// Revision 1.36  2002/08/15 21:51:00  kainhofe
// Fixed the error messages (were not printed to the log), finished the categories sync of the todo conduit
//
// Revision 1.35  2002/06/24 19:29:11  adridg
// Allow daemon RW access to config file
//
// Revision 1.34  2002/06/08 09:17:07  adridg
// Added tooltip for daemon
//
// Revision 1.33  2002/02/02 11:46:02  adridg
// Abstracting away pilot-link stuff
//
// Revision 1.32  2002/01/25 21:43:13  adridg
// ToolTips->WhatsThis where appropriate; vcal conduit discombobulated - it doesn't eat the .ics file anymore, but sync is limited; abstracted away more pilot-link
//
// Revision 1.31  2001/12/29 15:49:01  adridg
// SyncStack changes
//
// Revision 1.30  2001/11/18 16:59:55  adridg
// New icons, DCOP changes
//
// Revision 1.29  2001/10/08 12:49:11  cschumac
// kde3 compile fixes.
//
// Revision 1.28  2001/09/29 16:26:18  adridg
// The big layout change
//
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
