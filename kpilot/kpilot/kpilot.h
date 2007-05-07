#ifndef _KPILOT_KPILOT_H
#define _KPILOT_KPILOT_H
/* kpilot.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <kmainwindow.h>
#include <KXmlGuiWindow>
//Added by qt3to4:
#include <Q3StrList>
#include <QCloseEvent>
#include <KPageView>

class QComboBox;
class KAction;
class KProgress;
class KJanusWidget;

class PilotComponent;
class FileInstallWidget;
class LogWidget;
class OrgKdeKpilotDaemonInterface;


class KPilotInstaller : public KXmlGuiWindow
{
Q_OBJECT

public:
	KPilotInstaller();
	KPilotInstaller(Q3StrList& fileList);
	~KPilotInstaller();

        enum DaemonMessages {
                None=0,
                StartOfHotSync=1,
                EndOfHotSync=2,
                DaemonQuit=4 } ;
        enum KPilotStatus {
                Startup=1,
                WaitingForDaemon=2,
                Normal=10,
                UIBusy=100,
                Error=101 } ;


	/**
	* Return a string with the version identifier (ie.
	* "KPilot v3.1b11") if kind == 0; otherwise return
	* a "long" string about KPilot -- currently the
	* id of kpilot.o
	*/
	static const char *version(int kind);


	// Adds 'name' to the pull down menu of components
	void addComponentPage(PilotComponent *, const QString &name);


	KPilotStatus status() const { return fAppStatus; } ;


protected:
	void closeEvent(QCloseEvent *e);
	KPageView *getManagingWidget() { return fManagingWidget; }

	/**
	* Provide access to the daemon's D-Bus interface
	* through an object of the stub class.
	*/
protected:
	OrgKdeKpilotDaemonInterface &getDaemon() { return *fDaemonInterface; } ;
private:
	OrgKdeKpilotDaemonInterface *fDaemonInterface;

	/**
	* Handle the functionality of kill-daemon-on-exit and
	* kill-daemon-if-started-by-my by killing it in those
	* cases.
	*/
protected:
	void killDaemonIfNeeded();

public slots:
	/**
	* These are slots for the menu actions for each kind of
	* sync that can be requested.
	*/
	void slotRestoreRequested();
	void slotBackupRequested();
	void slotHotSyncRequested();
	void slotFullSyncRequested();
	void slotHHtoPCRequested();
	void slotPCtoHHRequested();

	void startDaemonIfNeeded();

	/**
	* These are slots for the standard Configure ...
	* actions and not interesting. The show toolbar
	* functionality is in kdelibs starting with KDE 3.1,
	* but we need to remain backwards compatible.
	*/
	void optionsConfigureKeys();
	void optionsConfigureToolbars();


public:
	/**
	* This is the D-Bus interface from the daemon to KPilot.
	*/
	virtual void daemonStatus(int);
	virtual int kpilotStatus();

public slots:
	/**
	* This is the D-Bus interface from the daemon to KPilot
	* to configure KPilot.
	*/
	virtual void configure();
	virtual void configureWizard();

protected:
	void readConfig();


	/**
	* Run all the internal conduits' presync functions.
	*/
	bool componentPreSync();
	void setupSync(int kind,const QString& msg);
	void componentPostSync();
	/**
	* Run after a configuration change to force
	* the viewers to re-load data.
	*/
	void componentUpdate();

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
	bool            fQuitAfterCopyComplete; // Used for GUI-less interface
	KPageView    *fManagingWidget;
	bool fDaemonWasRunning;

	KPilotStatus fAppStatus;

	FileInstallWidget *fFileInstallWidget;
	LogWidget *fLogWidget;

	// Used to track if dialog is visible - needed for new D-Bus calls
	bool fConfigureKPilotDialogInUse;


protected slots:
	void quit();
	void fileInstalled(int which);
	void slotNewToolbarConfig();

	/**
	 * Get the daemon to reset the link. This uses reloadSettings()
	 * to achieve this result - the daemon calls reset() in there.
	 */
	void slotResetLink();

	/**
	 * Indicate that a particular component has been selected (through
	 * whatever mechanism). This will make that component visible and
	 * adjust any other user-visible state to indicate that that component
	 * is now active.
	 *
	 * This should be called (possibly by the component itself!)
	 * or activated through the signal mechanism.
	 * */
	void slotSelectComponent( PilotComponent *c );
	void slotAboutToShowComponent( const QModelIndex &, const QModelIndex & );

	/**
	* Delayed initialization of the components.
	* This improves perceived startup time.
	*/
	void initializeComponents();

signals:
	void modeSelected(int selected);
};




#endif
