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





class KPilotInstaller : public KMainWindow
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
	void slotFastSyncRequested();
	void optionsShowToolbar();
	void optionsConfigureKeys();
	void optionsConfigureToolbars();
	

protected:
	void readConfig();


	/**
	* Run all the internal conduits' presync functions.
	*/
	bool componentPreSync();
	void setupSync(int kind,const QString& msg);

	void initIcons();
	void initMenu();
	void setupWidget();
	void initComponents();

	/**
	* These are constants used in the KPilotInstaller code.
	* Most of them are IDs for UI elements. ID_FILE_QUIT is a big
	* number (compared to ID_COMBO) because elements that are
	* inserted into the combo box get numbers counting from
	* ID_COMBO+1, so we need room for all those numbers. 998
	* internal conduits seems like plenty.
	*/
	typedef enum { 
		ID_COMBO=1,
		ID_FILE_QUIT=1000,
		ID_FILE_SETTINGS,
		ID_FILE_BACKUP,
		ID_FILE_RESTORE,
		ID_FILE_HOTSYNC,
		ID_FILE_FASTSYNC,
		ID_HELP_ABOUT,
		ID_HELP_HELP,
		ID_CONDUITS_ENABLE,
		ID_CONDUITS_SETUP 
		} Constants ;
 

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

	/**
	* Delayed initialization of the components.
	* This improves perceived startup time.
	*/
	void initializeComponents();
	
signals:
	void modeSelected(int selected);
};



 

#endif
