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

#ifndef __KPILOT_H
#define __KPILOT_H


#include <qapp.h>
#include <qmenubar.h>
#include <qlist.h>
#include <qpopmenu.h>
#include <qpixmap.h>

#include <kapp.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <kprogress.h>

class QPopupMenu;
class QComboBox;
class KProcess;

class PilotComponent;

#include "kpilotlink.h"



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

    protected:
      void closeEvent(QCloseEvent *e);
      void setQuitAfterCopyComplete(bool quit) { fQuitAfterCopyComplete = quit; }
      QWidget* getManagingWidget() { return fManagingWidget; }
      
      // Not sure if this is the way to go or not... might want to make the link
      // persist...
      void initPilotLink();
      void initCommandSocket();

      void initStatusLink(); // Seperate so the components can be initialized
      KPilotLink* getPilotLink() { return fPilotLink; }
      void destroyPilotLink() { if (fPilotLink) {delete fPilotLink; fPilotLink = 0L; /* delete fLinkProcess; fLinkProcess = 0L; */ } }

public slots:
	void slotRestoreRequested();
	void slotBackupRequested();
	void slotHotSyncRequested();
	void slotFastSyncRequested();

protected:
	int testSocket(KSocket *);

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
      void initToolBar();
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
      QWidget*        fManagingWidget;
      QList<PilotComponent>  fPilotComponentList; // Has the widgets/components...
      QList<QWidget>  fVisibleWidgetList;
      KPilotLink*     fPilotLink;
      KSocket*        fPilotCommandSocket;
      KSocket*        fPilotStatusSocket;
      bool            fKillDaemonOnExit;
      char            fLinkCommand[10000];
      int             fLastWidgetSelected;

      Status fStatus;

	/**
	* We keep track of this one (conduitMenu)
	* because the various builtin conduits
	* can register themselves with KPilot and 
	* they have to show up somewhere.
	* @see conduitCombo
	* @see addComponentPage
	* @see initComponents
	* @see menuCallback
	*/
	QPopupMenu	*conduitMenu;
	/**
	* Remember the builtin conduits. 
	* @see conduitMenu
	*/
	QComboBox	*conduitCombo;

	QPixmap	icon_hotsync,icon_backup,icon_fastsync,icon_restore,
		icon_quit;

 protected slots:
      void menuCallback(int);
      void quit();
	void slotConfigureKPilot();
	void slotConfigureConduits();
      void fileInstalled(int which);
      void slotModeSelected(int selected);
      void slotSyncDone(KProcess* which);
      void slotDaemonStatus(KSocket*);

    signals:
    void modeSelected(int selected);
    };



 
#endif

#undef REALLY_KPILOT


// $Log$
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
