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
#include <kprocess.h>

class QPopupMenu;
class QComboBox;

#include "fileInstallWidget.h"
#include "memoWidget.h"


#include "options.h"



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

	/**
	* Returns the user's preference for the system-wide
	* fixed font.
	*/
	const QFont& fixed() const { return fixedFont; }  ;

    bool getQuitAfterCopyComplete() const { return fQuitAfterCopyComplete; }
    // Adds 'name' to the pull down menu of components
    void addComponentPage(QWidget* widget, QString name);


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
      void doRestore();
      void doBackup();

protected:
	int testSocket(KSocket *);

	void readConfig(KConfig&);


	void setupSync(int kind,const QString& msg);
	void doHotSync() 
	{ 
		setupSync(KPilotLink::HotSync,
			i18n("Hot-Syncing. ")+
			i18n("Please press the hot-sync button."));
	}
	void doFastSync()
	{ 
		setupSync(KPilotLink::FastSync,
			i18n("Fast-Syncing. ")+
			i18n("Please press the hot-sync button."));
	}

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
	QFont fixedFont;

 protected slots:
      void menuCallback(int);
      void quit();
      void fileInstalled(int which);
      void slotModeSelected(int selected);
      void slotSyncDone(KProcess* which);
      void slotDaemonStatus(KSocket*);

    signals:
    void modeSelected(int selected);
    };



 
#endif


// $Log:$
