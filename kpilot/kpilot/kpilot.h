// kpilot.h
//
// Copyright (C) 1998,1999 Dan Pilone
// Copyright (C) 2000 Adriaan de Groot
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING.
//

// $Revision$

#ifndef __KPILOT_H
#define __KPILOT_H

#include <qapp.h>
#include <qmenubar.h>
#include <qlist.h>
#include <qpopmenu.h>
#include <qpixmap.h>

#include <kapp.h>
#include <klocale.h>
#include <ktmainwindow.h>
#include <kprogress.h>
#include <kprocess.h>

class QPopupMenu;
class QComboBox;

#include "fileInstallWidget.h"
#include "memoWidget.h"


#include "options.h"



class KPilotInstaller : public KTMainWindow
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
	* Return a string with names of KPilot authors.
	* @see version
	*/
	static const char *authors();


	/**
	* Set the main widget to showing the kpilot
	* logo. This is useful if you're messing around with
	* the component list -- the logo always remains and
	* can hide whatever is going on in the background.
	*/
	void showTitlePage(bool force=false);

	/**
	* Returns the user's preference for the system-wide
	* fixed font.
	*/
	const QFont& fixed() const { return fixedFont; }  ;

    void testDir(QString name);
    bool getQuitAfterCopyComplete() const { return fQuitAfterCopyComplete; }
    // Adds 'name' to the pull down menu of components
    void addComponentPage(QWidget* widget, QString name);



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

private:
      void initIcons();
      void initMenu();
      void initStatusBar();
      void initToolBar();
      void setupWidget();
      void initComponents();
      
      static const int ID_FILE_QUIT;
      static const int ID_FILE_SETTINGS;
      static const int ID_FILE_BACKUP;
      static const int ID_FILE_RESTORE;
      static const int ID_FILE_HOTSYNC;
      static const int ID_HELP_ABOUT;
      static const int ID_HELP_HELP;
      static const int ID_CONDUITS_ENABLE;
      static const int ID_CONDUITS_SETUP;
      static const int ID_COMBO;
      
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

	QPixmap	icon_hotsync,icon_backup;
	QFont fixedFont;

 protected slots:
      void menuCallback(int);
      void quit();
      void doHotSync();
      void fileInstalled(int which);
      void slotModeSelected(int selected);
      void slotSyncDone(KProcess* which);
      void slotDaemonStatus(KSocket*);

    signals:
    void modeSelected(int selected);
    };



 
#endif
