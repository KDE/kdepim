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
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/

#ifndef _KPILOT_KPILOT_H
#define _KPILOT_KPILOT_H


#ifndef QAPP_H
#include <qapp.h>
#endif

#ifndef QMENUBAR_H
#include <qmenubar.h>
#endif

#ifndef QLIST_H
#include <qlist.h>
#endif

#ifndef QPOPMENU_H
#include <qpopmenu.h>
#endif

#ifndef QPIXMAP_H
#include <qpixmap.h>
#endif


#ifndef _KAPP_H_
#include <kapp.h>
#endif

#ifndef _KLOCALE_H_
#include <klocale.h>
#endif

#ifndef _KMAINWINDOW_H_
#include <kmainwindow.h>
#endif

#ifndef _KPROGRESS_H_
#include <kprogress.h>
#endif

#ifndef _KPILOT_KPILOTLINK_H
#include "kpilotlink.h"
#endif

#ifndef _KPILOT_KPILOTDCOP_H
#include "kpilotDCOP.h"
#endif

class QPopupMenu;
class QComboBox;
class QWidgetStack;
class KProcess;
class KAction;
class KToggleAction;

class PilotDaemonDCOP_stub;
class PilotComponent;
class FileInstallWidget;





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

	/**
	* Set the main widget to showing the kpilot
	* logo. This is useful if you're messing around with
	* the component list -- the logo always remains and
	* can hide whatever is going on in the background.
	*/
	void showTitlePage(const QString& statusMsg=QString::null,
		bool force=false);


    bool getQuitAfterCopyComplete() const { return fQuitAfterCopyComplete; }


	// Adds 'name' to the pull down menu of components
	void addComponentPage(PilotComponent *, const QString &name);


	typedef enum { Normal,
		Startup,
		WaitingForDaemon,
		UIBusy,
		Error } Status ;

	Status status() const { return fStatus; } ;

public:
	/**
	* DCOP interface.
	*/
	virtual ASYNC filesChanged();

    protected:
      void closeEvent(QCloseEvent *e);
      void setQuitAfterCopyComplete(bool quit) { fQuitAfterCopyComplete = quit; }
      QWidgetStack *getManagingWidget() { return fManagingWidget; }
      
      // Not sure if this is the way to go or not... might want to make the link
      // persist...
      // void initPilotLink();
      // void initCommandSocket();

      // void initStatusLink(); // Seperate so the components can be initialized
      // KPilotLink* getPilotLink() { return fPilotLink; }
      // void destroyPilotLink() { if (fPilotLink) {delete fPilotLink; fPilotLink = 0L; /* delete fLinkProcess; fLinkProcess = 0L; */ } }

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

public slots:
	void slotRestoreRequested();
	void slotBackupRequested();
	void slotHotSyncRequested();
	void slotFastSyncRequested();
	void slotShowTitlePage();
	void optionsShowStatusbar();
	void optionsShowToolbar();
	void optionsConfigureKeys();
	void optionsConfigureToolbars();
	

protected:
	// int testSocket(KSocket *);

	void readConfig(KConfig&);


	/**
	* Run all the internal conduits' presync functions.
	* Expects the link command to be empty if @p b is true.
	*/
	void componentPreSync(bool b=true);
	void setupSync(int kind,const QString& msg);

private:
      void initIcons();
      void initMenu();
      void initStatusBar();
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
      
      KMenuBar*       fMenuBar;
      KStatusBar*     fStatusBar;
      KToolBar*       fToolBar;
      bool            fQuitAfterCopyComplete; // Used for GUI-less interface
      QWidgetStack    *fManagingWidget;
      QList<PilotComponent>  fPilotComponentList; // Has the widgets/components...
      // KPilotLink*     fPilotLink;
      // KSocket*        fPilotCommandSocket;
      // KSocket*        fPilotStatusSocket;
      bool            fKillDaemonOnExit;
      char            fLinkCommand[10000];
      int             fLastWidgetSelected;

      Status fStatus;

	FileInstallWidget *fFileInstallWidget;

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
      void slotSyncDone(KProcess* which);
      // void slotDaemonStatus(KSocket*);

	/**
	 * Indicate that a particular component has been selected (through
	 * whatever mechanism). This will make that component visible and
	 * adjust any other user-visible state to indicate that that component
	 * is now active.
	 *
	 * This should be called (possibly by the component itself!) or activated
	 * through the signal mechanism.
	 * */
	void slotSelectComponent(PilotComponent *);

    signals:
    void modeSelected(int selected);
    };



 
#else
#ifdef DEBUG
#warning "File doubly included"
#endif
#endif

#undef REALLY_KPILOT


// $Log$
// Revision 1.27  2001/08/19 19:25:57  adridg
// Removed kpilotlink dependency from kpilot; added DCOP interfaces to make that possible. Also fixed a connect() type mismatch that was harmless but annoying.
//
// Revision 1.26  2001/06/13 21:32:35  adridg
// Dead code removal and replacing complicated stuff w/ QWidgetStack
//
// Revision 1.25  2001/04/23 06:30:38  adridg
// XML UI updates
//
// Revision 1.24  2001/04/16 13:48:35  adridg
// --enable-final cleanup and #warning reduction
//
// Revision 1.23  2001/04/14 15:21:35  adridg
// XML GUI and ToolTips
//
// Revision 1.22  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.21  2001/03/04 22:22:29  adridg
// DCOP cooperation between daemon & kpilot for d&d file install
//
// Revision 1.20  2001/03/02 13:07:18  adridg
// Completed switch to KAction
//
// Revision 1.19  2001/03/01 01:02:48  adridg
// Started changing to KAction
//
// Revision 1.18  2001/02/25 12:39:35  adridg
// Fixed component names (src incompatible)
//
// Revision 1.17  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.16  2001/02/06 08:05:19  adridg
// Fixed copyright notices, added CVS log, added surrounding #ifdefs. No code changes.
//
