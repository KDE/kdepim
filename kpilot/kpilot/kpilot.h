#ifndef _KPILOT_KPILOT_H
#define _KPILOT_KPILOT_H
/* kpilot.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This is the main program in KPilot.
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

#include <kmainwindow.h>


class QPopupMenu;
class QComboBox;
class KAction;
class KToggleAction;
class KProgress;
class KJanusWidget;

class PilotDaemonDCOP_stub;
class PilotComponent;
class FileInstallWidget;
class LogWidget;


#include "kpilotDCOP.h"



class KPilotInstaller : public KMainWindow, public KPilotDCOP
{
Q_OBJECT

public:
	KPilotInstaller();
	KPilotInstaller(QStrList& fileList);
	~KPilotInstaller();

	/**
	* Return a string with the version identifier (ie.
	* "KPilot v3.1b11") if kind == 0; otherwise return
	* a "long" string about KPilot -- currently the
	* id of kpilot.o
	*/
	static const char *version(int kind);





	// Adds 'name' to the pull down menu of components
	void addComponentPage(PilotComponent *, const QString &name);


	typedef enum { Normal,
		Startup,
		WaitingForDaemon,
		UIBusy,
		Error } Status ;

	Status status() const { return fStatus; } ;


protected:
	void closeEvent(QCloseEvent *e);
	KJanusWidget *getManagingWidget() { return fManagingWidget; }

	/**
	* Provide access to the daemon's DCOP interface
	* through an object of the stub class.
	*/
protected:
	PilotDaemonDCOP_stub &getDaemon() { return *fDaemonStub; } ;
private:
	PilotDaemonDCOP_stub *fDaemonStub;

	/**
	* Handle the functionality of kill-daemon-on-exit and
	* kill-daemon-if-started-by-my by killing it in those
	* cases.
	*/
protected:
	void killDaemonIfNeeded();
public:
	void startDaemonIfNeeded();

public slots:
	void slotRestoreRequested();
	void slotBackupRequested();
	void slotHotSyncRequested();
	void slotListSyncRequested();


	/**
	* These are slots for the standard Configure ...
	* actions and not interesting.
	*/
	void optionsShowToolbar();
	void optionsConfigureKeys();
	void optionsConfigureToolbars();


public:
	/**
	* This is the DCOP interface from the daemon to KPilot.
	*/
	virtual ASYNC daemonStatus(int);

protected:
	void readConfig();


	/**
	* Run all the internal conduits' presync functions.
	*/
	bool componentPreSync();
	void setupSync(int kind,const QString& msg);
	void componentPostSync();

	void initIcons();
	void initMenu();
	void setupWidget();
	void initComponents();

	/**
	* This is the private-d-pointer, KPilot style. Not everything
	* has moved there yet.
	*/
	class KPilotPrivate;
	KPilotPrivate *fP;

private:
	KMenuBar*       fMenuBar;
	KToolBar*       fToolBar;
	bool            fQuitAfterCopyComplete; // Used for GUI-less interface
	KJanusWidget    *fManagingWidget;
	bool            fKillDaemonOnExit;
	bool fDaemonWasRunning;

	Status fStatus;

	FileInstallWidget *fFileInstallWidget;
	LogWidget *fLogWidget;

	/**
	 * toggle action from Options menu
	 */
	KToggleAction  *m_toolbarAction;
	KToggleAction  *m_statusbarAction;


protected slots:
	void quit();
	void slotConfigureKPilot();
	void slotConfigureConduits();
	void fileInstalled(int which);
	void slotNewToolbarConfig();

	/**
	 * Indicate that a particular component has been selected (through
	 * whatever mechanism). This will make that component visible and
	 * adjust any other user-visible state to indicate that that component
	 * is now active.
	 *
	 * This should be called (possibly by the component itself!)
	 * or activated through the signal mechanism.
	 * */
	void slotSelectComponent(PilotComponent *);

signals:
	void modeSelected(int selected);
};





// $Log$
// Revision 1.37  2002/12/10 15:54:00  faure
// Mainwindow settings and KEditToolbar fix, as usual. (untested, other than compilation)
//
// Revision 1.36  2002/11/27 21:29:06  adridg
// See larger ChangeLog entry
//
// Revision 1.35  2001/11/18 16:59:55  adridg
// New icons, DCOP changes
//
// Revision 1.34  2001/11/11 22:10:38  adridg
// Switched to KJanuswidget
//
// Revision 1.33  2001/09/30 16:58:45  adridg
// Cleaned up preHotSync interface, removed extra includes, added private-d-ptr.
//
// Revision 1.30  2001/09/23 18:25:50  adridg
// New config architecture
//
// Revision 1.27  2001/08/19 19:25:57  adridg
// Removed kpilotlink dependency from kpilot; added DCOP interfaces to make
// that possible. Also fixed a connect() type mismatch that was harmless
// but annoying.
//
//
// Revision 1.23  2001/04/14 15:21:35  adridg
// XML GUI and ToolTips
//
// Revision 1.21  2001/03/04 22:22:29  adridg
// DCOP cooperation between daemon & kpilot for d&d file install
//
// Revision 1.20  2001/03/02 13:07:18  adridg
// Completed switch to KAction
//
// Revision 1.18  2001/02/25 12:39:35  adridg
// Fixed component names (src incompatible)
//
// Revision 1.16  2001/02/06 08:05:19  adridg
// Fixed copyright notices, added CVS log, added surrounding #ifdefs. No code changes.
//
#endif
